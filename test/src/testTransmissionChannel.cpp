/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testTransmissionChannel.cpp
 * @brief Tests the TransmissionChannel classes
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#define BOOST_TEST_MODULE testTransmissionChannel
#include <boost/test/unit_test.hpp>

#include "base/TransmissionChannel.h"
#include "base/BaseExceptions.h"
#include "base/PortID.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/test/data/test_case.hpp>

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Base;
namespace data = boost::unit_test::data;


/** @brief Test getChannelConfig() */
BOOST_AUTO_TEST_CASE(testGetChanelConfig)
{
	boost::property_tree::ptree config;
	const std::string key("The question of live, universe and everything");
	const std::string value("how many roads must a man go down");

	config.put(key, value);

	TransmissionChannel chn(config, "channel under test");
	BOOST_CHECK_EQUAL(chn.getChannelConfig().get<std::string>(key), value);
}

/** @brief Test storing and getting port IDs and associated configuration */
BOOST_AUTO_TEST_CASE(testPortIDStorage)
{
		boost::property_tree::ptree config0;
		config0.put("bob-dylan","Robert Allen Zimmerman");
		
		boost::property_tree::ptree config1;
		config1.put("var","1");

		boost::property_tree::ptree config2;
		config2.put("var","2");

		// Populate transmission channel configuration
		TransmissionChannel chn(config0, "channel under test");
		chn.pushBackPort(PortID(fmiTypeReal, 1), config1);
		chn.pushBackPort(PortID(fmiTypeReal, 2), config2);

		BOOST_REQUIRE_EQUAL(chn.getPortConfig().size(), 2);
		BOOST_REQUIRE_EQUAL(chn.getPortIDs().size(), 2);
		
		BOOST_CHECK_EQUAL(chn.getPortConfig()[0]->get<std::string>("var"), "1");
		BOOST_CHECK_EQUAL(chn.getPortConfig()[1]->get<std::string>("var"), "2");
		BOOST_CHECK_EQUAL(chn.getPortIDs()[0].second, 1);
		BOOST_CHECK_EQUAL(chn.getPortIDs()[1].second, 2);
}

/** @brief Test implicit connection config */
BOOST_AUTO_TEST_CASE(testImplicitConnectionConfig0)
{
		boost::property_tree::ptree config;
		TransmissionChannel chn(config, "channel under test");
		BOOST_CHECK(chn.isImplicitConnection());
}

/** @brief Test implicit connection config */
BOOST_AUTO_TEST_CASE(testImplicitConnectionConfig1)
{
		boost::property_tree::ptree config;
		config.put("key", "value");

		TransmissionChannel chn(config, "channel under test");
		BOOST_CHECK(chn.isImplicitConnection());
		BOOST_CHECK_EQUAL(chn.getConnectionID(), ".channel under test");
}

/** @brief Some valid connection IDs */
const char* VALID_CONNECTION_IDS[] = {"0", " " ,"-ö bäd name-"};

/** @brief Test explicit, valid connection ID */
BOOST_DATA_TEST_CASE(testValidConnectionID, data::make(VALID_CONNECTION_IDS), 
	id)
{
		boost::property_tree::ptree config;
		config.put("connection", id);

		TransmissionChannel chn(config, "channel under test");
		BOOST_CHECK(!chn.isImplicitConnection());
		BOOST_CHECK_EQUAL(chn.getConnectionID(), id);
}


/** @brief Some invalid connection IDs */
const char* INVALID_CONNECTION_IDS[] = {"", ".", "not.ok", "nope.", ".fail"};

/** @brief Test various invalid connection IDs */
BOOST_DATA_TEST_CASE(testInvalidConnectionID, 
	data::make(INVALID_CONNECTION_IDS), id)
{
		boost::property_tree::ptree config;
		config.put("connection", id);

		TransmissionChannel chn(config, "channel under test");
		
		BOOST_CHECK_THROW(chn.getConnectionID(), SystemConfigurationException);
}
