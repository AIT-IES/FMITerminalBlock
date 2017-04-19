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
#include "base/TransmissionChannel.h"

#include <assert.h>
#include <boost/format.hpp>

using namespace FMITerminalBlock::Network;

const std::string NetworkManager::PROP_PROTOCOL = "protocol";


NetworkManager::NetworkManager(Base::ApplicationContext &context, 
				Timing::EventDispatcher &dispatcher):
	publisher_()
{
	const Base::ChannelMapping  * channels = context.getOutputChannelMapping();
	assert(channels != NULL);

	for(int i = 0; i < channels->getNumberOfChannels(); i++)
	{
		const Base::TransmissionChannel &channel = channels->getTransmissionChannel(i);

		// instantiate a publisher for each output channel
		boost::optional<std::string> protocol;
		protocol = channel.getChannelConfig().get_optional<std::string>(PROP_PROTOCOL);
		if (!protocol)
		{
			throw Base::SystemConfigurationException("A channel's protocol identifier"
				" is not set");
		}

		Publisher * pub = instantiatePublisher(protocol.get());
		if(pub == NULL)
		{
			throw Base::SystemConfigurationException("Unknown Protocol", PROP_PROTOCOL,
				protocol.get());
		}
		publisher_.push_back(pub);

		pub->init(channel);
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
