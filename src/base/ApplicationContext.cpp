/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file ApplicationContext.cpp
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#include "base/ApplicationContext.h"
#include "base/BaseExceptions.h"

#include <assert.h>
#include <stdexcept>
#include <string>

#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/log/trivial.hpp>

using namespace FMITerminalBlock::Base;

/* Constant Initializations */
const std::string ApplicationContext::PROP_PROGRAM_NAME = "app.name";
const std::string ApplicationContext::PROP_START_TIME = "app.startTime";
const std::string ApplicationContext::PROP_LOOK_AHEAD_TIME = "app.lookAheadTime";
const std::string ApplicationContext::PROP_LOOK_AHEAD_STEP_SIZE = "app.lookAheadStepSize";
const std::string ApplicationContext::PROP_INTEGRATOR_STEP_SIZE = "app.integratorStepSize";

ApplicationContext::ApplicationContext(void):
	config_(), channelMap_(NULL)
{
}


ApplicationContext::~ApplicationContext(void)
{
	if(channelMap_ != NULL)
	{
		delete channelMap_;
	}
}

void 
ApplicationContext::addCommandlineProperties(int argc, const char *argv[])
{
	if(argc <= 0 || argv[0] == NULL)
	{
		throw std::invalid_argument("The program name is not set");
	}
	config_.put(PROP_PROGRAM_NAME, argv[0]);

	// Parse commandline arguments
	for(int i = 1; i < argc;i++)
	{
		if(argv[i] == NULL)
		{
			boost::format err("Program option nr. %1$ contains an invalid reference");
			err % i;
			throw std::invalid_argument(err.str());
		}

		std::string opt(argv[i]);

		addCommandlineOption(opt, i);
	}

}

void 
ApplicationContext::addCommandlineOption(std::string &opt, int i)
{
	size_t pos = opt.find_first_of('=');
	if(pos == std::string::npos)
	{
		boost::format err("The program option nr. %1% (\"%2%\") doesn't contain an = sign");
		err % i % opt;
		throw std::invalid_argument(err.str());
	}else if(pos == 0){
		boost::format err("The program option nr. %1% (\"%2%\") doesn't contain a key");
		err % i % opt;
		throw std::invalid_argument(err.str());
	}

	std::string key = opt.substr(0,pos);
	
	if(hasProperty(key))
	{
		boost::format err("The program option nr. %1% (\"%2%\") has arleady been set with value \"%3%\"");
		err % i % opt % config_.get<std::string>(key);
		throw std::invalid_argument(err.str());
	}

	config_.put(key, opt.substr(pos + 1, opt.size() - pos - 1));

	BOOST_LOG_TRIVIAL(trace) << "Added commandline option \"" << key 
		<< "\" = \"" << config_.get<std::string>(key) << "\"";

}

void 
ApplicationContext::addSensitiveDefaultProperties(const ModelDescription * description)
{
	assert(description != NULL);

	if(!hasProperty(PROP_START_TIME) && description->hasDefaultExperiment())
	{
		fmiReal startTime, stopTime, tolerance, stepSize;
		description->getDefaultExperiment(startTime, stopTime, tolerance, stepSize);
		// Set start time
		config_.put(PROP_START_TIME, std::to_string(startTime));
		BOOST_LOG_TRIVIAL(debug) << "Set start time property " << PROP_START_TIME 
			<< " to the model's defualt value: " << startTime;
		// TODO: Set default stop time.
	}


}

double 
ApplicationContext::getPositiveDoubleProperty(const std::string &path, double def) const
{
	assert(def >= 0.0);

	double ret = 0.0;

	try
	{
		ret = getProperty<double>(path, def);
	}catch(std::invalid_argument){
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
ApplicationContext::getPositiveDoubleProperty(const std::string &path) const
{
	if(!hasProperty(path))
	{
		throw Base::SystemConfigurationException("Missing property",
			path, "");
	}
	return getPositiveDoubleProperty(path, 0.0);
}

double 
ApplicationContext::getRealPositiveDoubleProperty(const std::string &path, double def) const
{
	assert(def > 0.0);

	double ret = getPositiveDoubleProperty(path, def);
	
	if(!(ret > 0.0))
	{ // Also checks NaN
		throw Base::SystemConfigurationException("Real positve value expected",
			path,	getProperty<std::string>(path));
	}

	return ret;
}

double 
ApplicationContext::getRealPositiveDoubleProperty(const std::string &path) const
{
	if(!hasProperty(path))
	{
		throw Base::SystemConfigurationException("Missing property",
			path, "");
	}
	return getRealPositiveDoubleProperty(path, 1.0);
}

const boost::property_tree::ptree & 
ApplicationContext::getPropertyTree(const std::string &path) const
{
	if(!hasProperty(path))
	{
		throw Base::SystemConfigurationException("Missing configuration tree",
			path, "");
	}
	return config_.get_child(path);
}

bool
ApplicationContext::hasProperty(const std::string &key) const
{
	boost::optional<const boost::property_tree::ptree&> node = config_.get_child_optional(key);
	return (bool) node;
}

bool
ApplicationContext::hasProperty(const char * key) const
{
	std::string kStr(key);
	return hasProperty(kStr);
}

const ChannelMapping * ApplicationContext::getChannelMapping()
{
	if(channelMap_ == NULL)
	{
		channelMap_ = new ChannelMapping(config_);
		BOOST_LOG_TRIVIAL(debug) << "Just created " << channelMap_->toString();
	}
	return channelMap_;
}
