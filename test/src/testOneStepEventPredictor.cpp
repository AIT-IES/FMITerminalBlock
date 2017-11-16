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
#include "SolverConfigurationSets.h"

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Model;
using namespace FMITerminalBlockTest::Model;

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

/// Test the instantiation and initialization of a model using name deduction 
BOOST_AUTO_TEST_CASE(testInstantiationAndInitialization2)
{
	Base::ApplicationContext appContext;

	const char * argv[] = {"testOneStepEventPredictor", 
		"fmu.path=" FMU_URI_PRE "dxiskx", // No name
		"app.startTime=0.0", "app.lookAheadTime=1.0",
		"out.0.0=x", "out.0.0.type=0",
		"in.0.0=u", "in.0.0.type=0"};
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);
		
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

/** @brief Test multiple events in one prediction step (fixed step size mode)*/
BOOST_DATA_TEST_CASE(testMultipleEventsPerPredictionStep, 
	data::make(ZIGZAG_FMU_NAMES)*data::make(createValidSolverParameterSet()), 
	name, solverParams)
{
	Base::ApplicationContext appContext;
	addModelBaseProperties(&appContext, name);

	const char * argv[] = {"testOneStepEventPredictor", 
		"app.startTime=0.0", "app.lookAheadTime=3.5", "app.variableStepSize=false",
		"out.0.0=x", "out.0.0.type=0", "out.0.1=der(x)", "out.0.1.type=0"};
	appContext.addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);
	appContext.addCommandlineProperties(solverParams);

	OneStepEventPredictor pred(appContext);
	pred.init();

	Timing::Event *ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 3.5, 1e-4);

	auto vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 2);

	BOOST_CHECK_CLOSE(vars[0].getRealValue(), -0.5, 1.0);
	BOOST_CHECK_CLOSE(vars[1].getRealValue(), 1.0, 1.0);
	delete ev;

	ev = pred.predictNext(); // Still the same event
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 3.5, 1e-4);
	delete ev;
}

/** 
 * @brief Test event detection in variable step size mode 
 * @details The next step will also be simulated in order to test the time 
 * management
 */
BOOST_DATA_TEST_CASE(testEventDetection, 
	data::make(ZIGZAG_FMU_NAMES)*data::make(createValidSolverParameterSet()), 
	name, solverParams)
{
	Base::ApplicationContext appContext;
	addModelBaseProperties(&appContext, name);

	const char * argv[] = {"testOneStepEventPredictor", 
		"app.startTime=0.0", "app.lookAheadTime=1.5", "app.variableStepSize=true",
		"out.0.0=x", "out.0.0.type=0", "out.0.1=der(x)", "out.0.1.type=0"};
	appContext.addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);
	appContext.addCommandlineProperties(solverParams);

	OneStepEventPredictor pred(appContext);
	pred.init();

	Timing::Event *ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 1.0, 1e-3);

	auto vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 2);

	BOOST_CHECK_CLOSE(vars[0].getRealValue(), 1.0, 1.0); // x
	BOOST_CHECK_CLOSE(vars[1].getRealValue(), -1.0, 1.0); // der(x)

	pred.eventTriggered(ev);
	delete ev;

	// No event in prediction horizon
	ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 2.5, 1e-3);

	vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 2);

	BOOST_CHECK_CLOSE(vars[0].getRealValue(), -0.5, 1.0);
	BOOST_CHECK_CLOSE(vars[1].getRealValue(), -1.0, 1.0);

	pred.eventTriggered(ev);
	delete ev;

	// Event triggered
	ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 3.0, 1e-3);

	vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 2);

	BOOST_CHECK_CLOSE(vars[0].getRealValue(), -1.0, 1.0);
	BOOST_CHECK_CLOSE(vars[1].getRealValue(), 1.0, 1.0);

	pred.eventTriggered(ev);
	delete ev;
}

/** @brief Test setting external default values for inputs and parameters */
BOOST_DATA_TEST_CASE(testDefaultInitialization, 
	data::make(ZIGZAG_FMU_NAMES), name)
{
	Base::ApplicationContext appContext;
	addModelBaseProperties(&appContext, name);

	const char * argv[] = {"testOneStepEventPredictor", 
		"app.startTime=0.0", "app.lookAheadTime=2.5", "app.variableStepSize=1",
		"out.0.0=x", "out.0.0.type=0", "out.0.1=der(x)", "out.0.1.type=0",
		"in.default.k=0.5"
	};
	appContext.addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);

	OneStepEventPredictor pred(appContext);
	pred.init();

	Timing::Event *ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 2.0, 1e-3);

	auto vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 2);

	BOOST_CHECK_CLOSE(vars[0].getRealValue(), 1.0, 1.0);
	BOOST_CHECK_CLOSE(vars[1].getRealValue(), -0.5, 1.0);

	pred.eventTriggered(ev);
	delete ev;
}

/** @brief Test the occurrence of multiple input events in one step */
BOOST_AUTO_TEST_CASE(testMultipleInputEvents)
{
	Base::ApplicationContext appContext;
	addModelBaseProperties(&appContext, "dxiskx");

	const char * argv[] = {"testOneStepEventPredictor", 
		"app.startTime=0.0", "app.lookAheadTime=1.0", "app.variableStepSize=0",
		"out.0.0=x", "out.0.0.type=0",
		"in.0.0=u", "in.0.0.type=0",
		"in.default.u=0.0", "in.default.k=1.0", "in.default.x0=0.0"
	};
	appContext.addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);
	
	OneStepEventPredictor pred(appContext);
	pred.init();

	Base::PortID uPortID = appContext.getInputChannelMapping()->getPortID("u");

	// First prediction
	Timing::Event *ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 1.0, 1e-3);

	auto vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 0);
	delete ev;

	// Apply the first event
	Timing::StaticEvent inEv1(-42.0, {Timing::Variable(uPortID, (fmiReal) 2.0)});
	pred.eventTriggered(&inEv1);

	// Still the first prediction
	ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 1.0, 1e-3);

	vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 0);
	delete ev;

	// Apply the second event
	Timing::StaticEvent inEv2(-41.0, {Timing::Variable(uPortID, (fmiReal) 1.0)});
	pred.eventTriggered(&inEv2);

	// Again, the first prediction
	ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 1.0, 1e-3);

	vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 0);

	pred.eventTriggered(ev); // Schedule predicted event
	delete ev;

	// Hooray, a new prediction.
	ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 2.0, 1e-3);

	vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 1);

	BOOST_CHECK_CLOSE(vars[0].getRealValue(), 1.0, 1.0);

	pred.eventTriggered(ev); // Schedule predicted event
	delete ev;
}

/** TODO: Test applying integer, real, boolean and string outputs */
/** TODO: Test applying integer, real, boolean and string inputs */

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
		std::invalid_argument);
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
		std::invalid_argument);
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

/** @brief Test invalid output variable name (non-existing variable) */
BOOST_AUTO_TEST_CASE(testInvalidOutputVariableName)
{
	Base::ApplicationContext appContext;
	addModelBaseProperties(&appContext, "dxiskx");

	const char * argv[] = {"testOneStepEventPredictor", 
		"app.startTime=0.0", "app.lookAheadTime=1.0", "app.variableStepSize=0",
		"out.0.0=x", "out.0.0.type=0", "out.0.1=invalid", "out.0.1.type=0",
		"in.0.0=u", "in.0.0.type=0",
		"in.default.u=0.0", "in.default.k=1.0", "in.default.x0=0.0"
	};
	appContext.addCommandlineProperties(sizeof(argv)/sizeof(argv[0]), argv);
	
	OneStepEventPredictor pred(appContext);
	BOOST_CHECK_THROW(pred.init(), Base::SystemConfigurationException);
}

/** TODO: Test an invalid model type (co-simulation) */
