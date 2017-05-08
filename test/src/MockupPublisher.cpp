/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file MockupPublisher.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "MockupPublisher.h"

#include <assert.h>

#include "base/BaseExceptions.h"

using namespace FMITerminalBlockTest;
using namespace FMITerminalBlockTest::Network;
using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Network;

const std::string MockupPublisher::PUBLISHER_ID = "MockupPublisher";

int MockupPublisher::nextSequenceID_ = 0;

int MockupPublisher::initSequenceID_ = -1;
int MockupPublisher::eventTriggeredSequenceID_ = -1;


void MockupPublisher::init(
	const FMITerminalBlock::Base::TransmissionChannel &channel)
{
	config_ = &channel;
	incrementSequenceID(&initSequenceID_);
	if (getFlag("throwOnInit"))
	{
		throw Base::SystemConfigurationException("Triggered Exception");
	}
}

void MockupPublisher::eventTriggered(FMITerminalBlock::Timing::Event * ev)
{
	incrementSequenceID(&eventTriggeredSequenceID_);
	if (getFlag("throwOnEventTriggered"))
	{
		throw std::runtime_error("Triggered Exception");
	}
}

int MockupPublisher::getInitSequenceID()
{
	return accessSequenceID(initSequenceID_);
}

int MockupPublisher::getEventTriggeredSequenceID()
{
	return accessSequenceID(eventTriggeredSequenceID_);
}

void MockupPublisher::resetCounter()
{
	nextSequenceID_ = 0;
	initSequenceID_ = -1;
	eventTriggeredSequenceID_ = -1;
}

int MockupPublisher::accessSequenceID(const int &id)
{
	// No need to synchronize yet -> Add synchronization if concurrent 
	// publishing is supported.
	return id;
}

void MockupPublisher::incrementSequenceID(int *id)
{
	assert(id != NULL);
	*id = nextSequenceID_;
	nextSequenceID_++;
}
			
bool MockupPublisher::getFlag(const std::string &name, bool defaultValue)
{
	assert(config_ != NULL);
	return config_->getChannelConfig().get<bool>(name, defaultValue);
}