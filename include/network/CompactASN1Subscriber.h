/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file CompactASN1Subscriber.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_NETWORK_COMPACT_ASN1_SUBSCRIBER
#define _FMITERMINALBLOCK_NETWORK_COMPACT_ASN1_SUBSCRIBER

#include "ConcurrentSubscriber.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <vector>
#include <stdint.h>
#include <functional>
#include <memory>

#include "timing/Event.h"
#include "network/PartialEvent.h"

namespace FMITerminalBlock
{
	namespace Network
	{
		/**
		 * @brief Handles incoming ASN.1 message processing
		 * @details <p> The class implements message parsing and message timeout 
		 * operations. Therefore, an timeout mechanism is implemented which handles
		 * partial chunks of data. A message may be split across multiple chunks. 
		 * Whenever a message is completely received or a timeout occurs, an event 
		 * is Emitted. An event may also be a partial event which does not cover 
		 * every registered port. Since the class does not expect data to be 
		 * received at once, it operates state-full. </p>
		 *
		 * <p>The class manages a boost::io_service object which handles the 
		 * asynchronous data processing and timeout mechanisms. The io_sercvice 
		 * object may be queried in order to run other services. A shutdown 
		 * mechanisms terminates pending requests and terminates the concurrent 
		 * execution.</p>
		 */
		class CompactASN1Subscriber: public ConcurrentSubscriber
		{
		public:

			/** 
			 * @brief Property name specifying the time in milliseconds until the 
			 * whole packet must be received 
			 */
			static const std::string PROP_PACKET_TIMEOUT;

			/** @brief Creates an uninitialized object */
			CompactASN1Subscriber();

		protected:
			/**
			 * @brief Initializes the protocol handler
			 * @details The function also calls initNetwork().
			 */
			virtual void init(const Base::TransmissionChannel &settings,
				std::shared_ptr<Timing::EventSink> eventSink);

			/**
			 * @brief Requests the caller to initialize its network connections
			 * @details The caller may safely assume that all service functions of 
			 * the CompactASN1Subscriber object are ready to be used. Nevertheless, 
			 * the io service may not be started yet. The function is executed in the
			 * context of the initializing thread. However, it is guaranteed that the
			 * executing thread is not started until the function returns.
			 */
			virtual void initNetwork() = 0;

			/**
			 * @brief Executes the io service 
			 * @details The function returns after the termination request has been 
			 * confirmed by the inheriting subclass.
			 */
			virtual void run();

			/**
			 * @brief Processes an external termination request.
			 */
			virtual void terminationRequest();

			/**
			 * @brief Shuts down the network connection
			 * @details The function is called in the context of the executing 
			 * thread. I.o. no other handler will be invoked concurrently. It is 
			 * assumed that the connection is closed synchronously. After the 
			 * function returns, the service object may be closed as well.
			 */
			virtual void terminateNetworkConnection() = 0;

			/**
			 * @brief Returns a valid pointer to the managed io_service object
			 * @details The pointer may be used to perform asynchronous tasks. At 
			 * least, it remains valid and active until terminateNetworkConnection() 
			 * returns.
			 */
			boost::asio::io_service* getIOService();

			/**
			 * @brief Returns a valid reference to the channel configuration
			 * @details The pointer remains valid until the subscriber is terminated.
			 */
			const Base::TransmissionChannel* getChannelConfiguration() const;

			/**
			 * @brief Requests a new destination buffer object.
			 * @details At most one object must be requested at a time. After the 
			 * function is called, commitData(size_t) must be called. The 
			 * prepareData() function must not be called again before the data is 
			 * committed. The returned buffer remains valid until the 
			 * commitData(size_t) function is called.
			 * @return The data buffer which has the expected data size.
			 */
			boost::asio::streambuf::mutable_buffers_type prepareData();

			/**
			 * @brief Registers incoming data and returns the requested buffer.
			 * @details Before calling the function, prepareData() must be called in 
			 * order to return the destination buffer. The actionSize parameter must 
			 * be set to the number of bytes which are written to the buffer. I.e. 
			 * it must not exceed the size of the requested buffer. By calling the 
			 * function, the requested data buffer is returned and the buffer must 
			 * not be used anymore.
			 * @param actualSize The number of bytes which were actually received.
			 */
			void commitData(size_t actualSize);

		private:

			/** @brief Encodes the decoding operation status code */
			typedef enum
			{
				Ok = 0, ///< The operation was completed successfully
				TypeConversionError, ///< Dropped unconvertible input
				InvalidBufferContent, ///< The whole content of the buffer is invalid
				Incomplete ///< More input data required
			} ParsingStatusCode;

			/** @brief Encapsulates some essential parser status information */
			typedef struct
			{
				ParsingStatusCode state; ///< The status code of the operation
				/** 
				 * @brief Amount of additional data which is required to complete the
				 * operation successfully.
				 * @details The field may only be set in case state == Incomplete
				 */
				size_t missingData;
			} ParsingStatus;

			/** 
			 * @brief The configuration of the channel
			 * @details The configuration will be NULL, if the channel was not 
			 * initialized before.
			 */
			const Base::TransmissionChannel* channelConfig_;

			/** @brief The interface to register received events */
			std::shared_ptr<Timing::EventSink> eventSink_;

			/**
			 * @brief All received but not registered event variables.
			 * @details The reference may be NULL in case all variables were 
			 * registered at the event sink.
			 */
			PartialEvent *partialData_ = NULL;

			/** @brief All unprocessed data elements */
			boost::asio::streambuf remainingRawData_;

			/** @brief The IO service which handles all tasks */
			boost::asio::io_service service_;
			/** 
			 * @brief Prevents the service_ object from exiting while no work is to 
			 * be done
			 * @details The function is just a safety measure to generalize the usage
			 * of the class. In case an external event is expected and no tasks are 
			 * to be done, it prevents the working thread to terminate before the 
			 * termination function is actually called.
			 */
			std::unique_ptr<boost::asio::io_service::work> busyKeeper_;

			/** @brief the timer which handles packet timeouts */
			std::shared_ptr<boost::asio::deadline_timer> packetTimeoutTimer_;

			/** @brief Clears the timer and starts the interval anew */
			void restartPacketTimer();

			/**
			 * @brief Cancels the receive operation, if a timeout happens.
			 * @details In case the operation is canceled, the partial event is 
			 * registered and all new data items are considered to be part of another
			 * event.
			 */
			void handlePacketTimeout(const boost::system::error_code& error);

			/** @brief Empties the buffer of unprocessed raw data */
			void clearUnprocessedData();

			/** @brief Moves the partial packet to the event sink */
			void pushPartialEvent();

			/** @brief Terminates the operation of the subscriber */
			void handleTermination();

			/** 
			 * @brief Returns the first byte in the raw data buffer
			 * @details The function assumes that there is at least one byte left in 
			 * the buffer
			 */
			uint8_t getFirstRawDataByte() const;

			/** 
			 * @brief Processes the remainingRawData_ until no more data is available
			 * or the PartialEvent is fully populated
			 * @details The function assumes that the partialData_ pointer is valid.
			 * The function is implemented in a best effort manner. It will try to 
			 * parse as many  inputs as possible. In case a single value is not 
			 * convertible, it will be ignored. Only if more data is expected or the 
			 * whole buffer needs to be cleared, it aborts the operation.
			 */
			ParsingStatus processRawDataToEvent();

			/**
			 * @brief Tries to parse the next variable
			 * @details If the operation is successful, the parsed content will be 
			 * written to the given destination variable dest. It is assumed that 
			 * dest is always valid. If the value in the remainingRawData_ cannot be 
			 * converted into the destination type, it will be removed from the 
			 * buffer. If further input data is needed, the data will not be 
			 * consumed.
			 * @param dest The memory location to write the parsed value
			 * @return The status of the operation.
			 */
			template<typename DestinationType>
			ParsingStatus readNextVariable(boost::any *dest);

			/**
			 * @brief Calls the given read function and tries to convert the result
			 * @details On success, the result will be stored at the given 
			 * destination. In case the read operation is nor successful or the 
			 * result cannot be converted, the status code will be set accordingly.
			 * @param readFunction The function which is used to obtain the value
			 * @param dest The valid destination of the parsed value. In case the 
			 * status is not successful (Ok), no value will be written.
			 */
			template<typename DestinationType, typename IntermediateType>
			ParsingStatus readAndConvertVariable(std::function<
				ParsingStatus(CompactASN1Subscriber&, IntermediateType*)> readFunction,
				boost::any *dest);

			/**
			 * @brief Parses the next xREAL value and writes it to dest
			 * @details <p>It is assumed that dest is a valid pointer and that the 
			 * first byte of the value corresponds to the correct ASN.1 Tag. After 
			 * the operation has been successfully performed, the raw data will be 
			 * consumed. It is assumed that the floating point type on the network as
			 * well as on the machine use an IEEE-754 compatible encoding. The 
			 * function will not convert the encoding manually.</p>
			 * <p>The function uses templates to instantiate all floating point 
			 * types. It requires two parameters. The first one gives an intermediate
			 * unsigned integer type of exactly the same size as the floating point 
			 * type. The second one gives the floating point type itself. It is 
			 * assumed that both types require the same number of bytes.</p>
			 * @param dest The memory location to write the parsed value
			 * @return The status of the operation.
			 */
			template<typename UIntType, typename FloatType>
			ParsingStatus readXREALVariable(FloatType *dest);

			/**
			 * @brief Parses the next string variable and sets the result
			 * @details It is assumed that the first byte contains the correct ASN.1 
			 * tag. If the length of the string could not be determined, the minimum 
			 * amount of data which is needed to decode the length field is 
			 * requested. If the length is still present all remaining bytes will be
			 * requested. Whenever the operation is successful, all bytes will be 
			 * consumed.
			 * @param des the location to store the parsed string
			 */
			ParsingStatus readSTRINGVariable(std::string *dest);

			/**
			 * @brief Parses the boolean 0 variable and sets the result
			 * @details The function is just a placeholder which sets the destination
			 * and consumes the tag byte.
			 */
			ParsingStatus readBOOL0Variable(fmiBoolean *dest);

			/**
			 * @brief Parses the boolean 0 variable and sets the result
			 * @details The function is just a placeholder which sets the destination
			 * and consumes the tag byte.
			 */
			ParsingStatus readBOOL1Variable(fmiBoolean *dest);

			/**
			 * @brief Reads a generic integer type and consumes the data
			 * @details The function assumes that the tag value at the first byte is
			 * correctly set. Please make sure that the given integer type has 
			 * exactly the same type and width as the corresponding network type.
			 * @param dest A valid pointer to the destination variable.
			 */
			template<typename IntType>
			ParsingStatus readXINTVariable(IntType *dest);
		};
	}
}
#endif
