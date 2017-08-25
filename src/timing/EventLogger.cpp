/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file EventLogger.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "timing/EventLogger.h"

#include <assert.h>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <boost/log/expressions/formatters/csv_decorator.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/thread/lock_guard.hpp>
// Essential for date_time formatter!
#include <boost/log/support/date_time.hpp>
#include <fstream>
#include <iomanip>

#include "base/LoggingAttributes.h"

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Timing;
using namespace boost::log;

const std::string EventLogger::PROP_FILE_NAME = "app.timingFile";

EventLogger::EventLogger(): 
	channel_logger_mt(locationUndefined), eventTimeAttribute_(-1.0), 
	objectMutex_()
{
	add_attribute(Base::ATTR_EVENT_TIME, eventTimeAttribute_);
}

void 
EventLogger::addEventFileSink(const Base::ApplicationContext& context)
{

	typedef sinks::synchronous_sink< sinks::text_ostream_backend > EventSink;

	std::string filename = context.getProperty<std::string>(PROP_FILE_NAME,"");
	if(filename != "")
	{
		// Add Logger
		boost::shared_ptr<EventSink> sink = boost::make_shared<EventSink>();
		
		sink->locked_backend()->add_stream(boost::make_shared<std::ofstream>(filename));
		
		sink->set_filter(
			expressions::has_attr(Base::eventTime)
		);

		sink->set_formatter(
			expressions::stream << std::setprecision(8) << std::fixed
			<< expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%w;%H;%M;%S.%f") 
			<< ";" 
			<< expressions::attr<fmiTime>(Base::ATTR_EVENT_TIME)
			<< ";"
			<< expressions::attr<ProcessingStage>("Channel")
			<< ";\""
			<< expressions::smessage // TODO: Should be escaped, but csv_decorator doesn't work
			<< "\""
		);

		// Immediately write a log record to preserve it on cancellation
		sink->locked_backend()->auto_flush(true);

		core::get()->add_sink(sink);
	}
}

void 
EventLogger::logEvent(Event * ev, ProcessingStage stage)
{
	assert(ev != NULL);
	boost::lock_guard<boost::mutex> lock(objectMutex_);

	eventTimeAttribute_.set(ev->getTime());

	record rec = open_record(keywords::channel = stage);
	if(rec)
	{
		record_ostream strm(rec);
		strm << ev->toString();
		strm.flush();
		push_record(boost::move(rec));
	}
}
