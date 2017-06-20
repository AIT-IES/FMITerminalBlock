/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file EventPredictorFactory.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "model/EventPredictorFactory.h"

#include <assert.h>

#include "model/EventPredictor.h"
#include "model/OneStepEventPredictor.h"
#include "base/BaseExceptions.h"

using namespace FMITerminalBlock::Model;

const std::string EventPredictorFactory::PROP_EVENT_PREDICTOR = "app.simulationMethod";

std::shared_ptr<AbstractEventPredictor>
EventPredictorFactory::makeEventPredictor(Base::ApplicationContext &appContext)
{
	std::string predictorName;
	predictorName = appContext.getProperty<std::string>(PROP_EVENT_PREDICTOR, 
		"multistep-prediction");

	if (predictorName == "multistep-prediction")
	{
		return std::make_shared<EventPredictor>(appContext);
	} else if (predictorName == "singlestep-delayed") {
		return std::make_shared<OneStepEventPredictor>(appContext);
	} else {
		throw Base::SystemConfigurationException("Invalid simulation method "
			"property", PROP_EVENT_PREDICTOR, predictorName);
	}
}
