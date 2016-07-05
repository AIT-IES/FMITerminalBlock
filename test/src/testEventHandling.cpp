/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testEventHandling.cpp
 * @details Tests the EventDispatcher and its helper classes
 * @details The EventDispatcher is tested using a fixture which contains the 
 * application context and an event predictor necessary to instantiate the
 * EventDispatcher.
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#define BOOST_TEST_MODULE testEventHandling
#include <boost/test/unit_test.hpp>

#include "base/ApplicationContext.h"
#include "model/AbstractEventPredictor.h"
#include "timing/EventDispatcher.h"
#include "timing/StaticEvent.h"
#include "timing/EventListener.h"

#include <stack>

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Timing;

/**
 * @brief Dummy event predictor issuing events in fixed intervals
 */
class SimpleTestEventPredictor: public Model::AbstractEventPredictor
{
public:

	/** @brief C'tor initializing the object */
	SimpleTestEventPredictor(fmiTime eventDistance = 0.2): 
		eventDistance_(eventDistance), currentTime_(0.0)
	{
	}

	/** @brief Does nothing */
	virtual void init(void) {}

	/** @brief Returns the next event */
	virtual Timing::Event * predictNext(void)
	{
		std::vector<Timing::Event::Variable> vars;
		return new Timing::StaticEvent(currentTime_ + eventDistance_, vars);
	}

	/** @brief Increases the object's time */
	virtual void eventTriggered(Event * ev)
	{
		BOOST_REQUIRE(ev != NULL);
		currentTime_ = ev->getTime();
	}

private:
	/** @brief The distance between two consecutive events */
	fmiTime eventDistance_;
	/** @brief The current time instant */
	fmiTime currentTime_;
};

/** @brief provides the test environment and checks each incoming event */
struct EventDispatcherFixture: Timing::EventListener
{

	/** @brief A minimal application configuration */
	Base::ApplicationContext appContext;
	/** @brief The expected time instants of the next events in reverse order*/
	std::stack<fmiTime> expectedTime;

	/** @brief Initializes the fixture */
	EventDispatcherFixture(): appContext(), expectedTime()
	{

	}

	/** 
	 * @brief Checks the current event's time
	 * @details The function will remove an element from the stack of expected 
	 * time instants.
	 */
	virtual void eventTriggered(Event * ev)
	{
		BOOST_REQUIRE(ev != NULL);
		BOOST_REQUIRE(!expectedTime.empty());
		BOOST_CHECK_CLOSE(ev->getTime(),expectedTime.top(), 0.0001);
		expectedTime.pop();
	}
};

/** @brief tests the event dispatcher's default initialization */
BOOST_FIXTURE_TEST_CASE(test_dispatcher_default_init, 
												EventDispatcherFixture)
{
	// Prepare environment
	SimpleTestEventPredictor pred(0.2);
	// Test event dispatcher's default initialization. The appContext is empty.
	EventDispatcher dispatcher(appContext, pred);
	dispatcher.addEventListener(this);
}

/** @brief tests a prediction run using predicted results only */
BOOST_FIXTURE_TEST_CASE(test_prediction_only, 
												EventDispatcherFixture)
{
	// Prepare environment
	const char * argv[] = {"testEventHandling", "app.stopTime=0.6", NULL};
	appContext.addCommandlineProperties(2, argv);

	expectedTime.push(0.6);
	expectedTime.push(0.4);
	expectedTime.push(0.2);

	SimpleTestEventPredictor pred(0.2);

	// Test event dispatcher
	EventDispatcher dispatcher(appContext, pred);
	dispatcher.addEventListener(this);

	dispatcher.run();

	BOOST_CHECK(expectedTime.empty());

}
