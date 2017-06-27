/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file main.cpp
 * @brief Instantiates the main program components and starts the operation
 * @details The Main file currently also handles exceptions and prints an
 * appropriate error message.
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "base/environment-helper.h"

#include <assert.h>
#include <stdexcept>
#include <string>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions/formatters/format.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/core.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/support/date_time.hpp>

#include "base/ApplicationContext.h"
#include "base/BaseExceptions.h"
#include "base/CLILoggingConfigurator.h"
#include "model/AbstractEventPredictor.h"
#include "model/EventPredictorFactory.h"
#include "timing/EventDispatcher.h"
#include "timing/EventLogger.h"
#include "timing/CSVDataLogger.h"
#include "network/NetworkManager.h"

using namespace FMITerminalBlock;

/* Global Function declarations */
void initBasicTerminalLogger(void);

/**
 * @brief Initializes the program and starts the execution
 * @param argc The number of elements stored in argv
 * @param argv The argument vector
 * @returns zero on success
 */
int main (int argc, const char *argv[])
{
	Base::CLILoggingConfigurator loggingConfig;

	// Print copyright notice
	BOOST_LOG_TRIVIAL(info) << "Copyright (c) 2017, AIT Austrian Institute of "
		<< "Technology GmbH.";
	BOOST_LOG_TRIVIAL(info) << "All rights reserved.";
	BOOST_LOG_TRIVIAL(info) << "----------------------------------------------"
		<< "----------------";

	Base::ApplicationContext context;
	try
	{
		// Initialize the application
		context.addCommandlineProperties(argc,argv);
		loggingConfig.configureLogger(context);

		std::shared_ptr<Model::AbstractEventPredictor> predictor;
		predictor = Model::EventPredictorFactory::makeEventPredictor(context);

		predictor->configureDefaultApplicationContext(&context);
		Timing::EventLogger::addEventFileSink(context);

		predictor->init();

		Timing::EventDispatcher dispatcher(context, *predictor);
		Network::NetworkManager nwManager(context, dispatcher);
		
		Timing::CSVDataLogger dataLogger(context);
		dispatcher.addEventListener(&dataLogger);

		// Run the simulation
		dispatcher.run();

	}catch(Base::SystemConfigurationException &ex){
		if(ex.hasConfig())
		{
			BOOST_LOG_TRIVIAL(fatal) << "Invalid system configuration regarding"
				" property " << ex.getKey() << "=\"" << ex.getValue() << "\": " 
				<< ex.what();
		}else{
			BOOST_LOG_TRIVIAL(fatal) << "Invalid system configuration: "
				<< ex.what();
		}
		return 2;
	}catch(Base::SolverException &ex){
		BOOST_LOG_TRIVIAL(fatal)
			<< "An error during solving the model occurred: "
			<< ex.what() << " (At time " << ex.getTimestamp() << ")"; 
		return 4;
	}catch(std::invalid_argument &ex){
		BOOST_LOG_TRIVIAL(fatal) << "Invalid command line argument detected: " 
			<< ex.what();
		BOOST_LOG_TRIVIAL(info) << "Usage: " 
			<< context.getProperty<std::string>(
				Base::ApplicationContext::PROP_PROGRAM_NAME, "FMITerminalBlock")
			<< " { <property>=<value> }";
		BOOST_LOG_TRIVIAL(info) << "Please consult the user documentation for "
			<< "more details on the usage of the program.";
		return 1;
	}catch(std::runtime_error &ex){
		BOOST_LOG_TRIVIAL(fatal) << "A runtime error occurred: " << ex.what();
		return 3;
	}catch(...){
		BOOST_LOG_TRIVIAL(fatal) << "Oops: Unspecified error happened (Sorry, I "
			"know that this isn't very helpful)";
		return 125;

	}

}
