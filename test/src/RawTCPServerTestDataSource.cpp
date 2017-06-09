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
#include <boost/log/trivial.hpp>

using namespace FMITerminalBlockTest::Network;
using namespace FMITerminalBlockTest;
namespace ip = boost::asio::ip;

RawTCPServerTestDataSource::RawTCPServerTestDataSource()
{
	acceptor_ = std::make_shared<ip::tcp::acceptor>(service_, 
		ip::tcp::endpoint(ip::tcp::v4(), 4242));
	socket_ = std::make_shared<ip::tcp::socket>(service_);

	// Prevents the thread from exiting since no work is scheduled yet.
	busyKeeper_ = std::unique_ptr<boost::asio::io_service::work>(
			new boost::asio::io_service::work(service_));
	ioThread_ = std::thread(&RawTCPServerTestDataSource::runIOService, this);
}

RawTCPServerTestDataSource::~RawTCPServerTestDataSource()
{
	busyKeeper_.reset();

	// Stop and join in case the program flow exited prematurely
	if (acceptor_->is_open())
		acceptor_->close();
	if (socket_->is_open())
		socket_->close();
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
	const RawTestData &buffer)
{
	std::lock_guard<std::mutex> guard(objectMutex_);
	size_t bytesSent = boost::asio::write(*socket_, 
		boost::asio::buffer(buffer.getData()));
	BOOST_CHECK_EQUAL(bytesSent, buffer.getData().size());
}

void RawTCPServerTestDataSource::preTerminateSubscriber()
{
}

void RawTCPServerTestDataSource::postTerminateSubscriber()
{
	std::lock_guard<std::mutex> guard(objectMutex_);
	socket_->close();
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
	try {
		service_.run();
	}	catch(std::exception &ex)	{ 
		BOOST_LOG_TRIVIAL(error) << "RawTCPServerTestDataSource: Received "
			<< "exception: " << ex.what();
	} catch(...) { 
		BOOST_LOG_TRIVIAL(error) << "RawTCPServerTestDataSource: Received "
			<< "unknown exception";
	}
}
