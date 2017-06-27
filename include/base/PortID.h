/* ------------------------------------------------------------------- *
* Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
* All rights reserved. See file FMITerminalBlock_LICENSE for details. *
* ------------------------------------------------------------------- */

/**
* @file PortID.h
* @author Michael Spiegel, michael.spiegel@ait.ac.at
*/

#ifndef _FMITERMINALBLOCK_BASE_PORT_ID
#define _FMITERMINALBLOCK_BASE_PORT_ID

#include <common/FMIVariableType.h>
#include <utility>
#include <functional>

namespace FMITerminalBlock
{
	namespace Base
	{

		 /**
  		* @brief Defines a port identifier type
		  * @details The FMIType specifies the type of the port and the integer
		  * stores a type-unique identifier. The identifier may not be densely
		  * assigned by channel mapping and may not start at zero.
		  */
		typedef std::pair<FMIVariableType, int> PortID;

		/** @brief Defines a function type which hashed a PortID */
		typedef std::function<unsigned int(PortID)> PortIDHashFunction;

		/** @brief Returns a hash value of the given PortID */
		unsigned int hashPortID(PortID id);

		/** @brief Returns <code>true</code> iff both PortIDs are equal. */
		bool operator==(PortID a, PortID b);
		/** @brief Returns <code>false</code> iff both PortIDs are equal. */
		bool operator!=(PortID a, PortID b);

		/** @brief Returns a string representation of the given variable type */
		std::string getVariableTypeString(FMIVariableType type);
	}
}

#endif
