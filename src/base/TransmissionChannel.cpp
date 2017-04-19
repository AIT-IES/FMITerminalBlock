/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file TransmissionChannel.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "base\TransmissionChannel.h"



using namespace FMITerminalBlock::Base;

TransmissionChannel::TransmissionChannel(
	const boost::property_tree::ptree &channelConfig):
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

void TransmissionChannel::pushBackPort(
	PortID id, const boost::property_tree::ptree &portConfig)
{
	assert(portConfig_.size() == portIDs_.size());
	portConfig_.push_back(&portConfig);
	portIDs_.push_back(id);
}
