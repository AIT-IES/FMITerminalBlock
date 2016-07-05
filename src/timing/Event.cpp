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

bool Event::isValid(const std::vector<Event::Variable> &values)
{
	for(unsigned i = 0; i < values.size(); i++)
	{
		const boost::any & val = values[i].second;
		bool ok;
		switch(values[i].first.first)
		{
		case fmiTypeReal:
			ok = val.type() == typeid(fmiReal);
			break;
		case fmiTypeInteger:
			ok = val.type() == typeid(fmiInteger);
			break;
		case fmiTypeBoolean:
			ok = val.type() == typeid(fmiBoolean);
			break;
		case fmiTypeString:
			ok = val.type() == typeid(std::string);
			break;
		case fmiTypeUnknown:
			ok = false;
			BOOST_LOG_TRIVIAL(debug) << "Value of unknown type found. (Type=<" 
				<< (int) values[i].first.first << ", " << values[i].first.second << ">, index=" 
				<< i << ")";
			break;
		default:
			assert(false);
		}
		if(!ok)
		{
			BOOST_LOG_TRIVIAL(warning) << "Invalid type found. (Type=<" 
				<< (int) values[i].first.first << ", " << values[i].first.second << ">, index=" 
				<< i << ")";
			return false;
		}
	}
	return true;
}

std::string 
Event::toString(const std::vector<Event::Variable> &vars)
{
	std::string ret("variables={");
	boost::format var("<t:%1%, id:%2%>=\"%3%\"");
		for(unsigned i = 0; i < vars.size(); i++)
		{
			var.clear();
			var % (int) vars[i].first.first;
			var % (int) vars[i].first.second;
		
			switch(vars[i].first.first)
			{
			case fmiTypeReal:
				var % boost::any_cast<fmiReal>(vars[i].second);
				break;
			case fmiTypeInteger:
				var % boost::any_cast<fmiInteger>(vars[i].second);
				break;
			case fmiTypeBoolean:
				var % (boost::any_cast<fmiBoolean>(vars[i].second) == fmiTrue?"true":"false");
				break;
			case fmiTypeString:
				var % boost::any_cast<std::string>(vars[i].second);
				break;
			default:
				assert(false);
			}

			ret += var.str();
			if(i < (vars.size() - 1))
			{
				ret += ", ";
			}
		}
		ret += "}";

		return ret;
}
