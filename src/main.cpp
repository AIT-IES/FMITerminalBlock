/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file main.cpp
 * @brief Instantiates the main program components and starts the operation
 * @details The Main file currently also handles exceptions and prints an
 * appropriate error message.
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
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
#include "model/EventPredictor.h"
#include "timing/EventDispatcher.h"
#include "timing/EventLogger.h"
#include "network/NetworkManager.h"

using namespace FMITerminalBlock;

/* Global Function declarations */
void initTerminalLogger(void);

/**
 * @brief Initializes the program and starts the execution
 * @param argc The number of elements stored in argv
 * @param argv The argument vector
 * @returns zero on success
 */
int main (int argc, const char *argv[])
{
	initTerminalLogger();

	// Print copyright notice
	BOOST_LOG_TRIVIAL(info) << "Copyright (c) 2015, AIT Austrian Institute of "
		<< "Technology GmbH.";
	BOOST_LOG_TRIVIAL(info) << "All rights reserved.";
	BOOST_LOG_TRIVIAL(info) << "----------------------------------------------"
		<< "----------------";

	Base::ApplicationContext context;
	try
	{
		// Initialize the application
		context.addCommandlineProperties(argc,argv);
		Model::EventPredictor predictor(context);

		context.addSensitiveDefaultProperties(predictor.getModelDescription());
		Timing::EventLogger::addEventFileSink(context);

		predictor.init();

		Timing::EventDispatcher dispatcher(context, predictor);
		Network::NetworkManager nwManager(context, dispatcher);

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
		BOOST_LOG_TRIVIAL(fatal) << "Invalid commandline argument detected: " 
			<< ex.what();
		BOOST_LOG_TRIVIAL(info) << "Usage: " 
			<< context.getProperty<std::string>(
				Base::ApplicationContext::PROP_PROGRAM_NAME, "FMITerminalBlock")
			<< " " << Model::EventPredictor::PROP_FMU_PATH << "=<path> "
			<< Model::EventPredictor::PROP_FMU_NAME
			<< "=<name> { <property>=<value> }";
		return 1;
	}catch(std::runtime_error &ex){
		BOOST_LOG_TRIVIAL(fatal) << "A runtime error occurred: " << ex.what();
		return 3;
	}catch(...){
		BOOST_LOG_TRIVIAL(fatal) << "Oops: Unspecified error happened (Sorry, I know"
			" that this isn't very helpful)";
		return 125;

	}

}

/**
 * @brief tries to initialize a simple logger
 * @details The logger will print debug information to stdout
 */
void initTerminalLogger()
{
	boost::log::add_common_attributes();
	boost::shared_ptr<boost::log::sinks::synchronous_sink< 
		boost::log::sinks::text_ostream_backend > > sink 
		= boost::log::add_console_log();
	
	sink->locked_backend()->auto_flush(true);
	sink->set_formatter( 
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
	sink->set_filter(
		! boost::log::expressions::has_attr(Timing::eventTime)
	);
}
