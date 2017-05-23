/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file RawTestData.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "RawTestData.h"

#include <assert.h>

using namespace FMITerminalBlockTest::Network;
using namespace FMITerminalBlockTest;

RawTestData::RawTestData()
{
}

RawTestData::RawTestData(std::initializer_list<uint8_t> sequence): 
	data_(sequence)
{
}

RawTestData::RawTestData(const std::vector<uint8_t> &data) : data_(data)
{
}

RawTestData::RawTestData(const RawTestData &left, const RawTestData &right)
{
	data_.reserve(left.data_.size() + right.data_.size());
	data_.assign(left.data_.begin(), left.data_.end());
	for (auto it = right.data_.begin(); it != right.data_.end(); ++it)
	{
		data_.push_back(*it);
	}
}

const std::vector<uint8_t>& RawTestData::getData() const
{
	return data_;
}

RawTestData RawTestData::operator+(const RawTestData &data) const
{
	return RawTestData(*this, data);
}

std::ostream& FMITerminalBlockTest::Network::operator<<(std::ostream& stream,
	const RawTestData& data)
{
	const std::vector<uint8_t> &buffer = data.getData();

	stream << "RawTestData: {";
	for (unsigned int i = 0; i < buffer.size(); ++i)
	{
		stream << std::hex << buffer[i];
		if (i + 1 < buffer.size()) stream << ", ";
	}
	stream << "}";
	return stream;
}