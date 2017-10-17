/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file AbstractConfigProvider.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "base/AbstractConfigProvider.h"

#include "base/BaseExceptions.h"

using namespace FMITerminalBlock::Base;

AbstractConfigProvider::AbstractConfigProvider()
{
}

double 
AbstractConfigProvider::getPositiveDoubleProperty(const std::string &path, double def) const
{
	assert(def >= 0.0);

	double ret = 0.0;

	try
	{
		ret = getProperty<double>(path, def);
	}catch(std::invalid_argument&){
		throw Base::SystemConfigurationException("The Property is not a floating point number",
			path,	getProperty<std::string>(path));
	}

	if(!(ret >= 0.0))
	{ // Also checks NaN
		throw Base::SystemConfigurationException("Non-negative value expected",
			path,	getProperty<std::string>(path));
	}

	return ret;
}

double 
AbstractConfigProvider::getPositiveDoubleProperty(const std::string &path) const
{
	if(!hasProperty(path))
	{
		throw Base::SystemConfigurationException("Missing property",
			path, "");
	}
	return getPositiveDoubleProperty(path, 0.0);
}

double 
AbstractConfigProvider::getRealPositiveDoubleProperty(const std::string &path, double def) const
{
	assert(def > 0.0);

	double ret = getPositiveDoubleProperty(path, def);
	
	if(!(ret > 0.0))
	{ // Also checks NaN
		throw Base::SystemConfigurationException("Real positive value expected",
			path,	getProperty<std::string>(path));
	}

	return ret;
}

double 
AbstractConfigProvider::getRealPositiveDoubleProperty(const std::string &path) const
{
	if(!hasProperty(path))
	{
		throw Base::SystemConfigurationException("Missing property",
			path, "");
	}
	return getRealPositiveDoubleProperty(path, 1.0);
}

const boost::property_tree::ptree & 
AbstractConfigProvider::getPropertyTree(const std::string &path) const
{
	if(!hasProperty(path))
	{
		throw Base::SystemConfigurationException("Missing configuration tree",
			path, "");
	}
	return getConfig().get_child(path);
}

bool
AbstractConfigProvider::hasProperty(const std::string &key) const
{
	boost::optional<const boost::property_tree::ptree&> node = getConfig().get_child_optional(key);
	return (bool) node;
}

bool
AbstractConfigProvider::hasProperty(const char * key) const
{
	std::string kStr(key);
	return hasProperty(kStr);
}
