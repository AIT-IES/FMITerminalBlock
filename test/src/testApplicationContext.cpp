/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testApplicationContext.cpp
 * @brief Tests the ApplicationContext and ChannelMapping classes
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#define BOOST_TEST_MODULE testApplicationContext
#include <boost/test/unit_test.hpp>

#include <string>
#include <stdexcept>
#include <float.h>
#include <vector>
#include <memory>

#include <boost/test/data/test_case.hpp>

#include "base/ApplicationContext.h"
#include "base/BaseExceptions.h"
#include "base/PortIDDrawer.h"
#include "PrintableFactory.h"


using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Base;
using namespace FMITerminalBlockTest::Network;
namespace data = boost::unit_test::data;

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
	const char * argv[] = {"testApplicationContext", 
		"channel.0.out-var.0=x", "channel.0.out-var.0.type=0", 
		"channel.0.out-var.1=y", "channel.0.out-var.1.type=1", 
		"channel.1.out-var.0=z", "channel.1.out-var.0.type=2", 
		"channel.1.out-var.1=w", "channel.1.out-var.1.type=3", 
		"channel.1.out-var.2=x", "channel.1.out-var.2.type=0", NULL};
	ApplicationContext context;
	context.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);
	
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
	const char * argv[] = {"testApplicationContext", 
		"channel.0.out-var.0=x", "channel.0.out-var.0.type=0", 
		"channel.0.out-var.1=y", "channel.0.out-var.1.type=1", 
		"channel.1.out-var.0=z", "channel.1.out-var.0.type=2", 
		"channel.1.out-var.1=w", "channel.1.out-var.1.type=3", 
		"channel.1.out-var.2=x", "channel.1.out-var.2.type=0", NULL};
	ApplicationContext context;
	context.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);
	
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
/** @brief Queries a channel mapping object and checks some basic properties */
BOOST_AUTO_TEST_CASE(test_get_input_channel_mapping)
{
	const char * argv[] = { "testApplicationContext", 
		"channel.0.in-var.0=x",	"channel.0.in-var.0.type=0", NULL };
	ApplicationContext context;
	context.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	const ChannelMapping * mapping = context.getInputChannelMapping();
	BOOST_REQUIRE(mapping != NULL);

	BOOST_REQUIRE_EQUAL(mapping->getNumberOfChannels(), 1);
	BOOST_REQUIRE_EQUAL(mapping->getTransmissionChannel(0).getPortIDs().size(), 1);
	BOOST_CHECK(mapping->getTransmissionChannel(0).getPortIDs()[0] ==
		std::make_pair(fmiTypeReal, 0));
	
	BOOST_REQUIRE_EQUAL(mapping->getVariableIDs(fmiTypeReal).size(), 1);
	BOOST_REQUIRE_EQUAL(mapping->getVariableNames(fmiTypeReal).size(), 1);
	BOOST_CHECK(mapping->getVariableIDs(fmiTypeReal)[0] == 
		std::make_pair(fmiTypeReal, 0));
	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeReal)[0], "x");
}

BOOST_AUTO_TEST_CASE(test_Port_id_drawer)
{
	PortIDDrawer idStore;

	for (int i = 0; i < 5; i++) {
		PortID id1 = idStore.getNextPortID((FMIVariableType) i);
		PortID id2 = idStore.getNextPortID((FMIVariableType) i);

		BOOST_CHECK_EQUAL(id1.first, id2.first);
		BOOST_CHECK_NE(id1.second, id2.second);
	}
}

BOOST_TEST_DONT_PRINT_LOG_VALUE(std::vector<const char*>);

/** @brief Vector of valid implicit connection test cases */
const std::vector<std::vector<const char*>> VALID_IMPLICIT_CONNECTION_ARGS = {
	{"testProgram",
	 "channel.0.in-var.0=x",	"channel.0.in-var.0.type=0",
	 "channel.0.addr=localhost"},
	{"testProgram",
	 "channel.0.out-var.0=y",	"channel.0.out-var.0.type=0",
	 "channel.0.addr=localhost"},
	{"testProgram",
	 "channel.0.out-var.0=y",	"channel.0.out-var.0.type=0",
	 "channel.0.in-var.0=x",	"channel.0.in-var.0.type=0",
	 "channel.0.addr=localhost"},
 	{"testProgram",
	 "channel.0.out-var.0=y",	"channel.0.out-var.0.type=0",
	 "channel.0.in-var.0=x",	"channel.0.in-var.0.type=0",
	 "channel.0.addr=localhost",
	 "connection.dummy.addr=ok"}
};

/** @brief Test valid implicit connection */
BOOST_DATA_TEST_CASE(testValidImplicitConnections, 
	data::make(VALID_IMPLICIT_CONNECTION_ARGS), args)
{
	ApplicationContext context;
	std::vector<const char*> localArgs = args;
	context.addCommandlineProperties(localArgs.size(), localArgs.data());

	auto connections = context.getConnectionConfig();
	BOOST_REQUIRE(connections);
	BOOST_REQUIRE_EQUAL(connections->count(".0"), 1);
	BOOST_REQUIRE(connections->at(".0"));
	BOOST_REQUIRE(connections->at(".0")->hasProperty("addr"));
	BOOST_CHECK_EQUAL(connections->at(".0")->getProperty<std::string>("addr"), 
		"localhost");
}

/** @brief Vector of valid explicit connection test cases */
const std::vector<std::vector<const char*>> VALID_EXPLICIT_CONNECTION_ARGS = {
	{"testProgram",
	 "channel.0.in-var.0=x",	"channel.0.in-var.0.type=0",
	 "channel.0.addr=not-used", "channel.0.connection=conspec",
	 "connection.conspec.addr=localhost"},
	{"testProgram",
	 "channel.0.out-var.0=y",	"channel.0.out-var.0.type=0",
	 "channel.0.addr=not-used", "channel.0.connection=conspec",
	 "connection.conspec.addr=localhost"},
	{"testProgram",
	 "channel.0.out-var.0=y",	"channel.0.out-var.0.type=0",
	 "channel.0.in-var.0=x",	"channel.0.in-var.0.type=0",
	 "channel.0.addr=not-used", "channel.0.connection=conspec",
	 "connection.conspec.addr=localhost"},
 	{"testProgram",
	 "channel.0.out-var.0=y",	"channel.0.out-var.0.type=0",
	 "channel.0.in-var.0=x",	"channel.0.in-var.0.type=0",
	 "channel.0.addr=not-used", "channel.0.connection=conspec",
	 "channel.1.out-var.0=y",	"channel.1.out-var.0.type=0",
	 "channel.1.in-var.0=x",	"channel.1.in-var.0.type=0",
	 "channel.1.addr=not-used", "channel.1.connection=conspec",
	 "connection.conspec.addr=localhost"}
};

/** @brief Test valid explicit connection */
BOOST_DATA_TEST_CASE(testValidExplicitConnections, 
	data::make(VALID_EXPLICIT_CONNECTION_ARGS), args)
{
	ApplicationContext context;
	std::vector<const char*> localArgs = args;
	context.addCommandlineProperties(localArgs.size(), localArgs.data());

	auto connections = context.getConnectionConfig();
	BOOST_REQUIRE(connections);
	BOOST_REQUIRE_EQUAL(connections->count("conspec"), 1);
	BOOST_REQUIRE(connections->at("conspec"));
	BOOST_REQUIRE(connections->at("conspec")->hasProperty("addr"));
	BOOST_CHECK_EQUAL(
		connections->at("conspec")->getProperty<std::string>("addr"), 
		"localhost");
}


/** @brief Vector of connection arguments which should trigger an exception */
const std::vector<std::vector<const char*>> INVALID_CONNECTION_ARGS = {
	{"testProgram",
	 "channel.0.in-var.0=x",	"channel.0.in-var.0.type=0",
	 "channel.0.connection="},
	{"testProgram",
	 "channel.0.out-var.0=y",	"channel.0.out-var.0.type=0",
	 "channel.0.connection="},
	{"testProgram",
	 "channel.0.out-var.0=y",	"channel.0.out-var.0.type=0",
	 "channel.0.in-var.0=x",	"channel.0.in-var.0.type=0",
	 "channel.0.connection="},

 	{"testProgram",
	 "channel.0.in-var.0=x",	"channel.0.in-var.0.type=0",
	 "channel.0.connection=non-existing"},
	{"testProgram",
	 "channel.0.out-var.0=y",	"channel.0.out-var.0.type=0",
	 "channel.0.connection=non-existing"},
	{"testProgram",
	 "channel.0.out-var.0=y",	"channel.0.out-var.0.type=0",
	 "channel.0.in-var.0=x",	"channel.0.in-var.0.type=0",
	 "channel.0.connection=non-existing"}
};

/** @brief Test invalid connection */
BOOST_DATA_TEST_CASE(testInvalidConnections, 
	data::make(INVALID_CONNECTION_ARGS), args)
{
	ApplicationContext context;
	std::vector<const char*> localArgs = args;
	context.addCommandlineProperties(localArgs.size(), localArgs.data());

	BOOST_CHECK_THROW(context.getConnectionConfig(), 
		SystemConfigurationException);
}

/** TODO: Test invalid (empty) connection on an input channel */
/** TODO: Test invalid (empty) connection on an output channel */
/** TODO: Test invalid reference on an input channel */
/** TODO: Test invalid reference on an output channel */