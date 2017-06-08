/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CompactASN1Publisher.cpp
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#include "network/CompactASN1Publisher.h"
#include "base/BaseExceptions.h"

#include <assert.h>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <string.h>

using namespace FMITerminalBlock::Network;

const std::string CompactASN1Publisher::PROP_ENCODE_TYPE = "encoding";

const bool CompactASN1Publisher::CASTABLE[5][ASN1Commons::DATA_TYPE_SIZE] = {
	//"REAL", "LREAL", "DINT", "BOOL", "STRING"
	{true, true, false, false, false}, // fmiTypeReal
	{false, false, true, false, false}, // fmiTypeInteger
	{false, false, false, true, false}, // fmiTypeBoolean
	{false, false, false, false, true}, // fmiTypeString
	{false, false, false, false, false} // fmiTypeUnknown
};

CompactASN1Publisher::CompactASN1Publisher(): outputVariables_(), outputTypes_()
{

}

void CompactASN1Publisher::init(const Base::TransmissionChannel &channel)
{
	initOutputVariables(channel.getPortIDs());
	initOutputTypes(channel);
}

void CompactASN1Publisher::eventTriggered(Timing::Event * ev)
{
	assert(ev != NULL);

	bool updated = updateOutputVariables(ev);
	if (updated)
	{
		std::vector<uint8_t> buffer;
		encodeASN1OutputVariables(buffer);
		sendData(buffer);
	}
}

std::string
CompactASN1Publisher::toString(const std::vector<uint8_t> & buffer)
{
	std::string ret("0x");
	boost::format byteFormat("%02x");
	for(unsigned i = 0; i < buffer.size(); i++)
	{
		if(i != 0)
			ret += " ";
		byteFormat.clear();
		byteFormat % (unsigned) buffer[i];
		ret += byteFormat.str();
	}
	return ret;
}

void 
CompactASN1Publisher::initOutputVariables(
	const std::vector<Base::PortID> & ports)
{
	outputVariables_.clear();

	for(unsigned i = 0; i < ports.size(); i++)
	{
		Timing::Variable var(ports[i]);
		switch(var.getID().first)
		{
		case fmiTypeReal:
			var.setValue((fmiReal) 0.0);
			break;
		case fmiTypeInteger:
			var.setValue((fmiInteger) 0);
			break;
		case fmiTypeBoolean:
			var.setValue((fmiBoolean) fmiFalse);
			break;
		case fmiTypeString:
			var.setValue(std::string());
			break;
		default:
			assert(false);
		}
		outputVariables_.push_back(var);
	}
}

void 
CompactASN1Publisher::initOutputTypes(const Base::TransmissionChannel &channel)
{
	const std::vector<Base::PortID> &ports = channel.getPortIDs();
	for(unsigned i = 0; i < ports.size(); i++)
	{
		ASN1Commons::DataType type = getDefaultType(ports[i].first);

		// Query encoding property
		const boost::optional<std::string> encodeTypeValue =
			channel.getPortConfig()[i]->get_optional<std::string>(PROP_ENCODE_TYPE);

		if((bool) encodeTypeValue)
		{
			bool found = false;
			for(unsigned j = 0; j < ASN1Commons::DATA_TYPE_SIZE; j++)
			{
				if(*encodeTypeValue == ASN1Commons::DATA_TYPE_NAMES[j])
				{
					type = (ASN1Commons::DataType) j;
					found = true;
					break;
				}
			}

			if(!found)
			{
				throw Base::SystemConfigurationException(
					"The given encoding type is not supported.", PROP_ENCODE_TYPE,
					*encodeTypeValue);
			}

			if(!CASTABLE[ports[i].first][type])
			{
				boost::format err("Can't convert the fmiType (%1%) to the IEC 61499/ASN.1"
					" type %2%. The operation is not supported but configured at the ASN.1 "
					"publisher port %3%");
				err % ports[i].first % ASN1Commons::DATA_TYPE_NAMES[type] % i;
				throw Base::SystemConfigurationException(err.str());
			}
		}

		outputTypes_.push_back(type);
	}
}

ASN1Commons::DataType 
CompactASN1Publisher::getDefaultType(FMIVariableType srcType)
{
	ASN1Commons::DataType type;
	// Assign default types
	switch(srcType)
	{
	case fmiTypeReal:
		type = ASN1Commons::DataType::LREAL;
		break;
	case fmiTypeInteger:
		type = ASN1Commons::DataType::DINT;
		break;
	case fmiTypeBoolean:
		type = ASN1Commons::DataType::BOOL;
		break;
	case fmiTypeString:
		type = ASN1Commons::DataType::STRING;
		break;
	default:
		assert(false);
	}
	return type;
}

bool
CompactASN1Publisher::updateOutputVariables(Timing::Event * ev)
{
	assert(ev != NULL);
	bool updated = false;
	std::vector<Timing::Variable> vars = ev->getVariables();
	for(unsigned i = 0; i < vars.size(); i++)
	{
		for(unsigned j = 0; j < outputVariables_.size(); j++)
		{
			if(vars[i].getID() == outputVariables_[j].getID())
			{
				// Update
				outputVariables_[j].setValue(vars[i].getValue());
				updated = true;
			}
		}
	}
	return updated;
}

void 
CompactASN1Publisher::encodeASN1OutputVariables(std::vector<uint8_t> &buffer)
{
	assert(outputVariables_.size() == outputTypes_.size());

	for(unsigned i = 0; i < outputVariables_.size(); i++)
	{

		switch(outputTypes_[i])
		{
		case ASN1Commons::DataType::LREAL:
			encodeLREALValue(buffer,outputVariables_[i].getRealValue());
			break;
		case ASN1Commons::DataType::REAL:
			encodeREALValue(buffer, outputVariables_[i].getRealValue());
			break;
		case ASN1Commons::DataType::DINT:
			encodeValue(buffer, outputVariables_[i].getIntegerValue());
			break;
		case ASN1Commons::DataType::BOOL:
			encodeValue(buffer, outputVariables_[i].getBooleanValue());
			break;
		case ASN1Commons::DataType::STRING:
			encodeValue(buffer, outputVariables_[i].getStringValue());
			break;
		default:
			assert(0);
		}

	}

}

void
CompactASN1Publisher::encodeLREALValue(std::vector<uint8_t> &buffer, fmiReal value)
{
	// Append tag encoding
	buffer.push_back(ASN1Commons::CLASS_APPLICATION | ASN1Commons::LREAL_TAG_NR);

	// Append value, lsb first
	uint64_t val = 0;
	memcpy(&val, &value, sizeof(value));

	for(int i = sizeof(val) - 1; i >= 0; i--)
		buffer.push_back((uint8_t) ((val >> (8*i)) & 0xFF) );
}

void
CompactASN1Publisher::encodeREALValue(std::vector<uint8_t> &buffer, fmiReal value)
{
	// Append tag encoding
	buffer.push_back(ASN1Commons::CLASS_APPLICATION | ASN1Commons::REAL_TAG_NR);

	// Append value, lsb first
	uint32_t val = 0;
	float tmpVal = (float) value; // Convert to 32Bit floating point number
	memcpy(&val, &tmpVal, sizeof(value));

	for(int i = sizeof(val) - 1; i >= 0; i--)
		buffer.push_back((uint8_t) ((val >> (8*i)) & 0xFF) );
}


void
CompactASN1Publisher::encodeValue(std::vector<uint8_t> &buffer, fmiInteger value)
{
	// Append tag encoding
	buffer.push_back(ASN1Commons::CLASS_APPLICATION | ASN1Commons::DINT_TAG_NR);

	for(int i = sizeof(value) - 1; i >= 0; i--)
		buffer.push_back((uint8_t) ((value >> (8*i)) & 0xFF) );
}

void
CompactASN1Publisher::encodeValue(std::vector<uint8_t> &buffer, fmiBoolean value)
{

	if(value)
	{
		buffer.push_back(ASN1Commons::CLASS_APPLICATION | ASN1Commons::BOOL1_TAG_NR);
	}else{
		buffer.push_back(ASN1Commons::CLASS_APPLICATION | ASN1Commons::BOOL0_TAG_NR);
	}

}

void
CompactASN1Publisher::encodeValue(std::vector<uint8_t> &buffer, const std::string & value)
{
	// Append tag encoding
	buffer.push_back(ASN1Commons::CLASS_APPLICATION | ASN1Commons::STRING_TAG_NR);
	
	unsigned size = value.size() & 0xFFFF;

	buffer.push_back((uint8_t) (size >> 8) & 0xFF);
	buffer.push_back((uint8_t) size & 0xFF);

	for(unsigned i = 0; i < size; i++)
		buffer.push_back((uint8_t) value[i]);
}
