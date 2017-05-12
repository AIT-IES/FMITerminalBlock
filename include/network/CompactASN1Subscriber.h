/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CompactASN1Subscriber.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_NETWORK_COMPACT_ASN1_SUBSCRIBER
#define _FMITERMINALBLOCK_NETWORK_COMPACT_ASN1_SUBSCRIBER

#include "ConcurrentSubscriber.h"

#include <boost/asio/io_service.hpp>

namespace FMITerminalBlock
{
	namespace Network
	{
		/**
		 * @brief Handles incoming ASN.1 message processing
		 * @details <p> The class implements message parsing and message timeout 
		 * operations. Therefore, an timeout mechanism is implemented which handles
		 * partial chunks of data. A message may be split across multiple chunks. 
		 * Whenever a message is completely received or a timeout occurs, an event 
		 * is Emitted. An event may also be a partial event which does not cover 
		 * every registered port. Since the class does not expect data to be 
		 * received at once, it operates state-full. </p>
		 *
		 * <p>The class manages a boost::io_service object which handles the 
		 * asynchronous data processing and timeout mechanisms. The io_sercvice 
		 * object may be queried in order to run other services. A shutdown 
		 * mechanisms terminates pending requests and terminates the concurrent 
		 * execution.</p>
		 */
		class CompactASN1Subscriber: public ConcurrentSubscriber
		{
		public:
			/** @brief Creates an uninitialized object */
			CompactASN1Subscriber() {}

		protected:
			/**
			 * @brief Initializes the protocol handler
			 * @details The function also calls initNetwork().
			 */
			virtual void init(const Base::TransmissionChannel &settings,
				std::shared_ptr<Timing::EventSink> eventSink);

			/**
			 * @brief Requests the caller to initialize its network connections
			 * @details The caller may safely assume that all service functions of 
			 * the CompactASN1Subscriber object are ready to be used. Nevertheless, 
			 * the io service may not be started yet. The function is executed in the
			 * context of the initializing thread. However, it is guaranteed that the
			 * executing thread is not started until the function returns.
			 */
			virtual void initNetwork() = 0;

			/**
			 * @brief Executes the io service 
			 * @details The function returns after the termination request has been 
			 * confirmed by the inheriting subclass.
			 */
			virtual void run();

			/**
			 * @brief Processes an external termination request.
			 */
			virtual void terminationRequest();

			/**
			 * @brief Shuts down the network connection
			 * @details The function is called in the context of the executing 
			 * thread. I.o. no other handler will be invoked concurrently. It is 
			 * assumed that the connection is closed synchronously. After the 
			 * function returns, the service object may be closed as well.
			 */
			virtual void terminateNetworkConnection() = 0;

			/**
			 * @brief Returns a valid pointer to the managed io_service object
			 * @details The pointer may be used to perform asynchronous tasks. At 
			 * least, it remains valid and active until terminateNetworkConnection() 
			 * returns.
			 */
			boost::asio::io_service* getIOService();

			/**
			 * @brief Returns a valid reference to the channel configuration
			 * @details The pointer remains valid until the subscriber is terminated.
			 */
			const Base::TransmissionChannel* getChannelConfiguration() const;
		};
	}
}
#endif