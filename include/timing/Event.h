/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file Event.h
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_TIMING_EVENT
#define _FMITERMINALBLOCK_TIMING_EVENT

#include "base/PortID.h"

#include <common/fmi_v1.0/fmiModelTypes.h>
#include <boost/any.hpp>
#include <vector>
#include <utility>


namespace FMITerminalBlock 
{
	namespace Timing
	{
		using namespace FMITerminalBlock;

		/**
		 * @brief Represents a point in time where one or more values change
		 * @details The event class provides timed events and encapsulates 
		 * changed variables. The values may be encapsulated into the event or may
		 * be retrieved on access.
		 */
		class Event
		{
		public:

			/** 
			 * @brief Defines a variable which may be changed during the event
			 * @details The first parameter identifies the variable and the second 
			 * parameter stores its value
			 */
			typedef std::pair<Base::PortID, boost::any> Variable;

			/**
			 * @brief C'tor initializing an empty event with the given time-stamp
			 * @param time The event's time-stamp.
			 */
			Event(fmiTime time);

			/** @brief Frees allocated resources */
			virtual ~Event(){};

			/**
			 * @brief Returns a copy of the event's variables
			 * @details The function shall only be called if the variable's values are
			 * really needed and if the event is processed. After retrieving the
			 * variables it might not be able to reset the event.
			 * @return The vector of changed or relevant variables
			 */
			virtual std::vector<Variable> getVariables(void) = 0;

			/**
			 * @brief Returns the previously set time
			 * @return The event's time-stamp
			 */
			fmiTime getTime(void) const { return time_; }

			/**
			 * @brief Returns the object's readable string representation
			 * @return The object's readable string representation
			 */
			virtual std::string toString(void) const;

		protected:

			/** @brief The event's time */
			const fmiTime time_;

			/**
			 * @brief Checks whether each PortID-fmiType corresponds to the
			 * appropriate any type
			 * @details The FmiType String will be mapped to std::string to avoid
			 * memory leakages and buffer issues
			 * @return The result of the consistency check
			 */
			static bool isValid(const std::vector<Variable> &values);


			/**
			 * @brief Helper function which converts the given variable list to a
			 * human readable string.
			 * @param vars The vector of variable values
			 * @return A human readable string representation of the variable list.
			 */
			static std::string toString(const std::vector<Variable> &vars);

		};

	}
}

#endif
