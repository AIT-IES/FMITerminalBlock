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

#include <common/FMIType.h>
#include <boost/property_tree/ptree.hpp>
#include <utility>
#include <vector>

namespace FMITerminalBlock
{
	namespace Base
	{

		/**
		 * @brief Encapsulates the correlation of model variables and output
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
		 * holds every variable identifier (port) which is transmit in one protocol
		 * entity. For instance, a single ASN.1-based channel may encapsulate
		 * several model variables into one network packet. Each channel is
		 * identified by a unique integer ID. In future versions the channel
		 * mapping object may be extended to feature input ports and input channels
		 * as well.</p>
		 */
		class ChannelMapping
		{
		public:

			/** @brief The key of the output channel property */
			static const std::string PROP_OUT;
			/** @brief The key of the channel type property */
			static const std::string PROP_TYPE;

			/**
			 * @brief C'tor initializing an empty ChannelMapping object
			 * @param portIDSource A reference to the global PortIDDrawer 
			 * object. The reference must be valid until the object is 
			 * destroyed.
			 */
			ChannelMapping(PortIDDrawer &portIDSource):
				outputVariableNames_(5, std::vector<std::string>()),
				outputChannels_(), portIDSource_(portIDSource) {};

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
			 */
			ChannelMapping(PortIDDrawer &portIDSource, 
				const boost::property_tree::ptree &prop);
			
			/**
			 * @brief Returns a vector which contains every output channel name
			 * @details The index of each name corresponds to the index in the 
			 * VariableID vector. The reference will be valid until the ChannelMapping
			 * object gets destroyed. Every model variable name returned will have the
			 * given type
			 * @param type The FMIType to query
			 * @return A vector which contains every output port name
			 */
			const std::vector<std::string> & getOutputVariableNames(
					FMIType type) const;

			/**
			 * @brief Returns a vector which contains every assigned port id
			 * @details The function returns a vector of assigned port IDs which is 
			 * filtered by a particular type. The index in the vector corresponds to 
			 * the index of the variable name vector. It is guaranteed that the 
			 * returned PortID stores exactly the same type as the given parameter. The 
			 * reference remains valid until the channel mapping object is destroyed.
			 * @param type The FMIType to query
			 * @return A vector which contains every output port ID of a particular type
			 */
			const std::vector<PortID> & getOutputVariableIDs(FMIType type) const;

			/**
			 * @brief Returns the number of configured output channels
			 * @details Configured channel IDs will range from zero to the return
			 * value minus one.
			 * @return The number of configured output channels
			 */
			int getNumberOfOutputChannels(void) const;

			/**
			 * @brief Returns a vector which contains a channel's associated ports.
			 * @details The vector reference will be valid until the object is
			 * destroyed.
			 * @return A vector which contains a channel's associated output ports
			 */
			const std::vector<PortID> & getOutputPorts(int channelID) const;

			/**
			 * @brief Returns a string which describes the channel mapping
			 * @return A string which describes the channel mapping
			 */
			std::string toString(void) const;

		private:

			/** @brief reference to the global PortID source */
			PortIDDrawer &portIDSource_;

			/** @brief The vector of available output variables per FMIType */
			std::vector<std::vector<std::string>> outputVariableNames_;
			/** @brief The vector of assigned PortIDs per FMIType */
			std::vector<std::vector<PortID>> outputVariableIDs_;

			/** @brief Vector storing configured variables for each output port */
			std::vector<std::vector<PortID>> outputChannels_;

			/**
			 * @brief Adds the given channel configuration to the given lists
			 * @details Every new name will be added to the list of known names and 
			 * every configured channel will be added to the list of channels.
			 * On detecting an invalid configuration, a
			 * Base::SystemConfigurationException will be thrown.
			 * @param prop The properties containing the channel configuration
			 * @param nameList The list of previously added port names
			 * @param idList The list of previously added channel IDs
			 * @param channelList The list of channels
			 */
			// TODO: Refactor signature
			void addChannels(const boost::property_tree::ptree &prop, 
				std::vector<std::vector<std::string>> &nameList,
				std::vector<std::vector<PortID>> &idList,
				std::vector<std::vector<PortID>> &channelList);

			/**
			 * @brief Adds the port configuration to the given lists
			 * @details Every new variable name will be added to the list of known
			 * names and every configured port will be added to the list of (a
			 * channel's) variable identifier. On detecting an invalid configuration,
			 * a Base::SystemConfigurationException will be thrown.
			 * @param channelProp The properties which contain the port configuration
			 * @param nameList The list of previously added channel names
			 * @param idList The list of previously added channel IDs
			 * @param variablelList The list of previously added ports
			 */			
			 // TODO: Refactor signature
			void addVariables(const boost::property_tree::ptree &channelProp,
				std::vector<std::vector<std::string>> &nameList,
				std::vector<std::vector<PortID>> &idList,
				std::vector<PortID> &variableList);

			/**
			 * @brief Queries the ChannelMapping::PortID of the given name
			 * @details If the list does not contain the given name, <FMUunknown, -1>
			 * will be returned
			 * @return The name's PortID or <fmiTypeUnknown, -1>
			 */
			static PortID getID(
				const std::vector<std::vector<std::string>> &nameList,
				const std::vector<std::vector<PortID>> &idList,
				const std::string &name, FMIType type);

		};


	}
}

#endif
