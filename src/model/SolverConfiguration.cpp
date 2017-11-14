/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file SolverConfiguration.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "model/SolverConfiguration.h"

#include <cmath>
#include <limits>
#include <list>

#include <boost/format.hpp>

#include "base/BaseExceptions.h"

using namespace FMITerminalBlock::Model;
using namespace FMITerminalBlock;

const std::string SolverConfiguration::PROP_FMU_DEBUGGING = "fmu.logging";
const std::string SolverConfiguration::PROP_EVENT_SEARCH_PRECISION = "app.eventSearchPrecision";
const std::string SolverConfiguration::PROP_INTEGRATOR_TYPE = "app.integration.method";
const std::string SolverConfiguration::PROP_INTEGRATION_ORDER = "app.integration.order";
const std::string SolverConfiguration::PROP_ABSOLUTE_TOLERANCE = "app.integration.absoluteTolerance";
const std::string SolverConfiguration::PROP_RELATIVE_TOLERANCE = "app.integration.relativeTolerance";

SolverConfiguration::SolverConfiguration(
	const Base::ApplicationContext& configSource)
{
	fmuDebugging_ = configSource.getProperty(PROP_FMU_DEBUGGING, false);
	eventSerachPrecision_ = configSource.getRealPositiveDoubleProperty(
		PROP_EVENT_SEARCH_PRECISION, 1e-4);
	props_ = makeIntegratorProperties(configSource);
}

std::string SolverConfiguration::getDiffString(
	const Integrator::Properties &refProps) const
{
	std::string ret("");

	ret += getDiffString(props_.abstol, refProps.abstol, PROP_ABSOLUTE_TOLERANCE);
	ret += getDiffString(props_.reltol, refProps.reltol, PROP_RELATIVE_TOLERANCE);

	if (props_.order != refProps.order)
	{
		boost::format diffMsg("%1%='%2%' is invalid. (E.g. use '%3%' instead.) ");
		diffMsg % PROP_INTEGRATION_ORDER % props_.order % refProps.order;
		ret += diffMsg.str();
	}

	if (props_.type != refProps.type)
	{
		boost::format diffMsg("%1%='%2%' is not accepted.");
		diffMsg % PROP_INTEGRATOR_TYPE % props_.name;
		ret += diffMsg.str();
	}

	return ret;
}

std::string SolverConfiguration::getDiffString(double setVal, double refVal,
	const std::string &name)
{
	if (std::isnan(setVal) != std::isnan(refVal) ||	
		!std::isnan(setVal) && setVal != refVal)
	{
		boost::format diffMsg("%1%='%2%' is invalid. (E.g. use '%3%' instead.) ");
		diffMsg % name % setVal % refVal;
		return diffMsg.str();
	}
	return "";
}

Integrator::Properties SolverConfiguration::makeIntegratorProperties(
	const Base::ApplicationContext& configSource)
{
	Integrator::Properties prop;

	// Set tolerances only if available
	if (configSource.hasProperty(PROP_ABSOLUTE_TOLERANCE))
	{
		prop.abstol = configSource.getRealPositiveDoubleProperty(
			PROP_ABSOLUTE_TOLERANCE);
	}
	if (configSource.hasProperty(PROP_RELATIVE_TOLERANCE))
	{
		prop.reltol = configSource.getRealPositiveDoubleProperty(
			PROP_RELATIVE_TOLERANCE);
	}

	prop.order = configSource.getProperty(PROP_INTEGRATION_ORDER, 0);
	if (prop.order < 0)
	{
		throw Base::SystemConfigurationException(
			"The integration order must be positive", PROP_INTEGRATION_ORDER, 
			configSource.getProperty<std::string>(PROP_INTEGRATION_ORDER));
	}

	std::string id;
	id = configSource.getProperty<std::string>(PROP_INTEGRATOR_TYPE, "dp");
	prop.type = toIntegratorType(id);
	prop.name = id;

	return prop;
}

IntegratorType SolverConfiguration::toIntegratorType(const std::string &id)
{
	if ("eu" == id)	return IntegratorType::eu;
	if ("rk" == id) return IntegratorType::rk;
	if ("abm" == id) return IntegratorType::abm;
	if ("ck" == id) return IntegratorType::ck;
	if ("dp" == id) return IntegratorType::dp;
	if ("fe" == id) return IntegratorType::fe;
	if ("bs" == id) return IntegratorType::bs;
	if ("ro" == id) return IntegratorType::ro;
#ifdef USE_SUNDIALS
	if ("bdf" == id) return IntegratorType::bdf;
	if ("abm2" == id) return IntegratorType::abm2;
#endif

	// Nothing matches
	boost::format err("Unknown integrator type '%1%'");
	err % id;
	throw Base::SystemConfigurationException(err.str());
}