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
			"in.0.0=x", "in.0.0.type=0", 
			"out.0.0=x", "out.0.0.type=0", NULL};
		appContext_.addCommandlineProperties((sizeof(argv)/sizeof(argv[0]))-1, argv);

    dispatcher_ = std::make_shared<Timing::EventDispatcher>(appContext_, 
      predictor_);
  }
};

/** @brief Tests the instantiation of a NetworkManager */
BOOST_FIXTURE_TEST_CASE(testInstantiation, BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"out.0.protocol=MockupPublisher",
		"in.0.protocol=ConcurrentMockupSubscriber", NULL};
  appContext_.addCommandlineProperties((sizeof(argv)/sizeof(argv[0]))-1, argv);

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
		"out.0.protocol=MockupPublisher",
		"in.0.protocol=ConcurrentMockupSubscriber", NULL};
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
	BOOST_CHECK_EQUAL(ConcurrentMockupSubscriber::getRunSequenceID(), 2);
	BOOST_CHECK_EQUAL(ConcurrentMockupSubscriber::getTerminateSequenceID(), 3);
	BOOST_CHECK_EQUAL(
		ConcurrentMockupSubscriber::getTerminationRequestSequenceID(), 4);

	BOOST_CHECK_EQUAL(MockupPublisher::getInitSequenceID(), 0);
	BOOST_CHECK_EQUAL(MockupPublisher::getEventTriggeredSequenceID(), -1);

}

/** @brief Applies a config with a missing subscriber protocol field */
BOOST_FIXTURE_TEST_CASE(testMissingSubscriber, BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"out.0.protocol=MockupPublisher", NULL};
  appContext_.addCommandlineProperties((sizeof(argv)/sizeof(argv[0]))-1, argv);

	BOOST_CHECK_THROW(NetworkManager(appContext_, *(dispatcher_.get())), 
		SystemConfigurationException);
}

/** @brief Applies a config with a missing publisher protocol field */
BOOST_FIXTURE_TEST_CASE(testMissingPublisher, BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"in.0.protocol=ConcurrentMockupSubscriber", NULL};
  appContext_.addCommandlineProperties((sizeof(argv)/sizeof(argv[0]))-1, argv);

	BOOST_CHECK_THROW(NetworkManager(appContext_, *(dispatcher_.get())), 
		SystemConfigurationException);
}

/** @brief Applies a config with an invalid subscriber protocol field */
BOOST_FIXTURE_TEST_CASE(testInvalidSubscriber, BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"out.0.protocol=MockupPublisher",
		"in.0.protocol=InvalidSubscriber", NULL};
  appContext_.addCommandlineProperties((sizeof(argv)/sizeof(argv[0]))-1, argv);

	BOOST_CHECK_THROW(NetworkManager(appContext_, *(dispatcher_.get())), 
		SystemConfigurationException);
}

/** @brief Applies a config with an invalid publisher protocol field */
BOOST_FIXTURE_TEST_CASE(testInvalidPublisher, BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"out.0.protocol=InvalidPublisher",
		"in.0.protocol=ConcurrentMockupSubscriber", NULL};
  appContext_.addCommandlineProperties((sizeof(argv)/sizeof(argv[0]))-1, argv);

	BOOST_CHECK_THROW(NetworkManager(appContext_, *(dispatcher_.get())), 
		SystemConfigurationException);
}

/** @brief Triggers an exception on initializing a publisher */
BOOST_FIXTURE_TEST_CASE(testPublisherInitException, BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"out.0.protocol=MockupPublisher", "out.0.throwOnInit=true",
		"in.0.protocol=ConcurrentMockupSubscriber", NULL};
  appContext_.addCommandlineProperties((sizeof(argv)/sizeof(argv[0]))-1, argv);

	BOOST_CHECK_THROW(NetworkManager(appContext_, *(dispatcher_.get())), 
		SystemConfigurationException);
}

/** @brief Triggers an exception on initializing a subscriber */
BOOST_FIXTURE_TEST_CASE(testSubscriberInitException1, 
	BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"out.0.protocol=MockupPublisher",
		"in.0.protocol=ConcurrentMockupSubscriber", 
		"in.0.throwOnInitAndStart=true", NULL};
  appContext_.addCommandlineProperties((sizeof(argv)/sizeof(argv[0]))-1, argv);

	BOOST_CHECK_THROW(NetworkManager(appContext_, *(dispatcher_.get())), 
		SystemConfigurationException);
}

/** @brief Triggers an exception on initializing a subscriber */
BOOST_FIXTURE_TEST_CASE(testSubscriberInitException2, 
	BasicNetworkManagerFixture)
{
  // Set parameters
  const char * argv[] = {"testNetworkManager", 
		"out.0.protocol=MockupPublisher",
		"in.0.protocol=ConcurrentMockupSubscriber", 
		"in.0.throwOnInit=true", NULL};
  appContext_.addCommandlineProperties((sizeof(argv)/sizeof(argv[0]))-1, argv);

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
		"out.0.protocol=MockupPublisher",
		"in.0.protocol=ConcurrentMockupSubscriber", 
		"in.0.throwOnRun=true", NULL};
  appContext_.addCommandlineProperties((sizeof(argv)/sizeof(argv[0]))-1, argv);

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
		"out.0.protocol=MockupPublisher",
		"in.0.protocol=ConcurrentMockupSubscriber",  
		"in.1.0=y", "in.1.0.type=1", "in.1.protocol=ConcurrentMockupSubscriber",
		"in.2.0=z", "in.2.0.type=1", "in.2.protocol=ConcurrentMockupSubscriber",
		"in.1.throwOnTerminate=true",
		NULL};
  appContext_.addCommandlineProperties((sizeof(argv)/sizeof(argv[0]))-1, argv);

	ConcurrentMockupSubscriber::resetCounter();
	MockupPublisher::resetCounter();
	
	try {
		NetworkManager nwManager(appContext_, *(dispatcher_.get()));
		nwManager.terminateSubscribers();
		BOOST_CHECK(false);
	} catch (std::runtime_error &) {
		// Everything is ok
	}

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
		"out.0.protocol=MockupPublisher",
		"in.0.protocol=ConcurrentMockupSubscriber",  
		"in.1.0=y", "in.1.0.type=1", "in.1.protocol=ConcurrentMockupSubscriber",
		"in.2.0=z", "in.2.0.type=1", "in.2.protocol=ConcurrentMockupSubscriber",
		"in.1.throwOnTerminationRequest=true",
		NULL};
  appContext_.addCommandlineProperties((sizeof(argv)/sizeof(argv[0]))-1, argv);

	ConcurrentMockupSubscriber::resetCounter();
	MockupPublisher::resetCounter();
	
	try {
		NetworkManager nwManager(appContext_, *(dispatcher_.get()));
		nwManager.terminateSubscribers();
		BOOST_CHECK(false);
	} catch (std::runtime_error &) {
		// Everything is ok
	}

	BOOST_CHECK_EQUAL(ConcurrentMockupSubscriber::getTerminateSequenceID(), 13);
	BOOST_CHECK_EQUAL(
		ConcurrentMockupSubscriber::getTerminationRequestSequenceID(), 14);

	BOOST_CHECK_EQUAL(MockupPublisher::getInitSequenceID(), 0);
	BOOST_CHECK_EQUAL(MockupPublisher::getEventTriggeredSequenceID(), -1);
}