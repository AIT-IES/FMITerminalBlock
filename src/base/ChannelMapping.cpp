/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file ChannelMapping.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "base/ChannelMapping.h"
#include "base/BaseExceptions.h"

#include <assert.h>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>

using namespace FMITerminalBlock::Base;

const std::string ChannelMapping::PROP_TYPE = "type";

ChannelMapping::ChannelMapping(PortIDDrawer &portIDSource,
	const boost::property_tree::ptree &prop) :
	variableNames_(5, std::vector<std::string>()),
	variableIDs_(5, std::vector<PortID>()), channels_(),
	portIDSource_(portIDSource)
{
	addChannels(prop);
}

const std::vector<std::string> & 
ChannelMapping::getVariableNames(FMIVariableType type) const
{
	assert(type < ((int)variableNames_.size()));
	assert(variableNames_.size() == variableIDs_.size());
	assert(variableNames_[(int)type].size() == 
		variableIDs_[(int)type].size());
	return variableNames_[(int) type];
}

std::vector<std::string> 
ChannelMapping::getAllVariableNames() const
{
	std::vector<std::string> ret;
	for (unsigned int i = 0; i < variableNames_.size(); i++)
	{
		ret.insert(ret.end(), variableNames_[i].begin(), variableNames_[i].end());
	}
	return ret;
}

const std::vector<PortID> &
ChannelMapping::getVariableIDs(FMIVariableType type) const
{
	assert(type < ((int)variableIDs_.size()));
	assert(variableNames_.size() == variableIDs_.size());
	assert(variableNames_[(int)type].size() ==
		variableIDs_[(int)type].size());
	return variableIDs_[(int)type];
}

std::vector<PortID> 
ChannelMapping::getAllVariableIDs() const
{
	std::vector<PortID> ret;
	for (unsigned int i = 0; i < variableIDs_.size(); i++)
	{
		ret.insert(ret.end(), variableIDs_[i].begin(), variableIDs_[i].end());
	}
	return ret;
}

int 
ChannelMapping::getTotalNumberOfVariables() const
{
	int ret = 0;
	for (unsigned int i = 0; i < variableIDs_.size(); i++)
	{
		ret += variableIDs_[i].size();
	}
	return ret;
}

PortID 
ChannelMapping::getPortID(const std::string &name) const
{
	assert(variableNames_.size() == variableIDs_.size());
	for (unsigned int i = 0; i < variableNames_.size(); i++)
	{
		assert(variableNames_[i].size() == variableIDs_[i].size());
		for (unsigned int j = 0; j < variableNames_[i].size(); j++)
		{
			if (variableNames_[i][j] == name) return variableIDs_[i][j];
		}
	}

	boost::format fmt("The variable \"%1%\" could not be resolved.");
	fmt % name;
	throw Base::SystemConfigurationException(fmt.str());
}

int 
ChannelMapping::getNumberOfChannels() const
{
	return channels_.size();
}

const std::vector<PortID> & 
ChannelMapping::getPorts(int channelID) const
{
	assert(channelID >= 0);
	assert(channelID < ((int)channels_.size()));
	return channels_[channelID].getPortIDs();
}

const TransmissionChannel & 
ChannelMapping::getTransmissionChannel(int channelID) const
{
	assert(channelID >= 0);
	assert(channelID < ((int)channels_.size()));
	return channels_[channelID];
}

std::string ChannelMapping::toString() const
{
	assert(variableIDs_.size() == variableNames_.size());

	std::string ret("ChannelMapping: ");

	// names
	boost::format name("name(%1%) = {%2%}, ");
	for(unsigned i = 0; i < variableNames_.size(); i++)
	{
		assert(variableIDs_[i].size() == variableNames_[i].size());

		name.clear();
		name % i;

		std::string nList;
		boost::format nListEntry("\"%1%\" (%2%,%3%)");
		for(unsigned j = 0; j < variableNames_[i].size();j++)
		{
			nListEntry.clear();
			nListEntry % variableNames_[i][j] 
			           % variableIDs_[i][j].first 
			           % variableIDs_[i][j].second;
			nList += nListEntry.str();
			if(j < (variableNames_[i].size() - 1))
			{
				nList += ", ";
			}
		}
		name %nList;
		ret += name.str();
	}
	//channels
	ret += "mapping = {";
	boost::format mp(" <t:%3%,id:%4%>->(%1%.%2%)");

	for(unsigned i = 0; i < channels_.size(); i++)
	{
		const std::vector<PortID> & portIDs = channels_[i].getPortIDs();
		for(unsigned j = 0; j < portIDs.size(); j++)
		{
			mp.clear();
			mp % i % j;
			mp % portIDs[j].first % portIDs[j].second;
			ret += mp.str();
			if(i < (channels_.size() - 1) 
				|| j < (portIDs.size() - 1))
			{
				ret += ", ";
			}
		}
	}
	ret += "}";
	return ret;
}

void ChannelMapping::addChannels(const boost::property_tree::ptree &prop)
{

	boost::format chnFormat("%1%");

	int channelNr = 0;
	boost::optional<const boost::property_tree::ptree&> channelProp;

	chnFormat % channelNr;
	while (channelProp = prop.get_child_optional(chnFormat.str()))
	{

		// Add associated variables
		TransmissionChannel channel(channelProp.get());
		addVariables(channelProp.get(), channel);
		channels_.push_back(channel);

		// Try next configuration directive
		channelNr++;
		chnFormat.clear();
		chnFormat % channelNr;
	}

}

void ChannelMapping::addVariables(const boost::property_tree::ptree &channelProp,
			TransmissionChannel &variableList)
{
	assert(variableNames_.size() >= 5);
	assert(variableIDs_.size() == variableNames_.size());

	boost::format varFormat("%1%");
	int variableNr = 0;
	boost::optional<const boost::property_tree::ptree&> variableProp;

	varFormat % variableNr;
	while(variableProp = channelProp.get_child_optional(varFormat.str()))
	{
		const std::string &name = variableProp.get().data();
		if(name.empty())
		{
			throw Base::SystemConfigurationException("At least one channel variable "
				"doesn't specify a variable name");
		}

		FMIVariableType type = (FMIVariableType) variableProp.get().get<int>(PROP_TYPE, (int) fmiTypeUnknown);
		if(((unsigned) type) >= variableNames_.size())
		{
			throw Base::SystemConfigurationException("FMI type code does not exist", 
				PROP_TYPE, variableProp.get().get<std::string>(PROP_TYPE, "4"));
		}

		PortID variableID = getID(name, type);
		if(variableID.second < 0){
			variableID = portIDSource_.getNextPortID(type);
			variableNames_[(int) type].push_back(name);
			variableIDs_[(int) type].push_back(variableID);
		}

		variableList.pushBackPort(variableID,variableProp.get());

		// Try next variable number
		variableNr++;
		varFormat.clear();
		varFormat % variableNr;
	}
}

PortID ChannelMapping::getID(const std::string &name, FMIVariableType type)
{
	assert(variableNames_.size() >= 5);
	assert(variableNames_.size() == variableIDs_.size());
	assert(variableNames_[(int)type].size() == variableIDs_[(int)type].size());
	assert(((int)type) < 5);

	for(unsigned i = 0; i < variableNames_[(int) type].size();i++)
	{
		if(name.compare(variableNames_[(int) type][i]) == 0)
			return variableIDs_[(int)type][i];
	}
	return PortID(fmiTypeUnknown, -1);
}
