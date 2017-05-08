/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file MockupPublisher.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCKTEST_NETWORK_MOCKUP_PUBLISHER
#define _FMITERMINALBLOCKTEST_NETWORK_MOCKUP_PUBLISHER

#include "network/Publisher.h"

#include <condition_variable>
#include <mutex>
#include <string>

namespace FMITerminalBlockTest {
  namespace Network {

    /**
     * @brief Mockup class which mimics a Publisher
     * @details The class records the sequence of function invocations and 
     * allows to trigger exception if certain events occur. It is configured 
     * via the TransmissionChannel. Some functions and variables are declared 
     * static in order to provide an interface outside of the NetworkManager 
     * instance.
     */
		class MockupPublisher: public FMITerminalBlock::Network::Publisher
		{
		public: 

			/** @brief The ID string of the publisher */
			static const std::string PUBLISHER_ID;

			virtual void init(
				const FMITerminalBlock::Base::TransmissionChannel &channel);
			virtual void eventTriggered(FMITerminalBlock::Timing::Event * ev);

			/** 
			 * @brief Returns the sequence ID of the last function call
			 * @details If the function was not called before, -1 will be returned
			 */
			static int getInitSequenceID();

			/** 
			 * @brief Returns the sequence ID of the last function call
			 * @details If the function was not called before, -1 will be returned
			 */
			static int getEventTriggeredSequenceID();

			/**
			 * @brief Resets all static counters and starts the sequence anew
			 */
			static void resetCounter();

		private:
			/** The primary configuration source of the object */
			const FMITerminalBlock::Base::TransmissionChannel  *config_;

			/** @brief The next event ID */
			static int nextSequenceID_;
			
			/* The sequence ids of the last function call */
			static int initSequenceID_;
			static int eventTriggeredSequenceID_;

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