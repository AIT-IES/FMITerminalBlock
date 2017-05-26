/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CompactASN1TCPPublisher.cpp
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#include "base/environment-helper.h"
#include "network/CompactASN1TCPClientPublisher.h"
#include "base/BaseExceptions.h"

#include <assert.h>
#include <boost/log/trivial.hpp>

using namespace FMITerminalBlock::Network;

const std::string CompactASN1TCPClientPublisher::PUBLISHER_ID = "CompactASN.1-TCP";
const std::string CompactASN1TCPClientPublisher::PROP_ADDR	= "addr";

CompactASN1TCPClientPublisher::CompactASN1TCPClientPublisher(): 
	service_(), socket_(NULL)
{

}

CompactASN1TCPClientPublisher::~CompactASN1TCPClientPublisher()
{
	if(socket_ != NULL)
	{
		delete socket_;
	}
}

void 
CompactASN1TCPClientPublisher::init(const Base::TransmissionChannel &channel)
{
	CompactASN1Publisher::init(channel);
	
	boost::optional<const boost::property_tree::ptree&> addrProp = 
		channel.getChannelConfig().get_child_optional(PROP_ADDR);
	if(! ((bool) addrProp))
	{
		throw Base::SystemConfigurationException("Address property of ASN.1 publisher not found");
	}
	
	std::string addr = addrProp.value().get_value<std::string>();

	size_t sepPos = addr.find(":");
	if(sepPos == std::string::npos || sepPos == (addr.size() - 1) || sepPos == 0)
	{
		throw Base::SystemConfigurationException("Invalid address format. Expected "
			"<addr>:<port>", PROP_ADDR, addr);
	}



	// Try to resolve the addresses
	tcp::resolver resolver(service_);
	tcp::resolver::query query(addr.substr(0, sepPos), addr.substr(sepPos + 1));
	tcp::resolver::iterator remoteCandidates = resolver.resolve(query);

	if(socket_ != NULL)
	{
		delete socket_;
	}
	socket_ = new tcp::socket(service_);
	boost::asio::connect(*socket_, remoteCandidates);

	BOOST_LOG_TRIVIAL(trace) << "Just initialized publishing ASN.1 TCP client"
		" connected to " << socket_->remote_endpoint().protocol().type() << ":" 
		<< addr.substr(0, sepPos) << ":" << socket_->remote_endpoint().port();
}

void CompactASN1TCPClientPublisher::eventTriggered(Timing::Event * ev)
{
	assert(socket_ != NULL);
	assert(socket_->is_open());
	assert(ev != NULL);

	// Update local state, send only if the event encodes some new variables
	if (updateOutputVariables(ev))
	{
		std::vector<uint8_t> buffer;
		encodeASN1OutputVariables(buffer);
	
		size_t trSize = socket_->send(boost::asio::buffer(buffer));
		if(trSize != buffer.size())
		{
			BOOST_LOG_TRIVIAL(warning) << "TCP message of event at " 
				<< ev->getTime() << " was only partly transferred (" << trSize << 
				"/" << buffer.size() << " bytes)";
		} else {
			BOOST_LOG_TRIVIAL(trace) << "Compact ASN.1 message sent: " 
				<< toString(buffer);
		}
	}
}
