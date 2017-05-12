/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CompactASN1Subscriber.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "network/CompactASN1Subscriber.h"

#include <assert.h>

using namespace FMITerminalBlock::Network;
using namespace FMITerminalBlock;


void CompactASN1Subscriber::init(const Base::TransmissionChannel &settings,
	std::shared_ptr<Timing::EventSink> eventSink)
{
	// TODO: Implement
}

void CompactASN1Subscriber::run()
{
	// TODO: Implement
}

void CompactASN1Subscriber::terminationRequest()
{
	// TODO: Implement
}

boost::asio::io_service* CompactASN1Subscriber::getIOService()
{
	return NULL;// TODO: Implement
}

const Base::TransmissionChannel* 
CompactASN1Subscriber::getChannelConfiguration() const
{
	return NULL;// TODO: Implement
}