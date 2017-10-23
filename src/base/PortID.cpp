/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file PortID.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include <base/PortID.h>

#include <cassert>

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

std::string FMITerminalBlock::Base::getVariableTypeString(FMIVariableType type)
{
	switch (type)
	{
		case fmiTypeReal: return "Real";
		case fmiTypeInteger: return "Integer";
		case fmiTypeBoolean: return "Boolean";
		case fmiTypeString: return "String";
		case fmiTypeUnknown: return "Unknown";
		default: 
			assert(false);
			return "";
	}
}

std::ostream& FMITerminalBlock::Base::operator<<(std::ostream& stream,
	const PortID& portID)
{
	// TODO: Human readable FMI type representation
	stream << "(" << (int) portID.first << "," << portID.second << ")";
	return stream;
}
