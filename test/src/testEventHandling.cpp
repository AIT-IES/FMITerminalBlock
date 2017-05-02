/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testEventHandling.cpp
 * @details Tests the EventDispatcher and its helper classes
 * @details The EventDispatcher is tested using a fixture which contains the 
 * application context and an event predictor necessary to instantiate the
 * EventDispatcher.
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#define BOOST_TEST_MODULE testEventHandling
#include <boost/test/unit_test.hpp>

#include "base/ApplicationContext.h"
#include "model/AbstractEventPredictor.h"
#include "timing/EventDispatcher.h"
#include "timing/StaticEvent.h"
#include "timing/EventListener.h"

#include <boost/log/trivial.hpp>

#include <stack>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <list>
#include <thread>

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

/**
 * @brief Listens for incoming events and triggers intermediate events.
 * @details The intermediate events are synchronized to the incoming ones. For 
 * each event, a delay to the last checkpoint may be given. If the delay is 
 * zero, the response will be triggered synchronously. Otherwise, a separate 
 * thread will be used to delay the execution.
 */
class SynchronizedEventSource : public EventListener
{
public:
	/**@brief  A magic variable which indicates an internal event */
	const Event::Variable MAGIC_VAR;

private:

	typedef struct {
		fmiTime triggerTime_; ///< The time of the corresponding predicted event
		std::chrono::milliseconds actionDelay_; ///< Delay from the last action
		fmiTime eventTime_; ///< The time of the triggered event
	} ActionRecord;

	/** @brief The event sink to put generated events */
	std::shared_ptr<EventSink> sink_;

	/** @brief Chronological list of actions (the trigger time and a delay). */
	std::list<ActionRecord> actionList_;

	/** @brief The time of the last external event */
	fmiTime lastExternalEvent_ = 0.0;

	/** @brief Guards the action structure */
	std::mutex actionMutex_;

	/** @brief Signals that a new external event was triggered */
	std::condition_variable externalEventTriggered_;

	/** @brief The thread which manages sending new events. */
	std::thread eventSender_;

	/** 
	 * @brief Waits and triggers internal events until all events for the last 
	 * external One are processed. 
	 * @details The function assumes that the mutex guarding the action structure 
	 * is already locked. The function won't try to lock the mutex again.
	 */
	void generateEvents()
	{
		BOOST_REQUIRE(!actionMutex_.try_lock());
		std::vector<Event::Variable> vars;
		vars.push_back(MAGIC_VAR);

		while (!actionList_.empty() &&
			actionList_.front().triggerTime_ <= lastExternalEvent_)
		{
			auto nextAction = actionList_.front();
			actionList_.pop_front();

			StaticEvent *ev = new StaticEvent(nextAction.eventTime_, vars);

			std::this_thread::sleep_for(nextAction.actionDelay_);

			sink_->pushExternalEvent(ev);
		}
	}

	/**
	 * @brief Checks whether events need to be issued and sleeps otherwise.
	 * @details The condition variable needs to be used to unblock the thread 
	 * in case a new external event has arrived. The function is made static to 
	 * easily start it in a new thread.
	 * @param th A valid pointer to "this" event source.
	 */
	static void run(SynchronizedEventSource *th)
	{
		BOOST_REQUIRE(th != NULL);
		std::unique_lock<std::mutex> lock(th->actionMutex_);

		while (!th->actionList_.empty())
		{
			th->externalEventTriggered_.wait(lock);
			th->generateEvents();
		}
	}

public:
	/** 
	 * @brief Creates an empty event source
	 * @param sink The sink to register triggered elements.
	 */
	SynchronizedEventSource(std::shared_ptr<EventSink> sink): sink_(sink), 
		MAGIC_VAR(Base::PortID(fmiTypeString, 42024), std::string("internal ev"))
	{
		BOOST_REQUIRE(sink);
	}

	/**
	 * @brief Starts the operation of the event source.
	 * @brief The function must be called after all actions are registered.
	 */
	void start()
	{
		eventSender_ = std::thread(run, this);
	}

	/**
	 * @brief Adds a new action.
	 * @details It is assumed that earlier actions are added first. In
	 * consecutive calls, the trigger condition may be equal. I.e. it is
	 * possible to specify more than one event for a single trigger condition.
	 * @params triggerCondition The time of the received event which triggers
	 * the next action.
	 * @param actionDelay The time delay of the triggered event from the
	 * previous action.
	 * @param eventTime the time of the triggered event
	 */
	void addAction(fmiTime triggerCondition, 
		std::chrono::milliseconds actionDelay, fmiTime eventTime)
	{
		std::unique_lock<std::mutex> lock(actionMutex_);
		BOOST_CHECK(actionList_.empty() || 
			actionList_.back().triggerTime_ <= triggerCondition);
		ActionRecord newAction = {triggerCondition,	actionDelay, eventTime };
		actionList_.push_back(newAction);
	}

	/**
	 * @brief Adds a new action.
	 * @details It is assumed that earlier actions are added first. In 
	 * consecutive calls, the trigger condition may be equal. I.e. it is 
	 * possible to specify more than one event for a single trigger condition. 
	 * The triggered event will be have a timestamp of triggerCondition + 
	 * actionDelay
	 * @params triggerCondition The time of the received event which triggers 
	 * the next action.
	 * @param actionDelay The time delay of the triggered event from the 
	 * previous action.
	 */
	void addAction(fmiTime triggerCondition, std::chrono::milliseconds actionDelay)
	{
		addAction(triggerCondition, actionDelay, 
			triggerCondition + ((fmiTime)actionDelay.count()) / 1000.0);
	}

	/** @brief Initiates sending the next events. */
	virtual void eventTriggered(Event * ev) 
	{
		BOOST_REQUIRE(ev != NULL);

		// Ignore internal events
		if (ev->getVariables().size() != 1 || 
			ev->getVariables()[0].first != MAGIC_VAR.first)
		{
			std::unique_lock<std::mutex> lock(actionMutex_);
			lastExternalEvent_ = ev->getTime();
			externalEventTriggered_.notify_one();
		}
	}

	/** 
	 * @brief Waits until the event source has been terminated.
	 * @details The function assumes that start() was called before.
	 */
	void waitForTermination() {
		BOOST_REQUIRE(eventSender_.joinable());
		eventSender_.join();
	}

};

/** @brief provides the test environment and checks each incoming event */
struct EventDispatcherFixture: Timing::EventListener
{

	/** @brief A minimal application configuration */
	Base::ApplicationContext appContext;
	/** 
	 * @brief The expected time instants of the next events in order of their 
	 * occurrence 
	 */
	std::list<fmiTime> expectedTime;

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
		if (expectedTime.empty())
		{
			BOOST_LOG_TRIVIAL(error) << "Received an unexpected event: " << 
				ev->toString();
			BOOST_CHECK(false);
		}
		else 
		{
			BOOST_LOG_TRIVIAL(debug) << "Received an expected event: " <<
				ev->toString();
			BOOST_CHECK_CLOSE(ev->getTime(), expectedTime.front(), 0.0001);
			expectedTime.pop_front();
		}
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

	expectedTime.push_back(0.2);
	expectedTime.push_back(0.4);
	expectedTime.push_back(0.6);

	SimpleTestEventPredictor pred(0.2);

	// Test event dispatcher
	EventDispatcher dispatcher(appContext, pred);
	dispatcher.addEventListener(this);

	dispatcher.run();

	BOOST_CHECK(expectedTime.empty());

}

/** @brief Tests multiple external events between two predicted ones */
BOOST_FIXTURE_TEST_CASE(test_multiple_external_events, EventDispatcherFixture)
{
	// Prepare environment
	const char * argv[] = { "testEventHandling", "app.stopTime=1.6", NULL };
	appContext.addCommandlineProperties(2, argv);

	// Generate the objects under test
	SimpleTestEventPredictor pred(0.6);
	EventDispatcher dispatcher(appContext, pred);
	SynchronizedEventSource eventSource(dispatcher.getEventSink());

	dispatcher.addEventListener(this);
	dispatcher.addEventListener(eventSource);

	// Add generated events
	expectedTime.push_back(0.6); // Predicted
	expectedTime.push_back(0.8); // External
	expectedTime.push_back(1.0); // External
	expectedTime.push_back(1.6); // Predicted

	eventSource.addAction(0.6, std::chrono::milliseconds(200), 0.8);
	eventSource.addAction(0.6, std::chrono::milliseconds(200), 1.0);

	// Perform the test
	BOOST_TEST_CHECKPOINT("Start test procedure");
	eventSource.start();
	dispatcher.run();

	// Check termination
	eventSource.waitForTermination();
	BOOST_CHECK(expectedTime.empty());
}

/** 
 * @brief Triggers a event which dates back before the a predicted one which
 * was already taken 
 */
BOOST_FIXTURE_TEST_CASE(test_late_external_events, EventDispatcherFixture)
{
	// Prepare environment
	const char * argv[] = { "testEventHandling", "app.stopTime=1.0", NULL };
	appContext.addCommandlineProperties(2, argv);

	// Generate the objects under test
	SimpleTestEventPredictor pred(0.6);
	EventDispatcher dispatcher(appContext, pred);
	SynchronizedEventSource eventSource(dispatcher.getEventSink());

	dispatcher.addEventListener(this);
	dispatcher.addEventListener(eventSource);

	// Add generated events
	expectedTime.push_back(0.6); // Predicted
	expectedTime.push_back(0.4); // External, late event
	expectedTime.push_back(1.0); // Predicted (Always 0.6 + last event)

	eventSource.addAction(0.6, std::chrono::milliseconds(100), 0.4); // Late event

	// Perform the test
	BOOST_TEST_CHECKPOINT("Start test procedure");
	eventSource.start();
	dispatcher.run();

	// Check termination
	eventSource.waitForTermination();
	BOOST_CHECK(expectedTime.empty());
}

/** 
 * @brief Triggers multiple predicted events before an external event is 
 * triggered. 
 */
BOOST_FIXTURE_TEST_CASE(test_multiple_predicted_events, EventDispatcherFixture)
{
	// Prepare environment
	const char * argv[] = { "testEventHandling", "app.stopTime=1.4", NULL };
	appContext.addCommandlineProperties(2, argv);

	// Generate the objects under test
	SimpleTestEventPredictor pred(0.4);
	EventDispatcher dispatcher(appContext, pred);
	SynchronizedEventSource eventSource(dispatcher.getEventSink());

	dispatcher.addEventListener(this);
	dispatcher.addEventListener(eventSource);

	// Add generated events
	expectedTime.push_back(0.4); // Predicted
	expectedTime.push_back(0.8); // Predicted
	expectedTime.push_back(1.0); // External
	expectedTime.push_back(1.4); // Predicted

	eventSource.addAction(0.8, std::chrono::milliseconds(200), 1.0);

	// Perform the test
	BOOST_TEST_CHECKPOINT("Start test procedure");
	eventSource.start();
	dispatcher.run();

	// Check termination
	eventSource.waitForTermination();
	BOOST_CHECK(expectedTime.empty());
}

/** 
 * @brief Triggers an external event with the same time as the last predicted 
 * one after the predicted event is triggered 
 */
BOOST_FIXTURE_TEST_CASE(test_concurrent_predicted_taken, EventDispatcherFixture)
{
	// Prepare environment
	const char * argv[] = { "testEventHandling", "app.stopTime=1.6", NULL };
	appContext.addCommandlineProperties(2, argv);

	// Generate the objects under test
	SimpleTestEventPredictor pred(0.4);
	EventDispatcher dispatcher(appContext, pred);
	SynchronizedEventSource eventSource(dispatcher.getEventSink());

	dispatcher.addEventListener(this);
	dispatcher.addEventListener(eventSource);

	// Add generated events
	expectedTime.push_back(0.4); // Predicted
	expectedTime.push_back(0.4); // External
	expectedTime.push_back(0.8); // Predicted
	expectedTime.push_back(0.8); // External
	expectedTime.push_back(1.2); // Predicted
	expectedTime.push_back(1.6); // Predicted

	eventSource.addAction(0.4, std::chrono::milliseconds(0), 0.4);
	eventSource.addAction(0.8, std::chrono::milliseconds(100), 0.8);

	// Perform the test
	BOOST_TEST_CHECKPOINT("Start test procedure");
	eventSource.start();
	dispatcher.run();

	// Check termination
	eventSource.waitForTermination();
	BOOST_CHECK(expectedTime.empty());
}

/** TODO: Triggers an external event with the same time as the last predicted one before the predicted event is triggered */
/** 
 * TODO: Triggers an external event which has a bigger timestamp than the 
 * predicted one in the queue
 * @details The external event is added after the first predicted event is 
 * added but no delay is used in adding the event. Hence, the predicted event 
 * is not deleted and returned regularly.
 */
 /** TODO: Triggers a predicted event and checks whether the returned system time is close to the event time. */