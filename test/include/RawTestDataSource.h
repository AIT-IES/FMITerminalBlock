/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file RawTestDataSource.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCKTEST_NETWORK_RAW_TEST_DATA_SOURCE
#define _FMITERMINALBLOCKTEST_NETWORK_RAW_TEST_DATA_SOURCE

#include "RawTestData.h"

namespace FMITerminalBlockTest
{
	namespace Network
	{

		/**
		 * @brief Abstract source of test data.
		 * @details The class provides a unique interface to test subscriber 
		 * instances. Since the data source operates in a state-full manner, it is 
		 * assumed that the functions are called in the order of declaration.
		 */
		class RawTestDataSource
		{
		public:
			/** @brief Frees allocated resources */
			virtual ~RawTestDataSource() {}

			/**
			 * @brief Opens and initializes the end-point
			 * @details The function is called before the client is started. It may 
			 * asynchronously wait until a client connects.
			 */
			virtual void preInitSubscriber() = 0;
			/**
			 * @brief Waits until a client is connected or actively connects to a 
			 * client.
			 * @detials The function is executed after the subscriber is started and 
			 * before any data will be sent.
			 */
			virtual void postInitSubscriber() = 0;
			/**
			 * @brief Synchronously pushes raw data to the sink under test
			 * @details The function may be called multiple times until the 
			 * connection is closed.
			 * @param buffer a valid buffer to the data which should be sent
			 */
			virtual void pushRawData(const RawTestData &buffer) = 0;
			/**
			 * @brief Initiates closing the end point.
			 * @details The function will be invoked before the subscriber is 
			 * terminated.
			 */
			virtual void preTerminateSubscriber() = 0;
			/**
			 * @brief CLoses any connection or waits until the connections are closed
			 * @details The function is called after the subscriber is terminated.
			 */
			virtual void postTerminateSubscriber() = 0;
		};
	}
}
#endif