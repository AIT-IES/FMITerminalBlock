/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file StaticEvent.cpp
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#include "timing/StaticEvent.h"

using namespace FMITerminalBlock::Timing;

StaticEvent::StaticEvent(fmiTime time, const std::vector<Variable> &var): 
	Event(time), var_(var)
{
	assert(Event::isValid(var));
}

std::vector<Event::Variable> 
StaticEvent::getVariables()
{
	return var_;
}

std::string 
StaticEvent::toString() const
{
	std::string ret = Event::toString();
	ret += " ";
	ret += Event::toString(var_);
	return ret;
}
