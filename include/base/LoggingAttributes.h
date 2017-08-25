/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file LoggingAttributes.h
 * @brief The file contains globally available logging attributes
 * @details The attribute definitions are located in a separate file in order 
 * to reduce circular and module dependencies.
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_BASE_LOGGING_ATTRIBUTES
#define _FMITERMINALBLOCK_BASE_LOGGING_ATTRIBUTES

#include <boost/log/attributes/mutable_constant.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/expressions/keyword_fwd.hpp>

#include <common/fmi_v1.0/fmiModelTypes.h>

namespace FMITerminalBlock
{
	namespace Base
	{

		/** @brief The event time attribute's name */
		extern const char* ATTR_EVENT_TIME;

		/** @brief Defines the eventTime attribute */
		BOOST_LOG_ATTRIBUTE_KEYWORD(eventTime, ATTR_EVENT_TIME, fmiTime)

		/** @brief The name of the time logging record timestamp attribute */
		extern const char* ATTR_TIMING_RECORD_TIME;

		/** 
		 * @brief Defines the timingRecordTime attribute
		 * @details The timestamp is given in seconds since epoch and may differ
		 * from the log entries time stamp. It is used to get a more condensed time
		 * format.
		 */
		BOOST_LOG_ATTRIBUTE_KEYWORD(timingRecordTime, ATTR_TIMING_RECORD_TIME, fmiTime)
		
	}
}

#endif
