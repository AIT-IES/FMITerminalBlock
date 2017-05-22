/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file Variable.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "timing/Variable.h"

#include <assert.h>

#include <boost/format.hpp>

using namespace FMITerminalBlock::Timing;
using namespace FMITerminalBlock;

Variable::Variable(): id_(fmiTypeUnknown, 0), data_()
{
}

Variable::Variable(const Base::PortID &id, const boost::any &data):
	id_(id), data_(data)
{
}

Variable::Variable(const std::pair <Base::PortID, boost::any> &pair) :
	id_(pair.first), data_(pair.second)
{
}

Base::PortID Variable::getID() const
{
	return id_;
}

boost::any Variable::getValue() const
{
	assert(isValid());
	return data_;
}

fmiReal Variable::getRealValue() const
{
	assert(id_.first == fmiTypeReal && isValid());
	return boost::any_cast<fmiReal>(data_);
}

fmiInteger Variable::getIntegerValue() const
{
	assert(id_.first == fmiTypeInteger && isValid());
	return boost::any_cast<fmiInteger>(data_);
}

fmiBoolean Variable::getBooleanValue() const
{
	assert(id_.first == fmiTypeBoolean && isValid());
	return boost::any_cast<fmiBoolean>(data_);
}

const std::string Variable::getStringValue() const
{
	assert(id_.first == fmiTypeString && isValid());
	return boost::any_cast<std::string>(data_);
}

void Variable::setID(const Base::PortID &id)
{
	id_ = id;
}

void Variable::setValue(const boost::any &value)
{
	data_ = value;
}

bool Variable::isValid() const
{
	bool ok;

	switch(id_.first)
	{
	case fmiTypeReal:
		ok = data_.type() == typeid(fmiReal);
		break;
	case fmiTypeInteger:
		ok = data_.type() == typeid(fmiInteger);
		break;
	case fmiTypeBoolean:
		ok = data_.type() == typeid(fmiBoolean);
		break;
	case fmiTypeString:
		ok = data_.type() == typeid(std::string);
		break;
	case fmiTypeUnknown:
		ok = false;
		break;
	default:
		assert(false);
	}

	return ok;
}

bool Variable::isTypeUnknown() const
{
	return id_.first == fmiTypeUnknown;
}

std::string Variable::toString() const
{
	boost::format varStr("Variable: <%1%, id:%2%>=%3%");
	static const char* typeNames[] = {"fmiReal", "fmiTypeInteger", 
		"fmiTypeBoolean", "fmiTypeString", "fmiTypeUnknown"};

	assert(((int) id_.first) < sizeof(typeNames) / sizeof(typeNames[0]));
	assert(((int) id_.first) >= 0);

	varStr % typeNames[(int) id_.first];
	varStr % id_.second;

	std::string valStr;
	if (isValid())
	{
		switch (id_.first)
		{
		case fmiTypeReal: valStr = std::to_string(getRealValue()); break;
		case fmiTypeInteger: valStr = std::to_string(getIntegerValue()); break;
		case fmiTypeBoolean: valStr = std::to_string(getBooleanValue()); break;
		case fmiTypeString: valStr += "\"" + getStringValue() + "\""; break;
		default: assert(false);
		}
	} else {
		valStr = "Invalid Value";
	}

	varStr % valStr;
	return varStr.str();
}

bool Variable::operator == (const Variable &other) const
{
	if (id_.first != other.id_.first || id_.second != other.id_.second || 
		  isValid() != other.isValid())
		return false;
	
	if (isValid() && !equalValue(other))
		return false;

	return true;
}

bool Variable::equalValue(const Variable &other) const
{
	assert(isValid() && other.isValid());
	if (id_.first != other.id_.first)
		return false;
	switch (id_.first)
	{
	case fmiTypeReal:    return getRealValue() == other.getRealValue();
	case fmiTypeInteger: return getIntegerValue() == other.getIntegerValue();
	case fmiTypeBoolean: return getBooleanValue() == other.getBooleanValue();
	case fmiTypeString:  return getStringValue() == other.getStringValue();
	default: assert(false); return false;
	}
}

std::ostream& FMITerminalBlock::Timing::operator<<(std::ostream& stream, const Variable& var)
{
	stream << var.toString();
	return stream;
}

