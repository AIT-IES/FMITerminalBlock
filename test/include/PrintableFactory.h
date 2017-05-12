/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file PrintableFactory.h
 * @brief Header only implementation of a printable factory class
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCKTEST_NETWORK_PRINTABLE_FACTORY
#define _FMITERMINALBLOCKTEST_NETWORK_PRINTABLE_FACTORY

#include <memory>
#include <iostream>
#include <functional>

namespace FMITerminalBlockTest
{
	namespace Network
	{

		/**
		 * @brief Base class which just holds printable information
		 * @details The base class was created in order to ease the << operator 
		 * overloading.
		 */
		class PrintableFactoryBase
		{
		public:
			friend std::ostream& operator<< (std::ostream& stream, 
				const PrintableFactoryBase& factory);

		protected:
			/* @brief Initializes the name of the factory */
			PrintableFactoryBase(const std::string &name): name_(name) {}

		private:
			/** The name of the object to instantiate */
			std::string name_;
		};

		/** 
		 * @brief Helper class which instantiates objects derived from BaseType 
		 */
		template <typename BaseType>
		class PrintableFactory: public PrintableFactoryBase
		{
		public:
			/** @brief Instantiates a new element */
			std::shared_ptr<BaseType> newElement() const
			{
				return generator_();
			}

			/** @brief Same as newElement() */
			std::shared_ptr<BaseType> operator () () const
			{
				return newElement();
			}

			/** @brief Generates A factory which instantiates the specific type */
			template<typename SpecificType>
			static PrintableFactory<BaseType> make(const std::string &name)
			{
				return PrintableFactory<BaseType>(name,
						[]() {return std::make_shared<SpecificType>(); }
					);
			}

		private:

			/** Generator function which instantiates the actual object */
			std::function<std::shared_ptr<BaseType>()> generator_;

			/** @brief C'tor setting the internals. (Not to be used externally) */
			PrintableFactory(const std::string &name, 
				std::function<std::shared_ptr<BaseType>()> generator):
				PrintableFactoryBase(name), generator_(generator)
			{
			}
		};

		/** @brief Appends a string representation of the given Factory */
		std::ostream& operator<< (std::ostream& stream,	
			const PrintableFactoryBase& factory)
		{
			stream << "Factory of:" << factory.name_;
			return stream;
		}
	}
}
#endif