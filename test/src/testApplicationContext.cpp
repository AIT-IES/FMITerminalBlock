/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testApplicationContext.cpp
 * @brief Tests the ApplicationContext and ChannelMapping classes
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#define BOOST_TEST_MODULE testApplicationContext
#include <boost/test/unit_test.hpp>

#include "base/ApplicationContext.h"
#include "base/BaseExceptions.h"
#include "base/PortIDDrawer.h"

#include <string>
#include <stdexcept>
#include <float.h>
#include <vector>

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Base;

/** @brief Tests the command line property parsing function */
BOOST_AUTO_TEST_CASE( test_add_command_line_properties )
{
	const char * argv[] = {"testApplicationContext", "1.one.1=1", "2.two.2=2", 
		"3.thröö.3=3", NULL};
	ApplicationContext context;

	context.addCommandlineProperties(4,argv);

	BOOST_CHECK(context.hasProperty("1.one.1"));
	BOOST_CHECK(context.hasProperty("2.two.2"));
	BOOST_CHECK(context.hasProperty("3.thröö.3"));

	BOOST_CHECK_EQUAL(context.getProperty<std::string>("1.one.1"), "1");
	BOOST_CHECK_EQUAL(context.getProperty<std::string>("2.two.2"), "2");
	BOOST_CHECK_EQUAL(context.getProperty<std::string>("3.thröö.3"), "3");

}

/** @brief Tests the command line property parsing function */
BOOST_AUTO_TEST_CASE( test_add_command_line_no_value )
{
	const char * argv[] = {"testApplicationContext", "1.one.1=", "3.thröö.3=3", 
		NULL};
	ApplicationContext context;

	context.addCommandlineProperties(3,argv);
	BOOST_CHECK(context.hasProperty("1.one.1"));
	BOOST_CHECK_EQUAL(context.getProperty<std::string>("1.one.1"), "");
}

/** @brief Tests the command line property parsing function */
BOOST_AUTO_TEST_CASE( test_add_command_line_double_properties )
{
	const char * argv[] = {"testApplicationContext", "1.one.1=1", "3.thröö.3=3", 
		"3.thröö.3=3", NULL};
	ApplicationContext context;

	BOOST_CHECK_THROW(context.addCommandlineProperties(4,argv), 
		std::invalid_argument);
}

/** @brief Tests the command line property parsing function */
BOOST_AUTO_TEST_CASE( test_add_command_line_no_key )
{
	const char * argv[] = {"testApplicationContext", "=1", "3.thröö.3=3", NULL};
	ApplicationContext context;

	BOOST_CHECK_THROW(context.addCommandlineProperties(3,argv), 
		std::invalid_argument);
}


/** @brief Tests the command line property parsing function */
BOOST_AUTO_TEST_CASE( test_add_command_line_invalid_reference )
{
	const char * argv[] = {"testApplicationContext", "1.one.1=1", "3.thröö.3=3", 
		"3.thröö.3=3", NULL, NULL};
	ApplicationContext context;

	BOOST_CHECK_THROW(context.addCommandlineProperties(5,argv), 
		std::invalid_argument);
}

/** @brief Tests the command line property parsing function */
BOOST_AUTO_TEST_CASE( test_add_command_line_invalid_program_name )
{
	const char * argv[] = {NULL, "1.one.1=1", "3.thröö.3=3", "3.thröö.3=3", NULL};
	ApplicationContext context;

	BOOST_CHECK_THROW(context.addCommandlineProperties(4,argv), std::invalid_argument);
}

/** @brief Tests the hasProperty function */
BOOST_AUTO_TEST_CASE( test_has_property )
{
	const char * argv[] = {"testApplicationContext", "1.one.1=1", "2.two.2=2", 
		"3.thröö.3=3", NULL};
	ApplicationContext context;

	context.addCommandlineProperties(4,argv);

	BOOST_CHECK(context.hasProperty("1.one.1"));
	BOOST_CHECK(context.hasProperty("2.two.2"));
	BOOST_CHECK(context.hasProperty("3.thröö.3"));

	BOOST_CHECK(!context.hasProperty("nope"));
}

/** @brief Tests the getProperty function without default value */
BOOST_AUTO_TEST_CASE( test_get_property )
{
	const char * argv[] = {"testApplicationContext", "1.one.1=1", "2.two.2=two", 
		"3.thröö.3=-3", NULL};
	ApplicationContext context;

	context.addCommandlineProperties(4,argv);

	BOOST_CHECK_THROW(context.getProperty<std::string>("nope"), 
		std::invalid_argument);
	BOOST_CHECK_THROW(context.getProperty<int>("2.two.2"), std::invalid_argument);

	BOOST_CHECK_EQUAL(context.getProperty<std::string>("1.one.1"), "1");
	BOOST_CHECK_EQUAL(context.getProperty<std::string>("3.thröö.3"), "-3");
	BOOST_CHECK_EQUAL(context.getProperty<int>("3.thröö.3"), -3);
	BOOST_CHECK_EQUAL(context.getProperty<double>("3.thröö.3"), -3.0);

}

/** @brief Tests the getProperty function with a given default value */
BOOST_AUTO_TEST_CASE( test_get_property_defaults )
{
	const char * argv[] = {"testApplicationContext", "1.one.1=1", "2.two.2=two", 
		"3.thröö.3=3", NULL};
	ApplicationContext context;

	context.addCommandlineProperties(4,argv);

	BOOST_CHECK_EQUAL(context.getProperty<std::string>("1.one.1", "nope"), "1");
	BOOST_CHECK_EQUAL(context.getProperty<std::string>("3.thröö.3", "nope"), "3");
	BOOST_CHECK_EQUAL(context.getProperty<int>("3.thröö.3", -1), 3);
	BOOST_CHECK_EQUAL(context.getProperty<double>("3.thröö.3", -1.0), 3.0);

	BOOST_CHECK_EQUAL(context.getProperty<double>("nope", -1.0), -1.0);
	BOOST_CHECK_EQUAL(context.getProperty<int>("nope", -1), -1);
	BOOST_CHECK_EQUAL(context.getProperty<std::string>("nope", "-one"), "-one");
}

/** @brief Tests the getPositiveDoubleProperty function */
BOOST_AUTO_TEST_CASE( test_get_positive_double_property )
{
	const char * argv[] = {"testApplicationContext", "0.zerö.0=-0.0", 
		"0.zero.0=-2.2204460492503131e-016", "1.one.1=1", "2.two.2=two", 
		"3.thröö.3=3e-3" , NULL};
	ApplicationContext context;
	context.addCommandlineProperties(6,argv);

	// without default value
	BOOST_CHECK_THROW(context.getPositiveDoubleProperty("0.zero.0"), 
		std::invalid_argument);
	BOOST_CHECK_THROW(context.getPositiveDoubleProperty("2.two.2"), 
		std::invalid_argument);
	BOOST_CHECK_THROW(context.getPositiveDoubleProperty("nope"), 
		std::invalid_argument);

	BOOST_CHECK_EQUAL(context.getPositiveDoubleProperty("0.zerö.0"), -0.0);
	BOOST_CHECK_EQUAL(context.getPositiveDoubleProperty("1.one.1"), 1.0);
	BOOST_CHECK_EQUAL(context.getPositiveDoubleProperty("3.thröö.3"), 3e-3);

	// with default value
	BOOST_CHECK_THROW(context.getPositiveDoubleProperty("0.zero.0", 1.0),
		std::invalid_argument);
	BOOST_CHECK_THROW(context.getPositiveDoubleProperty("2.two.2",  1.0), 
		std::invalid_argument);

	BOOST_CHECK_EQUAL(context.getPositiveDoubleProperty("0.zerö.0", 0.1), -0.0);
	BOOST_CHECK_EQUAL(context.getPositiveDoubleProperty("1.one.1", 0.1), 1.0);
	BOOST_CHECK_EQUAL(context.getPositiveDoubleProperty("3.thröö.3", 0.1), 3e-3);
	BOOST_CHECK_EQUAL(context.getPositiveDoubleProperty("nope", 0.0), 0.0);
	BOOST_CHECK_EQUAL(context.getPositiveDoubleProperty("nope", 0.1), 0.1);
}

/** @brief Tests the getRealPositiveDoubleProperty function */
BOOST_AUTO_TEST_CASE( test_get_real_positive_double_property )
{
	const char * argv[] = {"testApplicationContext", "0.zerö.0=0.0", 
		"0.zero.0=-2.2204460492503131e-016", "1.one.1=2.2204460492503131e-016", 
		"2.two.2=two", "3.thröö.3=3e-3" , NULL};
	ApplicationContext context;
	context.addCommandlineProperties(6,argv);

	// without default value
	BOOST_CHECK_THROW(context.getRealPositiveDoubleProperty("0.zero.0"), 
		std::invalid_argument);
	BOOST_CHECK_THROW(context.getRealPositiveDoubleProperty("2.two.2"), 
		std::invalid_argument);
	BOOST_CHECK_THROW(context.getRealPositiveDoubleProperty("nope"), 
		std::invalid_argument);
	BOOST_CHECK_THROW(context.getRealPositiveDoubleProperty("0.zerö.0"), 
		std::invalid_argument);
	BOOST_CHECK_EQUAL(context.getRealPositiveDoubleProperty("1.one.1"), 
		DBL_EPSILON);
	BOOST_CHECK_EQUAL(context.getRealPositiveDoubleProperty("3.thröö.3"), 3e-3);

	// with default value
	BOOST_CHECK_THROW(context.getRealPositiveDoubleProperty("0.zero.0", 1.0),
		std::invalid_argument);
	BOOST_CHECK_THROW(context.getRealPositiveDoubleProperty("2.two.2",  1.0), 
		std::invalid_argument);
	BOOST_CHECK_THROW(context.getRealPositiveDoubleProperty("0.zerö.0", 0.1), 
		std::invalid_argument);
	BOOST_CHECK_EQUAL(context.getRealPositiveDoubleProperty("1.one.1", 0.1), 
		DBL_EPSILON);
	BOOST_CHECK_EQUAL(context.getRealPositiveDoubleProperty("3.thröö.3", 0.1),
		3e-3);
	BOOST_CHECK_EQUAL(context.getRealPositiveDoubleProperty("nope", DBL_EPSILON),
		DBL_EPSILON);
	BOOST_CHECK_EQUAL(context.getRealPositiveDoubleProperty("nope", 0.1), 0.1);
}

/** @brief Tests the getPropertyTree function */
BOOST_AUTO_TEST_CASE( test_get_property_tree )
{
	const char * argv[] = {"testApplicationContext", "0.0=zero", 
		"0.1.zero=zero.one", "0.1.one=one.zero", "2=two" , NULL};
	ApplicationContext context;
	context.addCommandlineProperties(5,argv);

	const boost::property_tree::ptree & tree = context.getPropertyTree("0.1");
	BOOST_CHECK_EQUAL(tree.size(), 2);
	BOOST_CHECK_EQUAL(tree.get<std::string>("zero"), "zero.one");
	BOOST_CHECK_EQUAL(tree.get<std::string>("one"), "one.zero");
	BOOST_CHECK_THROW(context.getPropertyTree("nope"), Base::SystemConfigurationException);
}

/** @brief Tests the getPropertyTree function */
BOOST_AUTO_TEST_CASE( test_get_channel_mapping_output_variables )
{
	const char * argv[] = {"testApplicationContext", "out.0.0=x", 
		"out.0.0.type=0", "out.0.1=y", "out.0.1.type=1", "out.1.0=z", 
		"out.1.0.type=2", "out.1.1=w", "out.1.1.type=3", "out.1.2=x",
		"out.1.2.type=0", NULL};
	ApplicationContext context;
	context.addCommandlineProperties(11,argv);
	
	const ChannelMapping * mapping = context.getOutputChannelMapping();
	BOOST_REQUIRE(mapping != NULL);

	// Test output variables
	std::vector<std::string> varNames;
	varNames.push_back("x");
	BOOST_CHECK_EQUAL_COLLECTIONS(
		mapping->getVariableNames(fmiTypeReal).begin(), 
		mapping->getVariableNames(fmiTypeReal).end(), 
		varNames.begin(), varNames.end());
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeReal).size(),
		mapping->getVariableNames(fmiTypeReal).size());

	varNames.clear();
	varNames.push_back("y");
	BOOST_CHECK_EQUAL_COLLECTIONS(
		mapping->getVariableNames(fmiTypeInteger).begin(), 
		mapping->getVariableNames(fmiTypeInteger).end(), 
		varNames.begin(), varNames.end());
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeInteger).size(),
		mapping->getVariableNames(fmiTypeInteger).size());

	varNames.clear();
	varNames.push_back("z");
	BOOST_CHECK_EQUAL_COLLECTIONS(
		mapping->getVariableNames(fmiTypeBoolean).begin(), 
		mapping->getVariableNames(fmiTypeBoolean).end(), 
		varNames.begin(), varNames.end());
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeBoolean).size(),
		mapping->getVariableNames(fmiTypeBoolean).size());

	varNames.clear();
	varNames.push_back("w");
	BOOST_CHECK_EQUAL_COLLECTIONS(
		mapping->getVariableNames(fmiTypeString).begin(), 
		mapping->getVariableNames(fmiTypeString).end(), 
		varNames.begin(), varNames.end());
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeString).size(),
		mapping->getVariableNames(fmiTypeString).size());

}

/** @brief Tests the getPropertyTree function */
BOOST_AUTO_TEST_CASE( test_get_channel_mapping_output_channel )
{
	const char * argv[] = {"testApplicationContext", "out.0.0=x", 
		"out.0.0.type=0", "out.0.1=y", "out.0.1.type=1", "out.1.0=z", 
		"out.1.0.type=2", "out.1.1=w", "out.1.1.type=3", "out.1.2=x",
		"out.1.2.type=0", NULL};
	ApplicationContext context;
	context.addCommandlineProperties(11,argv);
	
	const ChannelMapping * mapping = context.getOutputChannelMapping();
	BOOST_REQUIRE(mapping != NULL);

	// Test output channels
	BOOST_CHECK_EQUAL(mapping->getNumberOfChannels(), 2);
	
	BOOST_CHECK_EQUAL(mapping->getPorts(0).size(), 2);
	BOOST_CHECK_EQUAL(mapping->getPorts(0)[0].first, fmiTypeReal);
	BOOST_CHECK_EQUAL(mapping->getPorts(0)[0].second, 0);
	BOOST_CHECK_EQUAL(mapping->getPorts(0)[1].first, fmiTypeInteger);
	BOOST_CHECK_EQUAL(mapping->getPorts(0)[1].second, 0);

	BOOST_CHECK_EQUAL(mapping->getPorts(1).size(), 3);
	BOOST_CHECK_EQUAL(mapping->getPorts(1)[0].first, fmiTypeBoolean);
	BOOST_CHECK_EQUAL(mapping->getPorts(1)[0].second, 0);
	BOOST_CHECK_EQUAL(mapping->getPorts(1)[1].first, fmiTypeString);
	BOOST_CHECK_EQUAL(mapping->getPorts(1)[1].second, 0);
	BOOST_CHECK_EQUAL(mapping->getPorts(1)[2].first, fmiTypeReal);
	BOOST_CHECK_EQUAL(mapping->getPorts(1)[2].second, 0);

}

BOOST_AUTO_TEST_CASE(test_Port_id_drawer)
{
	PortIDDrawer idStore;

	for (int i = 0; i < 5; i++) {
		PortID id1 = idStore.getNextPortID((FMIType) i);
		PortID id2 = idStore.getNextPortID((FMIType) i);

		BOOST_CHECK_EQUAL(id1.first, id2.first);
		BOOST_CHECK_NE(id1.second, id2.second);
	}
}
