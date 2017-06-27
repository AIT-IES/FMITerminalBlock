/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CompactASN1TCPClientSubscriber.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "network/CompactASN1TCPClientSubscriber.h"

#include <assert.h>
#include <thread>

#include <boost/asio/connect.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/log/trivial.hpp>
#include <boost/system/error_code.hpp>

#include "base/BaseExceptions.h"

using namespace FMITerminalBlock::Network;
using namespace FMITerminalBlock;

const std::string CompactASN1TCPClientSubscriber::SUBSCRIBER_ID = "CompactASN.1-TCP";
const std::string CompactASN1TCPClientSubscriber::PROP_ADDR	= "addr";
const std::string CompactASN1TCPClientSubscriber::PROP_RECON_INTERVALL	= "reconnectionInterval";
const std::string CompactASN1TCPClientSubscriber::PROP_RETRY_COUNT	= "reconnectionRetryCount";

void CompactASN1TCPClientSubscriber::initNetwork()
{
	socket_ = std::make_shared<boost::asio::ip::tcp::socket>(*getIOService());
	initConfigVariables();
	syncConnect();
	initiateAsyncReceiving();
}

void CompactASN1TCPClientSubscriber::terminateNetworkConnection()
{
	assert(socket_);
	socket_->close();
}

void CompactASN1TCPClientSubscriber::syncConnect()
{
	assert(socket_);
	assert(getIOService());

	using namespace boost::asio::ip;

	auto addr = getHostAndPortName();

	tcp::resolver::query destQuery(addr.first, addr.second);
	tcp::resolver resolver(*getIOService());
	boost::system::error_code err;

	auto addrItr = resolver.resolve(destQuery, err);
	if (err)
	{
		throw Base::SystemConfigurationException("Couldn't resolve address", 
			PROP_ADDR, addr.first + ":" + addr.second);
	}

	boost::asio::connect(*socket_, addrItr, err);
	if (err)
	{
		throw Base::SystemConfigurationException("Couldn't connect to server", 
			PROP_ADDR, addr.first + ":" + addr.second);
	}
}


void CompactASN1TCPClientSubscriber::syncReconnect()
{
	for (unsigned retry = 0; retry < reconnectionRetries_; retry++)
	{
		try {
			syncConnect();
		} catch (std::exception &ex) {
			BOOST_LOG_TRIVIAL(error) << "Could not re-connect: " << ex.what();
		}

		if (socket_->is_open())	break; 
		
		if (retry < reconnectionRetries_ - 1)
		{
			BOOST_LOG_TRIVIAL(info) << "Sleep for " << reconnectionTimeout_.count()
					<< " ms and try reconnecting again";
			std::this_thread::sleep_for(reconnectionTimeout_);
		}
	}

	if (!socket_->is_open())
	{
		throw std::runtime_error("Couldn't successfully re-connect to the "
			"TCP server");
	}

}

void CompactASN1TCPClientSubscriber::initiateAsyncReceiving()
{
	assert(socket_);
	auto buffer = prepareData();
	socket_->async_receive(buffer, boost::bind(
		&CompactASN1TCPClientSubscriber::handleReceive, this, 
		boost::asio::placeholders::error, 
		boost::asio::placeholders::bytes_transferred));
}

void CompactASN1TCPClientSubscriber::handleReceive(
	const boost::system::error_code& error,
	std::size_t bytesTransferred)
{
	commitData(bytesTransferred);
	if (!isTerminationRequestPending())
	{
		// is_open() does not necessarily reflect the connection status
		if (!socket_->is_open() || error == boost::asio::error::eof)
		{
			syncReconnect();
		}
		initiateAsyncReceiving();
	}
}

void CompactASN1TCPClientSubscriber::initConfigVariables()
{
	const boost::property_tree::ptree &config = 
		getChannelConfiguration()->getChannelConfig();
	
	reconnectionTimeout_ = std::chrono::milliseconds(
		config.get<uint64_t>(PROP_RECON_INTERVALL, 500));
	reconnectionRetries_ = config.get<uint32_t>(PROP_RETRY_COUNT, 4);
}

std::pair<std::string,std::string> 
CompactASN1TCPClientSubscriber::getHostAndPortName() const
{
	const Base::TransmissionChannel *chn = getChannelConfiguration();
	assert(chn != NULL);
	boost::optional<std::string> addr;
	addr = chn->getChannelConfig().get_optional<std::string>(PROP_ADDR);
	if (!addr)
	{
		throw Base::SystemConfigurationException("No addr property set.");
	}

	size_t colonPos = addr->find(':');
	if (colonPos == std::string::npos)
	{
		throw Base::SystemConfigurationException("Invalid address, no port "
			"separator ':' found.", PROP_ADDR, *addr);
	}

	std::string host = addr->substr(0, colonPos);
	std::string port = addr->substr(colonPos + 1, addr->length() - colonPos - 1);
	return std::make_pair(host, port);
}
