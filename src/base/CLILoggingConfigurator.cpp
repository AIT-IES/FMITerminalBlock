/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CLILoggingConfigurator.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "base/CLILoggingConfigurator.h"

#include <cassert>

#include <boost/log/expressions/formatters/format.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>

#include "base/BaseExceptions.h"
#include "base/LoggingAttributes.h"

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Base;

const std::string CLILoggingConfigurator::PROP_LOG_LEVEL = "app.logLevel";

CLILoggingConfigurator::CLILoggingConfigurator()
{
	boost::log::add_common_attributes();
	stdoutSink_	= boost::log::add_console_log();
	
	stdoutSink_->locked_backend()->auto_flush(true);
	stdoutSink_->set_formatter( 
		boost::log::expressions::format("[%1%] [%2%] %3%: %4% ")
			// See http://www.boost.org/doc/libs/1_56_0/doc/html/date_time/
			// date_time_io.html#date_time.format_flags
			% boost::log::expressions::format_date_time<boost::posix_time::ptime>
				("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
			% boost::log::expressions::attr<
				boost::log::attributes::current_thread_id::value_type>("ThreadID")
			% boost::log::trivial::severity
			% boost::log::expressions::smessage
	);
	stdoutSink_->set_filter(
		! boost::log::expressions::has_attr(Base::eventTime) && 
		boost::log::trivial::severity >= boost::log::trivial::info
	);
}

void CLILoggingConfigurator::configureLogger(
	const ApplicationContext &appContext)
{
	assert(stdoutSink_);
	std::string logLevel;
	logLevel = appContext.getProperty<std::string>(PROP_LOG_LEVEL, "debug");

	auto level = boost::log::trivial::trace;
	if (logLevel == "trace")
	{
		level = boost::log::trivial::trace;
	} else if (logLevel == "debug") {
		level = boost::log::trivial::debug;
	} else if (logLevel == "info") {
		level = boost::log::trivial::info;
	} else if (logLevel == "warning") {
		level = boost::log::trivial::warning;
	} else if (logLevel == "error") {
		level = boost::log::trivial::error;
	} else {
		throw SystemConfigurationException("Unknown log level.", PROP_LOG_LEVEL, 
			appContext.getProperty<std::string>(PROP_LOG_LEVEL, "debug"));
	}

	stdoutSink_->set_filter(
		!boost::log::expressions::has_attr(Base::eventTime) &&
		boost::log::trivial::severity >= level
	);
}