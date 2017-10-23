/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file AbstractConfigProvider.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_BASE_ABSTRACT_CONFIG_PROVIDER
#define _FMITERMINALBLOCK_BASE_ABSTRACT_CONFIG_PROVIDER

#include <exception>
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/format.hpp>


namespace FMITerminalBlock
{
	namespace Base
	{
		/**
		 * @brief Manages a property-based configuration
		 * @details Each AbstractConfigProvider maintains function to quickly
		 * access properties of a boost::property_tree sub tree. Validating access
		 * functions are provided which thrown an exception in case of an error.
		 * Hence, common user input validation can be reduced to a minimum.
		 */
		class AbstractConfigProvider
		{
		public:
			/** @brief Does nothing special */
			AbstractConfigProvider();

			/** @brief Frees allocated resources */
			virtual	~AbstractConfigProvider() {}

						/**
			 * @brief Returns the property's value
			 * @details The function queries the global configuration. It will throw
			 * std::invalid_argument, if the property was not found or if it couldn't
			 * be converted properly.
			 * @param path The property's path
			 * @return The property's value
			 */
			template<typename Type>
			Type getProperty(const std::string &path) const
			{
				try
				{
					return getConfig().get<Type>(path);
				}catch(std::exception &ex){
					boost::format err("The property \"%1%\" was not found: %2%");
					err % path % ex.what();
					throw std::invalid_argument(err.str());
				}
			}

			/**
			 * @brief Returns the property's value or its given default value
			 * @details The function queries the global configuration. It will throw
			 * std::invalid_argument if the value couldn't be converted properly.
			 * @param path The property's path
			 * @param def The property's default value
			 * @return The property's value or its default.
			 */
			template<typename Type>
			Type getProperty(const std::string &path, Type def) const
			{
				if(!hasProperty(path))
				{
					return def;
				}else{
					return getProperty<Type>(path);
				}
			}

			/**
			 * @brief Queries the property and checks its value
			 * @details If the property contains an invalid value,
			 * Base::SystemConfigurationException will be thrown.
			 * @param path The key to query
			 * @param def The property's default value
			 * @return A positive double value r, r >= 0
			 */
			double getPositiveDoubleProperty(const std::string &path, double def)
				const;

			/**
			 * @brief Queries the property and checks its value
			 * @details If the property doesn't exist or if the property contains an
			 * invalid value, Base::SystemConfigurationException will be thrown.
			 * @param path The key to query
			 * @return A positive double value r, r >= 0
			 */
			double getPositiveDoubleProperty(const std::string &path) const;


			/**
			 * @brief Queries the property and checks its value
			 * @details If the property contains an invalid value,
			 * Base::SystemConfigurationException will be thrown.
			 * @param path The key to query
			 * @param def The property's default value
			 * @return A real positive double value r, r > 0
			 */
			double getRealPositiveDoubleProperty(const std::string &path, double def)
				const;

			/**
			 * @brief Queries the property and checks its value
			 * @details If the property contains an invalid value or if it doesn't
			 * exist, Base::SystemConfigurationException will be thrown.
			 * @param path The key to query
			 * @return A real positive double value r, r > 0
			 */
			double getRealPositiveDoubleProperty(const std::string &path) const;

			/**
			 * @brief Returns the subtree given by the path string.
			 * @details If the path is not present in the global configuration
			 * Base::SystemConfigurationException will be thrown.
			 * @param path The property's path identifier
			 * @return The subtree given by the path string
			 */
			const boost::property_tree::ptree & getPropertyTree(
				const std::string &path)
				const;

			/**
			 * @brief Returns whether the properties already contain the given key
			 * @param key The checked path which is given as a valid zero terminated
			 * cstring
			 * @return <code>true</code> if the properties already contain the given
			 * key
			 */
			bool hasProperty(const char * key) const;

			/**
			 * @brief Returns whether the properties already contain the given key
			 * @param key The path to check
			 * @return <code>true</code> if the properties already contain the given
			 * key
			 */
			bool hasProperty(const std::string &key) const;

		protected:
			/**
			 * @brief Returns a reference to the root configuration object
			 * @details The reference needs to be valid until the object is 
			 * destroyed. The getConfig() function must be implemented by the 
			 * inheriting class.
			 */
			virtual const boost::property_tree::ptree& getConfig() const = 0;
		};

	}
}

#endif
