/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file PortID.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include <base/PortID.h>

using namespace FMITerminalBlock::Base;

unsigned int FMITerminalBlock::Base::hashPortID(PortID id)
{
	return ((unsigned int) id.first) + 5 * id.second;
}

bool FMITerminalBlock::Base::operator==(PortID a, PortID b)
{
	return a.first == b.first && a.second == b.second;
}

bool FMITerminalBlock::Base::operator!=(PortID a, PortID b)
{
	return !(a == b);
}