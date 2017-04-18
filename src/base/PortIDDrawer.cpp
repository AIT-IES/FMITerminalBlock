/* ------------------------------------------------------------------- *
* Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
* All rights reserved. See file FMITerminalBlock_LICENSE for details. *
* ------------------------------------------------------------------- */

/**
* @file PortIDDrawer.cpp
* @author Michael Spiegel, michael.spiegel@ait.ac.at
*/

#include "base/PortIDDrawer.h"

#include <assert.h>

using namespace FMITerminalBlock::Base;

PortIDDrawer::PortIDDrawer() : nextPortID_(5, 0)
{
}

PortID PortIDDrawer::getNextPortID(FMIType type)
{
	assert(((int) type) < nextPortID_.size());
	PortID id = std::make_pair(type, nextPortID_[(int) type]);
	nextPortID_[(int) type] ++;
	return id;
}