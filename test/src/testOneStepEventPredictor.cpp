/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testOneStepEventPredictor.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#define BOOST_TEST_MODULE testOneStepEventPredictor
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/log/trivial.hpp>

#include "model/OneStepEventPredictor.h"
#include "base/ApplicationContext.h"
#include "base/BaseExceptions.h"

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Model;

namespace data = boost::unit_test::data;

/** @brief All known zigzag implementations */
const char* ZIGZAG_FMU_NAMES[] = {"zigzag", "zigzag2"};

/** @brief Configures the model base properties */
void addModelBaseProperties(Base::ApplicationContext *appContext,
	const std::string &name)
{
	BOOST_REQUIRE(appContext);
	
	std::string pathProp("fmu.path=" FMU_URI_PRE);
	pathProp += name;

	std::string nameProp("fmu.name=");
	nameProp += name;

	const char *argv[] = {"testOneStepEventPredictor", pathProp.c_str(), 
		nameProp.c_str()};
	appContext->addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);
}

/** @brief Test the instantiation of various models */
BOOST_DATA_TEST_CASE(testInstantiation, data::make(ZIGZAG_FMU_NAMES), name)
{
	Base::ApplicationContext appContext;
	addModelBaseProperties(&appContext, name);

	const char * argv[] = {"testOneStepEventPredictor", 
		"out.0.0=x", "out.0.0.type=0"};
	appContext.addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);

	OneStepEventPredictor pred(appContext);
}

/** @brief Test the instantiation and initialization of various models */
BOOST_DATA_TEST_CASE(testInstantiationAndInitialization0, 
	data::make(ZIGZAG_FMU_NAMES), name)
{
	Base::ApplicationContext appContext;
	addModelBaseProperties(&appContext, name);

	const char * argv[] = {"testOneStepEventPredictor", 
		"app.startTime=0.0", "app.lookAheadTime=1.0",
		"out.0.0=x", "out.0.0.type=0"};
	appContext.addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);

	OneStepEventPredictor pred(appContext);
	pred.init();
}

/** @brief Test the instantiation and initialization of another model */
BOOST_AUTO_TEST_CASE(testInstantiationAndInitialization1)
{
	Base::ApplicationContext appContext;
	addModelBaseProperties(&appContext, "dxiskx");

	const char * argv[] = {"testOneStepEventPredictor", 
		"app.startTime=0.0", "app.lookAheadTime=1.0",
		"out.0.0=x", "out.0.0.type=0",
		"in.0.0=u", "in.0.0.type=0"};
	appContext.addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);
	
	try
	{
		OneStepEventPredictor pred(appContext);
		pred.init();
	} catch (Base::SystemConfigurationException &ex) {
		BOOST_LOG_TRIVIAL(error) << ex.what() << "Key: " << ex.getKey() << 
			" Value: " << ex.getValue();
		BOOST_CHECK(false);
	}
}

/** @brief Test add defaults */
BOOST_AUTO_TEST_CASE(test_configureDefaultApplicationContext)
{
	Base::ApplicationContext appContext;
	addModelBaseProperties(&appContext, "zerocrossing");

	const char * argv[] = {"testOneStepEventPredictor", 
		"app.startTime=0.0", "app.lookAheadTime=1.0",
		"out.0.0=x", "out.0.0.type=0",
		"in.0.0=u", "in.0.0.type=0"};
	appContext.addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);

	OneStepEventPredictor pred(appContext);
	pred.configureDefaultApplicationContext(&appContext);

	BOOST_CHECK_EQUAL(appContext.getProperty<fmiReal>("app.startTime"), 0.0);
}

/** TODO: Test multiple events in one prediction step (fixed step size mode) */
/** 
 * TODO: Test event detection in variable step size mode 
 * @details The next step will also be simulated in order to test the time 
 * management
 */
/** TODO: Test setting external default values for inputs and parameters */
/** TODO: Test the occurrence of multiple input events in one step */

/** @brief Test an invalid model name */
BOOST_DATA_TEST_CASE(testInvalidModelName, data::make(ZIGZAG_FMU_NAMES), name)
{
	std::string pathProp("fmu.path=" FMU_URI_PRE);
	pathProp += name;

	Base::ApplicationContext appContext;

	const char * argv[] = {"testOneStepEventPredictor", 
		"fmu.name=nemo", pathProp.c_str(),
		"app.startTime=0.0", "app.lookAheadTime=1.0",
		"out.0.0=x", "out.0.0.type=0",
		"in.0.0=u", "in.0.0.type=0"};
	appContext.addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);
	
	BOOST_CHECK_THROW(OneStepEventPredictor pred(appContext), 
		Base::SystemConfigurationException);
}

/** @brief Test an invalid model URL */
BOOST_AUTO_TEST_CASE(testInvalidModelPath)
{
	Base::ApplicationContext appContext;

	const char * argv[] = {"testOneStepEventPredictor", 
		"fmu.name=no-model", "fmu.path=file:/no-path-today",
		"app.startTime=0.0", "app.lookAheadTime=1.0",
		"out.0.0=x", "out.0.0.type=0",
		"in.0.0=u", "in.0.0.type=0"};
	appContext.addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);
	
	BOOST_CHECK_THROW(OneStepEventPredictor pred(appContext), 
		Base::SystemConfigurationException);
}

/** @brief Test invalid default variable key */
BOOST_DATA_TEST_CASE(testInvalidDefaultKey, 
	data::make(ZIGZAG_FMU_NAMES), name)
{
	Base::ApplicationContext appContext;
	addModelBaseProperties(&appContext, name);

	const char * argv[] = {"testOneStepEventPredictor", 
		"app.startTime=0.0", "app.lookAheadTime=1.0",
		"out.0.0=x", "out.0.0.type=0",
		"in.default.superman=0"};
	appContext.addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);

	OneStepEventPredictor pred(appContext);
	BOOST_CHECK_THROW(pred.init(), Base::SystemConfigurationException);
}

/** @brief Test invalid default variable value */
BOOST_DATA_TEST_CASE(testInvalidDefaultValue, 
	data::make(ZIGZAG_FMU_NAMES), name)
{
	Base::ApplicationContext appContext;
	addModelBaseProperties(&appContext, name);

	const char * argv[] = {"testOneStepEventPredictor", 
		"app.startTime=0.0", "app.lookAheadTime=1.0",
		"out.0.0=x", "out.0.0.type=0",
		"in.default.x=not-a-real-real-value"};
	appContext.addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);

	OneStepEventPredictor pred(appContext);
	BOOST_CHECK_THROW(pred.init(), std::invalid_argument);
}

/** TODO: Test an invalid model type (co-simulation) */
