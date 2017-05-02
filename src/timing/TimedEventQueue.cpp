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

	if(!predicted)
	{
		removePredictions(ev->getTime());
	}

	push(ev, predicted);

	newEventCondition_.notify_one();
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
TimedEventQueue::removePredictions(fmiTime time)
{
	if (queue_.empty()) return;
	auto ev = queue_.end();
	do {
		--ev;
		if(ev->second && ev->first->getTime() >= time)
		{
			// remove it
			BOOST_LOG_TRIVIAL(trace) << "De-queued Predicted " << ev->first->toString();
			delete (*ev).first;
			ev = queue_.erase(ev);
		}
	} while (ev != queue_.begin() && ev->first->getTime() >= time);
}

void
TimedEventQueue::push(Event * ev, bool predicted)
{
	assert(ev != NULL);
	fmiTime invariantOldTime = DBL_MIN;

	std::list<std::pair<Event*,bool>>::iterator evIt = queue_.begin();
	while(evIt != queue_.end() && evIt->first->getTime() <= ev->getTime())
	{
		assert(invariantOldTime <= evIt->first->getTime());
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
TimedEventQueue::isFutureEvent(const Event* ev)
{
	boost::system_time evTime = getSystemTime(ev);
	return evTime > boost::posix_time::microsec_clock::universal_time();
}
