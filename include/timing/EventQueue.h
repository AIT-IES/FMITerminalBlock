/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file EventQueue.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_EVENT_QUEUE
#define _FMITERMINALBLOCK_EVENT_QUEUE

#include "timing/Event.h"
#include "timing/EventSink.h"

namespace FMITerminalBlock 
{
	namespace Timing
	{
		/**
		 * @brief Abstract class which defines the basic queue functionality.
		 * @details <p>An EventQueue instance will manage events concurrently. It
		 * provides getter functions to obtain the next event and adder functions
		 * used to register new events. To avoid deadlocks, an event queue mustn't
		 * hold any locks unless a function is being executed. It also has to handle
		 * predicted events. A predicted event will have to be removed and deleted
		 * if non-predicted events are queued.</p>
		 * <p>The event queue only operates on event pointers. Any given pointer 
		 * must be valid until it is returned again. Before storing predicted 
		 * results and before querying any events, the initStartTimeNow(fmiTime) 
		 * function has to be called.</p>
		 */
		class EventQueue : public EventSink
		{
		public:

			/** @brief Frees allocated resources */
			virtual ~EventQueue() {}

			/**
			 * @brief Sets the start time of the simulation and indicates that this 
			 * time instance is now.
			 * @details For real-time simulations, the information may be utilized to
			 * adjust the local epoch accordingly. The function will be called before
			 * the actual simulation run starts. I.e. there should not be a predicted
			 * event which was registered before. Nevertheless, it may be possible 
			 * that an external event will be registered before. In case the outcome
			 * of any queue function may change by calling initStartTimeNow(), the 
			 * queue is advised to block any function call which may not be 
			 * reasonable before initStartTimeNow() is called.
			 * @param start The first simulation time instant which is exactly 
			 * now.
			 */
			virtual void initStartTimeNow(fmiTime start) = 0;

			/**
			 * @brief Adds the event to the event queue.
			 * @details Any non-predicted event will remove predicted events from the 
			 * queue and will release any thread which is currently waiting. A non-
			 * predicted event or the data thereof will always be returned by get().
			 * A predicted event may be refused, if it is already outdated. I.e. it 
			 * won't be added to the queue and the function returns without modifying 
			 * the queue.
			 * @param ev A valid pointer to the event to queue
			 * @param predicted Flag indicating the event's source
			 */
			virtual void add(Event * ev, bool predicted) = 0;

			/**
			 * @brief Returns the next event
			 * @details The function may block until a certain point in time or return 
			 * the event immediately.
			 * @return The previously stored event pointer, not NULL
			 */
			virtual Event * get(void) = 0;
		};

	}
}

#endif

