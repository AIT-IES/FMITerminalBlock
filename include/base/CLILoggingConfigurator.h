/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CLILoggingConfigurator.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_BASE_CLI_LOGGING_CONFIGURATOR
#define _FMITERMINALBLOCK_BASE_CLI_LOGGING_CONFIGURATOR

#include "base/ApplicationContext.h"

#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/utility/setup/console.hpp>

namespace FMITerminalBlock
{
	namespace Base
	{

		/**
		 * @brief Manages the global logging configuration for CLI logging.
		 * @details The class manages the global boost logging configuration for 
		 * the debug ind info logging part. It does not manage the dedicated event 
		 * logging facility.
		 */
		class CLILoggingConfigurator
		{
		public:

			/** @brief The name of the log level property */
			static const std::string PROP_LOG_LEVEL;

			/**
			 * @brief Configures a minimal CLI logging. 
			 */
			CLILoggingConfigurator();

			/** 
			 * @brief Frees allocated resources
			 * @details Doesn't change the logging settings
			 */
			~CLILoggingConfigurator() {}

			/**
			 * @brief Parses the application context and configures the global 
			 * logging facilities accordingly.
			 */
			void configureLogger(const Base::ApplicationContext &appContext);

		private:

			/** @brief Log sink which prints to the console */
			boost::shared_ptr<boost::log::sinks::synchronous_sink<
				boost::log::sinks::text_ostream_backend>> stdoutSink_;

		};


	}
}

#endif
