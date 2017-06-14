/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CSVDataLogger.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "timing/CSVDataLogger.h"

#include <boost/log/trivial.hpp>

#include "base/BaseExceptions.h"

using namespace FMITerminalBlock::Timing;

const std::string CSVDataLogger::PROP_CSV_FILE_NAME = "app.dataFile";

const char* CSVDataLogger::FMI_TYPE_NAMES[] = {
	"fmiReal","fmiInteger","fmiBoolean","fmiString","fmiUnknown"
};

CSVDataLogger::CSVDataLogger(std::ostream &destination,
	Base::ApplicationContext &context): 
	outputFileStream_(), header_(), outputStream_(NULL)
{
	if (context.hasProperty(PROP_CSV_FILE_NAME))
	{
		throw Base::SystemConfigurationException("The CSV file name must not be "
			"specified while externally setting the data destination.");
	}
	outputStream_ = &destination;
	initHeader(context.getInputChannelMapping(), 
		context.getOutputChannelMapping());
}

CSVDataLogger::CSVDataLogger(Base::ApplicationContext &context): 
	outputFileStream_(), header_(), outputStream_(NULL)
{
	if (context.hasProperty(PROP_CSV_FILE_NAME))
	{
		openFileStream(context.getProperty<std::string>(PROP_CSV_FILE_NAME));
		initHeader(context.getInputChannelMapping(), 
		context.getOutputChannelMapping());
	}
}

CSVDataLogger::~CSVDataLogger()
{
	if (outputFileStream_ && outputFileStream_->is_open())
	{
		try {
			outputFileStream_->close();
		} catch (...) {
			BOOST_LOG_TRIVIAL(warning) << "Unable to close the opened CSV data file";
		}
	}
}

void CSVDataLogger::eventTriggered(Event * ev)
{
	assert(ev != NULL);
	
	if (outputStream_ == NULL) return;

	auto &out = *outputStream_;
	out << ev->getTime() << SEPARATOR;
	
	auto variables = ev->getVariables();
	for (unsigned i = 0; i < header_.size(); i++)
	{
		auto var = findVariable(header_[i], variables);
		append(var);
		if ((i + 1) < header_.size()) out << SEPARATOR;
	}

	out << '\n';
	out.flush();
}

void CSVDataLogger::openFileStream(const std::string &filename)
{
	assert(!outputFileStream_);
	assert(!outputStream_);

	outputFileStream_ = std::unique_ptr<std::ofstream>(
		new std::ofstream(filename, std::ios_base::trunc | std::ios_base::out));
	if (outputFileStream_->fail())
	{
		outputFileStream_.reset();
		throw Base::SystemConfigurationException(
			"Couldn't open CSV file for writing.", PROP_CSV_FILE_NAME, filename);
	}

	outputStream_ = outputFileStream_.get();
}

void CSVDataLogger::initHeader(const Base::ChannelMapping *inChannelMapping,
	const Base::ChannelMapping *outChannelMapping)
{
	assert(header_.size() == 0);
	assert(outputStream_ != NULL);

	assert(inChannelMapping != NULL);
	assert(outChannelMapping != NULL);

	// Populate header variable
	auto allVarIDs = inChannelMapping->getAllVariableIDs();
	header_.insert(header_.end(), allVarIDs.begin(), allVarIDs.end());
	allVarIDs = outChannelMapping->getAllVariableIDs();
	header_.insert(header_.end(), allVarIDs.begin(), allVarIDs.end());

	auto nameHeader = inChannelMapping->getAllVariableNames();
	auto allVarNames = outChannelMapping->getAllVariableNames();
	nameHeader.insert(nameHeader.end(), allVarNames.begin(), allVarNames.end());

	// Write the content to the CSV file
	appendHeader(nameHeader, header_);
	outputStream_->flush();

	if (outputStream_->fail())
	{
		throw Base::SystemConfigurationException("Cannot write to CSV data file");
	}
}

void CSVDataLogger::appendHeader(const std::vector<std::string> &allNames,
	const std::vector<Base::PortID> &allIDs)
{
	assert(outputStream_ != NULL);
	auto &out = *outputStream_;

	out << "\"time\"" << SEPARATOR;
	append(allNames);
	out << "\n";
	append(FMI_TYPE_NAMES[fmiTypeReal]);
	out << SEPARATOR;
	appendTypes(allIDs);
	out << "\n";
}

void CSVDataLogger::appendTypes(const std::vector<Base::PortID> &ports)
{
	assert(outputStream_ != NULL);
	auto &out = *outputStream_;

	for (unsigned int i = 0; i < ports.size(); i++)
	{
		int typeID = ((int) ports[i].first);
		assert(0 <= typeID);
		assert(typeID < sizeof(FMI_TYPE_NAMES)/sizeof(FMI_TYPE_NAMES[0]));
		append(FMI_TYPE_NAMES[typeID]);
		if ((i + 1) < ports.size()) out << SEPARATOR;
	}
}

void CSVDataLogger::append(const std::vector<std::string> &fields)
{
	assert(outputStream_ != NULL);
	auto &out = *outputStream_;

	for (unsigned int i = 0; i < fields.size(); i++)
	{
		append(fields[i]);
		if ((i + 1) < fields.size()) out << SEPARATOR;
	}
}

void CSVDataLogger::append(const std::string &value)
{
	assert(outputStream_ != NULL);
	auto &out = *outputStream_;

	out << '"';
	for (unsigned int i = 0; i < value.size(); i++)
	{
		if (value[i] == '"')
		{
			out << '"' << '"';
		}
		else
		{
			out << value[i];
		}
	}
	out << '"';
}

void CSVDataLogger::append(boost::optional<Variable> var)
{
	assert(outputStream_ != NULL);
	auto &out = *outputStream_;

	if (var)
	{
		switch (var->getID().first)
		{
			case fmiTypeReal: 
				out << var->getRealValue(); break;
			case fmiTypeInteger:
				out << var->getIntegerValue(); break;
			case fmiTypeBoolean:
				out << (unsigned int) var->getBooleanValue(); break;
			case fmiTypeString:
				append(var->getStringValue()); break;
			case fmiTypeUnknown:
				append("unknown-type"); break;
			default:
				assert(false);
		}
	}
}

boost::optional<Variable> CSVDataLogger::findVariable(Base::PortID id,
	const std::vector<Variable> &variables)
{
	boost::optional<Variable> ret;

	for (auto it = variables.begin(); it != variables.end(); ++it)
	{
		if (it->getID() == id)
		{
			ret = *it;
			return ret;
		}
	}

	return ret;
}
