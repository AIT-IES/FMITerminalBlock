/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testEventPredictor.cpp
 * @brief Tests the event prediction mechanism.
 * @details The test collection uses a small fixture which contains an 
 * artificial application context
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#define BOOST_TEST_MODULE testEventPredictor
#include <boost/test/unit_test.hpp>

#include "model/EventPredictor.h"
#include "base/ApplicationContext.h"
#include "base/BaseExceptions.h"
#include "timing/StaticEvent.h"

#include <assert.h>
#include <boost/any.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <stdexcept>

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Model;

/**
 * @brief Defines the environment used during most of the EventPredictor test 
 * cases
 * @details The fixture includes a minimal application context configuration 
 * but doesn't create the EventPredictor object itself. It configures the 
 * zigzag test.
 */
struct EventPredictorZigzagFixture
{

	/** @brief The minimal application context */
	Base::ApplicationContext appContext;

	/** @brief Initializes the fixture's environment */
	EventPredictorZigzagFixture(): appContext()
	{
		const char * argv[] = {"testEventPredictor",
			"fmu.path=" FMU_URI_PRE "zigzag",
			"fmu.instanceName=zigzag", "fmu.name=zigzag", 
			"channel.0.out-var.0=x", "channel.0.out-var.0.type=0", NULL};
		appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);
	}
};

/**
* @brief Defines the environment used during most of the EventPredictor test
* cases
* @details The fixture includes a minimal application context configuration
* but doesn't create the EventPredictor object itself. It configures the
* dxiskx test.
*/
struct EventPredictorDxIsKxFixture
{

	/** @brief The minimal application context */
	Base::ApplicationContext appContext;

	/** @brief Initializes the fixture's environment */
	EventPredictorDxIsKxFixture() : appContext()
	{
		const char * argv[] = { "testEventPredictor",
			"fmu.path=" FMU_URI_PRE "dxiskx",
			"fmu.instanceName=dxiskx", "fmu.name=dxiskx", 
			"channel.0.out-var.0=x", "channel.0.out-var.0.type=0", 
			"channel.0.in-var.0=u", "channel.0.in-var.0.type=0", NULL };
		appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);
	}

};

// ============================================================================
// Negative Test Cases
// ============================================================================

/** @brief Tests the initialization's error handling */
BOOST_FIXTURE_TEST_CASE(test_init_missing_lookahead_time, EventPredictorZigzagFixture)
{
	const char * argv[] = { "testEventPredictor", 
		"app.lookAheadStepSize=0.11", "app.integratorStepSize=0.11", 
		"app.startTime=0.0", NULL };
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	EventPredictor pred(appContext);
	BOOST_CHECK_THROW(pred.init(), std::invalid_argument);
}

/** @brief Tests the initialization's error handling */
BOOST_FIXTURE_TEST_CASE(test_init_missing_start_time, EventPredictorZigzagFixture)
{
	const char * argv[] = { "testEventPredictor", "app.lookAheadTime=1.1",
		"app.lookAheadStepSize=0.11", "app.integratorStepSize=0.11", NULL };
	appContext.addCommandlineProperties(4, argv);
	EventPredictor pred(appContext);
	BOOST_CHECK_THROW(pred.init(), std::invalid_argument);
}

/** @brief Tests the initialization's error handling */
BOOST_AUTO_TEST_CASE(test_init_missing_path)
{
	Base::ApplicationContext appContext;
	const char * argv[] = {"testEventPredictor", 
		"fmu.name=zigzag", 
		"channel.0.out-var.0=x", "channel.0.out-var.0.type=0",
		"app.lookAheadTime=1.1", "app.startTime=0.0", 
		"app.lookAheadStepSize=0.11", "app.integratorStepSize=0.11", NULL };
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	BOOST_CHECK_THROW(EventPredictor pred(appContext), std::invalid_argument);
}

/** @brief Tests the initialization's error handling */
BOOST_AUTO_TEST_CASE(test_init_invalid_name)
{
	Base::ApplicationContext appContext;
	const char * argv[] = {"testEventPredictor", 
		"fmu.path=" FMU_URI_PRE "zigzag", "fmu.name=line", // Wrong name
		"channel.0.out-var.0=x", "channel.0.out-var.0.type=0",
		"app.lookAheadTime=1.1", "app.startTime=0.0", 
		"app.lookAheadStepSize=0.11", "app.integratorStepSize=0.11", NULL };
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	BOOST_CHECK_THROW(EventPredictor pred(appContext), std::invalid_argument);
}

BOOST_FIXTURE_TEST_CASE(test_init_invalid_integrator_step_size,
												EventPredictorZigzagFixture)
{
	// Set horizon parameter
	const char * argv[] = { "testEventPredictor", "app.lookAheadTime=1.1", 
		"app.lookAheadStepSize=0.11", "app.integratorStepSize=0.12", 
		"app.startTime=0.0", NULL };
	appContext.addCommandlineProperties(5, argv);

	// Create EventPredictor
	EventPredictor pred(appContext);
	BOOST_REQUIRE_THROW(pred.init(), Base::SystemConfigurationException);
}

BOOST_FIXTURE_TEST_CASE(test_init_invalid_look_ahead_step_size,
												EventPredictorZigzagFixture)
{
	// Set horizon parameter
	const char * argv[] = { "testEventPredictor", "app.lookAheadTime=1.1", 
		"app.lookAheadStepSize=1.2", "app.integratorStepSize=0.011", 
		"app.startTime=0.0", NULL };
	appContext.addCommandlineProperties(5, argv);

	// Create EventPredictor
	EventPredictor pred(appContext);
	BOOST_REQUIRE_THROW(pred.init(), Base::SystemConfigurationException);
}

// ============================================================================
// Positive Test Cases
// ============================================================================

/** @brief Tests the initialization's default value generation */
BOOST_AUTO_TEST_CASE(test_init_defaults_0)
{
	Base::ApplicationContext appContext;
	const char * argv[] = {"testEventPredictor",
			"fmu.path=" FMU_URI_PRE "zigzag", "fmu.name=zigzag", 
			"app.startTime=0.0", "app.lookAheadTime=1.1",
			"channel.0.out-var.0=x", "channel.0.out-var.0.type=0", NULL};
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	EventPredictor pred(appContext);
	pred.init();
}

/** @brief Tests the initialization's default value generation */
BOOST_AUTO_TEST_CASE(test_init_defaults_1)
{
	Base::ApplicationContext appContext;
	const char * argv[] = {"testEventPredictor",
			"fmu.path=" FMU_URI_PRE "zigzag", // No fmu.name parameter
			"app.startTime=0.0", "app.lookAheadTime=1.1",
			"channel.0.out-var.0=x", "channel.0.out-var.0.type=0", NULL};
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);
	
	EventPredictor pred(appContext);
	pred.init();
}

/** @brief Tests the initialization's default value generation for ME 2.0 */
BOOST_AUTO_TEST_CASE(test_init_defaults_2_0)
{
	Base::ApplicationContext appContext;
	const char * argv[] = {"testEventPredictor",
			"fmu.path=" FMU_URI_PRE "zigzag2", "fmu.name=zigzag2", 
			"app.startTime=0.0", "app.lookAheadTime=1.1",
			"channel.0.out-var.0=x", "channel.0.out-var.0.type=0", NULL};
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	EventPredictor pred(appContext);
	pred.init();
}

/** @brief Tests the configureDefaultApplicationContext(...) function */
BOOST_AUTO_TEST_CASE(test_configureDefaultApplicationContext)
{
	Base::ApplicationContext appContext;
	const char * argv[] = {"testEventPredictor",
			"fmu.path=" FMU_URI_PRE "zerocrossing", "fmu.name=zerocrossing", 
			"app.lookAheadTime=1.1", 
			"channel.0.out-var.0=x", "channel.0.out-var.0.type=0", NULL};
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	EventPredictor pred(appContext);
	pred.configureDefaultApplicationContext(&appContext);

	BOOST_CHECK_EQUAL(appContext.getProperty<fmiReal>("app.startTime"), 0.0);
}

/** @brief Tests the event detection and fmiReal-typed outputs*/
BOOST_FIXTURE_TEST_CASE(test_fmireal_events, EventPredictorZigzagFixture)
{
	// Set horizon parameter
	const char * argv[] = { "testEventPredictor", "app.lookAheadTime=1.1", 
		"app.lookAheadStepSize=0.11", "app.integratorStepSize=0.11", 
		"app.startTime=0.0", NULL };
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	// Create EventPredictor
	EventPredictor pred(appContext);

	pred.init();

	// Predict FMU event
	Timing::Event * ev = pred.predictNext();

	BOOST_REQUIRE(ev != NULL);
	BOOST_CHECK_CLOSE(ev->getTime(), 1.0, 1.0);

	// Use result: Afterwards no reset is possible
	std::vector<Timing::Variable> vars = ev->getVariables();
	BOOST_CHECK_EQUAL(vars.size(), 1);
	BOOST_CHECK_EQUAL(vars[0].getID().first, fmiTypeReal);
	BOOST_CHECK_EQUAL(vars[0].getID().second, 0);
	BOOST_CHECK_CLOSE(boost::any_cast<fmiReal>(vars[0].getValue()), 1.0, 1.0);
	delete ev;

	// Predict end of horizon event
	ev = pred.predictNext();

	BOOST_REQUIRE(ev != NULL);
	BOOST_CHECK_CLOSE(ev->getTime(), 2.1, 1.0);

	// Use result: Afterwards no reset is possible
	vars = ev->getVariables();
	BOOST_CHECK_EQUAL(vars.size(), 1);
	BOOST_CHECK_EQUAL(vars[0].getID().first, fmiTypeReal);
	BOOST_CHECK_EQUAL(vars[0].getID().second, 0);
	BOOST_CHECK_CLOSE(boost::any_cast<fmiReal>(vars[0].getValue()), -0.1, 1.0);
	delete ev;

	// Predict FMU event
	ev = pred.predictNext();

	BOOST_REQUIRE(ev != NULL);
	BOOST_CHECK_CLOSE(ev->getTime(), 3.0, 1.0);

	// Use result: Afterwards no reset is possible
	vars = ev->getVariables();
	BOOST_CHECK_EQUAL(vars.size(), 1);
	BOOST_CHECK_EQUAL(vars[0].getID().first, fmiTypeReal);
	BOOST_CHECK_EQUAL(vars[0].getID().second, 0);
	BOOST_CHECK_CLOSE(boost::any_cast<fmiReal>(vars[0].getValue()), -1.0, 1.0);
	delete ev;

}

/** 
 * @brief Tests multiple output events without an input event occurrence. 
 * @details Just the default values are used to initially define the inputs.
 */
BOOST_FIXTURE_TEST_CASE(test_pure_output_events, EventPredictorDxIsKxFixture)
{
	// Set horizon parameter
	const char * argv[] = { "testEventPredictor", "app.lookAheadTime=1",
		"app.lookAheadStepSize=0.1", "app.integratorStepSize=0.1",
		"app.startTime=0.0", "in-var.default.u=1", NULL };
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	// Create EventPredictor
	EventPredictor pred(appContext);

	pred.init();

	// Predict FMU event
	Timing::Event * ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 1.0, 0.001);
	
	// Use result: Afterwards no reset is possible
	std::vector<Timing::Variable> vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 1);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal);
	BOOST_CHECK_CLOSE(boost::any_cast<fmiReal>(vars[0].getValue()), 2.0, 0.1);
	pred.eventTriggered(ev);
	delete ev;

	// Predict FMU event
	ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 2.0, 0.001);

	// Use result: Afterwards no reset is possible
	vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 1);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal);
	BOOST_CHECK_CLOSE(boost::any_cast<fmiReal>(vars[0].getValue()), 3.0, 0.1);
	pred.eventTriggered(ev);
	delete ev;
}

/** 
 * @brief Triggers multiple input events before an output event is generated
 */
BOOST_FIXTURE_TEST_CASE(test_multiple_input_events, EventPredictorDxIsKxFixture)
{
	// Set horizon parameter
	const char * argv[] = { "testEventPredictor", "app.lookAheadTime=1",
		"app.lookAheadStepSize=0.01", "app.integratorStepSize=0.01",
		"app.startTime=0.0", "in-var.default.u=1", NULL };
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	// Create EventPredictor
	EventPredictor pred(appContext);

	pred.init();

	// Predict FMU event
	Timing::Event * ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 1.0, 0.001);
	delete ev;

	// Issue an event at 0.3 (x=1.3)
	std::vector<Timing::Variable> inVar;
	inVar.push_back(Timing::Variable(
		appContext.getInputChannelMapping()->getVariableIDs(fmiTypeReal)[0],
		-1.0));
	Timing::StaticEvent ev1(0.3, inVar);
	pred.eventTriggered(&ev1);

	// Predict FMU event
	ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 1.3, 0.001);
	delete ev;

	// Issue an event at 0.8 (x=0.8)
	inVar[0].setValue(1.0);
	Timing::StaticEvent ev2(0.8, inVar);
	pred.eventTriggered(&ev2);

	// Predict FMU event
	ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 1.8, 0.001);

	// Use result: Afterwards no reset is possible
	std::vector<Timing::Variable> vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 1);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal);
	BOOST_CHECK_CLOSE(boost::any_cast<fmiReal>(vars[0].getValue()), 1.8, 0.1);
	pred.eventTriggered(ev);
	delete ev;
}

/** 
 * @brief Alternately triggers input and output events 
 * @details Output events are fed back to the event predictor in order to test 
 * the filtering capabilities.
 */
BOOST_FIXTURE_TEST_CASE(test_alternating_input_events, EventPredictorDxIsKxFixture)
{
	// Set horizon parameter
	const char * argv[] = { "testEventPredictor", "app.lookAheadTime=1",
		"app.lookAheadStepSize=0.01", "app.integratorStepSize=0.01",
		"app.startTime=0.0", NULL };
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	// Create EventPredictor
	EventPredictor pred(appContext);

	pred.init();

	// Predict FMU event
	Timing::Event * ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 1.0, 0.001);
	delete ev;

	// Issue an event at 0.0 (x=0.0)
	std::vector<Timing::Variable> inVar;
	inVar.push_back(Timing::Variable(
		appContext.getInputChannelMapping()->getVariableIDs(fmiTypeReal)[0],
		-1.0));
	Timing::StaticEvent ev1(0.0, inVar);
	pred.eventTriggered(&ev1); // State at the beginning doesn't seem to be taken.

	// Predict  and take FMU event
	ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 1.0, 0.001);
	std::vector<Timing::Variable> vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 1);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal);
	// "+1.0" is used to reduce numerical errors
	BOOST_CHECK_CLOSE(boost::any_cast<fmiReal>(vars[0].getValue()) + 1.0, 0.0 + 1.0, 0.1);
	pred.eventTriggered(ev);
	delete ev;

	// Predict FMU event
	ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 2.0, 0.001);
	delete ev;

	// Issue an event at 1.5 (x=-0.5)
	inVar[0].setValue(1.0);
	Timing::StaticEvent ev2(1.5, inVar);
	pred.eventTriggered(&ev2);

	// Predict FMU event
	ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 2.5, 0.001);

	// Use result: Afterwards no reset is possible
	vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 1);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal);
	BOOST_CHECK_CLOSE(vars[0].getRealValue(), 0.5, 0.1);
	pred.eventTriggered(ev);
	delete ev;
}

/** 
 * @brief Tests the direct feedback mechanism by applying some input events. 
 * @details Output events are fed back to the event predictor in order to test 
 * the filtering capabilities.
 */
BOOST_FIXTURE_TEST_CASE(test_direct_feedback_events, 
	EventPredictorDxIsKxFixture)
{
	// Set horizon parameter
	const char * argv[] = { "testEventPredictor", "app.lookAheadTime=1",
		"app.lookAheadStepSize=0.01", "app.integratorStepSize=0.01",
		"app.startTime=0.0", "app.directOutputDependency=1", 
		"channel.0.out-var.1=der(x)", "channel.0.out-var.1.type=0", NULL };
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	// Create EventPredictor
	EventPredictor pred(appContext);
	
	// Auxiliary variables to check the results
	Timing::Event * ev;
	std::vector<Timing::Variable> vars;

	pred.init();

	// Predict FMU event (der(x) = k*u, u=0, k=1, x0=1) -> t=1.0
	// Don't take it, will be overwritten.
	ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 1.0, 0.001);
	delete ev;

	// Issue an event at 0.0 (x=1.0) -> Set u = -1
	std::vector<Timing::Variable> inVar;
	inVar.push_back(Timing::Variable(
		appContext.getInputChannelMapping()->getVariableIDs(fmiTypeReal)[0],
		-1.0));
	Timing::StaticEvent ev1(0.0, inVar);
	pred.eventTriggered(&ev1); // State at the beginning doesn't seem to be taken.

	// Get direct feedback event
	ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 0.0, 0.001);
	vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 2);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal); // x = 1.0
	BOOST_REQUIRE_EQUAL(vars[1].getID().first, fmiTypeReal); // der(x) = -1.0
	BOOST_CHECK_SMALL(boost::any_cast<fmiReal>(vars[0].getValue()) - 1.0, 0.001);
	BOOST_CHECK_SMALL(boost::any_cast<fmiReal>(vars[1].getValue()) + 1.0, 0.001);
	pred.eventTriggered(ev);
	delete ev;

	// Predict and take next FMU event, t=1.0, der(x) = -1.0, x=0.0
	ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 1.0, 0.001);
	vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 2);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal); // x = 0.0
	BOOST_REQUIRE_EQUAL(vars[1].getID().first, fmiTypeReal); // der(x) = -1.0
	BOOST_CHECK_SMALL(boost::any_cast<fmiReal>(vars[0].getValue()) - 0.0, 0.001);
	BOOST_CHECK_SMALL(boost::any_cast<fmiReal>(vars[1].getValue()) + 1.0, 0.001);
	delete ev;

	// Predict FMU event, don't take it
	ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 2.0, 0.001);
	delete ev;

	// Issue an event at t=1.5 (x=-0.5) -> Set u=1.0
	inVar[0].setValue(1.0); // u
	Timing::StaticEvent ev2(1.5, inVar);
	pred.eventTriggered(&ev2);

	// Get direct feedback event
	ev = pred.predictNext();
	BOOST_REQUIRE(ev);
	BOOST_CHECK_SMALL(ev->getTime() - 1.5, 0.001);
	vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 2);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal); // x = -0.5
	BOOST_REQUIRE_EQUAL(vars[1].getID().first, fmiTypeReal); // der(x) = 1.0
	BOOST_CHECK_SMALL(boost::any_cast<fmiReal>(vars[0].getValue()) + 0.5, 0.001);
	BOOST_CHECK_SMALL(boost::any_cast<fmiReal>(vars[1].getValue()) - 1.0, 0.001);
	pred.eventTriggered(ev);
	delete ev;

	// Predict FMU event at t=2.5, x=0.5
	ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 2.5, 0.001);
	vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 2);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal); // x = 0.5
	BOOST_REQUIRE_EQUAL(vars[1].getID().first, fmiTypeReal); // der(x) = 1.0
	BOOST_CHECK_SMALL(boost::any_cast<fmiReal>(vars[0].getValue()) - 0.5, 0.001);
	BOOST_CHECK_SMALL(boost::any_cast<fmiReal>(vars[1].getValue()) - 1.0, 0.001);
	pred.eventTriggered(ev);
	delete ev;
}

/**
 * @brief Triggers an event which is timed before a predicted one which was 
 * previously taken
 */
BOOST_FIXTURE_TEST_CASE(test_causality_violation, EventPredictorDxIsKxFixture)
{
	// Set horizon parameter
	const char * argv[] = { "testEventPredictor", "app.lookAheadTime=1",
		"app.lookAheadStepSize=0.01", "app.integratorStepSize=0.01",
		"app.startTime=0.0", "in-var.default.u=1", NULL };
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	// Create EventPredictor
	EventPredictor pred(appContext);

	pred.init();

	// Predict and take FMU event
	Timing::Event * ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 1.0, 0.001);
	std::vector<Timing::Variable> vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 1);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal);
	BOOST_CHECK_CLOSE(vars[0].getRealValue(), 2.0, 0.1);
	pred.eventTriggered(ev);
	delete ev;

	// Issue an event at 0.8. The event must be re-timed to 1.0
	std::vector<Timing::Variable> inVar;
	inVar.push_back(Timing::Variable(
		appContext.getInputChannelMapping()->getVariableIDs(fmiTypeReal)[0],
		-1.0));
	Timing::StaticEvent ev1(0.8, inVar);
	pred.eventTriggered(&ev1);

	// Predict FMU event
	ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 2.0, 0.001);
	vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 1);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal);
	BOOST_CHECK_CLOSE(vars[0].getRealValue(), 1.0, 0.1);
	pred.eventTriggered(ev);
	delete ev;
}

/**
 * @brief Triggers an event which has the same timing as a predicted one
 */
BOOST_FIXTURE_TEST_CASE(test_concurrent_in_out_event, EventPredictorDxIsKxFixture)
{
	// Set horizon parameter
	const char * argv[] = { "testEventPredictor", "app.lookAheadTime=1",
		"app.lookAheadStepSize=0.01", "app.integratorStepSize=0.01",
		"app.startTime=0.0", "in-var.default.u=1", NULL };
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	// Create EventPredictor
	EventPredictor pred(appContext);

	pred.init();

	// Predict and do not take FMU event
	Timing::Event * ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 1.0, 0.001);

	// Issue an event at 1.0.
	std::vector<Timing::Variable> inVar;
	inVar.push_back(Timing::Variable(
		appContext.getInputChannelMapping()->getVariableIDs(fmiTypeReal)[0],
		-1.0));
	Timing::StaticEvent ev1(ev->getTime(), inVar);
	delete ev;
	pred.eventTriggered(&ev1);

	// Predict FMU event
	ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 2.0, 0.001);
	std::vector<Timing::Variable> vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 1);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal);
	BOOST_CHECK_CLOSE(vars[0].getRealValue(), 1.0, 0.1);
	pred.eventTriggered(ev);
	delete ev;
}

/**
 * @brief Triggers an event which has the same timing as a predicted one which 
 * was previously taken.
 */
BOOST_FIXTURE_TEST_CASE(test_concurrent_in_out_event_taken, 
	EventPredictorDxIsKxFixture)
{
	// Set horizon parameter
	const char * argv[] = { "testEventPredictor", "app.lookAheadTime=1",
		"app.lookAheadStepSize=0.01", "app.integratorStepSize=0.01",
		"app.startTime=0.0", "in-var.default.u=1", NULL };
	appContext.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	// Create EventPredictor
	EventPredictor pred(appContext);

	pred.init();

	// Predict and take FMU event
	Timing::Event * ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 1.0, 0.001);
	std::vector<Timing::Variable> vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 1);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal);
	BOOST_CHECK_CLOSE(vars[0].getRealValue(), 2.0, 0.1);
	pred.eventTriggered(ev);

	// Issue an event at 1.0.
	std::vector<Timing::Variable> inVar;
	inVar.push_back(Timing::Variable(
		appContext.getInputChannelMapping()->getVariableIDs(fmiTypeReal)[0],
		-1.0));
	Timing::StaticEvent ev1(ev->getTime(), inVar);
	delete ev;
	pred.eventTriggered(&ev1);

	// Predict FMU event
	ev = pred.predictNext();
	BOOST_CHECK_CLOSE(ev->getTime(), 2.0, 0.001);
	vars = ev->getVariables();
	BOOST_REQUIRE_EQUAL(vars.size(), 1);
	BOOST_REQUIRE_EQUAL(vars[0].getID().first, fmiTypeReal);
	BOOST_CHECK_CLOSE(vars[0].getRealValue(), 1.0, 0.1);
	pred.eventTriggered(ev);
	delete ev;
}
