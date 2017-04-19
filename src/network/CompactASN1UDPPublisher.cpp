/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CompactASN1UDPPublisher.cpp
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#include "base/environment-helper.h"
#include "network/CompactASN1UDPPublisher.h"
#include "base/BaseExceptions.h"

#include <assert.h>
#include <boost/log/trivial.hpp>

using namespace FMITerminalBlock::Network;

const std::string CompactASN1UDPPublisher::PUBLISHER_ID = "CompactASN.1-UDP";
const std::string CompactASN1UDPPublisher::PROP_ADDR	= "addr";

CompactASN1UDPPublisher::CompactASN1UDPPublisher():
	service_(), destination_(), socket_(NULL)
{

}

CompactASN1UDPPublisher::~CompactASN1UDPPublisher(void)
{
	if(socket_ != NULL)
	{
		delete socket_;
	}
}

void CompactASN1UDPPublisher::init(const Base::TransmissionChannel &channel)
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
	udp::resolver resolver(service_);
	udp::resolver::query query(addr.substr(0, sepPos), addr.substr(sepPos + 1));

	// Returned iterator has at least one element -> throws boost::system::
	// system_error (subclass of std::runtime_error)
	destination_ = *resolver.resolve(query);

	if(socket_ != NULL)
	{
		delete socket_;
		socket_ = NULL;
	}

	socket_ = new udp::socket(service_);
	socket_->open(destination_.protocol());

	BOOST_LOG_TRIVIAL(trace) << "Just initialized CompactASN.1-UDP publisher sending"
		" to " << destination_.protocol().type() << ":" << addr.substr(0, sepPos) 
		<< ":" << destination_.port();
}

void
CompactASN1UDPPublisher::eventTriggered(Timing::Event * ev)
{
	assert(socket_ != NULL);

	// Update local state
	CompactASN1Publisher::eventTriggered(ev);

	std::vector<uint8_t> buffer;
	encodeASN1OutputVariables(buffer);

	//encodeValue(buffer, std::string("Ground Control to Major Tom Your" 
	//	"circuit's dead, there's something wrong"));
	
	BOOST_LOG_TRIVIAL(trace) << "Send event " << ev->toString();

	// throws boost::system_error
	size_t trSize = socket_->send_to(boost::asio::buffer(buffer), destination_);
	if(trSize != buffer.size())
	{
		BOOST_LOG_TRIVIAL(warning) << "UDP message of event at " << ev->getTime() 
			<< " only partly transferred (" << trSize << "/" << buffer.size() 
			<< " bytes)";
	}else{
		BOOST_LOG_TRIVIAL(trace) << "Compact ASN.1 message sent: " << toString(buffer);
	}
}
