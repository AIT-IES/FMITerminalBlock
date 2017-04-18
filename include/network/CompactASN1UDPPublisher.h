/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CompactASN1UDPPublisher.h
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_NETWORK_COMPACT_ASN1_UDP_PUBLISHER
#define _FMITERMINALBLOCK_NETWORK_COMPACT_ASN1_UDP_PUBLISHER

#include "network/CompactASN1Publisher.h"

namespace FMITerminalBlock 
{
	namespace Network
	{
		using namespace FMITerminalBlock;
		using namespace boost::asio::ip;

		/**
		 * @brief Provides the functionality of publishing ASN.1 messages via UDP
		 * @details The class uses it's base class to encode the compactly encoded
		 * ASN.1 messages. It will send these messages to a given UDP socket and 
		 * will ignore any reply.
		 */
		class CompactASN1UDPPublisher: public CompactASN1Publisher
		{
		public:

			/** @brief The human readable name of the publisher */
			static const std::string PUBLISHER_ID;

			/** @brief The address property's name*/
			static const std::string PROP_ADDR;

			/**
			 * @brief Creates an disconnected Publisher instance
			 */
			CompactASN1UDPPublisher(void);

			/** @brief Frees used IO resources and deletes the publisher */
			virtual ~CompactASN1UDPPublisher(void);

			/**
			 * @copydoc Publisher::init()
			 */
			virtual void init(const boost::property_tree::ptree &config, 
				const std::vector<Base::PortID> &ports);

			/**
			 * @brief Updates the output data and sends the message
			 */
			virtual void eventTriggered(Timing::Event * ev);

		private:

			/** @brief The service object used to manage the transmission */
			boost::asio::io_service service_;
			/** @brief The UDP destination end-point to transfer the data */
			udp::endpoint destination_;
			/** 
			 * @brief The socked pointer used to send messages. 
			 * @details The Socket object will be initialized on executing init().
			 */
			udp::socket * socket_;

		};

	}
}

#endif
