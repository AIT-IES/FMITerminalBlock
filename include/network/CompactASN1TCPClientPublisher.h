/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CompactASN1TCPClientPublisher.h
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_NETWORK_COMPACT_ASN1_TCP_CLIENT_PUBLISHER
#define _FMITERMINALBLOCK_NETWORK_COMPACT_ASN1_TCP_CLIENT_PUBLISHER

#include "network/CompactASN1Publisher.h"

namespace FMITerminalBlock 
{
	namespace Network
	{
		using namespace FMITerminalBlock;
		using namespace boost::asio::ip;

		/**
		 * @brief Provides the functionality of publishing ASN.1 messages via TCP
		 * @details The class uses its base class to encode the compactly encoded
		 * ASN.1 messages. It will send these messages to a given TCP socket and 
		 * will ignore any reply. The publisher thereby acts as a client which
		 * establishes the connection during initialization.
		 */
		class CompactASN1TCPClientPublisher: public CompactASN1Publisher
		{
		public:
			/** @brief The human readable name of the publisher */
			static const std::string PUBLISHER_ID;

			/** @brief The address property's name*/
			static const std::string PROP_ADDR;

			/**
			 * @brief Creates a disconnected Publisher instance
			 */
			CompactASN1TCPClientPublisher(void);

			/** @brief Disconnects and deletes the publisher */
			virtual ~CompactASN1TCPClientPublisher(void);

			/**
			 * @copydoc Publisher::init()
			 */
			virtual void init(const Base::TransmissionChannel &channel);

		protected:
			/** @brief Sends the given data */
			virtual void sendData(const std::vector<uint8_t> &buffer);

		private:
			/** @brief The service object used to manage the transmission */
			boost::asio::io_service service_;
			/** 
			 * @brief Pointer to a connected socket
			 * @details The pointer will be populated on initialization. It is used to
			 * transport messages to the remote end point.
			 */
			tcp::socket * socket_;
		};

	}
}

#endif
