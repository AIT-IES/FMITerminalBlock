/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file DelayedEvent.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "timing/DelayedEvent.h"

#include <assert.h>
#include <boost/format.hpp>

using namespace FMITerminalBlock::Timing;

DelayedEvent::DelayedEvent(fmiTime newtime, Event *ev): 
	Event(newtime), event_(ev)
{
	assert(ev);
}

DelayedEvent::~DelayedEvent()
{
	delete event_;
}

std::vector<Variable> DelayedEvent::getVariables(void)
{
	assert(event_);
	return event_->getVariables();
}

std::string DelayedEvent::toString(void) const
{
	assert(event_);
	boost::format msg("DelayedEvent: t_new=%1% of event: %2%");
	msg % getTime() % event_->toString();
	return msg.str();
}
