/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file NetworkManager.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_NETWORK_NETWORK_MANAGER
#define _FMITERMINALBLOCK_NETWORK_NETWORK_MANAGER

#include "base/ApplicationContext.h"
#include "timing/EventDispatcher.h"
#include "network/Publisher.h"
#include "network/Subscriber.h"

#include <exception>
#include <functional>
#include <list>
#include <memory>
#include <mutex>

namespace FMITerminalBlock 
{
	namespace Network
	{
		using namespace FMITerminalBlock;

		/**
		 * @brief The network manager manages the publisher and subscriber 
		 * instances.
		 * @details It will contain a list of networking instances. Currently it
		 * supports the transmission of data only. Future versions may enable the
		 * event reception.
		 */
		class NetworkManager
		{
		public:

			/**
			 * @brief The output protocol property's name
			 */
			static const std::string PROP_PROTOCOL;

			/**
			 * @brief Instantiates the network stack and the object's members
			 * @details The application context object is evaluated to retrieve the
			 * network configuration and the dispatcher is used to process events 
			 * which are transported via the network. The function will throw a
			 * Base::SystemConfigurationException or a std::runtime_error, if the
			 * network stack can't be instantiated.
			 * @param context The application context used to read the configuration.
			 * @param dispatcher The event dispatcher used to process events.
			 */
			NetworkManager(Base::ApplicationContext &context, 
				Timing::EventDispatcher &dispatcher);

			/** @brief Deletes managed resources */
			~NetworkManager();

			/**
			 * @brief Checks whether an exception from a network thread is pending
			 */
			bool hasPendingException();

			/**
			 * @brief Throws any pending exception
			 * @details If no exception was registered by another thread, the 
			 * function terminates regularly.
			 */
			void throwPendingException();

			/**
			 * @brief Terminates every registered subscriber instance
			 * @details Terminated subscriber instances will be removed from the 
			 * list of subscribers. The function is specified as a public function in
			 * order to trigger termination errors outside the destructor. If the 
			 * function is not called, all subscribers will be terminated in the 
			 * destructor but it is assumed that subscribers do not throw errors on 
			 * terminations.
			 */
			void terminateSubscribers();

		private:

			/**
			 * @brief Listens for incoming events and checks the exception status.
			 * @details If an exception was registered at the associated 
			 * NetworkManager, it will be re-thrown. Hence, the exception is 
			 * transferred to the main thread. The class may be removed in later 
			 * versions leading to a more controlled error handling.
			 */
			class ExceptionBomb : public Timing::EventListener
			{
			public:
				/** @brief Registers the parent instance which will be queried. */
				ExceptionBomb(NetworkManager &parent);
				/** @brief Checks for new exceptions */
				virtual void eventTriggered(Timing::Event *);
			private:
				NetworkManager &parent_;
			};

			/** @brief List which stores a pointer to every managed publisher */
			std::list<std::shared_ptr<Publisher>> publisher_;

			/** @brief List which stores a pointer to every managed subscriber */
			std::list<std::shared_ptr<Subscriber>> subscriber_;

			/** 
			 * @brief Holds an exception issued by another thread
			 * @details The variable holds a nullptr in case no exception was 
			 * registered or the exception was already thrown.
			 */
			std::exception_ptr pendingException_;
			/** @brief Synchronizes the external exception variable */
			std::mutex pendingExceptionMutex_;

			/** @brief Checks the exception status as soon as an event is received */
			std::shared_ptr<ExceptionBomb> exceptionTester_;

			/**
			 * @brief Instantiates and initializes the given network entity
			 * @details The network entity type, i.e. a Publisher or Subscriber, is 
			 * given in BaseType
			 * @param destinationList The list to append the newly instantiated 
			 * entities. Any Entities which are already present, will remain in the 
			 * list.
			 * @param channels The configuration structure which is used to create 
			 * the network entities.
			 * @param instFct The instantiation function which creates new network 
			 * entities.
			 * @param initFct The initialization function which is called on every 
			 * network entity.
			 */
			template<typename BaseType>
			static void addChannels(
				std::list<std::shared_ptr<BaseType>> *destinationList, 
				const Base::ChannelMapping * channels,
				std::function<std::shared_ptr<BaseType>(const std::string&)> instFct,
				std::function<void(std::shared_ptr<BaseType>, 
						const Base::TransmissionChannel&)> initFct);

			/**
			 * @brief Returns a pointer to an uninitialized publisher instance.
			 * @details The publisher will be chosen by the given string id. If the 
			 * id is invalid, a NULL instance will be returned.
			 * @return A reference to the newly created publisher or NULL
			 */
			static std::shared_ptr<Publisher> instantiatePublisher(
				const std::string &id);

			/**
			 * @brief Returns a pointer to an uninitialized subscriber instance.
			 * @details The subscriber will be chosen by the given string id. If the 
			 * id is invalid, a NULL instance will be returned.
			 * @return A reference to the newly created publisher or a NULL instance
			 */
			static std::shared_ptr<Subscriber> instantiateSubscriber(
				const std::string &id);

			/**
			 * @brief Adds all publisher as EventListener instances to the given 
			 * EventDispatcher.
			 */
			void addListeningPublisher(Timing::EventDispatcher &dispatcher);

			/**
			 * @brief Handles the exception from another thread.
			 * @details The function is thread save, i.e. it may be called from 
			 * another thread.
			 * @param exec A pointer to the exception which should be passed to the 
			 * main thread.
			 */
			void handleException(std::exception_ptr exc);
		};

	}
}

#endif
