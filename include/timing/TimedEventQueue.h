/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file TimedEventQueue.h
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_TIMING_TIMED_EVENT_QUEUE
#define _FMITERMINALBLOCK_TIMING_TIMED_EVENT_QUEUE

#include "timing/EventQueue.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <list>
#include <utility>

namespace FMITerminalBlock 
{
	namespace Timing
	{
		using namespace FMITerminalBlock;

		/**
		 * @brief EventQueue implementation issuing predicted events in time.
		 * @details The event queue maintains the reference clock (system clock). As
		 * soon as the reference clock maintains the next event's time, this event
		 * will be scheduled. Since the simulation time does not correspond to an
		 * absolute time-stamp, the reference time (t_event = 0.0) will be set to
		 * the C'tor's time of execution.
		 */
		class TimedEventQueue: public EventQueue
		{
		public:

			/**
			 * C'tor generating an empty TimedEventQueue
			 */
			TimedEventQueue(void);

			/** @brief Frees allocated resources */
			virtual ~TimedEventQueue() {}

			/**
			 * @copydoc EventQueue::add()
			 */
			virtual void add(Event * ev, bool predicted);

			/**
			 * @brief Returns the first event
			 * @details It will wait until the next event's time has expired. If no 
			 * event has been registered, the function will throw a std::logic_error.
			 * @return A previously registered event
			 */
			virtual Event * get(void);

		private:

			/**
			 * @brief Ordered list of upcoming events
			 * @details The most recent event will be at the first indices. The
			 * boolean flag indicates that the entry is a prediction which is
			 * invalidated by new events.
			 */
			std::list<std::pair<Event*,bool>> queue_;

			/**
			 * @brief Mutex which is required to read or write any queue related data
			 */
			boost::mutex queueMut_;

			/**
			 * @brief Condition variable which gets notified, if a new variable is
			 * added
			 */
			boost::condition_variable newEventCondition_;

			/** @brief Time-stamp of the fmiTime == 0 */
			boost::system_time  localEpoch_;

			/**
			 * @brief Dequeues every predicted value after the given time
			 * @details The function assumes that the mutex has been acquired
			 * @param time The event's time used to separate predictions to delete
			 * from those to keep.
			 */
			void removePredictions(fmiTime time);

			/**
			 * @brief Puts the event in the event queue
			 * @details The queue won't be cleaned nor locked. The function expects
			 * the caller to acquire the lock before actually calling the function.
			 * @param ev The event pointer to push, not NULL
			 * @param predicted Flag which indicates the event's prediction status.
			 */
			void push(Event * ev, bool predicted);

			/**
			 * @brief Returns the system time of the given event
			 * @details Converts the time based on the simulation's starting time.
			 * @param ev A valid event reference used to obtain the relative time
			 * @return The corresponding system time object
			 */
			boost::system_time getSystemTime(const Event* ev);

			/**
			 * @brief Returns whether the event's time is a future time-stamp
			 * @param ev A valid pointer to the event object
			 * @return The temporal status of the event
			 */
			bool isFutureEvent(const Event* ev);

		};

	}
}

#endif
