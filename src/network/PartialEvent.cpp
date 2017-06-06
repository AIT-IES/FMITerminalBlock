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

#include <boost/format.hpp>

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Network;


PartialEvent::PartialEvent(fmiTime time,
	const std::vector<Base::PortID> &portTemplate): 
	Timing::Event(time), portTemplate_(portTemplate), var_()
{
	var_.reserve(portTemplate.size());
}

std::vector<Timing::Variable> PartialEvent::getVariables()
{
	return var_;
}

std::string PartialEvent::toString() const
{
	boost::format strFmt("PartialEvent: %1% -- %2% of %3% variables registered");
	strFmt % Event::toString(var_) % nextTemplateIndex_;
	strFmt % portTemplate_.size();
	return strFmt.str();
}

bool PartialEvent::hasRemainingElements() const
{
	return nextTemplateIndex_ < portTemplate_.size();
}

FMIVariableType PartialEvent::getNextPortType() const
{
	assert(hasRemainingElements());
	return portTemplate_[nextTemplateIndex_].first;
}

void PartialEvent::pushNextValue(boost::any value)
{
	assert(hasRemainingElements());
	assert(var_.size() < portTemplate_.size());

	Timing::Variable nextVar(portTemplate_[nextTemplateIndex_], value);
	var_.push_back(nextVar);
	nextTemplateIndex_++;
}

void PartialEvent::ignoreNextValue()
{
	assert(hasRemainingElements());
	nextTemplateIndex_++;
}