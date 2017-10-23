/* ------------------------------------------------------------------- *
* Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
* All rights reserved. See file FMITerminalBlock_LICENSE for details. *
* ------------------------------------------------------------------- */

/**
* @file TransmissionChannel.h
* @author Michael Spiegel, michael.spiegel@ait.ac.at
*/

#ifndef _FMITERMINALBLOCK_BASE_TRANSMISSION_CHANNEL
#define _FMITERMINALBLOCK_BASE_TRANSMISSION_CHANNEL

#include "base/PortID.h"

#include <vector>

#include <boost/property_tree/ptree.hpp>

namespace FMITerminalBlock
{
	namespace Base
	{
		/**
		 * @brief Encapsulates the meta data of a channel
		 * @details Each channel contains an ordered list of PortIDs, and associated
		 * configuration. Configuration is stored as a reference to the global 
		 * configuration tree. One reference directly points to the configuration
		 * root of the channel. For each port, another reference to the port 
		 * configuration is managed.
		 */
		class TransmissionChannel
		{
		public:

			/** @brief The key of the connection reference property */
			static const std::string PROP_CONNECTION;

			/** 
			 * @brief Initializes an empty transmission channel 
			 * @param channelConfig A reference to the configuration sub-tree of the 
			 * channel. The reference must remain valid until the object is destroyed.
			 * @brief channelID The lexical identifier of the channel. It may be used
			 * to generate an implicit connection.
			 */
			TransmissionChannel(const boost::property_tree::ptree &channelConfig, 
				const std::string &channelID);

			/** @brief Frees allocated resources*/
			~TransmissionChannel() {};

			/**
			 * @brief Returns the configuration of the channel.
			 * @details The reference points to the root of all channel related 
			 * configuration directives.
			 */
			const boost::property_tree::ptree & getChannelConfig() const;

			/**
			 * @brief Returns a vector of port related configuration references.
			 * @details The returned references remain valid until the object is 
			 * destroyed. The index of the property tree references correspond to the 
			 * index in the PortID vector.
			 */
			const std::vector<const boost::property_tree::ptree *> &
			getPortConfig() const;

			/**
			 * @brief Returns the vector of associated PortIDs.
			 * @details The returned reference remains valid until the object is 
			 * destroyed.
			 */
			const std::vector<PortID> &getPortIDs() const;

			/**
			 * @brief Returns the unique identifier of the channel. 
			 * @details The reference remains valid until the object is destroyed. 
			 * The channel identifier corresponds to the bidirectional channel and 
			 * may be used for the input and output direction.
			 */
			const std::string& getChannelID() const { return channelID_;  }

			/**
			 * @brief Returns the unique identifier of the connection object
			 * @details In case the connection object is not referenced explicitly,
			 * the implicit connection id starting with a dot will be returned. In 
			 * case an invalid configuration is found, an exception will be thrown.
			 */
			const std::string getConnectionID() const;

			/**
			 * @brief Returns whether the connection is implicitly configured
			 * @details In case it is implicitly configured, no reference to an 
			 * external connection configuration is present. In that case, 
			 * configuration parameters need to be taken from the main configuration
			 * of the transmission channel.
			 */
			const bool isImplicitConnection() const;

			/**
			 * @brief Appends a port entry at the end of the channel.
			 * @details The given ptree reference must remain valid until the object is deleted.
			 * @param id The PortID of the particular port
			 * @param portConfig A reference to the associated configuration section.
			 */
			void pushBackPort(PortID id, const boost::property_tree::ptree &portConfig);

		private:

			/** @brief The unique identifier of the transmission channel */
			std::string channelID_;

			/**
			 * @brief A reference to the root property of the particular channel.
			 * @details The reference must remain valid until the object is destroyed.
			 */
			const boost::property_tree::ptree &channelConfig_;

			/**
			 * @brief A possibly empty list of associated ports
			 * @details The index of each port corresponds to the index in the 
			 * configuration list. 
			 */
			std::vector<PortID> portIDs_;
			/**
			 * @brief A list which stores the associated configuration of each port
			 * @details The index corresponds to the index in the portIDs vector. It
			 * must be guaranteed that the references remain valid until the object
			 * is destroyed. Every Entry directly points to the root property of 
			 * each port.
			 */
			std::vector<const boost::property_tree::ptree *> portConfig_;
		};
	}
}

#endif

