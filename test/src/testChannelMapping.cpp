/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testChannelMapping.cpp
 * @brief Tests the ChannelMapping classes
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#define BOOST_TEST_MODULE testChannelMapping
#include <boost/test/unit_test.hpp>

#include "base/ChannelMapping.h"
#include "base/BaseExceptions.h"

#include <memory>
#include <boost/property_tree/ptree.hpp>
#include <boost/log/trivial.hpp>

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Base;
using namespace boost::property_tree;

/**
 * @brief Simple fixture which stores common data
 */
struct BasicChannelMappingFixture {

	/** 
	 * @brief  The property tree object which may be used to initialize a 
	 * ChannelMapping Object
	 */
	ptree configRoot_;
	/** @brief Central source of unique type IDs */
	PortIDDrawer idSource_;
};

/**
 * @brief The fixture sets some default properties
 */
struct InitializedChannelMappingFixture: public BasicChannelMappingFixture {
	InitializedChannelMappingFixture() {
		// Channel 0
		configRoot_.put("0.addr", "An address");
		configRoot_.put("0.0.type", (int) fmiTypeReal);
		configRoot_.put("0.0", "a");
		configRoot_.put("0.0.mission", "Apollo13");

		configRoot_.put("0.1.type", (int) fmiTypeInteger);
		configRoot_.put("0.1", "b");

		configRoot_.put("0.2.type", (int) fmiTypeBoolean);
		configRoot_.put("0.2", "c");

		configRoot_.put("0.3.type", (int) fmiTypeString);
		configRoot_.put("0.3", "d");
		configRoot_.put("0.3.dest", "Moon");

		// Channel 1
		configRoot_.put("1.lunch", "At Noon");
		configRoot_.put("1.0.type", (int)fmiTypeReal);
		configRoot_.put("1.0", "a");
	};
};

/**
 * @brief Tests the exception handling if an FMI type attribute is missing
 */
BOOST_FIXTURE_TEST_CASE(testMissingType, BasicChannelMappingFixture)
{
	configRoot_.put("0.0.type", (int) fmiTypeReal);
	configRoot_.put("0.0", "a"); // OK
	configRoot_.put("0.1", "b"); // Type missing


	std::unique_ptr<ChannelMapping> mapping =
		std::unique_ptr<ChannelMapping>(new ChannelMapping(idSource_, configRoot_));
	
	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeUnknown).size(), 1);
	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeUnknown)[0], "b");

	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeUnknown).size(), 1);
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeUnknown)[0], PortID(fmiTypeUnknown, 0));
}

/**
* @brief Tests the exception handling if an FMI type attribute is missing
*/
BOOST_FIXTURE_TEST_CASE(testMissingVariableName, BasicChannelMappingFixture)
{
	configRoot_.put("0.0", "a");
	configRoot_.put("0.0.type", (int) fmiTypeReal); // OK
	configRoot_.put("0.1.type", (int) fmiTypeReal); // Type name

	BOOST_CHECK_THROW(new ChannelMapping(idSource_, configRoot_), SystemConfigurationException);
}

/** @brief Creates a channel without a variable */
BOOST_FIXTURE_TEST_CASE(testNoVariables, BasicChannelMappingFixture)
{

	configRoot_.put("0.0", "a"); // OK
	configRoot_.put("0.0.type", (int) fmiTypeReal);

	configRoot_.put("1.addr", "addr"); // OK, but no variables

	std::unique_ptr<ChannelMapping> mapping = 
		std::unique_ptr<ChannelMapping>(new ChannelMapping(idSource_, configRoot_));
	
	BOOST_CHECK_EQUAL(mapping->getNumberOfChannels(), 2);
	BOOST_CHECK_EQUAL(mapping->getPorts(0).size(), 1);
	BOOST_CHECK_EQUAL(mapping->getPorts(1).size(), 0);
}

/** @brief Invokes a ChannelMapping object with an empty property tree */
BOOST_FIXTURE_TEST_CASE(testEmptyConfig, BasicChannelMappingFixture)
{
	std::unique_ptr<ChannelMapping> mapping =
		std::unique_ptr<ChannelMapping>(new ChannelMapping(idSource_, configRoot_));

	BOOST_CHECK_EQUAL(mapping->getNumberOfChannels(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeReal).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeInteger).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeBoolean).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeString).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeUnknown).size(), 0);

	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeReal).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeInteger).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeBoolean).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeString).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeUnknown).size(), 0);

	BOOST_CHECK_EQUAL(mapping->getAllVariableIDs().size(), 0);
	BOOST_CHECK_EQUAL(mapping->getAllVariableNames().size(), 0);
}

/** @brief Creates an empty ChannelMapping Object and tests it */
BOOST_FIXTURE_TEST_CASE(testEmptyMappingCtor, BasicChannelMappingFixture)
{
	std::unique_ptr<ChannelMapping> mapping =
		std::unique_ptr<ChannelMapping>(new ChannelMapping(idSource_));

	BOOST_CHECK_EQUAL(mapping->getNumberOfChannels(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeReal).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeInteger).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeBoolean).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeString).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeUnknown).size(), 0);

	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeReal).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeInteger).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeBoolean).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeString).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeUnknown).size(), 0);

	BOOST_CHECK_EQUAL(mapping->getAllVariableIDs().size(), 0);
	BOOST_CHECK_EQUAL(mapping->getAllVariableNames().size(), 0);
}


/**
 * @brief Checks the variable list of the mapping object
 * @details Constructs a one element reference list and compares all entries.
 */
void checkVariableVector(std::shared_ptr<ChannelMapping> mapping, FMIVariableType type, std::string varName)
{

	std::vector<PortID> varIDs;
	std::vector<std::string> varNames;
	varIDs.push_back(PortID(type, 0));
	varNames.push_back(varName);

	BOOST_CHECK_EQUAL_COLLECTIONS(mapping->getVariableIDs(type).begin(),
		mapping->getVariableIDs(type).end(), varIDs.begin(), varIDs.end());
	BOOST_CHECK_EQUAL_COLLECTIONS(mapping->getVariableNames(type).begin(),
			mapping->getVariableNames(type).end(), varNames.begin(), varNames.end());
}

/** @brief Tests a standard population of the list elements */
BOOST_FIXTURE_TEST_CASE(testVariableList, InitializedChannelMappingFixture)
{
	std::shared_ptr<ChannelMapping> mapping =
		std::make_shared<ChannelMapping>(idSource_, configRoot_);

	checkVariableVector(mapping, fmiTypeReal, "a");
	checkVariableVector(mapping, fmiTypeInteger, "b");
	checkVariableVector(mapping, fmiTypeBoolean, "c");
	checkVariableVector(mapping, fmiTypeString, "d");

	BOOST_CHECK_EQUAL(mapping->getVariableNames(fmiTypeUnknown).size(), 0);
	BOOST_CHECK_EQUAL(mapping->getVariableIDs(fmiTypeUnknown).size(), 0);
}

/** @brief Tests a standard population of the list elements */
BOOST_FIXTURE_TEST_CASE(testAllVariableList, InitializedChannelMappingFixture)
{
	std::shared_ptr<ChannelMapping> mapping =
		std::make_shared<ChannelMapping>(idSource_, configRoot_);

	std::vector<PortID> allVarIDsRef = {
		PortID(fmiTypeReal, 0), PortID(fmiTypeInteger, 0), 
		PortID(fmiTypeBoolean, 0), PortID(fmiTypeString, 0)
	};
	std::vector<std::string> allVarNamesRef = {"a", "b", "c", "d"};

	auto allVarIDs = mapping->getAllVariableIDs();
	auto allVarNames = mapping->getAllVariableNames();

	BOOST_CHECK_EQUAL_COLLECTIONS(allVarIDs.begin(), allVarIDs.end(), 
		allVarIDsRef.begin(), allVarIDsRef.end());
	BOOST_CHECK_EQUAL_COLLECTIONS(allVarNames.begin(), allVarNames.end(), 
		allVarNamesRef.begin(), allVarNamesRef.end());

	BOOST_CHECK_EQUAL(mapping->getTotalNumberOfVariables(), allVarIDsRef.size());
}

/** @brief Tests the getPortID() function */
BOOST_FIXTURE_TEST_CASE(testGetPortID, InitializedChannelMappingFixture)
{
	std::shared_ptr<ChannelMapping> mapping =
		std::make_shared<ChannelMapping>(idSource_, configRoot_);

	std::vector<PortID> allVarIDsRef = {
		PortID(fmiTypeReal, 0), PortID(fmiTypeInteger, 0), 
		PortID(fmiTypeBoolean, 0), PortID(fmiTypeString, 0)
	};
	std::vector<std::string> allVarNamesRef = {"a", "b", "c", "d"};

	PortID aID = mapping->getPortID("a");
	BOOST_CHECK_EQUAL(aID.first, fmiTypeReal);
	BOOST_CHECK_EQUAL(aID.second, 0);

	PortID bID = mapping->getPortID("b");
	BOOST_CHECK_EQUAL(bID.first, fmiTypeInteger);
	BOOST_CHECK_EQUAL(bID.second, 0);

	BOOST_CHECK_THROW(mapping->getPortID("abcd"), 
		Base::SystemConfigurationException);

	PortID cID = mapping->getPortID("c");
	BOOST_CHECK_EQUAL(cID.first, fmiTypeBoolean);
	BOOST_CHECK_EQUAL(cID.second, 0);

	PortID dID = mapping->getPortID("d");
	BOOST_CHECK_EQUAL(dID.first, fmiTypeString);
	BOOST_CHECK_EQUAL(dID.second, 0);
}

/** @brief Debugging aid to present PortIDs */
std::ostream& operator<< (std::ostream& stream, const PortID& portID)
{
	stream << "(";

	switch (portID.first) {
	case fmiTypeReal:
		stream << "fmiTypeReal"; break;
	case fmiTypeInteger:
		stream << "fmiTypeInteger"; break;
	case fmiTypeBoolean:
		stream << "fmiTypeBoolean"; break;
	case fmiTypeString:
		stream << "fmiTypeString"; break;
	default:
		stream << "fmiTypeUnknown";
	}

	stream << ", " << portID.second << ")";
	return stream;
}

/** @brief Tests a standard channel mapping */
BOOST_FIXTURE_TEST_CASE(testChannelStructure, InitializedChannelMappingFixture)
{

	std::unique_ptr<ChannelMapping> mapping =
		std::unique_ptr<ChannelMapping>(new ChannelMapping(idSource_, configRoot_));

	BOOST_CHECK_EQUAL(mapping->getNumberOfChannels(), 2);

	std::vector<PortID> portReference;
	portReference.push_back(PortID(fmiTypeReal, 0));
	portReference.push_back(PortID(fmiTypeInteger, 0));
	portReference.push_back(PortID(fmiTypeBoolean, 0));
	portReference.push_back(PortID(fmiTypeString, 0));

	BOOST_CHECK_EQUAL_COLLECTIONS(mapping->getPorts(0).begin(), 
		mapping->getPorts(0).end(), portReference.begin(), 
		portReference.end());

	portReference.clear();
	portReference.push_back(PortID(fmiTypeReal, 0));
	BOOST_CHECK_EQUAL_COLLECTIONS(mapping->getPorts(1).begin(),
		mapping->getPorts(1).end(), portReference.begin(),
		portReference.end());
}

/** @brief Tests the transmission channel by a standard channel mapping */
BOOST_FIXTURE_TEST_CASE(testTransmissionChannel, InitializedChannelMappingFixture)
{

	std::unique_ptr<ChannelMapping> mapping =
		std::unique_ptr<ChannelMapping>(new ChannelMapping(idSource_, configRoot_));

	BOOST_CHECK_EQUAL(mapping->getNumberOfChannels(), 2);

	// Check Channel 0
	std::vector<PortID> portReference;
	portReference.push_back(PortID(fmiTypeReal, 0));
	portReference.push_back(PortID(fmiTypeInteger, 0));
	portReference.push_back(PortID(fmiTypeBoolean, 0));
	portReference.push_back(PortID(fmiTypeString, 0));

	const TransmissionChannel &channel0 = mapping->getTransmissionChannel(0);

	BOOST_CHECK_EQUAL_COLLECTIONS(channel0.getPortIDs().begin(),
		channel0.getPortIDs().end(), portReference.begin(),
		portReference.end());

	BOOST_CHECK_EQUAL(channel0.getPortConfig().size(),	portReference.size());
	BOOST_CHECK(channel0.getPortConfig()[0] != NULL);
	BOOST_CHECK(channel0.getPortConfig()[0]->get_child_optional("mission"));
	BOOST_CHECK_EQUAL(channel0.getPortConfig()[0]->get<std::string>("mission"), "Apollo13");

	BOOST_CHECK(channel0.getPortConfig()[3] != NULL);
	BOOST_CHECK(!channel0.getPortConfig()[3]->get_child_optional("moon"));
	BOOST_CHECK(channel0.getPortConfig()[3]->get_child_optional("dest"));
	BOOST_CHECK_EQUAL(channel0.getPortConfig()[3]->get<std::string>("dest"), "Moon");

	// Check Channel 1
	portReference.clear();
	portReference.push_back(PortID(fmiTypeReal, 0));
	const TransmissionChannel &channel1 = mapping->getTransmissionChannel(1);

	BOOST_CHECK_EQUAL_COLLECTIONS(channel1.getPortIDs().begin(),
		channel1.getPortIDs().end(), portReference.begin(),
		portReference.end());

	BOOST_CHECK_EQUAL(channel1.getPortConfig().size(),
		portReference.size());

	BOOST_CHECK(channel1.getChannelConfig().get_child_optional("lunch"));
	BOOST_CHECK_EQUAL(channel1.getChannelConfig().get<std::string>("lunch"), "At Noon");
}

