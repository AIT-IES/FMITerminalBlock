/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file TimedEventQueue.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "timing/TimedEventQueue.h"

#include <boost/log/trivial.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/lock_types.hpp>
#include <float.h>
#include <math.h>

using namespace FMITerminalBlock::Timing;

TimedEventQueue::TimedEventQueue():
	queue_(), queueMut_(), newEventCondition_(),
	localEpoch_(boost::posix_time::microsec_clock::universal_time()), 
	timeInitBarrier_()
{ 
}

void 
TimedEventQueue::initStartTimeNow(fmiTime start)
{
	assert(queue_.empty());

	// Set preliminary local epoch
	localEpoch_ = boost::posix_time::microsec_clock::universal_time();
	// Correct local epoch by starting time
	localEpoch_ -= getRelativeTime(start);

	EventLogger::setGlobalSimulationEpoch(localEpoch_);
	timeInitBarrier_.notifyInitialized();
}

void
TimedEventQueue::add(Event * ev, bool predicted)
{
	assert(ev != NULL);
	// May be reduced for performance reasons ->
	// There should be no predicted events before the time is initialized, anyway
	timeInitBarrier_.waitIfUninitialized();

	boost::lock_guard<boost::mutex> guard(queueMut_);

	BOOST_LOG_TRIVIAL(trace) << "TimedEventQueue: Add(" << ev->toString() 
		<< ", " << predicted << "): Pre-State: " << toString();

	removeFuturPredictions(ev->getTime());

	if (predicted && hasPriorEvents(ev->getTime()))
	{
		deleteEvent(ev); // The event is already outdated
		return;
	}
	if (predicted)
	{
		removeConcurrentPrediction(ev->getTime());
	}

	push(ev, predicted);
	newEventCondition_.notify_one();

	BOOST_LOG_TRIVIAL(trace) << "TimedEventQueue: Add(...): Post-State: " 
		<< toString();
}

Event * 
TimedEventQueue::get(void)
{
	timeInitBarrier_.waitIfUninitialized();
	boost::unique_lock<boost::mutex> lock(queueMut_);
	Event* ret = NULL;

	while(ret == NULL)
	{

		if(queue_.empty())
		{
			BOOST_LOG_TRIVIAL(trace) << "Wait for a new event";
			newEventCondition_.wait(lock);
		}else if(isFutureEvent(queue_.front().first)){
			// Wait until the time is reached
			BOOST_LOG_TRIVIAL(trace) << "Wait until " << queue_.front().first->toString();
			(void) newEventCondition_.timed_wait(lock, 
				getSystemTime(queue_.front().first));
		}else{
			// Process event immediately
			ret = queue_.front().first;
			queue_.pop_front();
		}
	}

	return ret;
}

void 
TimedEventQueue::pushExternalEvent(Event *ev)
{
	timeInitBarrier_.waitIfUninitialized();
	eventLoggerInstance_.logEvent(ev, ProcessingStage::realTimeGeneration);
	add(ev, false);
}

fmiTime 
TimedEventQueue::getTimeStampNow()
{
	boost::system_time currentTime;
	currentTime = boost::posix_time::microsec_clock::universal_time();
	timeInitBarrier_.waitIfUninitialized();
	return getSimulationTime(currentTime);
}

TimedEventQueue::InitializationBarrier::InitializationBarrier():
	accessMut_(), initializationCondition_(), initialized_(false) 
{
}

void
TimedEventQueue::InitializationBarrier::notifyInitialized()
{
	boost::lock_guard<boost::mutex> guard(accessMut_);
	assert(!initialized_);

	initialized_ = true;
	initializationCondition_.notify_all();
}

void
TimedEventQueue::InitializationBarrier::waitIfUninitialized()
{
	boost::unique_lock<boost::mutex> lock(accessMut_);

	while (!initialized_)
	{
		initializationCondition_.wait(lock);
	}
}

void 
TimedEventQueue::removeFuturPredictions(fmiTime time)
{
	assert(isQueuePredictionConsistent());
	if (!queue_.empty() && queue_.front().second &&
		queue_.front().first->getTime() > time + eps_)
	{
		BOOST_LOG_TRIVIAL(trace) << "De-queued future predicted "
			<< queue_.front().first->toString();
		deleteEvent(queue_.front().first);
		queue_.erase(queue_.begin());
	}
}

void 
TimedEventQueue::removeConcurrentPrediction(fmiTime time)
{
	assert(isQueuePredictionConsistent());
	if (!queue_.empty() && queue_.front().second && 
		fabs(queue_.front().first->getTime() - time) <= eps_)
	{
		BOOST_LOG_TRIVIAL(trace) << "De-queued concurrent predicted " 
			<< queue_.front().first->toString();
		deleteEvent(queue_.front().first);
		queue_.erase(queue_.begin());
	}
}

void
TimedEventQueue::push(Event * ev, bool predicted)
{
	assert(ev != NULL);
	assert(queue_.empty() || !queue_.front().second || !predicted);
	fmiTime invariantOldTime = -1*DBL_MAX; // DBL_MIN is still positive!

	auto evIt = queue_.begin();
	while(evIt != queue_.end() && 
		evIt->first->getTime() <= ev->getTime() + eps_ && !predicted )
	{
		assert(invariantOldTime <= evIt->first->getTime() + eps_);
		invariantOldTime = evIt->first->getTime();

		++evIt;
	}
	queue_.insert(evIt, std::make_pair(ev, predicted));	
}

boost::system_time
TimedEventQueue::getSystemTime(const Event* ev) const
{
	assert(ev != NULL);
	fmiTime time = ev->getTime();
	return localEpoch_ + getRelativeTime(time);
}

boost::posix_time::time_duration 
TimedEventQueue::getRelativeTime(fmiTime time)
{
	return	boost::posix_time::seconds((long) floor(time)) + 
		boost::posix_time::microseconds(
			(int64_t) ((time - floor(time)) * 1000*1000));
}

fmiTime
TimedEventQueue::getSimulationTime(const boost::system_time &sysTime) const
{
	boost::posix_time::time_duration evTime = sysTime - localEpoch_;
	return ((fmiTime) evTime.ticks()) / evTime.ticks_per_second();
}

bool
TimedEventQueue::isFutureEvent(const Event* ev) const
{
	boost::system_time evTime = getSystemTime(ev);
	return evTime > boost::posix_time::microsec_clock::universal_time();
}

bool
TimedEventQueue::hasPriorEvents(fmiTime maxTime) const
{
	return !queue_.empty() && queue_.front().first->getTime() < maxTime - eps_;
}


bool 
TimedEventQueue::isQueuePredictionConsistent() const
{
	auto it = queue_.cbegin();
	// Ignore the first position, it may be a prediction.
	if (it != queue_.cend()) ++it;
	while(it != queue_.cend())
	{
		if (it->second) return false;
		++it;
	}
	return true;
}

std::string 
TimedEventQueue::toString()
{
	std::string ret("TimedEventQueue: [");
	auto it = queue_.begin();
	while ( it != queue_.end() )
	{
		ret += it->first->toString();
		ret += it->second ? " (predicted)" : " (external)";
		++it;
		if (it != queue_.end())	ret += ", ";
	}
	ret += "]";
	return ret;
}

void 
TimedEventQueue::deleteEvent(Event* ev)
{
	assert(ev);
	eventLoggerInstance_.logEvent(ev, ProcessingStage::outdated);
	delete ev;
}
