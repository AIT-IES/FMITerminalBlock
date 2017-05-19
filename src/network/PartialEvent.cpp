/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file PartialEvent.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "network/PartialEvent.h"

#include <assert.h>

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Network;


PartialEvent::PartialEvent(fmiTime time,
	const std::vector<Base::PortID> &portTemplate): 
	Timing::Event(time), portTemplate_(portTemplate), var_()
{
	var_.reserve(portTemplate.size());
}

std::vector<Timing::Event::Variable> PartialEvent::getVariables()
{
	return var_;
}

std::string PartialEvent::toString() const
{
	std::string ret = Event::toString();
	ret += Event::toString(var_);
	ret += " " + nextTemplateIndex_;
	ret += " of " + portTemplate_.size();
	ret += " variables registered";
	return ret;
}

bool PartialEvent::hasRemainingElements() const
{
	return nextTemplateIndex_ < portTemplate_.size();
}

FMIType PartialEvent::getNextPortType() const
{
	assert(hasRemainingElements());
	return portTemplate_[nextTemplateIndex_].first;
}

void PartialEvent::pushNextValue(boost::any value)
{
	assert(hasRemainingElements());
	assert(var_.size() < portTemplate_.size());

	Timing::Event::Variable nextVar;
	nextVar.first = portTemplate_[nextTemplateIndex_];
	nextVar.second = value;
	var_.push_back(nextVar);
	nextTemplateIndex_++;
}

void PartialEvent::ignoreNextValue()
{
	assert(hasRemainingElements());
	nextTemplateIndex_++;
}