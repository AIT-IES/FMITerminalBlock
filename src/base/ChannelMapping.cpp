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

const std::string ChannelMapping::PROP_OUT = "out";
const std::string ChannelMapping::PROP_TYPE = "type";

ChannelMapping::ChannelMapping(PortIDDrawer &portIDSource,
	const boost::property_tree::ptree &prop) :
	outputVariableNames_(5, std::vector<std::string>()),
	outputVariableIDs_(5, std::vector<PortID>()), outputChannels_(),
	portIDSource_(portIDSource)
{

	boost::optional<const boost::property_tree::ptree&> node = 
		prop.get_child_optional(PROP_OUT);
	if( (bool) node )
	{
		addChannels(node.get(), outputVariableNames_, outputVariableIDs_, outputChannels_);
	}

}

const std::vector<std::string> & 
ChannelMapping::getOutputVariableNames(FMIType type) const
{
	assert(type < ((int)outputVariableNames_.size()));
	assert(outputVariableNames_.size() == outputVariableIDs_.size());
	assert(outputVariableNames_[(int)type].size() == 
		outputVariableIDs_[(int)type].size());
	return outputVariableNames_[(int) type];
}

const std::vector<PortID> &
ChannelMapping::getOutputVariableIDs(FMIType type) const
{
	assert(type < ((int)outputVariableIDs_.size()));
	assert(outputVariableNames_.size() == outputVariableIDs_.size());
	assert(outputVariableNames_[(int)type].size() ==
		outputVariableIDs_[(int)type].size());
	return outputVariableIDs_[(int)type];
}

int 
ChannelMapping::getNumberOfOutputChannels() const
{
	return outputChannels_.size();
}

const std::vector<PortID> & 
ChannelMapping::getOutputPorts(int channelID) const
{
	assert(channelID >= 0);
	assert(channelID < ((int)outputChannels_.size()));
	return outputChannels_[channelID];
}

std::string ChannelMapping::toString() const
{
	assert(outputVariableIDs_.size() == outputVariableNames_.size());

	std::string ret("ChannelMapping: ");

	// names
	boost::format name("out-name(%1%) = {%2%}, ");
	for(unsigned i = 0; i < outputVariableNames_.size(); i++)
	{
		assert(outputVariableIDs_[i].size() == outputVariableNames_[i].size());

		name.clear();
		name % i;

		std::string nList;
		boost::format nListEntry("\"%1%\" (%2%,%3%)");
		for(unsigned j = 0; j < outputVariableNames_[i].size();j++)
		{
			nListEntry.clear();
			nListEntry % outputVariableNames_[i][j] 
			           % outputVariableIDs_[i][j].first 
			           % outputVariableIDs_[i][j].second;
			nList += nListEntry.str();
			if(j < (outputVariableNames_[i].size() - 1))
			{
				nList += ", ";
			}
		}
		name %nList;
		ret += name.str();
	}
	//channels
	ret += "out-mapping = {";
	boost::format mp(" <t:%3%,id:%4%>->(%1%.%2%)");

	for(unsigned i = 0; i < outputChannels_.size(); i++)
	{
		for(unsigned j = 0; j < outputChannels_[i].size(); j++)
		{
			mp.clear();
			mp % i % j;
			mp % outputChannels_[i][j].first % outputChannels_[i][j].second;
			ret += mp.str();
			if(i < (outputChannels_.size() - 1) 
				|| j < (outputChannels_[i].size() - 1))
			{
				ret += ", ";
			}
		}
	}
	ret += "}";
	return ret;
}

void ChannelMapping::addChannels(const boost::property_tree::ptree &prop, 
				std::vector<std::vector<std::string>> &nameList,
				std::vector<std::vector<PortID>> &idList,
				std::vector<std::vector<PortID>> &channelList)
{

	boost::format chnFormat("%1%");

	int channelNr = 0;
	boost::optional<const boost::property_tree::ptree&> channelProp;

	chnFormat % channelNr;
	while (channelProp = prop.get_child_optional(chnFormat.str()))
	{

		// Add associated variables
		std::vector<PortID> variables;
		addVariables(channelProp.get(), nameList, idList, variables);
		channelList.push_back(variables);

		// Try next configuration directive
		channelNr++;
		chnFormat.clear();
		chnFormat % channelNr;
	}

}

void ChannelMapping::addVariables(const boost::property_tree::ptree &channelProp, 
				std::vector<std::vector<std::string>> &nameList,
				std::vector<std::vector<PortID>> &idList,
				std::vector<PortID> &variableList)
{
	assert(nameList.size() >= 5);
	assert(idList.size() == nameList.size());

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

		FMIType type = (FMIType) variableProp.get().get<int>(PROP_TYPE, (int) fmiTypeUnknown);
		if(((unsigned) type) >= nameList.size())
		{
			throw Base::SystemConfigurationException("FMI type code does not exist", 
				PROP_TYPE, variableProp.get().get<std::string>(PROP_TYPE, "4"));
		}

		PortID variableID = getID(nameList, idList, name, type);
		if(variableID.second < 0){
			variableID = portIDSource_.getNextPortID(type);
			nameList[(int) type].push_back(name);
			idList[(int) type].push_back(variableID);
		}

		variableList.push_back(variableID);

		// Try next variable number
		variableNr++;
		varFormat.clear();
		varFormat % variableNr;
	}
}

PortID ChannelMapping::getID(
				const std::vector<std::vector<std::string>> &nameList, 
				const std::vector<std::vector<PortID>> &idList,
				const std::string &name, FMIType type)
{
	assert(nameList.size() >= 5);
	assert(nameList.size() == idList.size());
	assert(nameList[(int)type].size() == idList[(int)type].size());
	assert(((int)type) < 5);

	for(unsigned i = 0; i < nameList[(int) type].size();i++)
	{
		if(name.compare(nameList[(int) type][i]) == 0)
			return idList[(int)type][i];
	}
	return PortID(fmiTypeUnknown, -1);
}
