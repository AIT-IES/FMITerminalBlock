/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file StaticEvent.h
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_TIMING_STATIC_EVENT
#define _FMITERMINALBLOCK_TIMING_STATIC_EVENT

#include "timing\Event.h"

#include <vector>

namespace FMITerminalBlock 
{
	namespace Timing
	{
		using namespace FMITerminalBlock;

		/**
		 * @brief Event which holds predefined variables.
		 * @details A StaticEvent holds an immutable copy of every modified
		 * variable. The variables will be set at the C'tor and can't be changed
		 * afterwards.
		 */
		class StaticEvent: public Event
		{
		public:
			/**
			 * @brief C'tor setting the event's data
			 * @param time The event's time
			 * @param var The list of populated variables
			 */
			StaticEvent(fmiTime time, const std::vector<Variable> &var);

			/** @brief Frees allocated resources */
			virtual ~StaticEvent(void){};

			/**
			 * @copydoc FMITerminalBlock::Timing::Event::getVariables()
			 */
			virtual std::vector<Variable> getVariables(void);

			/**
			 * @copydoc FMITerminalBlock::Timing::Event::toString()
			 */
			virtual std::string toString(void) const;

		private:
			/** @brief The list of associated variables */
			const std::vector<Variable> var_;

		};

	}
}

#endif
