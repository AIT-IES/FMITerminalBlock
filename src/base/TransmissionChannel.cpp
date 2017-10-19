/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file TransmissionChannel.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "base/TransmissionChannel.h"

#include <boost/format.hpp>

#include "base/BaseExceptions.h"

using namespace FMITerminalBlock::Base;

const std::string TransmissionChannel::PROP_CONNECTION = "connection";

TransmissionChannel::TransmissionChannel(
	const boost::property_tree::ptree &channelConfig,
	const std::string &channelID): channelID_(channelID), 
	channelConfig_(channelConfig)
{
}

const boost::property_tree::ptree &
TransmissionChannel::getChannelConfig() const
{
	return channelConfig_;
}

const std::vector<const boost::property_tree::ptree *> &
TransmissionChannel::getPortConfig() const
{
	assert(portConfig_.size() == portIDs_.size());
	return portConfig_;
}

const std::vector<PortID> & TransmissionChannel::getPortIDs() const
{
	assert(portConfig_.size() == portIDs_.size());
	return portIDs_;
}

const std::string TransmissionChannel::getConnectionID() const
{
	boost::optional<std::string> optConID;
	optConID = channelConfig_.get_optional<std::string>(PROP_CONNECTION);

	if(optConID) { // Explicit connection name
		if (optConID->size() <= 0){
			boost::format err("Empty connection name string in channel '%1%' found");
			err % channelID_;
			throw SystemConfigurationException(err.str(), PROP_CONNECTION, 
				optConID.get());
		}
		if (optConID->find('.') != std::string::npos)	{
			boost::format err("The connection ID at channel '%1%' must not contain "
				"a dot character");
			err % channelID_;
			throw SystemConfigurationException(err.str(), PROP_CONNECTION, 
				optConID.get());
		}
		return optConID.get();
	} else { // Implicit connection name
		return std::string(".") + channelID_;
	}
}

const bool TransmissionChannel::isImplicitConnection() const
{
	boost::optional<std::string> optConID;
	optConID = channelConfig_.get_optional<std::string>(PROP_CONNECTION);
	return !optConID;
}

void TransmissionChannel::pushBackPort(
	PortID id, const boost::property_tree::ptree &portConfig)
{
	assert(portConfig_.size() == portIDs_.size());
	portConfig_.push_back(&portConfig);
	portIDs_.push_back(id);
}
