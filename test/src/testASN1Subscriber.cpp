/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testASN1Subscriber.cpp
 * @brief Tests the ASN.1 inbound communication facility
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "base/environment-helper.h"

#define BOOST_TEST_MODULE testASN1Subscriber
//#include "base/environment-helper.h"
#include <boost/test/unit_test.hpp>


#include "RawTestDataSource.h"
#include "RawTCPServerTestDataSource.h"
#include "RawTestDataCollection.h"
#include "PrintableFactory.h"
#include "timing/EventSink.h"
#include "network/Subscriber.h"
#include "network/CompactASN1TCPClientSubscriber.h"
#include "base/BaseExceptions.h"

#include <boost/asio/buffer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/log/trivial.hpp>

#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <list>
#include <ostream>
#include <limits>

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Network;
using namespace FMITerminalBlockTest;
using namespace FMITerminalBlockTest::Network;
using namespace FMITerminalBlockTest::Network::ASN1TestData;

namespace data = boost::unit_test::data;

/**
 * @brief Test class which registers triggered events
 * @details The class is capable of storing one single event which may be 
 * registered concurrently. The event time is increased by one second each time
 * an event is registered.
 */
class ConcurrentEventSink: public Timing::EventSink
{
private:
	/** @brief The time of the next event */
	fmiTime nextEventTime_ = 0.0;
	/** @brief A pointer to the current event or null. */
	Timing::Event *currentEvent_ = NULL;

	/** @brief locks the entire object */
	std::mutex objectMutex_;
	/** @brief Signals that a new event has been received. */
	std::condition_variable newEvent_;

public:
	/** @brief Registers the event */
	virtual void pushExternalEvent(Timing::Event *ev)
	{
		std::unique_lock<std::mutex> lock(objectMutex_);
		BOOST_REQUIRE(ev != NULL);
		BOOST_CHECK(currentEvent_ == NULL);

		currentEvent_ = ev;
		newEvent_.notify_all();

		nextEventTime_ += 1.0;
	}

	/** @brief Returns the test time-stamp of the next event. */
	virtual fmiTime getTimeStampNow() 
	{
		std::unique_lock<std::mutex> lock(objectMutex_);
		return nextEventTime_;
	}

	/**
	 * @brief Waits until the next event is received and returns it.
	 * @details The ownership of the returned event pointer is transferred to the
	 * caller. Delete the event accordingly.
	 */
	Timing::Event* fetchNextEvent()
	{
		std::unique_lock<std::mutex> lock(objectMutex_);
		while (currentEvent_ == NULL)
		{
			std::cv_status stat = newEvent_.wait_for(lock, 
				std::chrono::milliseconds(500));
			BOOST_REQUIRE(stat != std::cv_status::timeout);
		}

		Timing::Event *tmp = currentEvent_;
		currentEvent_ = NULL;
		return tmp;
	}
};

/**
 * @brief Fixture which hosts a basic configuration
 */
class ASN1SubscriberFixture
{
public:
	/** @brief Event Sink which may be used to retrieve Events */
	std::shared_ptr<ConcurrentEventSink> eventSink_;

	/** @brief Initializes all variables */
	ASN1SubscriberFixture(): lastException_()
	{
		eventSink_ = std::make_shared<ConcurrentEventSink>();
		channel_ = std::make_shared<Base::TransmissionChannel>(config_);
	}

	/** 
	 * @brief Generates a transmission channel or returns the previous instance 
	 * @details The raw pointer remains valid until the fixture is destructed. 
	 */
	std::shared_ptr<Base::TransmissionChannel> getTransmissionChannel()
	{
		assert(channel_);
		return channel_;
	}

	/** @brief Returns an error callback storing the given exception */
	std::function<void(std::exception_ptr)> getErrorCallback()
	{
		return [this](std::exception_ptr exception)
		{
			std::unique_lock<std::mutex> lock(exceptionMutex_);
			BOOST_REQUIRE(!lastException_);
		};
	}

	/** @brief Throws the last external exception, if any. */
	void throwLastException()
	{
		std::unique_lock<std::mutex> lock(exceptionMutex_);
		if (lastException_)
		{
			// lastException must not be a null pointer!
			std::rethrow_exception(lastException_);
		}
		
	}

	/** 
	 * @brief Adds another port of the given type 
	 */
	void addPortConfig(FMIType type)
	{
		assert(channel_);

		std::string portPrefix("");
		portPrefix += nextPortNumber_;
		portPrefix += "";

		std::string portNumber("x");
			portNumber += nextPortNumber_;

		config_.add(portPrefix, portNumber);
		config_.add(portPrefix + ".type", (int) type);

		// Register the port at the channel
		channel_->pushBackPort(Base::PortID(type, nextPortNumber_), 
			config_.get_child(portPrefix));

		nextPortNumber_++;
	}

	/** @brief Sets the valid address configuration */
	void setValidAddressConfig()
	{
		setAddress("localhost:4242");
	}

	/** @brief Sets the given address string */
	void setAddress(const std::string &addr)
	{
		config_.add("addr", addr);
	}

private:
	/** @brief Transmission channel reference which is constructed on demand */
	std::shared_ptr<Base::TransmissionChannel> channel_;
	/** @brief Stores the basic channel configuration */
	boost::property_tree::ptree config_;

	/** The last external exception, if any */
	std::exception_ptr lastException_;
	/** @brieg Guards the last system exception */
	std::mutex exceptionMutex_;

	/** @brief The next valid port number */
	int nextPortNumber_ = 0;
};

/** @brief Factory objects which generate the subscriber under test */
PrintableFactory<Subscriber> SUBSCRIBER_GENERATOR[] = {
	PrintableFactory<Subscriber>::make<CompactASN1TCPClientSubscriber>(
		"CompactASN1TCPClientSubscriber")
};

/** @brief Factory objects which generate the test data sources */
PrintableFactory<RawTestDataSource> RAW_SOURCE_GENERATOR[] = {
	PrintableFactory<RawTestDataSource>::make<RawTCPServerTestDataSource>(
		"RawTCPServerTestDataSource")
};

/** @brief Provides print functionality of fmiTypes vectors */
std::ostream& operator << (std::ostream& stream,	const FMIType& type)
{
	stream << "FMIType: " << (int) type;
	return stream;
}

/** @brief Test minimal configuration instantiation */
BOOST_DATA_TEST_CASE_F(ASN1SubscriberFixture, testMinimalConfiguration,
	data::make(SUBSCRIBER_GENERATOR) ^ data::make(RAW_SOURCE_GENERATOR), 
	subscriberFactory, sourceFactory)
{
	std::shared_ptr<Subscriber> subscriber = subscriberFactory();
	std::shared_ptr<RawTestDataSource> dataSource = sourceFactory();

	addPortConfig(fmiTypeReal);
	setValidAddressConfig();

	dataSource->preInitSubscriber();
	subscriber->initAndStart(*getTransmissionChannel(), eventSink_, 
		getErrorCallback());
	dataSource->postInitSubscriber();

	dataSource->preTerminateSubscriber();
	subscriber->terminate();
	dataSource->postTerminateSubscriber();

	try {
		throwLastException();
	} catch (std::exception &ex) {
		BOOST_LOG_TRIVIAL(error) << "Cough an exception: " << ex.what();
		BOOST_CHECK(false);
	}
	
}

/** @brief Applies a missing address field */
BOOST_DATA_TEST_CASE_F(ASN1SubscriberFixture, testMissingAddress,
	data::make(SUBSCRIBER_GENERATOR),	subscriberFactory)
{
	std::shared_ptr<Subscriber> subscriber = subscriberFactory();

	addPortConfig(fmiTypeReal);

	BOOST_CHECK_THROW(
		subscriber->initAndStart(*getTransmissionChannel(), eventSink_,
			getErrorCallback()), 
		Base::SystemConfigurationException);

	BOOST_CHECK_NO_THROW(throwLastException());
}

const std::string INVALID_ADDRESSES[] = {"",":",":4242","localhost:"};

/** @brief Applies an invalid address field */
BOOST_DATA_TEST_CASE_F(ASN1SubscriberFixture, testInvalidAddress0,
	data::make(SUBSCRIBER_GENERATOR)*data::make(INVALID_ADDRESSES),	
	subscriberFactory, address)
{
	std::shared_ptr<Subscriber> subscriber = subscriberFactory();

	addPortConfig(fmiTypeReal);
	setAddress(address);

	BOOST_CHECK_THROW(
		subscriber->initAndStart(*getTransmissionChannel(), eventSink_,
			getErrorCallback()), 
		Base::SystemConfigurationException);

	BOOST_CHECK_NO_THROW(throwLastException());
}

const std::string NONEXISTING_ADDRESSES[] = {"nonlocalhost:4242", 
	"localhost:666"};

/** @brief Tries to connect with a non-existing address */
BOOST_DATA_TEST_CASE_F(ASN1SubscriberFixture, testNonexistingEndpoints,
	data::make(SUBSCRIBER_GENERATOR) * NONEXISTING_ADDRESSES, 
	subscriberFactory, address)
{
	std::shared_ptr<Subscriber> subscriber = subscriberFactory();

	addPortConfig(fmiTypeReal);
	setAddress(address);

	BOOST_CHECK_THROW(
		subscriber->initAndStart(*getTransmissionChannel(), eventSink_, 
			getErrorCallback()), 
		Base::SystemConfigurationException);

	BOOST_CHECK_NO_THROW(throwLastException());
}

/** 
 * @brief Closes and re-opens the server after an initially successful 
 * connection 
 * @details The subscriber has to be able to receive events after a 
 * re-connection timeout.
 */
BOOST_DATA_TEST_CASE_F(ASN1SubscriberFixture, testReconnection,
	data::make(SUBSCRIBER_GENERATOR) ^ data::make(RAW_SOURCE_GENERATOR), 
	subscriberFactory, sourceFactory)
{
	std::shared_ptr<Subscriber> subscriber = subscriberFactory();
	std::shared_ptr<RawTestDataSource> dataSource = sourceFactory();

	RawTestData rawData = RAW_TEST_BOOL_TRUE();

	addPortConfig(fmiTypeBoolean);
	setValidAddressConfig();

	dataSource->preInitSubscriber();
	subscriber->initAndStart(*getTransmissionChannel(), eventSink_, 
		getErrorCallback());
	dataSource->postInitSubscriber();

	BOOST_TEST_CHECKPOINT("Test sending an event");
	dataSource->pushRawData(rawData);
	Timing::Event *ev = eventSink_->fetchNextEvent();
	BOOST_REQUIRE(ev != NULL);
	delete ev;

	BOOST_TEST_CHECKPOINT("Re-connection cycle");
	std::this_thread::sleep_for(std::chrono::milliseconds(400));
	dataSource->preTerminateSubscriber();
	dataSource->postTerminateSubscriber();
	dataSource->preInitSubscriber();
	// Automatic re-connect expected
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	dataSource->postInitSubscriber();

	BOOST_TEST_CHECKPOINT("Test sending an event");
	dataSource->pushRawData(rawData);
	ev = eventSink_->fetchNextEvent();
	BOOST_REQUIRE(ev != NULL);
	delete ev;

	BOOST_TEST_CHECKPOINT("Close as usual");
	dataSource->preTerminateSubscriber();
	subscriber->terminate();
	dataSource->postTerminateSubscriber();

	BOOST_CHECK_NO_THROW(throwLastException());
}

const RawTestData NON_CONVERTIBLE_STRING_PACKETS[] = {
	(RAW_TEST_BOOL_TRUE() + RAW_TEST_STRING_HII() + RAW_TEST_BOOL_FALSE()),
	(RAW_TEST_BOOL_TRUE() + RAW_TEST_STRING_EMPTY() + RAW_TEST_BOOL_FALSE())
};

const FMIType NON_STRING_TYPES[] = {fmiTypeReal, fmiTypeInteger, 
	fmiTypeBoolean};

/** 
 * @brief Sends a string which is not convertible to any other type
 * @details The subscriber has to gracefully ignore the string and process all
 * other data types
 */
BOOST_DATA_TEST_CASE_F(ASN1SubscriberFixture, testInvalidStringConversion,
	(data::make(SUBSCRIBER_GENERATOR) ^ data::make(RAW_SOURCE_GENERATOR))*
	data::make(NON_CONVERTIBLE_STRING_PACKETS) * data::make(NON_STRING_TYPES), 
	subscriberFactory, sourceFactory, rawPacket, destinationType)
{
	std::shared_ptr<Subscriber> subscriber = subscriberFactory();
	std::shared_ptr<RawTestDataSource> dataSource = sourceFactory();

	addPortConfig(fmiTypeBoolean);
	addPortConfig(destinationType);
	addPortConfig(fmiTypeBoolean);
	setValidAddressConfig();

	dataSource->preInitSubscriber();
	subscriber->initAndStart(*getTransmissionChannel(), eventSink_, 
		getErrorCallback());
	dataSource->postInitSubscriber();

	dataSource->pushRawData(rawPacket);
	Timing::Event *ev = eventSink_->fetchNextEvent();
	BOOST_REQUIRE(ev != NULL);

	BOOST_CHECK_EQUAL(ev->getTime(), 0.0);
	BOOST_REQUIRE_EQUAL(ev->getVariables().size(), 2);
	BOOST_CHECK_EQUAL(ev->getVariables()[0].getID().first, fmiTypeBoolean);
	BOOST_CHECK_EQUAL(ev->getVariables()[1].getID().first, fmiTypeBoolean);
	BOOST_CHECK_EQUAL(ev->getVariables()[0].getBooleanValue(), fmiTrue);
	BOOST_CHECK_EQUAL(ev->getVariables()[1].getBooleanValue(), fmiFalse);
	delete ev;

	dataSource->preTerminateSubscriber();
	subscriber->terminate();
	dataSource->postTerminateSubscriber();

	BOOST_CHECK_NO_THROW(throwLastException());
}

const RawTestData FIRST_RAW_DATA_PACKET[] = {
	RAW_TEST_REAL_0_3(), RAW_TEST_DINT_INT_MIN(), RAW_TEST_BOOL_TRUE()
};

const RawTestData SECOND_RAW_DATA_PACKET[] = {
	RAW_TEST_LREAL_DBL_EPSILON(), RAW_TEST_DINT_INT_MAX(), RAW_TEST_BOOL_FALSE()
};

Timing::Variable FIRST_REFERENCE_VAR[] = {
	Timing::Variable(Base::PortID(fmiTypeReal,0), ((double) 0.3f)),
	Timing::Variable(Base::PortID(fmiTypeInteger,0),INT_MIN),
	// Must be converted to fmiBoolean, otherwise int and bad anycast!!
	Timing::Variable(Base::PortID(fmiTypeBoolean,0),((fmiBoolean) fmiTrue))
};

Timing::Variable SECOND_REFERENCE_VAR[] = {
	Timing::Variable(Base::PortID(fmiTypeReal,0),DBL_EPSILON),
	Timing::Variable(Base::PortID(fmiTypeInteger,0),INT_MAX),
	// Must be converted to fmiBoolean, otherwise int and bad anycast!!
	Timing::Variable(Base::PortID(fmiTypeBoolean,0),((fmiBoolean) fmiFalse))
};

/** 
 * @brief Tests the type conversion by sending various acceptable packets 
 * @details For simplicity, each packet contains a single variable.
 */
BOOST_DATA_TEST_CASE_F(ASN1SubscriberFixture, testRealPacketSequence,
	(data::make(SUBSCRIBER_GENERATOR) ^ data::make(RAW_SOURCE_GENERATOR))*
	(data::make(FIRST_RAW_DATA_PACKET) ^ data::make(SECOND_RAW_DATA_PACKET) ^
	data::make(FIRST_REFERENCE_VAR) ^ data::make(SECOND_REFERENCE_VAR)), 
	subscriberFactory, sourceFactory, firstRawPacket, secondRawPacket, firstRef, 
	secondRef)
{
	std::shared_ptr<Subscriber> subscriber = subscriberFactory();
	std::shared_ptr<RawTestDataSource> dataSource = sourceFactory();

	addPortConfig(firstRef.getID().first);
	setValidAddressConfig();

	dataSource->preInitSubscriber();
	subscriber->initAndStart(*getTransmissionChannel(), eventSink_, 
		getErrorCallback());
	dataSource->postInitSubscriber();

	// First Packet
	dataSource->pushRawData(firstRawPacket);

	Timing::Event *ev = eventSink_->fetchNextEvent();
	BOOST_REQUIRE(ev != NULL);
	BOOST_CHECK_EQUAL(ev->getTime(), 0.0);
	BOOST_REQUIRE_EQUAL(ev->getVariables().size(), 1);
	BOOST_CHECK_EQUAL(ev->getVariables()[0], firstRef);
	delete ev;

	// Second Packet
	dataSource->pushRawData(secondRawPacket);

	ev = eventSink_->fetchNextEvent();
	BOOST_REQUIRE(ev != NULL);
	BOOST_CHECK_EQUAL(ev->getTime(), 1.0);
	BOOST_REQUIRE_EQUAL(ev->getVariables().size(), 1);
	BOOST_CHECK_EQUAL(ev->getVariables()[0], secondRef);
	delete ev;

	dataSource->preTerminateSubscriber();
	subscriber->terminate();
	dataSource->postTerminateSubscriber();

	BOOST_CHECK_NO_THROW(throwLastException());
}


/** @brief Sends a complex message in a single packet */
BOOST_DATA_TEST_CASE_F(ASN1SubscriberFixture, testComplexPacket,
	data::make(SUBSCRIBER_GENERATOR) ^ data::make(RAW_SOURCE_GENERATOR), 
	subscriberFactory, sourceFactory)
{
	std::shared_ptr<Subscriber> subscriber = subscriberFactory();
	std::shared_ptr<RawTestDataSource> dataSource = sourceFactory();

	addPortConfig(fmiTypeReal);
	addPortConfig(fmiTypeInteger);
	addPortConfig(fmiTypeReal);
	setValidAddressConfig();

	dataSource->preInitSubscriber();
	subscriber->initAndStart(*getTransmissionChannel(), eventSink_, 
		getErrorCallback());
	dataSource->postInitSubscriber();

	RawTestData rawData = RAW_TEST_REAL_0_3() + RAW_TEST_DINT_INT_MAX() + 
		RAW_TEST_LREAL_DBL_EPSILON();
	dataSource->pushRawData(rawData);

	Timing::Event *ev = eventSink_->fetchNextEvent();
	BOOST_REQUIRE(ev != NULL);
	BOOST_CHECK_EQUAL(ev->getTime(), 0.0);
	BOOST_REQUIRE_EQUAL(ev->getVariables().size(), 3);
	BOOST_CHECK_EQUAL(ev->getVariables()[0], 
		Timing::Variable(Base::PortID(fmiTypeReal, 0), ((fmiReal) 0.3f)));
	BOOST_CHECK_EQUAL(ev->getVariables()[1], 
		Timing::Variable(Base::PortID(fmiTypeInteger, 1), INT_MAX));
	BOOST_CHECK_EQUAL(ev->getVariables()[2], 
		Timing::Variable(Base::PortID(fmiTypeReal, 2), DBL_EPSILON));
	delete ev;

	dataSource->preTerminateSubscriber();
	subscriber->terminate();
	dataSource->postTerminateSubscriber();

	BOOST_CHECK_NO_THROW(throwLastException());
}

/** 
 * TODO: Splits a complex message in two packets. 
 * @brief The function sends another packet in order to test whether the object
 * is reset correctly.
 */
/** 
 * TODO: Sends an incomplete packet
 * @details After a timeout, the subscriber needs to issue the (incomplete) 
 * event and must be able to receive other complete events correctly.
 */

/** TODO: Test an invalid type code */

/** TODO: Test type conversion system extensively */

/** 
 * TODO: Test initializing a subscriber with no associates network ports 
 * @details An exception must be thrown
 */