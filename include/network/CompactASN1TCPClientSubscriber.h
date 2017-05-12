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

namespace FMITerminalBlock
{
	namespace Network
	{
		class CompactASN1TCPClientSubscriber: public CompactASN1Subscriber
		{
		public:
			/** @brief A human readable protocol identifier */
			static const std::string SUBSCRIBER_ID;

			/** @brief Creates an uninitialized object */
			CompactASN1TCPClientSubscriber() {}

		protected:
			/** @brief Tries to connect the subscriber */
			virtual void initNetwork();

			/** @brief disconnects and closes the network connection */
			virtual void terminateNetworkConnection();
		};
	}
}
#endif