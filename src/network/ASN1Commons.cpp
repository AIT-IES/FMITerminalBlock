/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file ASN1Commons.cpp
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#include "network/ASN1Commons.h"

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Network;


const char * ASN1Commons::DATA_TYPE_NAMES[DATA_TYPE_SIZE] = {"REAL", "LREAL", "DINT", "BOOL", "STRING"};
