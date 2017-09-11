/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file TimedEventQueue.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_TIMING_TIMED_EVENT_QUEUE
#define _FMITERMINALBLOCK_TIMING_TIMED_EVENT_QUEUE

#include "timing/EventQueue.h"
#include "timing/EventLogger.h"

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
		/**
		 * @brief EventQueue implementation issuing predicted events in time.
		 * @details <p> The event queue maintains the reference clock (system 
		 * clock). As soon as the reference clock maintains the next event's time, 
		 * this event will be scheduled. Since the simulation time does not 
		 * correspond to an absolute time-stamp, the reference time (t_event = 0.0)
		 * will be set to the C'tor's time of execution. </p>
		 * 
		 * <p>The queue automatically outdates predicted events whenever another 
		 * event is scheduled before the predicted one. It is assumed that 
		 * predicted events depend on all previously scheduled events. Hence, they 
		 * are deleted, if they do not correspond to the first event. If multiple 
		 * events occur at the same instant of time, they are considered as a 
		 * unique single event. Still, a new prediction may outdate an old one at 
		 * the same instant of time. Hence, equally timed events are stored in 
		 * separate places in the queue. In order to maintain the invariant, that 
		 * the predicted event is always at the first place in the queue, 
		 * simultaneous events will be sorted accordingly. </p>
		 *
		 * <p>For future implementations it is advised to merge events which occur 
		 * at the same instant of time. Such an implementation may avoid redundant 
		 * predictions and may improve realtime performance.</p>
		 */
		class TimedEventQueue: public EventQueue
		{
		public:

			/** 
			 * @brief The tolerance used to compare time instants
			 * @details The value is always greater or equal (yes folks, welcome to 
			 * discrete math :-) ) to zero.
			 */
			const fmiTime eps_ = 1e-3;

			/**
			 * @brief C'tor generating an empty TimedEventQueue
			 * @details The start of simulation time is taken at the C'tor and 
			 * globally registered for all EventLogger instances.
			 */
			TimedEventQueue(void);

			/** @brief Frees allocated resources */
			virtual ~TimedEventQueue() {}

			/** @copydoc EventQueue::initStartTimeNow(fmiTime) */
			virtual void initStartTimeNow(fmiTime start);

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

			/** @copydoc EventSink::pushExternalEvent(Event) */
			virtual void pushExternalEvent(Event *ev);

			/** @copydoc EventSink::getTimeStampNow() */
			virtual fmiTime getTimeStampNow();

		private:
			/**
			 * @brief Ordered list of upcoming events
			 * @details The most recent event will be at the first indices. The
			 * boolean flag indicates that the entry is a prediction which is
			 * invalidated by new events. The queue may contain at most one 
			 * predicted entry at the very first position.
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

			/** 
			 * @brief Mutex which is released as soon as the localEpoch_ and related
			 * variables are initialized
			 * @details Please release the mutex as soon as it is acquired. Otherwise 
			 * other threads may be unnecessarily blocked.
			 */
			boost::mutex timeInitMut_;

			/** @brief Time-stamp of the fmiTime == 0 */
			boost::system_time  localEpoch_;

			/** @brief Used to record external events and timed queue specifics */
			EventLogger eventLoggerInstance_;

			/** 
			 * @brief Blocks the current thread until the localEpoch_ and related 
			 * variables may be safely accessed. 
			 * @details In case the time was initialized before, the function 
			 * returns immediately.
			 */
			inline void waitUntilTimeIsInitiailzed();

			/**
			 * @brief Dequeues every predicted value after the given time
			 * @details The function assumes that the mutex has been acquired 
			 * before. Removed events will have a time instant which is strictly 
			 * greater than the given instant in time.
			 * @param time The event's time used to separate predictions to delete
			 * from those to keep.
			 */
			void removeFuturPredictions(fmiTime time);

			/**
			 * @brief Removes any prediction which has the same instant of time.
			 * @details It is assumed that the queue is locked. The function will 
			 * not lock the queue again.
			 * @param time The time of any prediction to remove
			 */
			void removeConcurrentPrediction(fmiTime time);

			/**
			 * @brief Puts the event in the event queue
			 * @details The queue won't be cleaned nor locked. The function expects
			 * the caller to acquire the lock before actually calling the function.
			 * A predicted event at the same time instant of an external one will
			 * always be scheduled first.
			 * @param ev The event pointer to push, not NULL
			 * @param predicted Flag which indicates the event's prediction status.
			 */
			void push(Event * ev, bool predicted);

			/**
			 * @brief Returns the system time of the given event
			 * @details Converts the time based on the simulation's starting time. 
			 * The function does not wait until the time is initialized. Hence, it 
			 * can be used during the initialization process.
			 * @param ev A valid event reference used to obtain the relative time
			 * @return The corresponding system time object
			 */
			boost::system_time getSystemTime(const Event* ev) const;

			/**
			 * @brief Returns the duration of the fmiTime instant relative to 
			 * the epoch
			 * @param time A valid simulation time instant
			 * @return The corresponding duration object
			 */
			static boost::posix_time::time_duration getRelativeTime(fmiTime time);

			/**
			 * @brief Returns the simulation time of the given system time instant
			 * @details The simulation time will be based on the local notion of time 
			 * which is stored in localEpoch_. The function does not wait until the 
			 * time is initialized. Hence, it can be used during the initialization 
			 * process.
			 * @param sysTime The system time of the event.
			 */
			fmiTime getSimulationTime(const boost::system_time &sysTime) const;

			/**
			 * @brief Returns whether the event's time is a future time-stamp
			 * @details The function does not wait until the time is initialized.
			 * @param ev A valid pointer to the event object
			 * @return The temporal status of the event
			 */
			bool isFutureEvent(const Event* ev) const;

			/**
			 * @brief Checks whether the queue holds events strictly before maxTime
			 * @details The queue will not be locked. Hence, it is assumed that the 
			 * current thread has acquired the lock externally.
			 * @return <code>true</code> if an events exists which has a time 
			 * instant strictly less than the given one.
			 */
			bool hasPriorEvents(fmiTime maxTime) const;

			/**
			 * @brief Checks whether the queue is in a consistent state according 
			 * to the predictions
			 * @details The function will assumes that the queue is already locked.
			 */
			bool isQueuePredictionConsistent() const;

			/**
			 * @brief Returns a string representation of the queue.
			 * @details The function assumes that the queue mutex is already locked.
			 */
			std::string toString();

			/**
			 * @brief Marks the given event as deleted and destroys it.
			 * @details The function will log a timing event which indicates that the
			 * event has been destroyed without actually triggering it.
			 */
			void deleteEvent(Event* ev);
		};

	}
}

#endif
