/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testSolverConfiguration.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#define BOOST_TEST_MODULE testSolverConfiguration
#include <boost/test/unit_test.hpp>

#include <cmath>
#include <list>
#include <memory>

#include <boost/test/data/test_case.hpp>
#include <boost/log/trivial.hpp>

#include "model/SolverConfiguration.h"
#include "PrintableFactory.h"
#include "base/BaseExceptions.h"

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Model;
using namespace FMITerminalBlockTest;
using namespace FMITerminalBlockTest::Network;

namespace data = boost::unit_test::data;

/** @brief Test empty solver configuration (default values) */
BOOST_AUTO_TEST_CASE(test_default_configuration)
{
	Base::ApplicationContext emptyContext = {};
	SolverConfiguration config(emptyContext);

	BOOST_CHECK_EQUAL(config.getEventSearchPrecision(), 1e-4);
	BOOST_CHECK_EQUAL(config.getFMUDebuggingMode(), false);
	
	Integrator::Properties props = config.getIntegratorProperties();
	BOOST_CHECK_EQUAL(props.type, IntegratorType::dp);
	BOOST_CHECK_EQUAL(props.name, "Dormand Prince");
	BOOST_CHECK_EQUAL(props.order, 5);
	BOOST_CHECK_EQUAL(props.abstol, 1e-6);
	BOOST_CHECK_EQUAL(props.reltol, 1e-6);
}

/** @brief Test all values populated */
BOOST_AUTO_TEST_CASE(test_all_init)
{
	Base::ApplicationContext emptyContext = {
		"fmu.logging=true",
		"app.eventSearchPrecision=1e-2",
		"app.integration.method=eu",
		"app.integration.order=1",
		"app.integration.absoluteTolerance=0.001",
		"app.integration.relativeTolerance=0.005"
	};
	SolverConfiguration config(emptyContext);

	BOOST_CHECK_EQUAL(config.getEventSearchPrecision(), 1e-2);
	BOOST_CHECK_EQUAL(config.getFMUDebuggingMode(), true);
	
	Integrator::Properties props = config.getIntegratorProperties();
	BOOST_CHECK_EQUAL(props.type, IntegratorType::eu);
	BOOST_CHECK_EQUAL(props.name, "Euler");
	BOOST_CHECK_EQUAL(props.order, 1);
	BOOST_CHECK_EQUAL(props.abstol, 1e-3);
	BOOST_CHECK_EQUAL(props.reltol, 5e-3);
}

/** 
 * @brief Returns a list of invalid configurations
 * @details Each configuration must result in a 
 * Base::SystemConfigurationExcaption, when applied to a SolverConfiguration.
 */
static std::list<Base::ApplicationContext> createInvalidConfigs()
{
	std::list<Base::ApplicationContext> ret;

	ret.push_back({"fmu.logging=please log something"	});
	
	ret.push_back({"app.eventSearchPrecision=0"});
	ret.push_back({"app.eventSearchPrecision=-1e-10"});
	ret.push_back({"app.eventSearchPrecision=no-number"});

	ret.push_back({"app.integration.method=its-always-pi"});
	ret.push_back({"app.integration.method="});

	ret.push_back({"app.integration.order=-1"});
	ret.push_back({"app.integration.order=0.5"});

	ret.push_back({"app.integration.absoluteTolerance=0"});
	ret.push_back({"app.integration.absoluteTolerance=-1e-10"});
	ret.push_back({"app.integration.absoluteTolerance=no-number"});

	ret.push_back({"app.integration.relativeTolerance=0"});
	ret.push_back({"app.integration.relativeTolerance=-1e-10"});
	ret.push_back({"app.integration.relativeTolerance=no-number"});

	return ret;
}

/** @brief Test invalid configuration values */
BOOST_DATA_TEST_CASE(test_invalid_config, data::make(createInvalidConfigs()),
	appContext)
{
	try {
		std::make_shared<SolverConfiguration>(appContext);
		BOOST_CHECK(false);
	} catch (std::invalid_argument&) {
		BOOST_CHECK(true);
	}
}

/** 
 * @brief Returns a list of all valid integrator names 
 * @details The index in the list will correspond to the value in the 
 * IntegratorType enumeration
 */
static std::list<std::string> createValidIntegratorNames()
{
	std::list<std::string> ret;

	ret.push_back("eu");
	ret.push_back("rk");
	ret.push_back("abm");
	ret.push_back("ck");
	ret.push_back("dp");
	ret.push_back("fe");
	ret.push_back("bs");
	ret.push_back("ro");

#ifdef USE_SUNDIALS
	ret.push_back("bdf");
	ret.push_back("abm2");
#endif

	return ret;
}

/** @brief Test all integrator type names */
BOOST_DATA_TEST_CASE(test_integrator_names, 
	data::make(createValidIntegratorNames()) ^ 
	data::xrange((int) IntegratorType::NSTEPPERS),
	name, type)
{
	std::string propIntegratorType("app.integration.method=");
	propIntegratorType += name;
	Base::ApplicationContext context({propIntegratorType});
	
	SolverConfiguration config(context);
	BOOST_CHECK_EQUAL(config.getIntegratorProperties().type, type);
}

/** @brief Encapsulates all data of a single diff test case */
struct DiffExperiment
{
	Base::ApplicationContext config;
	Integrator::Properties refProp;
};

/**
 * @brief Returns test setups which should result in non-empty diff strings
 */
static std::list<DiffExperiment> createDiffExperiments()
{
	std::list<DiffExperiment> ret;
	DiffExperiment experiment;

	experiment.config = {};
	experiment.refProp = Integrator::Properties();
	experiment.refProp.type = IntegratorType::eu;
	ret.push_back(experiment);

	experiment.config = {};
	experiment.refProp = Integrator::Properties();
	experiment.refProp.type = IntegratorType::eu;
	experiment.refProp.name = "eu";
	ret.push_back(experiment);

	experiment.config = {"app.integration.absoluteTolerance=0.001"};
	experiment.refProp = Integrator::Properties();
	experiment.refProp.abstol = 1.0;
	ret.push_back(experiment);

	experiment.config = {"app.integration.absoluteTolerance=0.001"};
	experiment.refProp = Integrator::Properties();
	ret.push_back(experiment);

	experiment.config = {};
	experiment.refProp = Integrator::Properties();
	experiment.refProp.abstol = 1.0;
	ret.push_back(experiment);

	experiment.config = {};
	experiment.refProp = Integrator::Properties();
	experiment.refProp.reltol = 1.0;
	ret.push_back(experiment);

	experiment.config = {};
	experiment.refProp = Integrator::Properties();
	experiment.refProp.order = 1;
	ret.push_back(experiment);

	experiment.config = {"app.integration.order=2"};
	experiment.refProp = Integrator::Properties();
	ret.push_back(experiment);

	return ret;
}

BOOST_TEST_DONT_PRINT_LOG_VALUE(DiffExperiment);

/** @brief Test diff strings, one difference each */
BOOST_DATA_TEST_CASE(test_diff, data::make(createDiffExperiments()), ex)
{
	SolverConfiguration config(ex.config);
	std::string diff = config.getDiffString(ex.refProp);
	BOOST_CHECK_NE(diff, "");
	BOOST_LOG_TRIVIAL(debug) << "Got diff string: \"" << diff << "\"";
}

/** @brief Test diff strings, without a difference */
BOOST_AUTO_TEST_CASE(test_no_diff_0)
{
	Base::ApplicationContext context;
	SolverConfiguration config(context);
	std::string diff = config.getDiffString(config.getIntegratorProperties());
	BOOST_CHECK_EQUAL(diff, "");
}

/** @brief Test diff strings, without a difference */
BOOST_AUTO_TEST_CASE(test_no_diff_1)
{
	Base::ApplicationContext context = {
		"app.integration.absoluteTolerance=1e-10"
	};
	SolverConfiguration config(context);
	std::string diff = config.getDiffString(config.getIntegratorProperties());
	BOOST_CHECK_EQUAL(diff, "");
}
