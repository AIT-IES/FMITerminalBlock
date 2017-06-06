/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file PartialEvent.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_NETWORK_PARTIAL_EVENT
#define _FMITERMINALBLOCK_NETWORK_PARTIAL_EVENT

#include "timing\Event.h"

#include <vector>

namespace FMITerminalBlock 
{
	namespace Network
	{
		using namespace FMITerminalBlock;

		/**
		 * @brief Event which holds a partial variable assignment.
		 * @details The Event may be iteratively populated by pushing the next 
		 * variable. The event also holds a template with all port IDs in order to 
		 * keep track of the status. It is assumed that all variables will be 
		 * populated in the order of the template vector. Hence, it is not 
		 * necessary to keep track of the port IDs as they will be automatically 
		 * set.
		 */
		class PartialEvent: public Timing::Event
		{
		public:
			/**
			 * @brief C'tor setting the event's data
			 * @details The Event does not contain any variable and will be initially
			 * empty
			 * @param time The event's time
			 * @param portTemplate The vector of expected port IDs. It is assumed 
			 * that the first port ID will also be registered first.
			 */
			PartialEvent(fmiTime time, 
				const std::vector<Base::PortID> &portTemplate);

			/** @brief Frees allocated resources */
			virtual ~PartialEvent(void){};

			/**
			 * @copydoc FMITerminalBlock::Timing::Event::getVariables()
			 */
			virtual std::vector<Timing::Variable> getVariables(void);

			/**
			 * @copydoc FMITerminalBlock::Timing::Event::toString()
			 */
			virtual std::string toString(void) const;

			/** @brief Returns whether all variables were been received or ignored */
			bool hasRemainingElements() const;

			/** 
			 * @brief Returns the fmiType of the next PortID 
			 * @details It is assumed that there are sill remaining elements left.
			 */
			FMIVariableType getNextPortType() const;

			/**
			 * @brief Appends the value to the list of variables
			 * @details It is assumed that there are still remaining elements. 
			 * Additionally, it is assumed that the type of the value corresponds to 
			 * the type of the port.
			 */
			void pushNextValue(boost::any value);

			/**
			 * @brief Skips the next port and don't append it.
			 * @brief It is assumed that there are still remaining elements.
			 */
			void ignoreNextValue();

		private:
			/** @brief The list of registered variables */
			std::vector<Timing::Variable> var_;

			/** @brief The full list of expected PortIDs */
			const std::vector<Base::PortID> &portTemplate_;

			/** 
			 * @brief The index of the next variable
			 * @details If the index is greater or equal the size of the 
			 * portTemplate_ vector, the event is considered to be fully populated.
			 */
			unsigned int nextTemplateIndex_ = 0;
		};

	}
}

#endif
