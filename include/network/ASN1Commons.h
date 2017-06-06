/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file ASN1Commons.h
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_NETWORK_ASN1_COMMONS
#define _FMITERMINALBLOCK_NETWORK_ASN1_COMMONS

#include <stdint.h>

namespace FMITerminalBlock 
{
	namespace Network
	{
		using namespace FMITerminalBlock;

		/**
		 * @brief Encapsulates some common numbers used in every ASN.1
		 * implementation
		 * @details The numbers are defined in the ASN.1 and IEC 61499 standard 
		 * respectively. To avoid compiler issues, most numbers are defined as
		 * stdint types. Most flags will be given as mask byte and may be
		 * concatenated by a logical 'or' operation.
		 */
		class ASN1Commons
		{
		public:
			/** @brief Universal tag class flags */
			static const uint8_t CLASS_UNIVERSAL = 0x00;
			/** @brief Application specific tag class flags */
			static const uint8_t CLASS_APPLICATION = 0x40;
			/** @brief Context specific tag class flags */
			static const uint8_t CLASS_CONTEXT = 0x80;
			/** @brief Private tag class flags */
			static const uint8_t CLASS_PRIVATE = 0xC0;

			/** @brief The tag number without any flags of the IEC 61499 type */
			static const uint8_t BOOL0_TAG_NR = 0;
			/** @brief The tag number without any flags of the IEC 61499 type */
			static const uint8_t BOOL1_TAG_NR = 1;
			/** @brief The tag number without any flags of the IEC 61499 type */
			static const uint8_t DINT_TAG_NR = 4;
			/** @brief The tag number without any flags of the IEC 61499 type */
			static const uint8_t LINT_TAG_NR = 5;
			/** @brief The tag number without any flags of the IEC 61499 type */
			static const uint8_t REAL_TAG_NR = 10;
			/** @brief The tag number without any flags of the IEC 61499 type */
			static const uint8_t LREAL_TAG_NR = 11;
			/** @brief The tag number without any flags of the IEC 61499 type */
			static const uint8_t STRING_TAG_NR = 16;

			/** @brief The number of supported ASN.1/IEC 61499 data types */
			static const unsigned DATA_TYPE_SIZE = 5;

			/**
			 * @brief Enumerates supported ASN.1 types
			 * @details the type values are sequentially enumerated but doesn't
			 * correspond to any particular standard. The enumeration might be used to
			 * address certain types dynamically. The entries are named by the
			 * addressed IEC 61499/IEC 61131 type. The list of known types might have
			 * to be extended in future versions. Please update DATA_TYPE_SIZE on
			 * modifying the DataType enumeration.
			 */
			enum DataType
			{
				REAL = 0,
				LREAL,
				DINT,
				BOOL,
				STRING
			};

			/** @brief Human readable names for each DataType element */
			static const char * DATA_TYPE_NAMES[DATA_TYPE_SIZE];

		};

	}
}

#endif
