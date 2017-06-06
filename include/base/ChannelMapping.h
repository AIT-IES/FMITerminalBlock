/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file ChannelMapping.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_BASE_CHANNEL_MAPPING
#define _FMITERMINALBLOCK_BASE_CHANNEL_MAPPING

#include "base/PortID.h"
#include "base/PortIDDrawer.h"
#include "base/TransmissionChannel.h"

#include <common/FMIVariableType.h>
#include <boost/property_tree/ptree.hpp>
#include <utility>
#include <vector>

namespace FMITerminalBlock
{
	namespace Base
	{

		/**
		 * @brief Encapsulates the correlation of model variables and in-/output
		 * channels.
		 * @details <p> Every model variable has a name listed in the model
		 * description and a type-unique identifier which is assigned by the channel
		 * mapping object. The identifier is called PortID or simply
		 * port. It mustn't be confused with FMI's variable identifier which is
		 * listed in the model description. The FMI identifier is not directly
		 * exposed by FMI++ which handles the FMI communication. Every PortID is 
		 * uniquely assigned along all input and output ports but may not have a
		 * consecutive number.</p>
		 * <p> Ports are grouped into output channels. An output channel
		 * holds every variable identifier (port) which is transmit or received in 
		 * one protocol entity. For instance, a single ASN.1-based channel may 
		 * encapsulate several model variables into one network packet. Each channel 
		 * is identified by a unique integer ID. All ports of a channel are 
		 * encapsulated into a common TransmissionChannel object. The ChannelMapping
		 * object does not specify the direction of the data-flow. Hence, it may be
		 * used for input and output channels alike.</p>
		 */
		class ChannelMapping
		{
		public:

			/** @brief The key of the channel type property */
			static const std::string PROP_TYPE;

			/**
			 * @brief C'tor initializing an empty ChannelMapping object
			 * @param portIDSource A reference to the global PortIDDrawer 
			 * object. The reference must be valid until the object is 
			 * destroyed.
			 */
			ChannelMapping(PortIDDrawer &portIDSource):
				variableNames_(5, std::vector<std::string>()),
				variableIDs_(5, std::vector<PortID>()),
				channels_(), portIDSource_(portIDSource) {};

			/**
			 * @brief C'tor generating the channel mapping based on the current 
			 * configuration.
			 * @details If the property subtree is invalid, a 
			 * Base::SystemConfigurationException will be thrown.
			 * @param prop The property subtree which contains the channel
			 * configuration
			 * @param portIDSource A reference to the global PortIDDrawer
			 * object. The reference must be valid until the object is
			 * destroyed.
			 * @throws Base::SystemConfigurationException if an invalid configuration is found
			 */
			ChannelMapping(PortIDDrawer &portIDSource, 
				const boost::property_tree::ptree &prop);
			
			/**
			 * @brief Returns a vector which contains every variable name
			 * @details The index of each name corresponds to the index in the 
			 * VariableID vector. The reference will be valid until the ChannelMapping
			 * object gets destroyed. Every model variable name returned will have the
			 * given type
			 * @param type The FMIVariableType to query
			 * @return A vector which contains every output port name
			 */
			const std::vector<std::string> & getVariableNames(
					FMIVariableType type) const;

			/**
			 * @brief Returns a vector which contains every assigned PortID
			 * @details The function returns a vector of assigned port IDs which is 
			 * filtered by a particular type. The index in the vector corresponds to 
			 * the index of the variable name vector. It is guaranteed that the 
			 * returned PortID stores exactly the same type as the given parameter. The 
			 * reference remains valid until the channel mapping object is destroyed.
			 * @param type The FMIVariableType to query
			 * @return A vector which contains every PortID of a particular type which 
			 * is managed by the ChannelMapping 
			 */
			const std::vector<PortID> & getVariableIDs(FMIVariableType type) const;

			/**
			 * @brief Returns the number of configured channels
			 * @details Configured channel IDs will range from zero to the return
			 * value minus one.
			 * @return The number of configured channels which are managed by 
			 * the ChannelMapping object
			 */
			int getNumberOfChannels(void) const;

			/**
			 * @brief Returns a vector which contains a channel's associated ports.
			 * @details The vector reference will be valid until the object is
			 * destroyed.
			 * @return A vector which contains a channel's associated output ports
			 */
			const std::vector<PortID> & getPorts(int channelID) const;

			/**
			 * @brief Returns the transmission channel object of a particular channel ID
			 * @details The reference remains valid until the ChannelMapping object is 
			 * removed. It is equivalent to the getPorts function but also encapsulates 
			 * configuration references.
			 * @param cannelID a valid channel identifier ranging from zero to 
			 * getNumberOfChannels()-1.
			 */
			const TransmissionChannel & getTransmissionChannel(int channelID) const;

			/**
			 * @brief Returns a string which describes the channel mapping
			 * @return A string which describes the channel mapping
			 */
			std::string toString(void) const;

		private:

			/** @brief reference to the global PortID source */
			PortIDDrawer &portIDSource_;

			/** @brief The vector of available variables per FMIVariableType */
			std::vector<std::vector<std::string>> variableNames_;
			/** @brief The vector of assigned PortIDs per FMIVariableType */
			std::vector<std::vector<PortID>> variableIDs_;

			/** @brief Vector storing configured variables for each output port */
			std::vector<TransmissionChannel> channels_;

			/**
			 * @brief Adds the given channel configuration
			 * @details Every new name will be added to the list of variable names and 
			 * every configured channel will be added to the list of channels.
			 * On detecting an invalid configuration, a
			 * Base::SystemConfigurationException will be thrown.
			 * @param prop The properties containing the channel configuration
			 */
			void addChannels(const boost::property_tree::ptree &prop);

			/**
			 * @brief Adds the port configuration to the list of variables and channels
			 * @details Every new variable name will be added to the list of known
			 * names and every configured port will be added to the list of (a
			 * channel's) variable identifier. On detecting an invalid configuration,
			 * a Base::SystemConfigurationException will be thrown.
			 * @param channelProp The properties which contain the port configuration
			 * @param variablelList The list of previously added ports of the channel
			 */			
			void addVariables(const boost::property_tree::ptree &channelProp,
				TransmissionChannel &variableList);

			/**
			 * @brief Queries the ChannelMapping::PortID of the given name
			 * @details If the variable list does not contain the given name, 
			 * <FMUunknown, -1> will be returned
			 * @return The name's PortID or <fmiTypeUnknown, -1>
			 */
			PortID getID(const std::string &name, FMIVariableType type);

		};


	}
}

#endif
