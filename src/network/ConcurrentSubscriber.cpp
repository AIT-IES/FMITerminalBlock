/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file ConcurrentSubscriber.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "network/ConcurrentSubscriber.h"

#include <assert.h>

#include <boost/log/trivial.hpp>

using namespace FMITerminalBlock::Network;
using namespace FMITerminalBlock;

ConcurrentSubscriber::ConcurrentSubscriber() : subscriptionThread_(), 
	terminationRequest_(false),	objectMut_()
{
}

ConcurrentSubscriber::~ConcurrentSubscriber()
{
	if (subscriptionThread_.joinable())
	{
		BOOST_LOG_TRIVIAL(warning) << "The ConcurrentSubscriber was not "
			<< "terminated regularly. Try to terminate the subscription thread "
			<< "again.";
		terminate();
	}
}

void ConcurrentSubscriber::initAndStart(
	const Base::TransmissionChannel &settings,
	std::shared_ptr<Timing::EventSink> eventSink,
	std::function<void(std::exception_ptr)> errorCallback)
{
	std::lock_guard<std::mutex> guard(objectMut_);
	terminationRequest_ = false;
	errorCallback_ = errorCallback;

	init(settings, eventSink);

	subscriptionThread_ = std::thread(&ConcurrentSubscriber::executeRun, this);
}

void ConcurrentSubscriber::terminate()
{
	assert(subscriptionThread_.joinable());

	{
		std::lock_guard<std::mutex> guard(objectMut_);
		terminationRequest_ = true;
	}
	terminationRequest();
	subscriptionThread_.join();
}

bool ConcurrentSubscriber::isTerminationRequestPending()
{
	std::lock_guard<std::mutex> guard(objectMut_);
	return terminationRequest_;
}

void ConcurrentSubscriber::executeRun()
{
	try {
		run();
	}
	catch (std::exception &ex)
	{
		BOOST_LOG_TRIVIAL(debug) << "ConcurrentSubscriber prematurely terminated"
			<< " by throwing an exception: " << ex.what() ;
		std::lock_guard<std::mutex> guard(objectMut_);
		errorCallback_(std::current_exception());
	}
	catch (...)
	{
		std::lock_guard<std::mutex> guard(objectMut_);
		errorCallback_(std::current_exception());
	}

	// Termination request is successfully executed
	std::lock_guard<std::mutex> guard(objectMut_);
	terminationRequest_ = false;
}