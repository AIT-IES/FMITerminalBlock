/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file ConcurrentSubscriber.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_NETWORK_CONCURRENT_SUBSCRIBER
#define _FMITERMINALBLOCK_NETWORK_CONCURRENT_SUBSCRIBER

#include "Subscriber.h"

#include <thread>
#include <mutex>

namespace FMITerminalBlock
{
	namespace Network
	{

		/**
		 * @brief Abstract Subscriber class which manages a new thread of execution
		 * @details The ConcurrentSubscriber implements the functionality of 
		 * starting and terminating a thread. It provides a run function which 
		 * remains to be implemented by any inheriting class. Hence, the main 
		 * purpose of the class is to decouple the control flow of the main program
		 * and the subscriber instance.
		 */
		class ConcurrentSubscriber : public Subscriber
		{
		public:

			/** @brief Creates a Subscriber which doesn't execute yet. */
			ConcurrentSubscriber();

			/**
			 * @brief Frees allocated resources.
			 * @details The function assumes that terminate() is called before 
			 * whenever initAndStart() returns successfully.
			 */
			virtual ~ConcurrentSubscriber();

			/**
			 * @copydoc Subscriber::initAndStart(const Base::TransmissionChannel, \
						std::shared_ptr<Timing::EventSink>, \
						std::function<void(std::exception_ptr)>)
			 * @details Initializes the object and starts a new thread of execution.
			 * The function shall not be overloaded by child instances in order to
			 * ensure proper initialization. Use the provided init() function
			 * instead.
			 */
			virtual void initAndStart(
				const Base::TransmissionChannel &settings,
				std::shared_ptr<Timing::EventSink> eventSink,
				std::function<void(std::exception_ptr)> errorCallback);

			/**
			 * @brief Processes an external termination request
			 * @details The function will block until the run() function returns. It 
			 * should not be overloaded by child classes in order to ensure that the 
			 * isTerminationRequestPending() returns the correct result. Use 
			 * terminationRequest() to be notified of pending termination requests.
			 */
			virtual void terminate();

		protected:
			/**
			 * @brief Initializes the subscriber
			 * @details The function is called before run() is executed. It may block
			 * until the subscriber is initialized. If the object cannot be 
			 * initialized, a Base::SystemConfigurationException may be thrown. As 
			 * soon as the function returns successfully, the new thread of execution
			 * is started and run() is executed.
			 */
			virtual void init(const Base::TransmissionChannel &settings,
				std::shared_ptr<Timing::EventSink> eventSink) = 0;

			/**
			 * @brief Executes the domain logic in a newly created thread.
			 * @details As soon as the function returns, the thread terminates. An 
			 * inheriting class must ensure that the function returns as soon as a 
			 * termination request is received and all pending tasks are completed. 
			 * Therefore, it may periodically poll the termination flag and register 
			 * a termination handler. The function may also throw an exception which 
			 * is processed via the registered event handler.
			 */
			virtual void run() = 0;

			/**
			 * @brief Indicates that the thread should terminate
			 * @details The function is called by the requesting thread as soon as 
			 * the current thread should stop its operation. In case the function is
			 * executed, isTerminationRequestPending() must return <code>true</code>.
			 */
			virtual void terminationRequest() {}

			/**
			 * @brief checks whether a termination request is pending
			 * @details The function is thread save and may be called concurrently.
			 */
			bool isTerminationRequestPending();
			
		private:
			/** @brief The thread which manages the concurrent communication */
			std::thread subscriptionThread_;

			/** 
			 * @brief A flag which indicates that the subscriptionThread should 
			 * terminate 
			 */
			bool terminationRequest_;

			/** @brief Callback which handles premature termination */
			std::function<void(std::exception_ptr)> errorCallback_;

			/** 
			 * @brief Guards all shared member variables of the 
			 * ConcurrentSubscriber
			 */
			std::mutex objectMut_;

			/**
			 * @brief Calls the run() function and processes any exception thrown
			 */
			void executeRun();
		};

	}
}
#endif
