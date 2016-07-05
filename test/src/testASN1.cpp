/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testASN1.cpp
 * @brief Tests the ASN.1 communication facility
 * @details The test suit uses a fixture based model. Each fixture provides a 
 * common initialization and the functionality to open a listening socket and 
 * checks incoming messages. The base fixture, ASN1Fixture is refined by
 * protocol dependent fixtures which handle protocol specifics. To process any
 * communication event the ioService's run_once function has to be called at 
 * the right place. 
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#define BOOST_TEST_MODULE testASN1
#include <boost/test/unit_test.hpp>

#include <assert.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

#include <vector>
#include <utility>
#include <limits.h>
#include <float.h>
#include <stdint.h>
#include <stdexcept>

#include "network/CompactASN1UDPPublisher.h"
#include "network/CompactASN1TCPClientPublisher.h"
#include "timing/StaticEvent.h"
#include "base/BaseExceptions.h"

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Network;

/**
 * @brief Structure encapsulating data common to most of the test cases
 * @details The structure holds a basic configuration snippet and provides
 * facilities which check received data.
 */
struct ASN1Fixture
{
	/** @brief The configuration property tree */
	boost::property_tree::ptree config;
	/** @brief The vector of configured ports */
	std::vector<const Base::ChannelMapping::PortID> ports;

	/** @brief IO Service used to test the network connection */
	boost::asio::io_service ioService;
	/**
	 * @brief Reference buffer which is populated with the expected byte sequence
	 */
	std::vector<uint8_t> receiveReference;
	/** @brief The size of the receive buffer */
	static const size_t BUFFER_SIZE = 2024;
	/** @brief Buffer which stores the received byte sequence */
	std::vector<uint8_t> receiveBuffer;

	/**
	 * @brief The number of received messages 
	 * @details The variable will be written by the message testing logic and may
	 * be read by the test case's code to determine the number of received 
	 * packets.
	 */
	unsigned int validMessages;

	/** @brief C'tor which initializes a minimal configuration */
	ASN1Fixture(): config(), ports(), ioService(), validMessages(0)
	{
		config.put<std::string>("addr", "127.0.0.1:4242");

		ports.push_back(std::make_pair(fmiTypeReal, 666));
		ports.push_back(std::make_pair(fmiTypeInteger, 0));
		ports.push_back(std::make_pair(fmiTypeBoolean, 0));
		ports.push_back(std::make_pair(fmiTypeString, 0));

		// tidy up buffers
		receiveBuffer.assign(BUFFER_SIZE, 0xAA);
		receiveReference.clear();
	};

	/**
	 * @brief Handler checking the received byte sequence
	 * @param error The status of the operation
	 * @param transferred The number of bytes transferred
	 */
	void checkReceived(const boost::system::error_code& error, 
		std::size_t bytes_transferred)
	{

		BOOST_CHECK_MESSAGE(!error, error.message());

		receiveBuffer.resize(bytes_transferred);

		BOOST_CHECK_EQUAL(bytes_transferred, receiveReference.size());
		BOOST_CHECK_EQUAL_COLLECTIONS(receiveBuffer.begin(), receiveBuffer.end(), 
			receiveReference.begin(), receiveReference.end());

		BOOST_LOG_TRIVIAL(trace) << "Received message which passed the tests.";
		validMessages++;
		receiveBuffer.assign(BUFFER_SIZE, 0xAA);
	}

};

// ============================================================================
// Test TCP Client Publisher
// ============================================================================
/**
 * @brief specialized fixture containing UDP specifics
 * @details After initializing and after transferring the packets the ioService
 * object needs to run once to process the client's requests.
 */
struct ASN1TCPFixture: ASN1Fixture
{

	/** @brief The publisher under test */
	CompactASN1TCPClientPublisher publisher;

	/** @brief The socket which holds any accepted connection */
	tcp::socket * socket;
	/** @brief The acceptor listening for new incoming connections */
	tcp::acceptor * acceptor;

	/** @brief Some remote end point buffer which holds the datagram's source */
	tcp::endpoint remoteEndpoint;


	/**
	 * @brief C'tor configuring the TCP socket
	 */
	ASN1TCPFixture():publisher(), remoteEndpoint()
	{
		socket = new tcp::socket(ioService);
		acceptor = new tcp::acceptor(ioService, tcp::endpoint(tcp::v4(), 4242));
		acceptor->async_accept(*socket, 
			boost::bind(&ASN1TCPFixture::acceptCallback, this, 
				boost::asio::placeholders::error)
			);
	};

	/** @brief Closes the opened socket */
	~ASN1TCPFixture()
	{
		assert(acceptor != NULL);
		assert(socket != NULL);
		delete socket;
		delete acceptor;
	}

	/** @brief Initiates reading the next value */
	void acceptCallback(const boost::system::error_code& error)
	{
		assert(socket != NULL);

		BOOST_CHECK_MESSAGE(!error, error.message());

		socket->async_receive(boost::asio::buffer(receiveBuffer), 
			boost::bind(&ASN1TCPFixture::checkReceived, this, 
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	/**
	 * @brief Handler checking the received byte sequence
	 * @details The function will trigger the next read request
	 * @param error The status of the operation
	 * @param transferred The number of bytes transferred
	 */
	void checkReceived(const boost::system::error_code& error, 
		std::size_t bytes_transferred)
	{
		assert(socket != NULL);
		ASN1Fixture::checkReceived(error, bytes_transferred);
		socket->async_receive(boost::asio::buffer(receiveBuffer), 
			boost::bind(&ASN1TCPFixture::checkReceived, this, 
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

};

/** @brief Tests an invalid TCP address */
BOOST_FIXTURE_TEST_CASE( test_invalid_ASN1_TCP_address_0, ASN1TCPFixture )
{
	config.put("addr", ":1234");
	BOOST_CHECK_THROW(publisher.init(config, ports), Base::SystemConfigurationException);
	BOOST_CHECK_EQUAL(validMessages, 0);
}

/** @brief Tests an invalid TCP address */
BOOST_FIXTURE_TEST_CASE( test_invalid_ASN1_TCP_address_1, ASN1TCPFixture )
{
	config.clear();
	BOOST_CHECK_THROW(publisher.init(config, ports), Base::SystemConfigurationException);
	BOOST_CHECK_EQUAL(validMessages, 0);
}

/** @brief Tests an invalid TCP address */
BOOST_FIXTURE_TEST_CASE( test_invalid_ASN1_TCP_address_2, ASN1TCPFixture )
{
	config.put("addr", "no-host-or-something-else:1234");
	BOOST_CHECK_THROW(publisher.init(config, ports), std::runtime_error);
	BOOST_CHECK_EQUAL(validMessages, 0);
}

/** @brief Tests an invalid fmiReal type conversion */
BOOST_FIXTURE_TEST_CASE( test_invalid_ASN1_TCP_fmiReal_conversion_0, ASN1TCPFixture )
{
	config.put("0.encoding", "");
	BOOST_CHECK_THROW(publisher.init(config, ports), Base::SystemConfigurationException);
	BOOST_CHECK_EQUAL(validMessages, 0);
}

/** @brief Tests an invalid fmiReal type conversion */
BOOST_FIXTURE_TEST_CASE( test_invalid_ASN1_TCP_fmiReal_conversion_1, ASN1TCPFixture )
{
	config.put("0.encoding", "BOOL");
	BOOST_CHECK_THROW(publisher.init(config, ports), Base::SystemConfigurationException);
	BOOST_CHECK_EQUAL(validMessages, 0);
}

/** @brief Loosely tests the TCP publisher instantiation and destruction */
BOOST_FIXTURE_TEST_CASE( test_create_ASN1_TCP_publisher, ASN1Fixture )
{
	Publisher * pub = new CompactASN1TCPClientPublisher();
	// No server is currently listening
	BOOST_CHECK_THROW(pub->init(config, ports), std::runtime_error);
	delete pub;
	BOOST_CHECK_EQUAL(validMessages, 0);
}

/** 
 * @brief tests the ability of sending correct messages using a manual 
 * encoding scheme 
 */
BOOST_FIXTURE_TEST_CASE( test_publish_ASN1_TCP_manual_enc_0, ASN1TCPFixture )
{

	// Set up configuration
	config.put("0.encoding", "LREAL");
	config.put("1.encoding", "DINT");
	config.put("2.encoding", "BOOL");
	config.put("3.encoding", "STRING");

	// Set-up event values
	std::vector<Timing::Event::Variable> vars;

	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) DBL_EPSILON));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MIN));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiTrue));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("H3llö!")));
	
	// Set-up reference
	uint8_t ref[] = {0x4b, 0x3c,0xb0,0x00,0x00,0x00,0x00,0x00,0x00,
									 0x44, 0x80,0x00,0x00,0x00,
									 0x41, 
									 0x50,0x00,0x06,0x48,0x33,0x6c,0x6c,0xf6,0x21};
	receiveReference.assign(ref, ref+sizeof(ref));
	// Init publisher
	publisher.init(config, ports);
	// Let the client connect
	ioService.run_one();

	// Trigger event
	Timing::Event * ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 

	// Fetch the IO result
	ioService.run_one();
	
	delete ev;
	BOOST_CHECK_EQUAL(validMessages, 1);
}

/** 
 * @brief tests the ability of sending correct messages using a manual 
 * encoding scheme 
 */
BOOST_FIXTURE_TEST_CASE( test_publish_ASN1_TCP_manual_enc_1, ASN1TCPFixture )
{

	// Set up configuration
	config.put("0.encoding", "REAL");
	config.put("1.encoding", "DINT"); // Still default
	config.put("2.encoding", "BOOL"); // Still default
	config.put("3.encoding", "STRING"); // Still default

	// Set-up event values
	std::vector<Timing::Event::Variable> vars;
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) 0.3 ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MAX ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiFalse ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("") ));

	// Set-up reference
	uint8_t ref[] = {0x4a, 0x3e,0x99,0x99,0x9a,
									 0x44, 0x7F,0xFF,0xFF,0xFF,
									 0x40, 
									 0x50,0x00,0x00};
	receiveReference.assign(ref, ref+sizeof(ref));
	// Init publisher
	publisher.init(config, ports);
	// Let the client connect
	ioService.run_one();

	// Trigger event
	Timing::Event * ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 

	// Fetch the IO result
	ioService.run_one();
	
	delete ev;
	BOOST_CHECK_EQUAL(validMessages, 1);
}

/** @brief tests the ability of sending correct messages */
BOOST_FIXTURE_TEST_CASE( test_publish_ASN1_TCP_0, ASN1TCPFixture )
{

	// Set-up event values
	std::vector<Timing::Event::Variable> vars;

	vars.push_back(std::make_pair( 	// dummy
			std::make_pair(fmiTypeInteger, INT_MAX), 
			(fmiInteger) 3));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) DBL_EPSILON));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MIN));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiTrue));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("H3llö!")));
	vars.push_back(std::make_pair( // dummy
			std::make_pair(fmiTypeReal, 665), 
			(fmiReal) 43.0));
	vars.push_back(std::make_pair( // dummy
			std::make_pair(fmiTypeString, INT_MAX), 
			std::string("Nope")));
	vars.push_back(std::make_pair( // dummy
			std::make_pair(fmiTypeBoolean, INT_MAX), 
			(fmiBoolean) fmiFalse));
	
	// Set-up reference
	uint8_t ref[] = {0x4b, 0x3c,0xb0,0x00,0x00,0x00,0x00,0x00,0x00,
									 0x44, 0x80,0x00,0x00,0x00,
									 0x41, 
									 0x50,0x00,0x06,0x48,0x33,0x6c,0x6c,0xf6,0x21};
	receiveReference.assign(ref, ref+sizeof(ref));
	// Init publisher
	publisher.init(config, ports);

	// Let the client connect
	ioService.run_one();

	// Trigger event
	Timing::Event * ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 

	// Fetch the IO result
	ioService.run_one();
	
	delete ev;
	BOOST_CHECK_EQUAL(validMessages, 1);
}

/** @brief tests the ability of sending correct messages */
BOOST_FIXTURE_TEST_CASE( test_publish_ASN1_TCP_1, ASN1TCPFixture )
{

	// Set-up event values
	std::vector<Timing::Event::Variable> vars;
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) 0.3 ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MAX ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiFalse ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("") ));

	// Set-up reference
	uint8_t ref[] = {0x4b, 0x3f,0xd3,0x33,0x33,0x33,0x33,0x33,0x33,
									 0x44, 0x7F,0xFF,0xFF,0xFF,
									 0x40, 
									 0x50,0x00,0x00};
	receiveReference.assign(ref, ref+sizeof(ref));
	// Init publisher
	publisher.init(config, ports);
	// Let the client connect
	ioService.run_one();

	// Trigger event
	Timing::Event * ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 

	// Fetch the IO result
	ioService.run_one();
	
	delete ev;
	BOOST_CHECK_EQUAL(validMessages, 1);
}

/** @brief tests the ability of sending correct messages */
BOOST_FIXTURE_TEST_CASE( test_publish_ASN1_TCP_multi_event_0, ASN1TCPFixture )
{

	// Set-up event values
	std::vector<Timing::Event::Variable> vars;
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) 0.3 ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MAX ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiFalse ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("") ));

	// Set-up reference
	uint8_t ref[] = {0x4b, 0x3f,0xd3,0x33,0x33,0x33,0x33,0x33,0x33,
									 0x44, 0x7F,0xFF,0xFF,0xFF,
									 0x40, 
									 0x50,0x00,0x00};
	receiveReference.assign(ref, ref+sizeof(ref));
	// Init publisher
	publisher.init(config, ports);
	// Let the client connect
	ioService.run_one();

	// Trigger first event
	Timing::Event * ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 
	delete ev;
	
	// Fetch the IO result
	ioService.run_one();
	
	vars.clear();
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiTrue));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) DBL_EPSILON));

	// Set-up reference
	uint8_t ref2[] = {0x4b, 0x3c,0xb0,0x00,0x00,0x00,0x00,0x00,0x00, // changed
									 0x44, 0x7F,0xFF,0xFF,0xFF,
									 0x41, // changed
									 0x50,0x00,0x00};
	receiveReference.assign(ref2, ref2+sizeof(ref2));

	// Trigger second event
	ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 
	delete ev;

	// Fetch the IO result
	ioService.run_one();
	BOOST_CHECK_EQUAL(validMessages, 2);
}

/** @brief tests the ability of sending correct messages */
BOOST_FIXTURE_TEST_CASE( test_publish_ASN1_TCP_multi_event_1, ASN1TCPFixture )
{

	// Set-up event values
	std::vector<Timing::Event::Variable> vars;
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) 0.3 ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MAX ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiFalse ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("") ));

	// Set-up reference
	uint8_t ref[] = {0x4b, 0x3f,0xd3,0x33,0x33,0x33,0x33,0x33,0x33,
									 0x44, 0x7F,0xFF,0xFF,0xFF,
									 0x40, 
									 0x50,0x00,0x00};
	receiveReference.assign(ref, ref+sizeof(ref));
	// Init publisher
	publisher.init(config, ports);
	// Let the client connect
	ioService.run_one();

	// Trigger first event
	Timing::Event * ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 
	delete ev;
	
	// Fetch the IO result
	ioService.run_one();
	
	vars.clear();
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("H3llö!")));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MIN));

	// Set-up reference
	uint8_t ref2[] = {0x4b, 0x3f,0xd3,0x33,0x33,0x33,0x33,0x33,0x33,
									 0x44, 0x80,0x00,0x00,0x00, // changed
									 0x40,
									 0x50,0x00,0x06,0x48,0x33,0x6c,0x6c,0xf6,0x21 /* changed */ };
	receiveReference.assign(ref2, ref2+sizeof(ref2));

	// Trigger second event
	ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 
	delete ev;

	// Fetch the IO result
	ioService.run_one();
	BOOST_CHECK_EQUAL(validMessages, 2);
}


// ============================================================================
// Test UDP Publisher
// ============================================================================

/**
 * @brief Specialized fixture containing UDP specifics
 */
struct ASN1UDPFixture: ASN1Fixture
{

	/**
	 * @brief C'tor configuring the UDP socket
	 */
	ASN1UDPFixture():publisher(), remoteEndpoint()
	{
		// Open Socket
		socket = new udp::socket(ioService, udp::endpoint(udp::v4(), 4242));
		// Receive message
		socket->async_receive_from(boost::asio::buffer(receiveBuffer), 
			remoteEndpoint,
			boost::bind(&ASN1UDPFixture::checkReceived, this, 
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	};

	/** @brief Closes the opened socket */
	~ASN1UDPFixture()
	{
		assert(socket != NULL);
		socket->close();
		delete socket;
	}

	/**
	 * @brief Handler checking the received byte sequence
	 * @details The function will trigger the next read request
	 * @param error The status of the operation
	 * @param transferred The number of bytes transferred
	 */
	void checkReceived(const boost::system::error_code& error, 
		std::size_t bytes_transferred)
	{
		assert(socket != NULL);
		ASN1Fixture::checkReceived(error, bytes_transferred);
		socket->async_receive_from(boost::asio::buffer(receiveBuffer), 
			remoteEndpoint,
			boost::bind(&ASN1UDPFixture::checkReceived, this, 
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	/** @brief The publisher under test */
	CompactASN1UDPPublisher publisher;

	/** @brief The socket ready to receive new datagrams */
	udp::socket * socket;
	/** @brief Some remote end point buffer holing the datagram's source */
	udp::endpoint remoteEndpoint;

};

/** @brief Tests an invalid UDP address */
BOOST_FIXTURE_TEST_CASE( test_invalid_ASN1_UDP_address_0, ASN1UDPFixture )
{
	config.put("addr", ":1234");
	BOOST_CHECK_THROW(publisher.init(config, ports), Base::SystemConfigurationException);
}

/** @brief Tests an invalid UDP address */
BOOST_FIXTURE_TEST_CASE( test_invalid_ASN1_UDP_address_1, ASN1UDPFixture )
{
	config.clear();
	BOOST_CHECK_THROW(publisher.init(config, ports), Base::SystemConfigurationException);
}

/** @brief Tests an invalid UDP address */
BOOST_FIXTURE_TEST_CASE( test_invalid_ASN1_UDP_address_2, ASN1UDPFixture )
{
	config.put("addr", "no-host-or-something-else:1234");
	BOOST_CHECK_THROW(publisher.init(config, ports), std::runtime_error);
}

/** @brief Tests an invalid fmiReal type conversion */
BOOST_FIXTURE_TEST_CASE( test_invalid_ASN1_UDP_fmiReal_conversion_0, ASN1UDPFixture )
{
	config.put("0.encoding", "");
	BOOST_CHECK_THROW(publisher.init(config, ports), Base::SystemConfigurationException);
	BOOST_CHECK_EQUAL(validMessages, 0);
}

/** @brief Tests an invalid fmiReal type conversion */
BOOST_FIXTURE_TEST_CASE( test_invalid_ASN1_UDP_fmiReal_conversion_1, ASN1UDPFixture )
{
	config.put("0.encoding", "BOOL");
	BOOST_CHECK_THROW(publisher.init(config, ports), Base::SystemConfigurationException);
	BOOST_CHECK_EQUAL(validMessages, 0);
}

/** @brief Loosely tests the UDP publisher instantiation and destruction */
BOOST_FIXTURE_TEST_CASE( test_create_ASN1_UDP_publisher, ASN1Fixture )
{
	Publisher * pub = new CompactASN1UDPPublisher();
	pub->init(config, ports);
	delete pub;
}

/** 
 * @brief tests the ability of sending correct messages using a manual 
 * encoding scheme 
 */
BOOST_FIXTURE_TEST_CASE( test_publish_ASN1_UDP_manual_enc_0, ASN1UDPFixture )
{

	// Set up configuration
	config.put("0.encoding", "LREAL");
	config.put("1.encoding", "DINT");
	config.put("2.encoding", "BOOL");
	config.put("3.encoding", "STRING");

	// Set-up event values
	std::vector<Timing::Event::Variable> vars;

	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) DBL_EPSILON));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MIN));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiTrue));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("H3llö!")));
	
	// Set-up reference
	uint8_t ref[] = {0x4b, 0x3c,0xb0,0x00,0x00,0x00,0x00,0x00,0x00,
									 0x44, 0x80,0x00,0x00,0x00,
									 0x41, 
									 0x50,0x00,0x06,0x48,0x33,0x6c,0x6c,0xf6,0x21};
	receiveReference.assign(ref, ref+sizeof(ref));
	// Init publisher
	publisher.init(config, ports);

	// Trigger event
	Timing::Event * ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 

	// Fetch the IO result
	ioService.run_one();
	
	delete ev;
	BOOST_CHECK_EQUAL(validMessages, 1);
}

/** 
 * @brief tests the ability of sending correct messages using a manual 
 * encoding scheme 
 */
BOOST_FIXTURE_TEST_CASE( test_publish_ASN1_UDP_manual_enc_1, ASN1UDPFixture )
{

	// Set up configuration
	config.put("0.encoding", "REAL");
	config.put("1.encoding", "DINT"); // Still default
	config.put("2.encoding", "BOOL"); // Still default
	config.put("3.encoding", "STRING"); // Still default

	// Set-up event values
	std::vector<Timing::Event::Variable> vars;
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) 0.3 ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MAX ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiFalse ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("") ));

	// Set-up reference
	uint8_t ref[] = {0x4a, 0x3e,0x99,0x99,0x9a,
									 0x44, 0x7F,0xFF,0xFF,0xFF,
									 0x40, 
									 0x50,0x00,0x00};
	receiveReference.assign(ref, ref+sizeof(ref));
	// Init publisher
	publisher.init(config, ports);

	// Trigger event
	Timing::Event * ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 

	// Fetch the IO result
	ioService.run_one();
	
	delete ev;
	BOOST_CHECK_EQUAL(validMessages, 1);
}

/** @brief tests the ability of sending correct messages */
BOOST_FIXTURE_TEST_CASE( test_publish_ASN1_UDP_0, ASN1UDPFixture )
{

	// Set-up event values
	std::vector<Timing::Event::Variable> vars;

	vars.push_back(std::make_pair( 	// dummy
			std::make_pair(fmiTypeInteger, INT_MAX), 
			(fmiInteger) 3));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) DBL_EPSILON));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MIN));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiTrue));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("H3llö!")));
	vars.push_back(std::make_pair( // dummy
			std::make_pair(fmiTypeReal, 665), 
			(fmiReal) 43.0));
	vars.push_back(std::make_pair( // dummy
			std::make_pair(fmiTypeString, INT_MAX), 
			std::string("Nope")));
	vars.push_back(std::make_pair( // dummy
			std::make_pair(fmiTypeBoolean, INT_MAX), 
			(fmiBoolean) fmiFalse));
	
	// Set-up reference
	uint8_t ref[] = {0x4b, 0x3c,0xb0,0x00,0x00,0x00,0x00,0x00,0x00,
									 0x44, 0x80,0x00,0x00,0x00,
									 0x41, 
									 0x50,0x00,0x06,0x48,0x33,0x6c,0x6c,0xf6,0x21};
	receiveReference.assign(ref, ref+sizeof(ref));
	// Init publisher
	publisher.init(config, ports);

	// Trigger event
	Timing::Event * ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 

	// Fetch the IO result
	ioService.run_one();
	
	delete ev;
	BOOST_CHECK_EQUAL(validMessages, 1);
}

/** @brief tests the ability of sending correct messages */
BOOST_FIXTURE_TEST_CASE( test_publish_ASN1_UDP_1, ASN1UDPFixture )
{

	// Set-up event values
	std::vector<Timing::Event::Variable> vars;
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) 0.3 ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MAX ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiFalse ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("") ));

	// Set-up reference
	uint8_t ref[] = {0x4b, 0x3f,0xd3,0x33,0x33,0x33,0x33,0x33,0x33,
									 0x44, 0x7F,0xFF,0xFF,0xFF,
									 0x40, 
									 0x50,0x00,0x00};
	receiveReference.assign(ref, ref+sizeof(ref));
	// Init publisher
	publisher.init(config, ports);

	// Trigger event
	Timing::Event * ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 

	// Fetch the IO result
	ioService.run_one();
	
	delete ev;
	BOOST_CHECK_EQUAL(validMessages, 1);
}

/** @brief tests the ability of sending correct messages */
BOOST_FIXTURE_TEST_CASE( test_publish_ASN1_UDP_multi_event_0, ASN1UDPFixture )
{

	// Set-up event values
	std::vector<Timing::Event::Variable> vars;
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) 0.3 ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MAX ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiFalse ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("") ));

	// Set-up reference
	uint8_t ref[] = {0x4b, 0x3f,0xd3,0x33,0x33,0x33,0x33,0x33,0x33,
									 0x44, 0x7F,0xFF,0xFF,0xFF,
									 0x40, 
									 0x50,0x00,0x00};
	receiveReference.assign(ref, ref+sizeof(ref));
	// Init publisher
	publisher.init(config, ports);

	// Trigger first event
	Timing::Event * ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 
	delete ev;
	
	// Fetch the IO result
	ioService.run_one();
	
	vars.clear();
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiTrue));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) DBL_EPSILON));

	// Set-up reference
	uint8_t ref2[] = {0x4b, 0x3c,0xb0,0x00,0x00,0x00,0x00,0x00,0x00, // changed
									 0x44, 0x7F,0xFF,0xFF,0xFF,
									 0x41, // changed
									 0x50,0x00,0x00};
	receiveReference.assign(ref2, ref2+sizeof(ref2));

	// Trigger second event
	ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 
	delete ev;

	// Fetch the IO result
	ioService.run_one();

	BOOST_CHECK_EQUAL(validMessages, 2);
}

/** @brief tests the ability of sending correct messages */
BOOST_FIXTURE_TEST_CASE( test_publish_ASN1_UDP_multi_event_1, ASN1UDPFixture )
{

	// Set-up event values
	std::vector<Timing::Event::Variable> vars;
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeReal, 666), 
			(fmiReal) 0.3 ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MAX ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeBoolean, 0), 
			(fmiBoolean) fmiFalse ));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("") ));

	// Set-up reference
	uint8_t ref[] = {0x4b, 0x3f,0xd3,0x33,0x33,0x33,0x33,0x33,0x33,
									 0x44, 0x7F,0xFF,0xFF,0xFF,
									 0x40, 
									 0x50,0x00,0x00};
	receiveReference.assign(ref, ref+sizeof(ref));
	// Init publisher
	publisher.init(config, ports);

	// Trigger first event
	Timing::Event * ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 
	delete ev;
	
	// Fetch the IO result
	ioService.run_one();
	
	vars.clear();
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeString, 0), 
			std::string("H3llö!")));
	vars.push_back(std::make_pair(
			std::make_pair(fmiTypeInteger, 0), 
			(fmiInteger) INT_MIN));

	// Set-up reference
	uint8_t ref2[] = {0x4b, 0x3f,0xd3,0x33,0x33,0x33,0x33,0x33,0x33,
									 0x44, 0x80,0x00,0x00,0x00, // changed
									 0x40,
									 0x50,0x00,0x06,0x48,0x33,0x6c,0x6c,0xf6,0x21 /* changed */ };
	receiveReference.assign(ref2, ref2+sizeof(ref2));

	// Trigger second event
	ev = new Timing::StaticEvent(0.0, vars);
	publisher.eventTriggered(ev); 
	delete ev;

	// Fetch the IO result
	ioService.run_one();

	BOOST_CHECK_EQUAL(validMessages, 2);
}
