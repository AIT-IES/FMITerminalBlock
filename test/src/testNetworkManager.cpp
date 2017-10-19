/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testNetworkManager.cpp
 * @brief Contains the test-cases examining the NetworkManager
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#define BOOST_TEST_MODULE testNetworkManager
#include <boost/test/unit_test.hpp>

#include "base/BaseExceptions.h"
#include "network/NetworkManager.h"
#include "model/AbstractEventPredictor.h"
#include "ConcurrentMockupSubscriber.h"
#include "MockupPublisher.h"

#include <memory>
#include <thread>

#include <boost/log/trivial.hpp>

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Network;
using namespace FMITerminalBlock::Base;

using namespace FMITerminalBlockTest;
using namespace FMITerminalBlockTest::Network;

/** @brief Refuses to predict any event */
class NoEventPredictor: public Model::AbstractEventPredictor
{
public:
  NoEventPredictor() {}
	virtual void configureDefaultApplicationContext(
		Base::ApplicationContext *appContext) {}
  virtual void eventTriggered(Timing::Event * ev) {}
  virtual void init(void) {}
  virtual Timing::Event * predictNext(void)
  {
    BOOST_REQUIRE(false);
    return NULL;
  }
};

/**
 * @brief Provides a basic environment of NetworkManager tests.
 */
class BasicNetworkManagerFixture
{
private:
  NoEventPredictor predictor_;

public:
  Base::ApplicationContext appContext_;
  std::shared_ptr<Timing::EventDispatcher> dispatcher_;

  BasicNetworkManagerFixture()
  {
		const char * argv[] = {"testNetworkManager", 
			"channel.0.in-var.0=x", "channel.0.in-var.0.type=0", 
			"channel.0.out-var.0=x", "channel.0.out-var.0.type=0", NULL};
		appContext_.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

    dispatcher_ = std::make_shared<Timing::EventDispatcher>(appContext_, 
      predictor_);
  }
};

/** @brief Tests the instantiation of a NetworkManager */
BOOST_FIXTURE_TEST_CASE(testInstantiation, BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"channel.0.protocol=Mockup", NULL};
  appContext_.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	try {
		NetworkManager(appContext_, *(dispatcher_.get()));
	} catch (std::exception &ex) {
		BOOST_LOG_TRIVIAL(error) << "Cough exception: " << ex.what();
		BOOST_REQUIRE(false);
	} catch (...) {
		BOOST_REQUIRE(false);
	}
}

/** 
 * @brief Check whether run() and terminationRequest() are regularly called at 
 * the ConcurrentSubscriber 
 * @details The ConcurrentSubscriber will be instantiated via the 
 * NetworkManager in order to test a regular instantiation process
 */
BOOST_FIXTURE_TEST_CASE(testStandardExecution, BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"channel.0.protocol=Mockup", NULL};
  appContext_.addCommandlineProperties((sizeof(argv)/sizeof(argv[0]))-1, argv);

	ConcurrentMockupSubscriber::resetCounter();
	MockupPublisher::resetCounter();

	try {
		NetworkManager(appContext_, *(dispatcher_.get()));
	} catch (std::exception &ex) {
		BOOST_LOG_TRIVIAL(error) << "Cough exception: " << ex.what();
		BOOST_REQUIRE(false);
	} catch (...) {
		BOOST_REQUIRE(false);
	}

	BOOST_CHECK_EQUAL(ConcurrentMockupSubscriber::getInitAndStartSequenceID(), 
		0);
	BOOST_CHECK_EQUAL(ConcurrentMockupSubscriber::getInitSequenceID(), 1);

	// In the particular test case, run() does not depend on terminate() and 
	// terminationRequest(). Hence it may be executed in between any function.
	BOOST_CHECK_GE(ConcurrentMockupSubscriber::getRunSequenceID(), 2);
	BOOST_CHECK_GE(ConcurrentMockupSubscriber::getTerminateSequenceID(), 2);
	BOOST_CHECK_GE(
		ConcurrentMockupSubscriber::getTerminationRequestSequenceID(), 
		3);
	BOOST_CHECK_LT(
		ConcurrentMockupSubscriber::getTerminateSequenceID(),
		ConcurrentMockupSubscriber::getTerminationRequestSequenceID());

	BOOST_CHECK_EQUAL(MockupPublisher::getInitSequenceID(), 0);
	BOOST_CHECK_EQUAL(MockupPublisher::getEventTriggeredSequenceID(), -1);

}

/** @brief Applies a config with a missing protocol field */
BOOST_FIXTURE_TEST_CASE(testMissingProtocol, BasicNetworkManagerFixture)
{
  // Set parameters
	BOOST_CHECK_THROW(NetworkManager(appContext_, *(dispatcher_.get())), 
		SystemConfigurationException);
}

/** @brief Applies a config with an invalid protocol field */
BOOST_FIXTURE_TEST_CASE(testInvalidSubscriber, BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"channel.0.protocol=Be-Nice-And-Polite", NULL};
  appContext_.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	BOOST_CHECK_THROW(NetworkManager(appContext_, *(dispatcher_.get())), 
		SystemConfigurationException);
}

/** @brief Triggers an exception on initializing a publisher */
BOOST_FIXTURE_TEST_CASE(testPublisherInitException, BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"channel.0.protocol=Mockup", "channel.0.pub_throwOnInit=true", NULL};
  appContext_.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	BOOST_CHECK_THROW(NetworkManager(appContext_, *(dispatcher_.get())), 
		SystemConfigurationException);
}

/** @brief Triggers an exception on initializing a subscriber */
BOOST_FIXTURE_TEST_CASE(testSubscriberInitException1, 
	BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"channel.0.protocol=Mockup",
		"channel.0.subs_throwOnInitAndStart=true", NULL};
  appContext_.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	BOOST_CHECK_THROW(NetworkManager(appContext_, *(dispatcher_.get())), 
		SystemConfigurationException);
}

/** @brief Triggers an exception on initializing a subscriber */
BOOST_FIXTURE_TEST_CASE(testSubscriberInitException2, 
	BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"channel.0.protocol=Mockup",
		"channel.0.subs_throwOnInit=true", NULL};
  appContext_.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	BOOST_CHECK_THROW(NetworkManager(appContext_, *(dispatcher_.get())), 
		SystemConfigurationException);
}

/** 
 * @brief Triggers an exception during the run function of the 
 * ConcurrentSubscriber 
 * @details The Network manager must handle the appropriate exception and must 
 * re-throw it on issuing an event
 */
BOOST_FIXTURE_TEST_CASE(testSubscriberRunException, BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"channel.0.protocol=Mockup",
		"channel.0.subs_throwOnRun=true", NULL};
  appContext_.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	ConcurrentMockupSubscriber::resetCounter();
	MockupPublisher::resetCounter();
	
	{
		NetworkManager nwManager(appContext_, *(dispatcher_.get()));
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		BOOST_CHECK(nwManager.hasPendingException());
		BOOST_CHECK_THROW(nwManager.throwPendingException(), std::runtime_error);
	}

	BOOST_CHECK_EQUAL(ConcurrentMockupSubscriber::getInitAndStartSequenceID(), 
		0);
	BOOST_CHECK_EQUAL(ConcurrentMockupSubscriber::getInitSequenceID(), 1);
	BOOST_CHECK_EQUAL(ConcurrentMockupSubscriber::getRunSequenceID(), 2);
	BOOST_CHECK_EQUAL(ConcurrentMockupSubscriber::getTerminateSequenceID(), 3);
	BOOST_CHECK_EQUAL(
		ConcurrentMockupSubscriber::getTerminationRequestSequenceID(), 4);

	BOOST_CHECK_EQUAL(MockupPublisher::getInitSequenceID(), 0);
	BOOST_CHECK_EQUAL(MockupPublisher::getEventTriggeredSequenceID(), -1);
}


/** 
 * @brief Throw an exception during terminating a subscriber 
 * @details All other subscribers still need to be terminated regularly.
 */
 BOOST_FIXTURE_TEST_CASE(testSubscriberTerminateException1, 
	 BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"channel.0.protocol=Mockup",
		"channel.1.in-var.0=y", "channel.1.in-var.0.type=1", 
		"channel.1.protocol=Mockup",
		"channel.2.in-var.0=z", "channel.2.in-var.0.type=1", 
		"channel.2.protocol=Mockup",
		"channel.1.subs_throwOnTerminate=true",
		NULL};
  appContext_.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	ConcurrentMockupSubscriber::resetCounter();
	MockupPublisher::resetCounter();
	
	try {
		NetworkManager nwManager(appContext_, *(dispatcher_.get()));
		nwManager.terminateSubscribers();
		BOOST_CHECK(false);
	} catch (std::runtime_error &) {
		// Everything is ok
	}

	BOOST_LOG_TRIVIAL(debug) << "Mockup subscriber state: " 
		<< ConcurrentMockupSubscriber::toString();

	BOOST_CHECK_EQUAL(ConcurrentMockupSubscriber::getTerminateSequenceID(), 13);
	BOOST_CHECK_EQUAL(
		ConcurrentMockupSubscriber::getTerminationRequestSequenceID(), 14);

	BOOST_CHECK_EQUAL(MockupPublisher::getInitSequenceID(), 0);
	BOOST_CHECK_EQUAL(MockupPublisher::getEventTriggeredSequenceID(), -1);
}

 /** 
 * @brief Throw an exception during terminating a subscriber 
 * @details All other subscribers still need to be terminated regularly.
 */
 BOOST_FIXTURE_TEST_CASE(testSubscriberTerminateException2, 
	 BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"channel.0.protocol=Mockup",
		"channel.1.in-var.0=y", "channel.1.in-var.0.type=1", 
		"channel.1.protocol=Mockup",
		"channel.2.in-var.0=z", "channel.2.in-var.0.type=1", 
		"channel.2.protocol=Mockup",
		"channel.1.subs_throwOnTerminationRequest=true",
		NULL};
  appContext_.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	ConcurrentMockupSubscriber::resetCounter();
	MockupPublisher::resetCounter();
	
	try {
		NetworkManager nwManager(appContext_, *(dispatcher_.get()));
		nwManager.terminateSubscribers();
		BOOST_CHECK(false);
	} catch (std::runtime_error &) {
		// Everything is ok
	}

	BOOST_LOG_TRIVIAL(debug) << "Mockup subscriber state: " 
		<< ConcurrentMockupSubscriber::toString();

	BOOST_CHECK_EQUAL(ConcurrentMockupSubscriber::getTerminateSequenceID(), 13);
	BOOST_CHECK_EQUAL(
		ConcurrentMockupSubscriber::getTerminationRequestSequenceID(), 14);

	BOOST_CHECK_EQUAL(MockupPublisher::getInitSequenceID(), 0);
	BOOST_CHECK_EQUAL(MockupPublisher::getEventTriggeredSequenceID(), -1);
}