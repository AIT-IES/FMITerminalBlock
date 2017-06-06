/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file EventListener.h
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_TIMING_EVENT_LISTENER
#define _FMITERMINALBLOCK_TIMING_EVENT_LISTENER

#include "timing/Event.h"

namespace FMITerminalBlock 
{
	namespace Timing
	{
		using namespace FMITerminalBlock;

		/**
		 * @brief Abstract class which specifies some functions called, if an event
		 * occurs.
		 * @details The event listener interface class indicates that the
		 * implementing class is capable of receiving event notifications.
		 */
		class EventListener
		{
		public:

			/**
			 * @brief Signals that an event was triggered
			 * @details The event pointer only stays valid during the function 
			 * invocation. The event may be deleted after the function returns.
			 */
			virtual void eventTriggered(Event * ev) = 0;

		};

	}
}

#endif
