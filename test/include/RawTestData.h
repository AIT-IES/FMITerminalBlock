/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file RawTestData.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCKTEST_NETWORK_RAW_TEST_DATA
#define _FMITERMINALBLOCKTEST_NETWORK_RAW_TEST_DATA

#include <vector>
#include <stdint.h>
#include <ostream>
#include <initializer_list>

namespace FMITerminalBlockTest
{
	namespace Network
	{
	
		/**
		 * @brief Encapsulates raw test data
		 * @details The test data buffer may be used to construct test packets or 
		 * to compare received values with a reference sequence.
		 */
		class RawTestData
		{
		public:
			/** @brief Creates an empty test data sequence */
			RawTestData();

			/** @brief Sets the given initial sequence */
			RawTestData(std::initializer_list<uint8_t> sequence);
			/** @brief Copies the data sequence to be the initial sequence */
			RawTestData(const std::vector<uint8_t>& data);
			/** @brief Concatenates both sequences and returns a new one */
			RawTestData(const RawTestData &left, const RawTestData &right);

			/** @brief Returns a reference to the managed data vector */
			const std::vector<uint8_t>& getData() const;

			/**
			 * @brief Appends the given sequence at the end of this object and 
			 * returns a new RawTestData object
			 */
			RawTestData operator+(const RawTestData &data) const;

		private:
			/** @brief The encapsulated test data sequence */
			std::vector<uint8_t> data_;
		};

		/** @brief Prints a string representation of the data buffer */
		std::ostream& operator<<(std::ostream& stream,
			const RawTestData& data);
	}
}
#endif
