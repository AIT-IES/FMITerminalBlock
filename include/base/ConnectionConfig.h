/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file ConnectionConfig.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_BASE_CONNECTION_CONFIG
#define _FMITERMINALBLOCK_BASE_CONNECTION_CONFIG

#include <base/AbstractConfigProvider.h>

namespace FMITerminalBlock
{
	namespace Base
	{

		/**
		 * @brief Encapsulates all configuration options of a single connection.
		 * @details Each connection has a unique ID which will either be implicitly
		 * assigned or explicitly referenced. Implicit connections may be generated
		 * if a channel does not have any external reference. By the implicit
		 * generation, a connection object may contain children such as variable
		 * configuration subtrees which do not directly belong to a connection
		 * configuration. A network connection must ignore these configuration
		 * fragments.
		 */
		class ConnectionConfig: public AbstractConfigProvider
		{
		public:

			/**
			 * @brief Initializes the connection configuration
			 * @param configTree A valid reference to the configuration subtree. The
			 * reference must remain valid until the object is destroyed. The object
			 * will not copy the given property tree.
			 * @param id The unique identifier of the configuration. The given
			 * reference may be invalidated as soon as the object is constructed.
			 */
			ConnectionConfig(const boost::property_tree::ptree& configTree,
				const std::string& id);

			/** @brief Frees allocated resources */
			virtual	~ConnectionConfig() {}

			std::string getID() const { return id_; }

		protected:
			/** @copydoc AbstractConfigProvider::getConfig() */
			virtual const boost::property_tree::ptree& getConfig() const
			{
				return config_;
			}

		private:
			/** @brief The unique identifier of the configuration object */
			std::string id_;
			/** A reference to the configuration subtree */
			const boost::property_tree::ptree& config_;
		};

	}
}

#endif
