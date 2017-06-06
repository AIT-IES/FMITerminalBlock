/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file RawTCPServerTestDataSource.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCKTEST_NETWORK_RAW_TCP_SERVER_TEST_DATA_SOURCE
#define _FMITERMINALBLOCKTEST_NETWORK_RAW_TCP_SERVER_TEST_DATA_SOURCE

#include "RawTestDataSource.h"

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace FMITerminalBlockTest
{
	namespace Network
	{
		/**
		 * @brief Implements a TCP/IP server which acts as a test data source
		 */
		class RawTCPServerTestDataSource: public RawTestDataSource
		{
		public:

			RawTCPServerTestDataSource();

			/** @brief Frees allocated resources */
			virtual ~RawTCPServerTestDataSource();

			/**
			 * @copydoc RawTestDataSource::preInitSubscriber()
			 */
			virtual void preInitSubscriber();
			/**
			 * @copydoc RawTestDataSource::postInitSubscriber()
			 */
			virtual void postInitSubscriber();
			/**
			 * @copydoc RawTestDataSource::pushRawData(const RawTestData &)
			 */
			virtual void pushRawData(const RawTestData &buffer);
			/**
			 * @copydoc RawTestDataSource::preTerminateSubscriber()
			 */
			virtual void preTerminateSubscriber();
			/**
			 * @copydoc RawTestDataSource::postTerminateSubscriber()
			 */
			virtual void postTerminateSubscriber();

		private:
			/** @brief The basic endpoint which handles asynchronous communication */
			boost::asio::io_service service_;

			/** @brief Keeps the io_service running while no work is to do */
			std::unique_ptr<boost::asio::io_service::work> busyKeeper_;

			/** @brief The acceptor which listens for incoming connections */
			std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
			/** @brief The socket of the first opened connection. */
			std::shared_ptr<boost::asio::ip::tcp::socket> socket_;

			/** @brief Flag which indicates that the connection has been accepted */
			bool accepted_ = false;
			/** 
			 * @brief Notifies all waiting threads that an object condition has 
			 * changed 
			 */
			std::condition_variable stateChanged_;
			/** @brief Guards all member variables */
			std::mutex objectMutex_;
			/** @brief Runs the IO service */
			std::thread ioThread_;

			/** @brief Sets the acceptance flag. and notifies any waiting thread */
			void acceptSocket(const boost::system::error_code& error);
			/** @brief Runs the IO service */
			void runIOService();
		};
	}
}
#endif