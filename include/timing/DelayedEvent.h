/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file DelayedEvent.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_TIMING_DELAYED_EVENT
#define _FMITERMINALBLOCK_TIMING_DELAYED_EVENT

#include "Event.h"

namespace FMITerminalBlock 
{
	namespace Timing
	{
		/**
		 * @brief Encapsulates an event instance and alters the notion of time
		 * @details The dDelayedEvent class provides a mechanism of changing the 
		 * otherwise immutable time of an event. The encapsulation mechanism was 
		 * chosen to explicitly change the time of an event without introducing a 
		 * setter function which may not be always called. Hence, it may be safely
		 * assumed that the time of a single event instance always stays constant.
		 */
		class DelayedEvent: public Event
		{
		public:

			/**
			 * @brief Initializes the re-timed event with the new time-stamp
			 * @param newTime The new time instant of the event
			 * @param ev A valid pointer to an event object. The ownership of the 
			 * given event is transfered to the newly constructed event and ev will 
			 * be deleted as soon as the object gets deleted. ev Will be used to 
			 * ignored.
			 */
			DelayedEvent(fmiTime newtime, Event *ev);

			/** @brief Frees allocated resources */
			virtual ~DelayedEvent();

			/** @brief Returns the variables of the encapsulated event instance */
			virtual std::vector<Variable> getVariables(void);

			/** @brief Returns a string representation of the event object */
			virtual std::string toString(void) const;

		private:
			/** @brief A reference to the encapsulated event instance */
			Event *event_;

		};
	}
}

#endif
