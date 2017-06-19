/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testEventPredictorFactory.cpp
 * @details Shortly assesses the EventPredictorFactory
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#define BOOST_TEST_MODULE testEventHandling
#include <boost/test/unit_test.hpp>

#include "model/EventPredictorFactory.h"
#include "model/EventPredictor.h"
#include "base/BaseExceptions.h"

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Model;

/** @brief Tests the default event predictor generation */
BOOST_AUTO_TEST_CASE( testDefaultConstruction )
{
	Base::ApplicationContext appContext;
	const char* props[] = {
		"testEventPredictorFactory", 
		"fmu.path=" FMU_URI_PRE "zerocrossing", "fmu.name=zerocrossing", 
		"app.lookAheadTime=1.1", "out.0.0=x", "out.0.0.type=0"
	};
	appContext.addCommandlineProperties(sizeof(props) / sizeof(props[0]), props);

	std::shared_ptr<AbstractEventPredictor> pred;
	pred = EventPredictorFactory::makeEventPredictor(appContext);

	BOOST_CHECK(pred);
	BOOST_CHECK(std::dynamic_pointer_cast<EventPredictor>(pred));
}

/** @brief Tests the explicit construction of EventPredictor */
BOOST_AUTO_TEST_CASE( testExplicitEventPredictorConstruction )
{
	Base::ApplicationContext appContext;
	const char* props[] = {
		"testEventPredictorFactory", 
		"fmu.path=" FMU_URI_PRE "zerocrossing", "fmu.name=zerocrossing", 
		"app.lookAheadTime=1.1", "out.0.0=x", "out.0.0.type=0",
		"app.simulationMethod=multistep-prediction"
	};
	appContext.addCommandlineProperties(sizeof(props) / sizeof(props[0]), props);

	std::shared_ptr<AbstractEventPredictor> pred;
	pred = EventPredictorFactory::makeEventPredictor(appContext);

	BOOST_CHECK(pred);
	BOOST_CHECK(std::dynamic_pointer_cast<EventPredictor>(pred));
}

/** @brief Assesses the error handling of invalid predictor name properties */
BOOST_AUTO_TEST_CASE(testInvalidPredictorName)
{
		Base::ApplicationContext appContext;
	const char* props[] = {
		"testEventPredictorFactory", 
		"fmu.path=" FMU_URI_PRE "zerocrossing", "fmu.name=zerocrossing", 
		"app.lookAheadTime=1.1", "out.0.0=x", "out.0.0.type=0",
		"app.simulationMethod=ask-the-oracle"
	};
	appContext.addCommandlineProperties(sizeof(props) / sizeof(props[0]), props);
	BOOST_CHECK_THROW(EventPredictorFactory::makeEventPredictor(appContext), 
		Base::SystemConfigurationException);
}
