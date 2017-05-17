/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file RawTCPServerTestDataSource.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "base/environment-helper.h"

#include "RawTCPServerTestDataSource.h"

#include <assert.h>

#include <boost/test/test_tools.hpp>
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>

using namespace FMITerminalBlockTest::Network;
using namespace FMITerminalBlockTest;
namespace ip = boost::asio::ip;

RawTCPServerTestDataSource::RawTCPServerTestDataSource()
{
	acceptor_ = std::make_shared<ip::tcp::acceptor>(service_, 
		ip::tcp::endpoint(ip::tcp::v4(), 4242));
	socket_ = std::make_shared<ip::tcp::socket>(service_);

	ioThread_ = std::thread(&RawTCPServerTestDataSource::runIOService, this);
}

RawTCPServerTestDataSource::~RawTCPServerTestDataSource()
{
	// Stop and join in case the program flow exited prematurely
	if (!service_.stopped())
		service_.stop();
	if (ioThread_.joinable())
		ioThread_.join();
}

void RawTCPServerTestDataSource::preInitSubscriber()
{
	std::lock_guard<std::mutex> guard(objectMutex_);

	accepted_ = false;
	acceptor_->async_accept(*socket_, boost::bind(
		&RawTCPServerTestDataSource::acceptSocket, this, 
		boost::asio::placeholders::error));
}

void RawTCPServerTestDataSource::postInitSubscriber()
{
	std::unique_lock<std::mutex> guard(objectMutex_);
	while (!accepted_)
	{
		std::cv_status stat;
		stat = stateChanged_.wait_for(guard, std::chrono::milliseconds(500));
		BOOST_REQUIRE(stat != std::cv_status::timeout);
	}
}

void RawTCPServerTestDataSource::pushRawData(
	const std::vector<uint8_t> &buffer)
{
	std::lock_guard<std::mutex> guard(objectMutex_);
	size_t bytesSent = boost::asio::write(*socket_, boost::asio::buffer(buffer));
	BOOST_CHECK_EQUAL(bytesSent, buffer.size());
}

void RawTCPServerTestDataSource::preTerminateSubscriber()
{
}

void RawTCPServerTestDataSource::postTerminateSubscriber()
{
	socket_->close();
	acceptor_->close();
	service_.stop();
	ioThread_.join();
	accepted_ = false;
}

void RawTCPServerTestDataSource::acceptSocket(
	const boost::system::error_code& error)
{
	BOOST_REQUIRE(!error);
	std::lock_guard<std::mutex> guard(objectMutex_);
	accepted_ = true;
	stateChanged_.notify_all();
}

void RawTCPServerTestDataSource::runIOService()
{
	service_.run();
}