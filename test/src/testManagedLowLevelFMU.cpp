/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testManagedLowLevelFMU.cpp
 * @brief Testss the helper class interfacing FMI++ ModelManager
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#define BOOST_TEST_MODULE testManagedLowLevelFMU
#include <boost/test/unit_test.hpp>

#include <assert.h>
#include <stdexcept>
#include <memory>

#include <boost/any.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

#include <import/base/include/ModelManager.h>

#include "base/ApplicationContext.h"
#include "base/BaseExceptions.h"
#include "model/ManagedLowLevelFMU.h"

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Model;
using namespace FMITerminalBlock::Base;

namespace data = boost::unit_test::data;

/// Data set of valid paths/model identifiers/model types
const char* VALID_PATHS[] = {
	FMU_URI_PRE "zigzag2",
	FMU_URI_PRE "zigzag",
	FMU_URI_PRE "sine_standalone"
};
const char* VALID_MODEL_IDENTIFIERS[] = {
	"zigzag2",
	"zigzag",
	"sine_standalone"
};
FMUType VALID_FMU_TYPE[] = {
	FMUType::fmi_2_0_me,
	FMUType::fmi_1_0_me,
	FMUType::fmi_1_0_cs
};

bool ALL_BOOLEANS[] = {true,false}; // To ease auto test case generation

/// Tests a typical usage cycle
BOOST_DATA_TEST_CASE(test_me_2_0_instantiation, 
	(data::make(VALID_PATHS)^data::make(VALID_MODEL_IDENTIFIERS)^
			data::make(VALID_FMU_TYPE))*
		(data::make(ALL_BOOLEANS)),
	path, modelIdentifier, fmuType, useAutoname)
{
	// Set up the application context and all required properties
	ApplicationContext context;
	std::string pathProperty = std::string("fmu.path=") + path;
	const char * argv[] = {"testManagedLowLevelFMU",
			pathProperty.c_str(), NULL};
	context.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	// Add a name property, if necessary
	if(!useAutoname) {
		std::string nameProperty = std::string("fmu.name=") + modelIdentifier;
		const char* nargv[] = {"testManagedLowLevelFMU", 
			nameProperty.c_str(), NULL};
		context.addCommandlineProperties(ARG_NUM_OF_ARGV(nargv), nargv);
	}

	// Start test from scratch and wipe the cache
	ModelManager &mgr = ModelManager::getModelManager();
	(void) mgr.unloadAllFMUs();

	{
		// Check the first instantiation
		std::shared_ptr<ManagedLowLevelFMU> lowLevelFMU1;
		BOOST_REQUIRE_NO_THROW(
			lowLevelFMU1 = std::make_shared<ManagedLowLevelFMU>(context)
		);
		BOOST_CHECK_EQUAL(lowLevelFMU1->getModelIdentifier(), modelIdentifier);
		BOOST_CHECK_EQUAL(lowLevelFMU1->getType(), fmuType);
		BOOST_CHECK_EQUAL(lowLevelFMU1->getPath(), path);

		// Check that the FMU cannot be unloaded
		ModelManager::UnloadFMUStatus status = mgr.unloadFMU(modelIdentifier);
		BOOST_CHECK_EQUAL(status, ModelManager::in_use);

		// Check a second instantiation
		std::shared_ptr<ManagedLowLevelFMU> lowLevelFMU2;
		BOOST_REQUIRE_NO_THROW(
			lowLevelFMU2 = std::make_shared<ManagedLowLevelFMU>(context)
		);
		BOOST_CHECK_EQUAL(lowLevelFMU2->getModelIdentifier(), modelIdentifier);
		BOOST_CHECK_EQUAL(lowLevelFMU2->getType(), fmuType);
		BOOST_CHECK_EQUAL(lowLevelFMU2->getPath(), path);
	}

	// Check that the FMU now can be unloaded
	ModelManager::UnloadFMUStatus status = mgr.unloadFMU(modelIdentifier);
	BOOST_CHECK_EQUAL(status, ModelManager::ok);
}

/// Tests applying an invalid path with the automatic name deduction option
BOOST_AUTO_TEST_CASE(test_invalid_path_autoname)
{
	// Set up the application context and all required properties
	ApplicationContext context;
	const char * argv[] = {"testManagedLowLevelFMU",
			"fmu.path=" FMU_URI_PRE "non-existing-fmu", NULL};
	context.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	// Check the exception handling
	BOOST_CHECK_THROW(ManagedLowLevelFMU m(context), std::invalid_argument);
}

/// Tests applying an invalid path without the automatic name deduction option
BOOST_AUTO_TEST_CASE(test_invalid_path_no_autoname)
{
	// Set up the application context and all required properties
	ApplicationContext context;
	const char * argv[] = {"testManagedLowLevelFMU",
			"fmu.path=" FMU_URI_PRE "non-existing-fmu", "fmu.name=zigzag", NULL};
	context.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	// Check the exception handling
	BOOST_CHECK_THROW(ManagedLowLevelFMU m(context), std::invalid_argument);
}

/// Tests applying an invalid name property
BOOST_AUTO_TEST_CASE(test_invalid_name)
{
	// Set up the application context and all required properties
	ApplicationContext context;
	const char * argv[] = {"testManagedLowLevelFMU",
			"fmu.path=" FMU_URI_PRE "zigzag", "fmu.name=line", NULL};
	context.addCommandlineProperties(ARG_NUM_OF_ARGV(argv), argv);

	// Check the exception handling
	BOOST_CHECK_THROW(ManagedLowLevelFMU m(context), std::invalid_argument);
}
