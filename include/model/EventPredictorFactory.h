/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file EventPredictorFactory.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_MODEL_EVENT_PREDICTOR_FACTORY
#define _FMITERMINALBLOCK_MODEL_EVENT_PREDICTOR_FACTORY

#include <string>
#include <memory>

#include "model/AbstractEventPredictor.h"
#include "base/ApplicationContext.h"

namespace FMITerminalBlock
{
	namespace Model
	{

		/** 
		 * @brief Encapsulates some functions which create a event predictor
		 * @details Currently, available event predictors are manually coded into 
		 * the factory functions.
		 */
		class EventPredictorFactory
		{
		public:
			/** @brief Property name which specifies the event predictor to use */
			static const std::string PROP_EVENT_PREDICTOR;

			/**
			 * @brief Generates a new event predictor
			 * @details Reads the configuration and constructs the appropriate event
			 * predictor. The event predictor will not be initialized but the 
			 * application context may be passed to the event predictor.
			 * The function may throw a SystemConfigurationException, in case an 
			 * invalid configuration is found.
			 */
			static std::shared_ptr<AbstractEventPredictor> makeEventPredictor(
				Base::ApplicationContext &appContext);

		};
	}
}
#endif
