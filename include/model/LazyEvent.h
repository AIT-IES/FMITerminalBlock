/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file LazyEvent.h
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_MODEL_LAZY_EVENT
#define _FMITERMINALBLOCK_MODEL_LAZY_EVENT

#include "timing/Event.h"
#include "base/ChannelMapping.h"
#include "model/EventPredictor.h"
#include <vector>

namespace FMITerminalBlock
{
	namespace Model
	{

		using namespace FMITerminalBlock;

		/**
		 * @brief Event which dynamically fetches its variable values
		 * @details The lazy loading event will query the variable values on their 
		 * first access. It stores a reference to the event predictor and updates 
		 * the predictor's state, before the values are retrieved. The values
		 * should only be queried if the event actually settled. After the value
		 * is read, the event mustn't be reset.
		 */
		class LazyEvent: public Timing::Event
		{
		public:
			/**
			 * @brief C'tor initializing the event
			 */
			LazyEvent(fmiTime time, EventPredictor &predictor);

			/** @brief Frees allocated resources */
			virtual ~LazyEvent(){};

			/**
			 * @brief Returns a copy of the event's variables
			 * @details The function may only be called if the variable's values are 
			 * really needed and if the event is processed. It queries the predictor 
			 * and returns the model's output variables. After retrieving the
			 * variables, the model might not be able to reset the event.
			 * @return The vector of changed or relevant variables
			 */
			virtual std::vector<Timing::Variable> getVariables(void);

			/**
			 * @brief Returns the object's readable string representation
			 * @details The function will return a reduced string if the variables 
			 * havn't been queried before. It won't settle the event's state.
			 * @return The object's readable string representation
			 */
			virtual std::string toString(void) const;

		private:
			/** @brief A reference to the event predictor holding the event's data */
			EventPredictor &predictor_;

		};

	}
}

#endif
