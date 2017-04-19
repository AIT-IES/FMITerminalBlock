/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file NetworkManager.h
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_NETWORK_NETWORK_MANAGER
#define _FMITERMINALBLOCK_NETWORK_NETWORK_MANAGER

#include "base/ApplicationContext.h"
#include "timing/EventDispatcher.h"
#include "network\Publisher.h"

#include <list>

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
		private:

			/**
			 * @brief Returns a pointer to an uninitialized publisher instance.
			 * @details The publisher will be chosen by the given string id. If the id
			 * is invalid, NULL will be returned.
			 * @return A reference to the newly created publisher or NULL
			 */
			static Publisher * instantiatePublisher(const std::string &id);

			/** @brief List which stores a pointer to every managed publisher */
			std::list<Publisher *> publisher_;
		};

	}
}

#endif
