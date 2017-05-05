/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file NetworkManager.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "network/NetworkManager.h"
#include "network/CompactASN1UDPPublisher.h"
#include "network/CompactASN1TCPClientPublisher.h"
#include "base/ChannelMapping.h"
#include "base/BaseExceptions.h"
#include "base/TransmissionChannel.h"

#include <assert.h>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>

using namespace FMITerminalBlock::Network;

const std::string NetworkManager::PROP_PROTOCOL = "protocol";

NetworkManager::NetworkManager(Base::ApplicationContext &context, 
				Timing::EventDispatcher &dispatcher):
	publisher_(), subscriber_()
{
	
	// Create and initialize all Publisher
	auto initPublisher = [](std::shared_ptr<Publisher> pub, 
		const Base::TransmissionChannel &chn) {
		pub->init(chn);
	};
	addChannels<Publisher>(&publisher_, context.getOutputChannelMapping(), 
		&NetworkManager::instantiatePublisher, initPublisher);

	// Create and initialize all Subscriber
	auto exceptionHandler = [this](std::exception_ptr exc) {
		handleException(exc);
	};
	auto initSubscriber = [exceptionHandler, &dispatcher](
		std::shared_ptr<Subscriber> pub, const Base::TransmissionChannel &chn) {
		pub->initAndStart(chn, dispatcher.getEventSink(), exceptionHandler);
	};
	addChannels<Subscriber>(&subscriber_, context.getInputChannelMapping(),
		&NetworkManager::instantiateSubscriber, initSubscriber);

	addListeningPublisher(dispatcher);

	// Add the exception tester
	exceptionTester_ = std::make_shared<ExceptionBomb>(*this);
	dispatcher.addEventListener(exceptionTester_.get());
}

NetworkManager::~NetworkManager()
{
	terminateSubscribers();
}

bool 
NetworkManager::hasPendingException()
{
	std::lock_guard<std::mutex> guard(pendingExceptionMutex_);
	return (bool)pendingException_;
}

void 
NetworkManager::throwPendingException()
{
	if (hasPendingException())
	{
		std::lock_guard<std::mutex> guard(pendingExceptionMutex_);
		std::exception_ptr pending = pendingException_;
		pendingException_ = std::exception_ptr();
		std::rethrow_exception(pending);
	}
}

NetworkManager::ExceptionBomb::ExceptionBomb(NetworkManager &parent): 
	parent_(parent)
{
}

void
NetworkManager::ExceptionBomb::eventTriggered(Timing::Event *)
{
	parent_.throwPendingException();
}

template<typename BaseType>
void NetworkManager::addChannels(
	std::list<std::shared_ptr<BaseType>> *destinationList,
	const Base::ChannelMapping * channels,
	std::function<std::shared_ptr<BaseType>(const std::string&)> instFct,
	std::function<void(std::shared_ptr<BaseType>, 
			const Base::TransmissionChannel&)> initFct)
{
	assert(destinationList != NULL);
	assert(channels != NULL);

	for (int i = 0; i < channels->getNumberOfChannels(); i++)
	{
		auto &channel = channels->getTransmissionChannel(i);

		// instantiate a publisher for each output channel
		boost::optional<std::string> protocol;
		protocol = channel.getChannelConfig()
			.get_optional<std::string>(PROP_PROTOCOL);
		if (!protocol)
		{
			throw Base::SystemConfigurationException("A channel's protocol "
				"identifier is not set");
		}

		std::shared_ptr<BaseType> pub = instFct(protocol.get());
		if (!pub)
		{
			throw Base::SystemConfigurationException("Unknown Protocol", 
				PROP_PROTOCOL, protocol.get());
		}
		initFct(pub, channel);
		destinationList->push_back(pub);
	}

}

std::shared_ptr<Publisher>
NetworkManager::instantiatePublisher(const std::string &id)
{

	if(id == CompactASN1UDPPublisher::PUBLISHER_ID)
	{
		return std::make_shared<CompactASN1UDPPublisher>();
	}else if(id == CompactASN1TCPClientPublisher::PUBLISHER_ID)
	{
		return std::make_shared<CompactASN1TCPClientPublisher>();
	}else{
		return std::shared_ptr<Publisher>();
	}
}

std::shared_ptr<Subscriber> 
NetworkManager::instantiateSubscriber(const std::string &id)
{
	return std::shared_ptr<Subscriber>();
}

void
NetworkManager::addListeningPublisher(Timing::EventDispatcher &dispatcher)
{
	for (auto itr = publisher_.begin(); itr != publisher_.end(); ++itr)
	{
		dispatcher.addEventListener(itr->get());
	}
}

void 
NetworkManager::terminateSubscribers()
{
	std::exception_ptr exc;
	for (auto it = subscriber_.begin(); it != subscriber_.end(); ++it)
	{
		try {
			(*it)->terminate();
		}	catch (std::exception &ex) {
			exc = std::current_exception();
			BOOST_LOG_TRIVIAL(error)
				<< "Cough an exception while terminating a subscriber: "
				<< ex.what();
		}	catch (...) {
			exc = std::current_exception();
			BOOST_LOG_TRIVIAL(error) 
				<< "Cough an exception while terminating a subscriber";
		}
	}
	subscriber_.clear();
	if ((bool) exc) std::rethrow_exception(exc);
}

void 
NetworkManager::handleException(std::exception_ptr exc)
{
	assert(exc);

	std::lock_guard<std::mutex> guard(pendingExceptionMutex_);
	pendingException_ = exc;
}
