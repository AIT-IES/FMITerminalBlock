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
	localEpoch_(boost::posix_time::microsec_clock::universal_time())
{

}

void
TimedEventQueue::add(Event * ev, bool predicted)
{
	assert(ev != NULL);
	boost::lock_guard<boost::mutex> guard(queueMut_);

	BOOST_LOG_TRIVIAL(trace) << "TimedEventQueue: Add(" << ev->toString() 
		<< ", " << predicted << "): Pre-State: " << toString();

	removeFuturPredictions(ev->getTime());

	if (predicted && hasPriorEvents(ev->getTime()))
	{
		delete ev; // The event is already outdated
		return;
	}
	if (predicted)
	{
		removeConcurrentPrediction(ev->getTime());
	}

	push(ev, predicted);
	newEventCondition_.notify_one();

	BOOST_LOG_TRIVIAL(trace) << "TimedEventQueue: Add(...): Post-State: " << toString();
}

Event * 
TimedEventQueue::get(void)
{
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
	add(ev, false);
}

fmiTime 
TimedEventQueue::getTimeStampNow()
{
	boost::system_time currentTime;
	currentTime = boost::posix_time::microsec_clock::universal_time();
	return getSimulationTime(currentTime);
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
		delete queue_.front().first;
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
		delete queue_.front().first;
		queue_.erase(queue_.begin());
	}
}

void
TimedEventQueue::push(Event * ev, bool predicted)
{
	assert(ev != NULL);
	assert(queue_.empty() || !queue_.front().second || !predicted);
	fmiTime invariantOldTime = DBL_MIN;

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
	boost::posix_time::time_duration evTime = 
		boost::posix_time::seconds((long) floor(time)) + 
		boost::posix_time::microseconds((int64_t) ((time - floor(time)) * 1000*1000));
	return localEpoch_ + evTime;
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
	for (auto it = queue_.begin(); it != queue_.end(); ++it)
	{
		ret += it->first->toString();
		ret += it->second ? " (predicted)" : " (external)";
		ret += ", ";
	}
	ret += "]";
	return ret;
}