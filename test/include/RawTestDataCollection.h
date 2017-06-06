/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file RawTestDataCollection.h
 * @brief The file contains a bunch of constant test data values
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCKTEST_NETWORK_RAW_TEST_DATA_COLLECTION
#define _FMITERMINALBLOCKTEST_NETWORK_RAW_TEST_DATA_COLLECTION

#include "RawTestData.h"

namespace FMITerminalBlockTest 
{
	namespace Network 
	{
		namespace ASN1TestData 
		{
			/* Some basic test values */
			RawTestData RAW_TEST_BOOL_TRUE();
			RawTestData RAW_TEST_BOOL_FALSE();

			RawTestData RAW_TEST_STRING_EMPTY();
			RawTestData RAW_TEST_STRING_HII();
			RawTestData RAW_TEST_STRING_0_3();

			RawTestData RAW_TEST_REAL_0_3();
			RawTestData RAW_TEST_REAL_PI();
			RawTestData RAW_TEST_LREAL_DBL_EPSILON();
			RawTestData RAW_TEST_LREAL_PI();

			RawTestData RAW_TEST_DINT_INT_MIN();
			RawTestData RAW_TEST_DINT_INT_MAX();
		}
	}
}
#endif
