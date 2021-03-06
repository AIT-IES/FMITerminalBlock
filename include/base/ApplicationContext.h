/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file ApplicationContext.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_BASE_APPLICATION_CONTEXT
#define _FMITERMINALBLOCK_BASE_APPLICATION_CONTEXT

#include <base/ChannelMapping.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/format.hpp>
// Fixes an include dependency flaw/feature(?) of ModelDescription.h
#include <common/fmi_v1.0/fmiModelTypes.h>
#include <import/base/include/ModelDescription.h>
#include <string>
#include <initializer_list>
#include <vector>

/**
 * @brief returns the number of arguments in a valid argument vector array.
 * @details It is assumed that the argument vector array argv is null 
 * terminated. Hence, one element will be subtracted from the total count of 
 * elements. The macro is mainly intended for testing purpose. 
 */
#define ARG_NUM_OF_ARGV( argv ) (sizeof((argv)) / sizeof((argv)[0]) - 1)

namespace FMITerminalBlock
{
	namespace Base
	{

		/**
		 * @brief Utility class which provides some application scoped information
		 * and functionality.
		 * @details <p>It encapsulates the configuration structure as well as some
		 * commonly used functionality. It provides a simple interface to retrieve
		 * and check configuration values and to obtain the channel mapping. The
		 * ApplicationContext class is intended to be passed to all program modules
		 * which require a dynamic configuration.</p>
		 * <p> In not state otherwise, function's which return a property's value or
		 * a subtree access the global configuration which is maintained by the
		 * ApplicationContext object. In general, properties are accessed via the
		 * default path identifier. Each hierarchic level in the property tree is
		 * separated by a single dot character.
		 * </p>
		 */
		class ApplicationContext
		{
		public:

			/** @brief The key of the program-name property */
			static const char* const PROP_PROGRAM_NAME;

			/** @brief The key of the start time property */
			static const std::string PROP_START_TIME;

			/** @brief The key of the look-ahead horizon time property */
			static const std::string PROP_LOOK_AHEAD_TIME;
			
			/** @brief The key of the integrator step-size property */
			static const std::string PROP_LOOK_AHEAD_STEP_SIZE;

			/** @brief The key of the integrator step-size property */
			static const std::string PROP_INTEGRATOR_STEP_SIZE;

			/** @brief The key of the output channel property */
			static const std::string PROP_OUT;

			/** @brief The key of the input channel property */
			static const std::string PROP_IN;


			/**
			 * @brief Default C'tor initializing an empty application context object
			 */
			ApplicationContext();

			/**
			 * @brief Initializes the application context with the given set of 
			 * arguments
			 * @details The C'tor is mainly intended for debugging purpose to 
			 * quickly create a new ApplicationContext. In case some of the given 
			 * properties are ill formatted, a Base::SystemConfigurationException 
			 * will be thrown. A default program name will be set in order to avoid 
			 * spurious exceptions.
			 * @param initList A list of strings which are formatted according to 
			 * the command line property format
			 */
			ApplicationContext(std::initializer_list<std::string> initList);

			/**
			 * @brief Frees allocated resources
			 */
			~ApplicationContext();

			/**
			 * @brief Parses the command line argument list and appends the
			 * information.
			 * @details If the given argument vector is invalid a
			 * std::invalid_argument will be thrown. Each argument must have a
			 * key=value format. Each given key must be unique.
			 * @param argc The number of arguments in the argument vector
			 * @param argv The argument vector
			 */
			void addCommandlineProperties(int argc, const char *argv[]);

			/**
			 * @brief Parses the command line argument list and appends the 
			 * information
			 * @details It is assumed that every entry is a valid key=value pair. No
			 * program name must be given. In case an invalid entry is found, a 
			 * std::invalid_argument will be thrown. The function is mainly intended 
			 * to ease debugging.
			 * @param args The list of arguments to add.
			 */
			void addCommandlineProperties(const std::vector<std::string> &args);

			/**
			 * @brief Generates sensitive default values based on the model 
			 * description and adds them.
			 * @details Previously set properties are not overwritten. The function
			 * assumes that the given reference is valid. If some previously set 
			 * property is invalid, Base::SystemConfigurationException will be thrown.
			 * @param description A reference to the model's static description
			 */
			void addSensitiveDefaultProperties(const ModelDescription * description);

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
					return config_.get<Type>(path);
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

			/**
			 * @brief Returns a pointer to the global output Base::ChannelMapping object
			 * @details The first invocation of the function will create the object. 
			 * Subsequent configuration changes may not be reflected by the output 
			 * channel mapping object. The function may throw a 
			 * SystemConfigurationException if some properties are missing. In this 
			 * case no object is generated.
			 */
			const ChannelMapping * getOutputChannelMapping(void);

			/**
			 * @brief Returns a pointer to the global input Base::ChannelMapping object
			 * @details The first invocation of the function will create the object.
			 * Subsequent configuration changes may not be reflected by the input
			 * channel mapping object. The function may throw a
			 * SystemConfigurationException if some properties are missing. In this
			 * case no object is generated.
			 */
			const ChannelMapping * getInputChannelMapping(void);

			/**
			 * @brief Returns a human readable string representation
			 * @details The function will not construct a channel mapping. In case 
			 * the channel mapping was not constructed beforehand, it will not be 
			 * included in the output.
			 */
			std::string toString() const;

		private:

			/** @brief Size of the internal error message buffers */
			static const int ERR_MSG_SIZE = 256;

			/**
			 * @brief The global configuration which stored the application's
			 * parameters
			 * @details The tree has to be populated by loading the program's 
			 * configuration sources such as CMD arguments or sensitive default
			 * values.
			 */
			boost::property_tree::ptree config_;

			/**
			 * @brief The globally unique source of PortIDs.
			 * @details The object is used to create unique PortIDs across multiple
			 * channel mapping objects.
			 */
			PortIDDrawer portIDSource_;

			/**
			 * @brief Pointer to the output channel mapping configuration
			 * @details The object will be created by the first query using
			 * getOutputChannelMapping().
			 */
			ChannelMapping * outputChannelMap_;

			/**
			 * @brief Pointer to the input channel mapping configuration.
			 * @details The object will be created by the first query using
			 * getInputChannelMapping().
			 */
			ChannelMapping * inputChannelMap_;

			/**
			 * @brief Extracts the key-value pair and adds it to the global
			 * configuration
			 * @details It is expected that the key is not empty and that the two
			 * parts are separated by a = sign. If the given option is invalid,
			 * std::invalid_argument will be thrown.
			 * @param opt A reference to the option string
			 * @param i The option's index used to generate meaningful error messages
			 */
			void addCommandlineOption(const std::string &opt, int i);

			/**
			 * @brief Returns a newly created channel mapping object.
			 * @details The object must be deleted outside the function. If 
			 * the ChannelMapping cannot be created, a 
			 * Base::SystemConfigurationException will be thrown.
			 * @param propertyPrefix The prefix of the channels inside the property 
			 * tree. The sting is not stored and may be freed after the function 
			 * returns.
			 */
			ChannelMapping * newChannelMapping(const std::string &propertyPrefix);

		};

		/**
		 * @brief Prints the content of the ApplicationContext
		 */
		// Must be declared in the namespace of the argument! 
		// -> Otherwise some boost macros cannot resolve it
		std::ostream& operator<<(std::ostream& stream, 
			const ApplicationContext& appContext);
	}
}

#endif
