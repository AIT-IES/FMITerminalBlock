/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file ConcurrentMockupSubscriber.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "ConcurrentMockupSubscriber.h"

#include <assert.h>

#include "base/BaseExceptions.h"

using namespace FMITerminalBlockTest;
using namespace FMITerminalBlockTest::Network;
using namespace FMITerminalBlock;

const std::string ConcurrentMockupSubscriber::SUBSCRIBER_ID = "ConcurrentMockupSubscriber";

int ConcurrentMockupSubscriber::nextSequenceID_ = 0;
int ConcurrentMockupSubscriber::initAndStartSequenceID_ = -1;
int ConcurrentMockupSubscriber::terminateSequenceID_ = -1;
int ConcurrentMockupSubscriber::initSequenceID_ = -1;
int ConcurrentMockupSubscriber::runSequenceID_ = -1;
int ConcurrentMockupSubscriber::terminationRequestSequenceID_ = -1;

std::mutex ConcurrentMockupSubscriber::classMutex_;

void ConcurrentMockupSubscriber::initAndStart(
  const FMITerminalBlock::Base::TransmissionChannel &settings,
  std::shared_ptr<FMITerminalBlock::Timing::EventSink> eventSink,
  std::function<void(std::exception_ptr)> errorCallback)
{
  config_ = &settings; // No need to synchronize, no thread is started yet
  incrementSequenceID(&initAndStartSequenceID_);

  if (getFlag("throwOnInitAndStart"))
  {
    throw Base::SystemConfigurationException("Triggered Exception");
  }

  ConcurrentSubscriber::initAndStart(settings, eventSink, errorCallback);
}

void ConcurrentMockupSubscriber::terminate()
{
  incrementSequenceID(&terminateSequenceID_);
  ConcurrentSubscriber::terminate();
  if (getFlag("throwOnTerminate"))
  {
    throw std::runtime_error("Triggered Exception");
  }

}

int ConcurrentMockupSubscriber::getInitAndStartSequenceID()
{
  return accessSequenceID(initAndStartSequenceID_);
}

int ConcurrentMockupSubscriber::getTerminateSequenceID()
{
  return accessSequenceID(terminateSequenceID_);
}

int ConcurrentMockupSubscriber::getInitSequenceID()
{
  return accessSequenceID(initSequenceID_);
}

int ConcurrentMockupSubscriber::getRunSequenceID()
{
  return accessSequenceID(runSequenceID_);
}

int ConcurrentMockupSubscriber::getTerminationRequestSequenceID()
{
  return accessSequenceID(terminationRequestSequenceID_);
}

void ConcurrentMockupSubscriber::resetCounter()
{
	nextSequenceID_ = 0;
	initAndStartSequenceID_ = -1;
	terminateSequenceID_ = -1;
	initSequenceID_ = -1;
	runSequenceID_ = -1;
	terminationRequestSequenceID_ = -1;
}

void ConcurrentMockupSubscriber::init(
  const FMITerminalBlock::Base::TransmissionChannel &settings,
  std::shared_ptr<FMITerminalBlock::Timing::EventSink> eventSink)
{
  incrementSequenceID(&initSequenceID_);
  if (getFlag("throwOnInit"))
  {
    throw Base::SystemConfigurationException("Triggered Exception");
  }
}

void ConcurrentMockupSubscriber::run()
{
  incrementSequenceID(&runSequenceID_);
  if (getFlag("throwOnRun"))
  {
    throw std::runtime_error("Triggered Exception");
  }

  if(getFlag("waitUntilTerminationRequest", true))
  {
    std::unique_lock<std::mutex> guard(classMutex_);
    while (!terminationRequestPending_)
    {
      terminationCondition_.wait(guard);
    }
  }
}

void ConcurrentMockupSubscriber::terminationRequest()
{
  incrementSequenceID(&terminationRequestSequenceID_);

	{
		std::unique_lock<std::mutex> guard(classMutex_);
		terminationRequestPending_ = true;
		terminationCondition_.notify_all();
		assert(isTerminationRequestPending());
	}

  if (getFlag("throwOnTerminationRequest"))
  {
    throw std::runtime_error("Triggered Error");
  }
}

int ConcurrentMockupSubscriber::accessSequenceID(const int &id)
{
  std::lock_guard<std::mutex> guard(classMutex_);
  return id;
}

void ConcurrentMockupSubscriber::incrementSequenceID(int *id)
{
  std::lock_guard<std::mutex> guard(classMutex_);
  *id = nextSequenceID_;
  nextSequenceID_++;
}

bool ConcurrentMockupSubscriber::getFlag(const std::string &name, 
  bool defaultValue)
{
  assert(config_ != NULL);
  std::lock_guard<std::mutex> guard(classMutex_);
  return config_->getChannelConfig().get<bool>(name, defaultValue);
}