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