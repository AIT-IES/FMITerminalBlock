/* ------------------------------------------------------------------- *
* Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
* All rights reserved. See file FMITerminalBlock_LICENSE for details. *
* ------------------------------------------------------------------- */

/**
* @file PortIDDrawer.h
* @author Michael Spiegel, michael.spiegel@ait.ac.at
*/

#ifndef _FMITERMINALBLOCK_BASE_PORT_ID_DRAWER
#define _FMITERMINALBLOCK_BASE_PORT_ID_DRAWER

#include "base/PortID.h"

#include <common/FMIVariableType.h>
#include <vector>

namespace FMITerminalBlock
{
	namespace Base
	{

		/**
		 * @brief Uniquely assigns PortIDs
		 * @details The class guarantees that each PortID is unique. Nevertheless,
		 * the fields of a port id (type, number) may not be unique on their own.
		 */
		class PortIDDrawer
		{
		public:
			/**
			 * @brief Generates an empty object which has not assigned any ID yet.
			 */
			PortIDDrawer();

			/**
			 * @brief Returns a newly generated unique PortID
			 */
			PortID getNextPortID(FMIVariableType type);

		private:
			/**
			 * @brief Vector of the next port identification number per fmiType.
			 * @details The index of the vector corresponds to the FMI type id.
			 */
			std::vector<unsigned> nextPortID_;
		};


	}
}

#endif
