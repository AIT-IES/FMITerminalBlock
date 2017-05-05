/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file Subscriber.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_NETWORK_SUBSCRIBER
#define _FMITERMINALBLOCK_NETWORK_SUBSCRIBER

#include "base/TransmissionChannel.h"
#include "timing/EventSink.h"

#include <memory>
#include <functional>
#include <exception>

namespace FMITerminalBlock
{
	namespace Network
	{

		/**
		 * @brief Defines a pure virtual event source interface
		 * @details <p>A subscriber is a network entity which listens to incoming 
		 * network traffic and triggers events in case new data is available. It 
		 * may start a new thread to decouple the receiving logic from the main 
		 * program flow. After the object was created, the initialization function 
		 * will be called exactly once before the termination function is called. 
		 * On returning the termination function, the Subscriber must not deliver 
		 * any Event.</p>
		 *
		 * <p> A subscriber may try to handle transient errors without notifying 
		 * the main thread. E.g. a Subscriber is advised to try to re-connect in 
		 * case connection is lost. However, it may notify the min thread if a 
		 * permanent error is encountered. A callback function to a exception 
		 * handler is set on initialization which may be used to shutdown the 
		 * entire application. Nevertheless, the function may not trigger immediate
		 * action and the terminate() function may still be called after triggering
		 * an exception. </p>
		 */
		class Subscriber
		{
		public:

			/**
			 * @brief Frees allocated resources
			 * @details If initAndStart() was called before, terminate() will also be
			 * called before the object is destructed.
			 */
			virtual ~Subscriber() {}

			/**
			 * @brief Initializes the subscription channel and starts the operation.
			 * @details The function is called after the object is created or after 
			 * terminate() was called. As soon as the function is called, new events
			 * may be registered. If the function fails to initialize the subscriber
			 * a Base::SystemConfigurationException may be thrown.
			 * @param settings A reference to the channel configuration. The 
			 * reference remains valid until the function returns.
			 * @param eventSink A managed pointer to event sink. The pointer remains
			 * valid until the final terminate() call returns. It must be used to 
			 * register received events.
			 * @param errorCallback A callback function which may be called if an 
			 * unrecoverable error occurs. The function may be called by another 
			 * thread but may not trigger immediate action. If an error during 
			 * initialization occurs, it is advised not to call the exception 
			 * handler but to directly throw the exception. In case of faulty 
			 * initializations, a direct user feedback may not be guaranteed on 
			 * using the exception handler.
			 */
			virtual void initAndStart(
				const Base::TransmissionChannel &settings, 
				std::shared_ptr<Timing::EventSink> eventSink,
				std::function<void(std::exception_ptr)> errorCallback
			) = 0;

			/**
			 * @brief Terminates the subscription
			 * @details The function may block until the object is ready to be 
			 * destroyed regularly. It will be called after the initAndStart function
			 * is executed and indicates that no more event shall be registered.
			 */
			virtual void terminate() = 0;
		};

	}
}
#endif