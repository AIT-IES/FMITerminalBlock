/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file NetworkManager.cpp
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#include "network/NetworkManager.h"
#include "network/CompactASN1UDPPublisher.h"
#include "network/CompactASN1TCPClientPublisher.h"
#include "base/ChannelMapping.h"
#include "base/BaseExceptions.h"

#include <assert.h>
#include <boost/format.hpp>

using namespace FMITerminalBlock::Network;

const std::string NetworkManager::PROP_OUT_CHN = Base::ChannelMapping::PROP_OUT + ".%1%";
const std::string NetworkManager::PROP_OUT_PROTOCOL = NetworkManager::PROP_OUT_CHN + ".protocol";


NetworkManager::NetworkManager(Base::ApplicationContext &context, 
				Timing::EventDispatcher &dispatcher):
	publisher_()
{
	const Base::ChannelMapping  * channels = context.getChannelMapping();
	assert(channels != NULL);
	boost::format outName(PROP_OUT_PROTOCOL);
	boost::format outTreeName(PROP_OUT_CHN);

	for(int i = 0; i < channels->getNumberOfOutputChannels(); i++)
	{
		outName.clear();
		outName % i;
		// instantiate a publisher for each output channel
		Publisher * pub = instantiatePublisher(context.getProperty<std::string>(outName.str()));
		if(pub == NULL)
		{
			throw Base::SystemConfigurationException("Unknown Protocol", outName.str(),
				context.getProperty<std::string>(outName.str()));
		}
		publisher_.push_back(pub);
		outTreeName.clear();
		outTreeName % i;
		pub->init(context.getPropertyTree(outTreeName.str()),
			context.getChannelMapping()->getOutputPorts(i));
	}

	for(std::list<Publisher *>::iterator itr = publisher_.begin(); itr != publisher_.end(); ++itr)
	{
		dispatcher.addEventListener(*itr);
	}

}

NetworkManager::~NetworkManager()
{
	for(std::list<Publisher *>::iterator itr = publisher_.begin(); itr != publisher_.end(); ++itr)
	{
		assert(*itr != NULL);
		delete *itr;
	}
}

Publisher * 
NetworkManager::instantiatePublisher(const std::string &id)
{

	if(id == CompactASN1UDPPublisher::PUBLISHER_ID)
	{
		return new CompactASN1UDPPublisher();
	}else if(id == CompactASN1TCPClientPublisher::PUBLISHER_ID)
	{
		return new CompactASN1TCPClientPublisher();
	}else{
		return NULL;
	}
}
