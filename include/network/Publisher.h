/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file Publisher.h
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_NETWORK_PUBLISHER
#define _FMITERMINALBLOCK_NETWORK_PUBLISHER

#include "timing/EventListener.h"
#include "base/PortID.h"
#include "base/TransmissionChannel.h"

#include <boost/property_tree/ptree.hpp>

#include <vector>

namespace FMITerminalBlock 
{
	namespace Network
	{
		using namespace FMITerminalBlock;

		/**
		 * @brief Abstract interface class which defines a publisher component
		 * @details The publisher component will be initialized separately to ease 
		 * its pseudo-dynamic instantiation. It manages a set of output variables 
		 * and may receive events to be published. On receiving events, it is
		 * intended to transmit these events to an event sink.
		 */
		class Publisher: public Timing::EventListener
		{
		public:

			/** @brief Frees allocated resources */
			virtual ~Publisher(void) {};

			/**
			 * @brief Initializes the publisher
			 * @details The function will be called before calling any other function. 
			 * If something went wrong a std::runtime_error or
			 * Base::SystemConfigurationException may be thrown.
			 * @param channel The channel configuration which includes any property subtrees.
			 */
			virtual void init(const Base::TransmissionChannel &channel) = 0;

		};

	}
}

#endif
