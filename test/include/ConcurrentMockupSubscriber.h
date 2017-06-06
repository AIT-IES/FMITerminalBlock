/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file ConcurrentMockupSubscriber.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCKTEST_NETWORK_CONCURRENT_MOCKUP_SUBSCRIBER
#define _FMITERMINALBLOCKTEST_NETWORK_CONCURRENT_MOCKUP_SUBSCRIBER

#include "network/ConcurrentSubscriber.h"

#include <condition_variable>
#include <mutex>
#include <string>

namespace FMITerminalBlockTest {
  namespace Network {
    
    /**
     * @brief Simulates the operation of a subscriber
     * @details The class allows to specify several failure types which will be
     * triggered. The configuration of the subscriber is taken from the channel
     * configuration.
     * Several functions and variables are declared static in order to provide
     * a testing interface outside of the NetworkManager instance
     */
    class ConcurrentMockupSubscriber: 
      public FMITerminalBlock::Network::ConcurrentSubscriber {
    public:

      /** @brief The ID string of the subscriber */
      static const std::string SUBSCRIBER_ID;

      ConcurrentMockupSubscriber() {}

      /** @brief Records the function invocation */
      virtual void initAndStart(
        const FMITerminalBlock::Base::TransmissionChannel &settings,
        std::shared_ptr<FMITerminalBlock::Timing::EventSink> eventSink,
        std::function<void(std::exception_ptr)> errorCallback);

      /** @brief Records the function invocation */
      virtual void terminate();

      /** 
       * @brief Returns the sequence ID of the last function call
       * @details If the function was not called before, -1 will be returned
       */
      static int getInitAndStartSequenceID();
      /**
       * @brief Returns the sequence ID of the last function call
       * @details If the function was not called before, -1 will be returned
       */
      static int getTerminateSequenceID();
      /**
       * @brief Returns the sequence ID of the last function call
       * @details If the function was not called before, -1 will be returned
       */
      static int getInitSequenceID();
      /**
       * @brief Returns the sequence ID of the last function call
       * @details If the function was not called before, -1 will be returned
       */
      static int getRunSequenceID();
      /**
       * @brief Returns the sequence ID of the last function call
       * @details If the function was not called before, -1 will be returned
       */
      static int getTerminationRequestSequenceID();

			/**
			 * @brief Resets all static counters and starts the sequence anew
			 */
			static void resetCounter();

    protected:
      /** @brief Records the function invocation */
      virtual void init(
        const FMITerminalBlock::Base::TransmissionChannel &settings,
        std::shared_ptr<FMITerminalBlock::Timing::EventSink> eventSink);
      /** @brief Records the function invocation */
      virtual void run();
      /** @brief Records the function invocation */
      virtual void terminationRequest();

    private:
      /** The primary configuration source of the object */
      const FMITerminalBlock::Base::TransmissionChannel  *config_;

      /** @brief Guards all mutable variables */
      static std::mutex classMutex_;

      /** @brief The next event ID */
      static int nextSequenceID_;

      /* The sequence ids of the last function call */
      static int initAndStartSequenceID_;
      static int terminateSequenceID_;
      static int initSequenceID_;
      static int runSequenceID_;
      static int terminationRequestSequenceID_;

      /** @brief Flag indicating that there is a termination request */
      bool terminationRequestPending_ = false;
      /** @brief Terminates the thread, if the termination flag is set. */
      std::condition_variable terminationCondition_;

      /** @brief Locks the object and accesses the given pointer */
      static int accessSequenceID(const int &id);
      /** @brief Locks the object and increments the sequence id */
      static void incrementSequenceID(int *id);

      /** @brief Returns The configured flag */
      bool getFlag(const std::string &name, bool defaultValue = false);
    };

  }
}

#endif
