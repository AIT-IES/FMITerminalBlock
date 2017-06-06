/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CompactASN1TCPClientSubscriber.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_NETWORK_COMPACT_ASN1_TCP_CLIENTSUBSCRIBER
#define _FMITERMINALBLOCK_NETWORK_COMPACT_ASN1_TCP_CLIENTSUBSCRIBER

#include "CompactASN1Subscriber.h"

#include <memory>

#include <boost/asio/ip/tcp.hpp>

namespace FMITerminalBlock
{
	namespace Network
	{
		class CompactASN1TCPClientSubscriber: public CompactASN1Subscriber
		{
		public:
			/** @brief A human readable protocol identifier */
			static const std::string SUBSCRIBER_ID;
			/** @brief The address configuration key */
			static const std::string PROP_ADDR;
			/** @brief The reconnection interval configuration key */
			static const std::string PROP_RECON_INTERVALL;
			/** @brief The retry count configuration key */
			static const std::string PROP_RETRY_COUNT;

			/** @brief Creates an uninitialized object */
			CompactASN1TCPClientSubscriber() {}

		protected:
			/** @brief Tries to connect the subscriber */
			virtual void initNetwork();

			/** @brief disconnects and closes the network connection */
			virtual void terminateNetworkConnection();

		private:
			/** @brief The socket which is used to communicate to the server */
			std::shared_ptr<boost::asio::ip::tcp::socket> socket_;

			/** @brief The interval between two reconnection attempts */
			std::chrono::milliseconds reconnectionTimeout_;
			/** @brief The number of retry operations until an exception is thrown.*/
			uint32_t reconnectionRetries_;

			/**
			 * @brief Tries to synchronously re-connect to the registered server
			 * @details The function may throw a SystemConfigurationException if it 
			 * is unable to connect to the server. It expects the socket object 
			 * pointer to be created properly.
			 */
			void syncConnect();

			/**
			 * @brief Tries to re-connect several times
			 * @details The re-connection interval and retry count is taken from the
			 * global variables. An exception will be thrown in case the function 
			 * fails to re-connect.
			 */
			void syncReconnect();

			/**
			 * @brief Initiates an asynchronous receive operation
			 * @details The function will request a new buffer and will return 
			 * immediately.
			 */
			void initiateAsyncReceiving();

			/**
			 * @brief Processes a completed receive operation
			 * @param error The status of the operation
			 * @param bytesTransferred The number of read bytes
			 */
			void handleReceive(const boost::system::error_code& error, 
				std::size_t bytesTransferred);


			/** @param initializes the global configuration variables */
			void initConfigVariables();

			/**
			 * @brief Queries the host and port name from the configuration
			 * @details The function throws a SystemConfigurationException if an 
			 * invalid configuration is found.
			 * @return The hostname in first and the port name in second
			 */
			std::pair<std::string,std::string> getHostAndPortName() const;
		};
	}
}
#endif