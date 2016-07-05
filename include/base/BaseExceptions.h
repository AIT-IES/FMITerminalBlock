/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file BaseExceptions.h
 * @brief The file contains a set of commonly used exceptions
 * @details The file contains a header only implementation. No .cpp file is
 * needed to use provided exceptions.
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_BASE_BASE_EXCEPTIONS
#define _FMITERMINALBLOCK_BASE_BASE_EXCEPTIONS

#include <stdexcept>
#include <string>

#include <common/fmi_v1.0/fmiModelTypes.h>

namespace FMITerminalBlock
{
	namespace Base
	{

		/**
		 * @brief Indicates an illegal system configuration
		 * @details The exception may be thrown if some configuration value is 
		 * missing or invalid. The invalid configuration may be stored
		 */
		class SystemConfigurationException : public std::invalid_argument
		{
		public:

			/**
			 * @brief C'tor which sets a custom message
			 * @param msg The error message to display
			 */
			SystemConfigurationException(const char* msg): 
				std::invalid_argument(msg), configSet(false), configKey(), 
				configValue() {}

			/**
			 * @brief C'tor which sets a custom message
			 * @param msg The error message to display
			 */
			SystemConfigurationException(const std::string &msg): 
				std::invalid_argument(msg), configSet(false), configKey(), 
				configValue() {}

			/**
			 * @brief C'tor which sets a custom message
			 * @param msg The error message to display
			 * @param key The invalid key
			 * @param value The invalid value
			 */
			SystemConfigurationException(const char* msg, const char* key, 
				const char* value): 
				std::invalid_argument(msg), configSet(true), configKey(key), 
				configValue(value) {}

			/**
			 * @brief C'tor which sets a custom message
			 * @param msg The error message to display
			 * @param key The invalid key
			 * @param value The invalid value
			 */
			SystemConfigurationException(const std::string &msg,
				const std::string &key, const std::string &value):
				std::invalid_argument(msg), configSet(true), configKey(key), 
				configValue(value) {}


			/** @brief Copy C'tor */
			SystemConfigurationException(SystemConfigurationException &ex): 
				std::invalid_argument(ex), configSet(ex.configSet),
				configKey(ex.configKey), configValue(ex.configValue) {}

			/**
			 * @brief Returns whether the invalid configuration key and value was set
			 * @return <code>true</code> if the invalid configuration key and its
			 * value was set
			 */
			bool hasConfig(void) const{ return configSet; }

			/**
			 * @brief Returns the invalid configuration value
			 * @return The invalid configuration value
			 */
			const std::string & getValue(void) const{ return configValue; }

			/**
			 * @brief Returns the invalid configuration key
			 * @return The invalid configuration key
			 */
			const std::string & getKey(void) const{ return configKey; }

		private:

			/**
			 * @brief Flag which indicates whether the configuration values were
			 * previously set
			 */
			const bool configSet;

			/** @brief The invalid configuration key */
			const std::string configKey;

			/** @brief The invalid configuration value */
			const std::string configValue;

		};

		/**
		 * @brief Indicates an exceptional state during solving the model
		 * @details The exception holds a time-stamp indicating the error's
		 * simulation time. If the exact simulation time can't be determined, the
		 * last step's time will be given.
		 */
		class SolverException: public std::invalid_argument
		{
		public:
			/**
			 * @brief Initializes the solver exception
			 * @param msg A descriptive error message
			 * @param time The error's time-stamp
			 */
			SolverException(const char* msg, fmiTime time): 
				std::invalid_argument(msg), time_(time) {}

			/**
			 * @brief Initializes the solver exception
			 * @param msg A descriptive error message
			 * @param time The error's time-stamp
			 */
			SolverException(std::string msg, fmiTime time): 
				std::invalid_argument(msg), time_(time) {}

			/** @brief Copy C'tor */
			SolverException(SolverException &ex): 
				std::invalid_argument(ex), time_(ex.time_) {}

			/**
			 * @brief Returns the error's time-stamp
			 * @return The error's time-stamp
			 */
			fmiTime getTimestamp(void) { return time_; }

		private:
			/** @brief The error's time-stamp */
			fmiTime time_;
		};

	}
}


#endif
