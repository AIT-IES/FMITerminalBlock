/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CSVDataLogger.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_TIMING_CSV_DATA_LOGGER
#define _FMITERMINALBLOCK_TIMING_CSV_DATA_LOGGER

#include <ostream>
#include <fstream>
#include <memory>

#include <boost/optional.hpp>

#include "timing/Event.h"
#include "base/ApplicationContext.h"
#include "timing/EventListener.h"

namespace FMITerminalBlock
{
	namespace Timing
	{
		/**
		 * @brief Listens for incoming events and writes the variables to a stream
		 * @details <p>The class supports files as well as arbitrary output 
		 * streams. In contrast to the event logging facility, the data logging 
		 * facility focuses on the simulation outcome only. An event is logged iff
		 * it is also actually taken. </p>
		 * <p> As soon as the class is instantiated and activated, a file header 
		 * and each event is logged. Each logged line corresponds to a single 
		 * event. Only the variables which are actually set will be logged. In 
		 * case the variable is not present in the file, the corresponding place 
		 * will be left empty. The header will contain the name of the variables in
		 * the first row and a string describing each variable type in the second 
		 * row.</p>
		 */
		class CSVDataLogger: public EventListener
		{
		public:

			/** @brief Property name of the file directive */
			static const std::string PROP_CSV_FILE_NAME;

			/** @details The character which is used to separate two fields */
			static const char SEPARATOR = ';';

			/**
			 * @brief Logs all output to the given destination.
			 * @details The C'tor is mainly in place to test the operation of the 
			 * CSVDataLogger. It is assumed that the given configuration does not 
			 * contain an output file directive. In case an invalid configuration is
			 * found, a SystemConfigurationException may be thrown.
			 * @param destination The sink to put the CSV stream to
			 * @param context The configuration of the CSV logger
			 */
			CSVDataLogger(std::ostream &destination, 
				Base::ApplicationContext &context);

			/**
			 * @brief Parses the given configuration and initialized the CSVLogger
			 * @details In case an invalid configuration is found, a 
			 * Base::SystemConfigurationException will be thrown.
			 * @param context The application context which describes the 
			 * configuration of the logger
			 */
			CSVDataLogger(Base::ApplicationContext &context);

			/** @brief Frees allocated resources */
			virtual ~CSVDataLogger();
			
			/** 
			 * @brief Writes the event to the output stream
			 * @details It is assumed that the given event instant is always valid.
			 */
			virtual void eventTriggered(Event * ev);

		private:

			/** @brief The name of the FMI type per code value */
			static const char* FMI_TYPE_NAMES[5];

			/** @brief The logging destination or NULL, if no logger is registered */
			std::ostream *outputStream_ = NULL;

			/** @brief A pointer to the managed output file, if any. */
			std::unique_ptr<std::ofstream> outputFileStream_;

			/** @brief the PortIDs in the order of their occurrence */
			std::vector<Base::PortID> header_;

			/** 
			 * @brief Opens the given file for writing and sets the corresponding 
			 * streams
			 * @details a Base::SystemConfigurationException will be thrown in case 
			 * the given filename is invalid.
			 */
			void openFileStream(const std::string &filename);

			/**
			 * @brief Writes the file header and initializes the header variables
			 * @details The function assumes that outputStream_ is properly 
			 * initialized.
			 * @param inChannelMapping The input channel configuration
			 * @param outChannelMapping The output channel configuration
			 */
			void initHeader(const Base::ChannelMapping *inChannelMapping, 
				const Base::ChannelMapping *outChannelMapping);

			/**
			 * @brief Writes the two header lines including a newline character
			 * @param allNames The variable names which are managed by the logger
			 * @param allIDs The variable IDs in the same order as the variable names
			 */
			void appendHeader(const std::vector<std::string> &allNames, 
				const std::vector<Base::PortID> &allIDs);

			/** 
			 * @brief Appends the vector of ports to the output stream
			 * @details Each element is separated by the default separator. For each 
			 * port id the type of the port will be encoded. The identifier will not 
			 * be printed.
			 */
			void appendTypes(const std::vector<Base::PortID> &ports);

			/** 
			 * @brief Appends the vector of string to the output stream
			 * @details Each element is separated by the default separator
			 */
			void append(const std::vector<std::string> &fields);

			/** 
			 * @brief Escapes and encapsulates the given string and appends it to 
			 * the stream
			 */
			void append(const std::string &value);

			/**
			 * @brief Appends the given variable, if it holds a value
			 * @details If var is not populated, the stream is left unmodified
			 */
			void append(boost::optional<Variable> var);

			/**
			 * @brief Finds the variable with the given id in the list of variables
			 * @details If the variable is not present in the list, an empty object 
			 * will be returned. 
			 * @param id The id to search
			 * @param values The list of variables to examine.
			 */
			static boost::optional<Variable> findVariable(Base::PortID id, 
				const std::vector<Variable> &variables);
		};
	}
}

#endif