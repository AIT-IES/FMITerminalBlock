/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file LazyEvent.cpp
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#include "model/LazyEvent.h"
#include "base/BaseExceptions.h"

using namespace FMITerminalBlock::Model;
using namespace FMITerminalBlock;

LazyEvent::LazyEvent(fmiTime time, EventPredictor &predictor):
	Event(time), predictor_(predictor)
{
}

std::vector<LazyEvent::Variable> 
LazyEvent::getVariables(void)
{
	assert(predictor_.solver_ != NULL);
	if (predictor_.currentTime_ 
		> getTime() + predictor_.solver_->getTimeDiffResolution())
	{
		throw Base::SolverException("The event is outdated", getTime());
	}
	std::vector<LazyEvent::Variable> & vars = predictor_.getOutputVariables(getTime());
	assert(isValid(vars));
	return vars;
}

std::string 
LazyEvent::toString(void) const
{
	assert(predictor_.solver_ != NULL);
	std::string ret = Event::toString();
	ret += ", ";
	if(predictor_.currentTime_ 
		> getTime() + predictor_.solver_->getTimeDiffResolution())
	{
		ret += "Event is outdated";
	}else if(!predictor_.outputEventVariablesPopulated_){
		ret += "Event variables were not queried before";
	}else{ // Directly fetch variables
		const std::vector<Event::Variable> & vars = predictor_.outputEventVariables_;
		ret += Event::toString(vars);
	}

	return ret;
}
