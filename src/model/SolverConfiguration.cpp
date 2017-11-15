/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file SolverConfiguration.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "model/SolverConfiguration.h"

#include <assert.h>
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
	if (props_.name != refProps.name)
	{
		boost::format diffMsg("Name \"%1%\" was changed to \"%2%\".");
		diffMsg % props_.name % refProps.name;
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

	std::string id;
	id = configSource.getProperty<std::string>(PROP_INTEGRATOR_TYPE, "dp");
	prop.type = toIntegratorType(id);
	prop.name = getDefaultName(prop.type);

	// Default tolerances may be NaN -> Do not use them as default arguments
	if (configSource.hasProperty(PROP_ABSOLUTE_TOLERANCE)) 
	{
		prop.abstol = configSource.getRealPositiveDoubleProperty(
			PROP_ABSOLUTE_TOLERANCE);
	} 
	else 
	{
		prop.abstol = getDefaultAbsoluteTolerance(prop.type);
	}

	if (configSource.hasProperty(PROP_RELATIVE_TOLERANCE))
	{
		prop.reltol = configSource.getRealPositiveDoubleProperty(
			PROP_RELATIVE_TOLERANCE);
	} 
	else
	{
		prop.reltol = getDefaultRelativeTolerance(prop.type);
	}
	

	prop.order = configSource.getProperty(PROP_INTEGRATION_ORDER, 
		getDefaultOrder(prop.type));
	if (prop.order < 0)
	{
		throw Base::SystemConfigurationException(
			"The integration order must be positive", PROP_INTEGRATION_ORDER, 
			configSource.getProperty<std::string>(PROP_INTEGRATION_ORDER));
	}

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

std::string SolverConfiguration::getDefaultName(IntegratorType type)
{
	switch (type) {
		case IntegratorType::eu:   return "Euler";
		case IntegratorType::rk:   return "Runge Kutta";
		case IntegratorType::abm:  return "ABM";
		case IntegratorType::ck:   return "Cash Karp";
		case IntegratorType::dp:   return "Dormand Prince";
		case IntegratorType::fe:   return "Fehlberg";
		case IntegratorType::bs:   return "Bulirsch Stoer";
		case IntegratorType::ro:   return "Rosenbrock";
#ifdef USE_SUNDIALS
		case IntegratorType::bdf:  return "BDF";
		case IntegratorType::abm2: return "ABM2";
#endif
		default:
			assert(false);
			return "";
	}
}

int SolverConfiguration::getDefaultOrder(IntegratorType type)
{
	switch (type) {
		case IntegratorType::eu:   return 1;
		case IntegratorType::rk:   return 4;
		case IntegratorType::abm:  return 5;
		case IntegratorType::ck:   return 5;
		case IntegratorType::dp:   return 5;
		case IntegratorType::fe:   return 8;
		case IntegratorType::bs:   return 0; // 1-16
		case IntegratorType::ro:   return 4;
#ifdef USE_SUNDIALS
		case IntegratorType::bdf:  return 0; // 1-5;
		case IntegratorType::abm2: return 0; // 1-12;
#endif
		default:
			assert(false);
			return 0;
	}
}

double SolverConfiguration::getDefaultAbsoluteTolerance(IntegratorType type)
{
	switch (type) {
		case IntegratorType::eu: // Inf
		case IntegratorType::rk: // Inf
		case IntegratorType::abm: // Inf
			return std::numeric_limits<double>::infinity();
		case IntegratorType::ck:
		case IntegratorType::dp: // 1e-6
		case IntegratorType::fe:
		case IntegratorType::bs:
		case IntegratorType::ro: // 1e-6
#ifdef USE_SUNDIALS
		case IntegratorType::bdf:
		case IntegratorType::abm2:
#endif
			return 1e-6;
		default:
			assert(false);
			return 0.0;
	}
}

double SolverConfiguration::getDefaultRelativeTolerance(IntegratorType type)
{
	switch (type) {
		case IntegratorType::eu: // Inf
		case IntegratorType::rk: // Inf
		case IntegratorType::abm: // Inf
			return std::numeric_limits<double>::infinity();
		case IntegratorType::ck:
		case IntegratorType::dp: // 1e-6
		case IntegratorType::fe:
		case IntegratorType::bs:
		case IntegratorType::ro: // 1e-6
#ifdef USE_SUNDIALS
		case IntegratorType::bdf:
		case IntegratorType::abm2:
#endif
			return 1e-6;
		default:
			assert(false);
			return 0.0;
	}
}
