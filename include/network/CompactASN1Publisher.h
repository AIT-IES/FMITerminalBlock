/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CompactASN1Publisher.h
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_NETWORK_COMPACT_ASN1_PUBLISHER
#define _FMITERMINALBLOCK_NETWORK_COMPACT_ASN1_PUBLISHER

#include "base/environment-helper.h"
#include "network/Publisher.h"
#include "network/ASN1Commons.h"
#include "timing/Event.h"

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <stdint.h>

namespace FMITerminalBlock 
{
	namespace Network
	{
		using namespace FMITerminalBlock;
		using namespace boost::asio::ip;

		/**
		 * @brief Implements the compact ASN.1 encoding described by IEC 61499.
		 * @details <p>The abstract class provides some functionality to maintain 
		 * and encode output variables. It uses compact encoding rules as defined in
		 * IEC 61499 to encode transferred values. The actual transfer logic which 
		 * sends the encoded data is not contained within this class. </p>
		 * <p> A rudimentary type casting mechanism supports different types of 
		 * output types. Supported type casts will be checked on initialization. 
		 * Please note that at the time of writing only a subset of specified output
		 * types is available. </p>
		 */
		class CompactASN1Publisher: public Publisher
		{
		public:

			/**
			 * @brief The property name which specifies the channels encoding
			 */
			static const std::string PROP_ENCODE_TYPE;

			/**
			 * @brief Creates an unconnected Publisher instance
			 */
			CompactASN1Publisher(void);

			/** @brief Deletes the publisher */
			virtual ~CompactASN1Publisher(void) {};

			/**
			 * @copydoc Publisher::init()
			 */
			virtual void init(const Base::TransmissionChannel &channel);

			/**
			 * @brief Updates the output data and requests sending the message
			 */
			virtual void eventTriggered(Timing::Event * ev);

		protected:

			/** 
			 * @brief Array of flags which indicating that casting between different
			 * types is supported. 
			 * @details The first dimension stores the FMIType as the source data type
			 * and the second dimension stores the ASN1Commons::DataType as the cast's
			 * destination. If the entry is set to <code>true</code>, the publisher
			 * will support type-casts from the FMIType to the IEC 61499 type. It is
			 * used to check the configuration before issuing the first event.
			 */
			static const bool CASTABLE[5][ASN1Commons::DATA_TYPE_SIZE];

			/**
			 * @brief Requests a CompartASN1Publisher instance to send the packet
			 * @details The function will be called whenever an event is triggered 
			 * and the data needs to be published.
			 * @param buffer The buffer instance which contains the encoded data. 
			 * The given reference is valid until the function returns.
			 */
			virtual void sendData(const std::vector<uint8_t> &buffer) = 0;


			/**
			 * @brief Returns a nicely formatted string of the buffer's content
			 * @param buffer The data buffer to format
			 * @return A more or less human readable string representing the buffer's 
			 * content. The function may be used for debugging purpose only.
			 */
			static std::string toString(const std::vector<uint8_t> & buffer);

		private:

			/**
			 * @brief Buffered state of the message's data
			 * @details The state will be updated by every event variable.
			 */
			std::vector<Timing::Variable> outputVariables_;

			/**
			 * @brief Vector of IEC 61499 output types
			 * @details The vector index corresponds to the output port index and is
			 * also used at outputVariables_. It won't be modified after the
			 * initialization completes.
			 */
			std::vector<ASN1Commons::DataType> outputTypes_;

			/**
			 * @brief Initializes the output variable vector based on the given 
			 * channel configuration.
			 * @details Each variable will be set to a statically coded default value.
			 * @param ports The vector of output ports which is used to obtain the
			 * port configuration.
			 */
			void initOutputVariables(
				const std::vector<Base::PortID> & ports);

			/**
			 * @brief Initializes the outputType_ vector based on the given 
			 * configuration and the parsed channel mapping.
			 * @details If no specific parameter is given, the best fitting one will
			 * be used. If a parameter stored in the given tree is invalid, a
			 * Base::SystemConfigurationException will be thrown.
			 * @param channel The transmission channel configuration of the channel.
			 */
			void initOutputTypes(const Base::TransmissionChannel &channel);

			/**
			 * @brief Returns the default ASN.1 type based on the fmiType
			 * @param type The FMI source type
			 * @return The best fitting ASN.1 type which avoids data loss
			 */
			static ASN1Commons::DataType getDefaultType(FMIType type);

			/**
			 * @brief Updates the outputVariables_ based on the given Event
			 * @details The event pointer must be valid. The function will traverse 
			 * the event's variables and set the internal state vector accordingly.
			 * The function of this class won't send any message.
			 * @param ev A valid pointer locating the event to process
			 * @brief <code>true</code> If the event contains at least one relevant 
			 * variable
			 */
			bool updateOutputVariables(Timing::Event * ev);

			/** 
			 * @brief Appends the registered output variables and its content to the
			 * ASN.1-based message.
			 * @details The function will use the compact encoding rules presented in
			 * IEC 61499. It adds the message at the end of the given buffer.
			 * @param buffer A valid reference to the output buffer
			 */
			void encodeASN1OutputVariables(std::vector<uint8_t> &buffer);

			/**
			 * @brief Encodes the given value and appends it to the buffer
			 * @details Compact encoding rules are applied.
			 * @param buffer A reference to the buffer used to append the encoded data
			 * @param value The value to encode
			 */
			static void encodeLREALValue(std::vector<uint8_t> &buffer, fmiReal value);

			/**
			 * @brief Encodes the given value and appends it to the buffer
			 * @details Compact encoding rules are applied. The fmiReal type will be 
			 * casted using a best effort approach. During this operation some
			 * precision might get lost.
			 * @param buffer A reference to the buffer used to append the encoded data
			 * @param value The value to encode
			 */
			static void encodeREALValue(std::vector<uint8_t> &buffer, fmiReal value);


			/**
			 * @brief Encodes the given value and appends it to the buffer
			 * @details Compact encoding rules are applied.
			 * @param buffer A reference to the buffer used to append the encoded data
			 * @param value The value to encode
			 */
			static void encodeValue(std::vector<uint8_t> &buffer, fmiInteger value);

			/**
			 * @brief Encodes the given value and appends it to the buffer
			 * @details Compact encoding rules are applied.
			 * @param buffer A reference to the buffer used to append the encoded data
			 * @param value The value to encode
			 */
			static void encodeValue(std::vector<uint8_t> &buffer, fmiBoolean value);

			/**
			 * @brief Encodes the given value and appends it to the buffer
			 * @details Compact encoding rules are applied.
			 * @param buffer A reference to the buffer used to append the encoded data
			 * @param value The value to encode
			 */
			static void encodeValue(std::vector<uint8_t> &buffer, const std::string & value);

		};

	}
}

#endif
