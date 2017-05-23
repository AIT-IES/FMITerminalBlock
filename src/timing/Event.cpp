/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file Event.cpp
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#include "timing/Event.h"

#include <assert.h>
#include <boost/any.hpp>
#include <boost/log/trivial.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

using namespace FMITerminalBlock::Timing;

Event::Event(fmiTime time):time_(time)
{
}

std::string
Event::toString(void) const
{
	std::string ret("Event: ");
	boost::format time("time=%1%");
	time % time_;
	ret += time.str();
	return ret;
}

bool Event::isValid(const std::vector<Variable> &values)
{
	for(unsigned i = 0; i < values.size(); i++)
	{
		const boost::any & val = values[i].getValue();

		if (values[i].isTypeUnknown())
		{
			BOOST_LOG_TRIVIAL(debug) << "Value of unknown type found." 
				<< values[i].toString();
		}

		if(!values[i].isValid())
		{
			BOOST_LOG_TRIVIAL(warning) << "Invalid type found. " 
				<< values[i].toString();
			return false;
		}
	}
	return true;
}

std::string 
Event::toString(const std::vector<Variable> &vars)
{
	std::string ret(" variables={");
	for(unsigned i = 0; i < vars.size(); i++)
	{
		ret += vars[i].toString();
		if(i < (vars.size() - 1))
		{
			ret += ", ";
		}
	}
	ret += "}";

	return ret;
}
