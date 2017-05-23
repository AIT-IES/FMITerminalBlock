/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CompactASN1Subscriber.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "network/CompactASN1Subscriber.h"

#include <assert.h>

#include <boost/asio.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/log/trivial.hpp>
#include <boost/convert.hpp>
#include <boost/convert/lexical_cast.hpp>

#include "network/ASN1Commons.h"
#include "base/BaseExceptions.h"

using namespace FMITerminalBlock::Network;
using namespace FMITerminalBlock;

const std::string CompactASN1Subscriber::PROP_PACKET_TIMEOUT = "packetTimeout";

CompactASN1Subscriber::CompactASN1Subscriber()
{
}

void CompactASN1Subscriber::init(const Base::TransmissionChannel &settings,
	std::shared_ptr<Timing::EventSink> eventSink)
{
	channelConfig_ = &settings;
	eventSink_ = eventSink;

	if (settings.getPortIDs().size() <= 0)
	{
		throw Base::SystemConfigurationException("Compact ASN.1 channel has no"
			" associated ports");
	}

	clearUnprocessedData();

	busyKeeper_ = std::make_unique<boost::asio::io_service::work>(service_);

	// Initialize the timer
	uint32_t packetTimeout;
	packetTimeout = settings.getChannelConfig().get(PROP_PACKET_TIMEOUT, 500);
	packetTimeoutTimer_= std::make_shared<boost::asio::deadline_timer>(service_, 
		boost::posix_time::milliseconds(packetTimeout));

	initNetwork();
}

void CompactASN1Subscriber::run()
{
	service_.run();
}

void CompactASN1Subscriber::terminationRequest()
{
	service_.post(boost::bind(&CompactASN1Subscriber::handleTermination, this));
}

boost::asio::io_service* CompactASN1Subscriber::getIOService()
{
	return &service_;
}

const Base::TransmissionChannel* 
CompactASN1Subscriber::getChannelConfiguration() const
{
	assert(channelConfig_ != NULL);
	return channelConfig_;
}

boost::asio::streambuf::mutable_buffers_type 
CompactASN1Subscriber::prepareData()
{
	// TODO: Estimate the size of the next or remaining packet and return the 
	//       buffer accordingly (#381)
	return remainingRawData_.prepare(128);
}

void CompactASN1Subscriber::commitData(size_t actualSize)
{
	restartPacketTimer();
	remainingRawData_.commit(actualSize);

	while (remainingRawData_.size() > 0)
	{
		if (partialData_ == NULL)
		{
			partialData_ = new PartialEvent(eventSink_->getTimeStampNow(), 
				channelConfig_->getPortIDs());
		}
		
		ParsingStatus status;
		status = processRawDataToEvent();
		if (status.state == InvalidBufferContent)
		{
			pushPartialEvent();
			clearUnprocessedData();
		}
		else if(status.state == Incomplete)
		{
			// TODO: Set the number of bytes which are missing
			break;
		}

		if (partialData_ != NULL && !partialData_->hasRemainingElements())
		{
			pushPartialEvent();
		}
	}
}

void CompactASN1Subscriber::restartPacketTimer()
{
	assert(packetTimeoutTimer_);

	packetTimeoutTimer_->cancel();
	packetTimeoutTimer_->async_wait(boost::bind(
		&CompactASN1Subscriber::handlePacketTimeout, this,
		boost::asio::placeholders::error));
}

void CompactASN1Subscriber::handlePacketTimeout(
	const boost::system::error_code& error)
{
	if (!error && partialData_ != NULL)
	{
		BOOST_LOG_TRIVIAL(warning) << "ASN.1 data missing: Triggering event " 
			<< partialData_->toString() << " due to a timeout";
		pushPartialEvent();
		clearUnprocessedData();
	}
}

void CompactASN1Subscriber::clearUnprocessedData()
{
	if (remainingRawData_.size() > 0)
	{
		BOOST_LOG_TRIVIAL(warning) << "Ignore " << remainingRawData_.size() <<
			" bytes of unprocessed ASN.1 data to gain a consistent decoding state";
		remainingRawData_.consume(remainingRawData_.size());
	}
}

void CompactASN1Subscriber::pushPartialEvent()
{
	assert(eventSink_);
	assert(partialData_);

	eventSink_->pushExternalEvent(partialData_);
	partialData_ = NULL;
}

void CompactASN1Subscriber::handleTermination()
{
	assert(packetTimeoutTimer_);
	
	busyKeeper_.reset();
	terminateNetworkConnection();
	packetTimeoutTimer_->cancel();
	service_.stop();

	// Clean buffered data
	if (partialData_ != NULL)
	{
		BOOST_LOG_TRIVIAL(debug) << "Clear partially received event because the "
			<< "subscriber is requested to terminate: " << partialData_->toString();
		delete partialData_;
		partialData_ = NULL;
	}
	clearUnprocessedData();
}

uint8_t CompactASN1Subscriber::getFirstRawDataByte() const
{
	assert(remainingRawData_.size() >= sizeof(uint8_t));
	return boost::asio::buffer_cast<const uint8_t *>(
		remainingRawData_.data())[0];
}

CompactASN1Subscriber::ParsingStatus
CompactASN1Subscriber::processRawDataToEvent()
{
	assert(partialData_ != NULL);

	ParsingStatus state = {Ok, 0};
	while (remainingRawData_.size() > 0 && partialData_->hasRemainingElements()
		&& (state.state == Ok || state.state == TypeConversionError))
	{
		boost::any value;
		switch (partialData_->getNextPortType())
		{
		case fmiTypeReal:	    state = readNextVariable<fmiReal>(&value);     break;
		case fmiTypeInteger:	state = readNextVariable<fmiInteger>(&value);  break;
		case fmiTypeBoolean:	state = readNextVariable<fmiBoolean>(&value);  break;
		case fmiTypeString:	  state = readNextVariable<std::string>(&value); break;
		default:
			assert(false);
		}

		if (state.state == Ok)
		{
			partialData_->pushNextValue(value);
		}
		else if (state.state == TypeConversionError)
		{
			partialData_->ignoreNextValue();
		}
	}
	return state;
}

template<typename DestinationType>
CompactASN1Subscriber::ParsingStatus
CompactASN1Subscriber::readNextVariable(boost::any *dest)
{
	assert(remainingRawData_.size() > 0);

	boost::optional<DestinationType> convertedValue;
	ParsingStatus state;

	int8_t tag = getFirstRawDataByte();
	switch (tag)
	{
	case ASN1Commons::CLASS_APPLICATION | ASN1Commons::LREAL_TAG_NR:
		state = readAndConvertVariable<DestinationType, double>(
			&CompactASN1Subscriber::readXREALVariable<uint64_t, double>, dest);
		break;
	case ASN1Commons::CLASS_APPLICATION | ASN1Commons::REAL_TAG_NR:
		state = readAndConvertVariable<DestinationType, float>(
			&CompactASN1Subscriber::readXREALVariable<uint32_t, float>, dest);
		break;
	case ASN1Commons::CLASS_APPLICATION | ASN1Commons::STRING_TAG_NR:
		state = readAndConvertVariable<DestinationType, std::string>(
			&CompactASN1Subscriber::readSTRINGVariable, dest);
		break;
	case ASN1Commons::CLASS_APPLICATION | ASN1Commons::BOOL0_TAG_NR:
		state = readAndConvertVariable<DestinationType, fmiBoolean>(
			&CompactASN1Subscriber::readBOOL0Variable, dest);
		break;
	case ASN1Commons::CLASS_APPLICATION | ASN1Commons::BOOL1_TAG_NR:
		state = readAndConvertVariable<DestinationType, fmiBoolean>(
			&CompactASN1Subscriber::readBOOL1Variable, dest);
		break;
	case ASN1Commons::CLASS_APPLICATION | ASN1Commons::DINT_TAG_NR:
		state = readAndConvertVariable<DestinationType, int32_t>(
			&CompactASN1Subscriber::readXINTVariable<int32_t>, dest);
		break;
	default:
		state.state = InvalidBufferContent;
		state.missingData = 0;
		BOOST_LOG_TRIVIAL(warning) << "Unsupported ASN.1 type with tag " 
			<< (int) tag;
	}
	return state;
}

template<typename DestinationType, typename IntermediateType>
CompactASN1Subscriber::ParsingStatus 
CompactASN1Subscriber::readAndConvertVariable(
	std::function<ParsingStatus(CompactASN1Subscriber&, IntermediateType*)> rFcn,
	boost::any *dest)
{
	assert(dest != NULL);
	IntermediateType value;
	ParsingStatus state = rFcn(*this, &value);
	if (state.state == Ok)
	{
		boost::cnv::lexical_cast converter; 
		boost::optional<DestinationType> convertedValue;
		convertedValue = boost::convert<DestinationType>(value, converter);
		if (convertedValue) 
		{
			*dest = convertedValue.value();
		} else {
			BOOST_LOG_TRIVIAL(warning) << "Fail to convert received value \""
				<< value << "\" to the defined model type.";
			state.state = TypeConversionError;
		}
	}
	return state;
}

template<typename UIntType, typename FloatType>
CompactASN1Subscriber::ParsingStatus 
CompactASN1Subscriber::readXREALVariable(FloatType *dest)
{
	assert(dest != NULL);
	assert(sizeof(UIntType) == sizeof(FloatType));

	ParsingStatus status;
	const size_t expectedSize = sizeof(*dest) + 1;

	if (remainingRawData_.size() < expectedSize)
	{
		status.state = Incomplete;
		status.missingData = expectedSize - remainingRawData_.size();
	}
	else
	{
		remainingRawData_.consume(sizeof(uint8_t)); // Assume the tag is correct

		const UIntType *buffer = boost::asio::buffer_cast<const UIntType*>(
			remainingRawData_.data());
		UIntType *rawDest = reinterpret_cast<UIntType*>(dest);
		*rawDest = boost::endian::big_to_native(*buffer);
		remainingRawData_.consume(sizeof(*buffer));

		status.state = Ok;
		status.missingData = 0;
	}

	return status;
}

CompactASN1Subscriber::ParsingStatus 
CompactASN1Subscriber::readSTRINGVariable(std::string *dest)
{
	using namespace boost::asio;

	assert(dest != NULL);

	const size_t expectedMetaDataSize = sizeof(uint8_t) + sizeof(uint16_t);
	if (remainingRawData_.size() < expectedMetaDataSize)
	{
		ParsingStatus status;
		status.state = Incomplete;
		status.missingData = expectedMetaDataSize - remainingRawData_.size();
		return status;
	}

	assert(getFirstRawDataByte() == 
		(ASN1Commons::CLASS_APPLICATION | ASN1Commons::STRING_TAG_NR));

	const uint8_t *buffer;
	buffer = buffer_cast<const uint8_t *>(remainingRawData_.data());
	buffer++;
	const size_t length = boost::endian::big_to_native<uint16_t>(
		*reinterpret_cast<const uint16_t*>(buffer));

	if (remainingRawData_.size() < expectedMetaDataSize + length)
	{
		ParsingStatus status;
		status.state = Incomplete;
		status.missingData = expectedMetaDataSize + length;
		status.missingData -= remainingRawData_.size();
		return status;
	}

	const char *str = buffer_cast<const char *>(remainingRawData_.data());
	str += (expectedMetaDataSize / sizeof(char));
	*dest = std::string(str, length);

	remainingRawData_.consume(expectedMetaDataSize + length);

	ParsingStatus status;
	status.state = Ok;
	status.missingData = 0;
	return status;
}

CompactASN1Subscriber::ParsingStatus
CompactASN1Subscriber::readBOOL0Variable(fmiBoolean *dest)
{
	assert(getFirstRawDataByte() == 
		(ASN1Commons::CLASS_APPLICATION | ASN1Commons::BOOL0_TAG_NR));
	assert(dest != NULL);

	*dest = fmiFalse;
	remainingRawData_.consume(sizeof(uint8_t));

	ParsingStatus status = {Ok,0};
	return status;
}

CompactASN1Subscriber::ParsingStatus 
CompactASN1Subscriber::readBOOL1Variable(fmiBoolean *dest)
{
	assert(getFirstRawDataByte() == 
		(ASN1Commons::CLASS_APPLICATION | ASN1Commons::BOOL1_TAG_NR));
	assert(dest != NULL);

	*dest = fmiTrue;
	remainingRawData_.consume(sizeof(uint8_t));

	ParsingStatus status = {Ok,0};
	return status;
}

template<typename IntType>
CompactASN1Subscriber::ParsingStatus 
CompactASN1Subscriber::readXINTVariable(IntType *dest)
{
	using namespace boost::asio;

	assert(dest != NULL);

	const size_t expectedSize = sizeof(uint8_t) + sizeof(IntType);
	if (remainingRawData_.size() < expectedSize)
	{
		ParsingStatus status;
		status.state = Incomplete;
		status.missingData = expectedSize - remainingRawData_.size();
		return status;
	}

	remainingRawData_.consume(sizeof(uint8_t)); // Consume the tag

	const IntType *dat = buffer_cast<const IntType*>(remainingRawData_.data());
	*dest = boost::endian::big_to_native<IntType>(*dat);

	remainingRawData_.consume(sizeof(IntType)); // Consume the value

	ParsingStatus status = {Ok, 0};
	return status;
}