/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file testCSVDataLogger.cpp
 * @brief Tests the CSV data logging facility
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#define BOOST_TEST_MODULE testCSVDataLogger
#include <boost/test/unit_test.hpp>

#include <fstream>
#include <sstream>
#include <vector>

#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

#include "timing/CSVDataLogger.h"
#include "timing/StaticEvent.h"
#include "base/BaseExceptions.h"
#include "base/ApplicationContext.h"

using namespace FMITerminalBlock::Base;
using namespace FMITerminalBlock::Timing;

namespace data = boost::unit_test::data;

/** @brief Instantiates a logger with an empty configuration */
BOOST_AUTO_TEST_CASE(testEmptyConfig)
{
	ApplicationContext appContext;
	CSVDataLogger logger(appContext);

	StaticEvent ev(0.1, {});
	logger.eventTriggered(&ev);
}

/** @brief Test invalid path */
BOOST_AUTO_TEST_CASE(testInvalidPath)
{
	ApplicationContext appContext;
	const char *args[] = {
		"testCSVLogger", "app.dataFile=not-all-paths-lead-to-rome/rome.txt"
	};
	appContext.addCommandlineProperties(sizeof(args) / sizeof(args[0]), args);

	BOOST_CHECK_THROW(CSVDataLogger logger(appContext), 
		Base::SystemConfigurationException);
}

/** @brief Test stream error handling while writing the header line */
BOOST_AUTO_TEST_CASE(testInvalidStream)
{
	ApplicationContext appContext;
	std::ofstream stream("i-am-the-one/who-sold-his-soul.hammerfall",
		std::ios_base::trunc | std::ios_base::out);
	BOOST_CHECK(stream.fail());
	BOOST_CHECK_THROW(CSVDataLogger logger(stream, appContext), 
		Base::SystemConfigurationException);
}

/** @brief Returns a configuration which covers all in- and output types */
ApplicationContext makeCompleteAppContext()
{
	ApplicationContext appContext;
	const char *args[] = {
		"testCSVLogger", 
		"in.0.0=ia", "in.0.0.type=0", 
		"in.0.1=ib", "in.0.1.type=1", 
		"in.0.2=ic", "in.0.2.type=2",
		"in.0.3=id", "in.0.3.type=3",

		"out.0.0=oa", "out.0.0.type=0", 
		"out.0.1=ob", "out.0.1.type=1", 
		"out.0.2=oc", "out.0.2.type=2",
		"out.0.3=od", "out.0.3.type=3"
	};
	appContext.addCommandlineProperties(sizeof(args) / sizeof(args[0]), args);
	return appContext;
}

/** @brief Constructs a single event with one variable only */
template<typename type>
StaticEvent makeEventVarOnly(int varID, FMIVariableType ftype, 
	type value, fmiReal time)
{
	std::vector<Variable> vars;
	vars.push_back(Variable(PortID(ftype, varID), value));
	return StaticEvent(time, vars);
}

/** @brief Generates the header string for the complete CSV header */
std::string makeCompleteCSVHeader()
{
	return "\"time\";\"ia\";\"ib\";\"ic\";\"id\";\"oa\";\"ob\";\"oc\";\"od\"\n"
		"\"fmiReal\";\"fmiReal\";\"fmiInteger\";\"fmiBoolean\";\"fmiString\";"
		"\"fmiReal\";\"fmiInteger\";\"fmiBoolean\";\"fmiString\"\n";
}

ApplicationContext SINGLE_EVENT_CONTEXT[] = {
	makeCompleteAppContext(),
	makeCompleteAppContext(),
	makeCompleteAppContext(),
	makeCompleteAppContext(),

	makeCompleteAppContext(),
	makeCompleteAppContext(),
	makeCompleteAppContext(),
	makeCompleteAppContext()
};
StaticEvent SINGLE_EVENT_EVENT[] = {
	makeEventVarOnly(1, fmiTypeReal, (fmiReal) 0.1, 0.0),
	makeEventVarOnly(1, fmiTypeInteger, (fmiInteger) -42, 0.1),
	makeEventVarOnly(1, fmiTypeBoolean, (fmiBoolean) fmiTrue, 0.2),
	makeEventVarOnly(1, fmiTypeString,  std::string("\"a,bäd;String"), 0.3),

	makeEventVarOnly(0, fmiTypeReal, (fmiReal) 0.2, 0.4),
	makeEventVarOnly(0, fmiTypeInteger, (fmiInteger) -1, 0.5),
	makeEventVarOnly(0, fmiTypeBoolean, (fmiBoolean) fmiFalse, 0.6),
	makeEventVarOnly(0, fmiTypeString,  std::string("aGoodString"), 0.7)
};
std::string SINGLE_EVENT_REFERENCE[] = {
	makeCompleteCSVHeader() + "0;0.1;;;;;;;\n",
	makeCompleteCSVHeader() + "0.1;;-42;;;;;;\n",
	makeCompleteCSVHeader() + "0.2;;;1;;;;;\n",
	makeCompleteCSVHeader() + "0.3;;;;\"\"\"a,bäd;String\";;;;\n",

	makeCompleteCSVHeader() + "0.4;;;;;0.2;;;\n",
	makeCompleteCSVHeader() + "0.5;;;;;;-1;;\n",
	makeCompleteCSVHeader() + "0.6;;;;;;;0;\n",
	makeCompleteCSVHeader() + "0.7;;;;;;;;\"aGoodString\"\n"
};

/** @brief Test multiple renderings on a single event */
BOOST_DATA_TEST_CASE(testSingleEvent, 
	data::make(SINGLE_EVENT_CONTEXT)^data::make(SINGLE_EVENT_EVENT)^
		data::make(SINGLE_EVENT_REFERENCE), 
	appContext, evt, refString)
{
	ApplicationContext context = appContext;
	StaticEvent ev = evt;
	std::ostringstream stream;

	{
		CSVDataLogger logger(stream, context);
		logger.eventTriggered(&ev);
	}

	BOOST_CHECK_EQUAL(stream.str(), refString);
}

/** @brief Test multiple events in a row */
BOOST_AUTO_TEST_CASE(testMultipleEvents)
{
	ApplicationContext appContext = makeCompleteAppContext();
	std::ostringstream stream;

	{
		CSVDataLogger logger(stream, appContext);
		StaticEvent ev1 = makeEventVarOnly(1, fmiTypeReal, (fmiReal) 0.1, 0.0);
		StaticEvent ev2 = makeEventVarOnly(0, fmiTypeReal, (fmiReal) 0.25, 0.1);

		logger.eventTriggered(&ev1);
		logger.eventTriggered(&ev2);
	}

	BOOST_CHECK_EQUAL(stream.str(), 
		makeCompleteCSVHeader() + "0;0.1;;;;;;;\n" + "0.1;;;;;0.25;;;\n");
}
