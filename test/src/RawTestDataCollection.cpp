/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file RawTestDataCollection.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "RawTestDataCollection.h"

using namespace FMITerminalBlockTest::Network::ASN1TestData;
using namespace FMITerminalBlockTest::Network;
using namespace FMITerminalBlockTest;


RawTestData ASN1TestData::RAW_TEST_BOOL_TRUE() { return {0x41}; }
RawTestData ASN1TestData::RAW_TEST_BOOL_FALSE() { return {0x40}; }

RawTestData ASN1TestData::RAW_TEST_STRING_EMPTY() { return {0x50, 0x00,0x00}; }
RawTestData ASN1TestData::RAW_TEST_STRING_HII()
{
	return { 0x50, 0x00, 0x03, 'H', 'i', '!' };
}


RawTestData ASN1TestData::RAW_TEST_REAL_0_3() 
{ 
	return {0x4a, 0x3e,0x99,0x99,0x9a}; 
}
RawTestData ASN1TestData::RAW_TEST_LREAL_DBL_EPSILON() 
{ 
	return {0x4b, 0x3c,0xb0,0x00,0x00,0x00,0x00,0x00,0x00}; 
}

RawTestData ASN1TestData::RAW_TEST_DINT_INT_MIN() 
{ 
	return {0x44, 0x80,0x00,0x00,0x00}; 
}
RawTestData ASN1TestData::RAW_TEST_DINT_INT_MAX() {
	return {0x44, 0x7F,0xFF,0xFF,0xFF}; 
}