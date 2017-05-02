/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file EventSink.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_TIMING_EVENT_SINK
#define _FMITERMINALBLOCK_TIMING_EVENT_SINK

#include "Event.h"

namespace FMITerminalBlock
{
	namespace Timing
	{

		/**
		 * @brief Interface to handle external events
		 * @details An EventSink is an abstract, object which allows to register 
		 * external events. Additionally, interfaces which resolve the current 
		 * instance of simulation time are provided. Every function of an event 
		 * sink must be able to handle concurrent accesses.
		 */
		class EventSink
		{
		public:
			/**
			 * @brief Registers an event which was triggered by an external application
			 * @details Ownership of the event instance is transferred to the receiver. 
			 * Hence, the Event object referenced by ev must be deleted by the called 
			 * object. The caller must ensure that the given pointer remains valid 
			 * until the called object eventually deletes the event. 
			 * The function must not throw an exception.
			 * @param ev A valid pointer to an Event object.
			 */
			virtual void pushExternalEvent(Event *ev) = 0;

			/**
			 * @brief Returns the current instance of simulation time
			 * @details In real-time operation, the function returns the current 
			 * instance of time which is expressed as simulation time. The returned 
			 * value may be used to describe the time of an event in cases where no 
			 * synchronized external time reference is available.
			 * @return The current instance of time
			 */
			virtual fmiTime getTimeStampNow() = 0;

			// TODO: Extend the EventSink such that external time references are
			//       Supported.
		};

	}
}

#endif
