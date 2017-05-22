/* ------------------------------------------------------------------- *
* Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
* All rights reserved. See file FMITerminalBlock_LICENSE for details. *
* ------------------------------------------------------------------- */

/**
* @file Variable.h
* @author Michael Spiegel, michael.spiegel@ait.ac.at
*/

#ifndef _FMITERMINALBLOCK_TIMING_VARIABLE
#define _FMITERMINALBLOCK_TIMING_VARIABLE

#include <utility>
#include <ostream>

#include <boost/any.hpp>

#include <common/fmi_v1.0/fmiModelTypes.h>
#include <common/FMIType.h>

#include "base/PortID.h"


namespace FMITerminalBlock
{
	namespace Timing
	{

		/**
		 * @brief Represents a model or network variable.
		 * @details <p> The variable encapsulates a unique PortID and the 
		 * associated value. Neither the PortID not the value are guaranteed to be 
		 * valid. The PortID may have an unknown type and the type of the value may
		 * not match the type registered in the id. In case the value is not 
		 * properly set, it must not be queried.</p>
		 * <p> The class is created as lightweight as possible in order to reduce 
		 * the overhead. Hence, it is mostly justifiable to pass and return it by 
		 * value.</p>
		 */
		class Variable
		{
		public:

			/** 
			 * @brief Constructs an empty variable of unknown type
			 */
			Variable();

			/** 
			 * @brief Initializes the variable by the given data and type id 
			 * @details It is assumed that the data is typed according to the type 
			 * of the id. In case the type of value does not correspond to the type 
			 * of the id, the value must not be queried.
			 * @param id The initial identifier of the variable
			 * @param value The initial value of the variable. The parameter may be 
			 * omitted. The default value must not be queried.
			 */
			Variable(const Base::PortID &id, const boost::any &value = boost::any());

			/**
			 * @brief Generates a variable from the given pair
			 * @details The C'tor is presented for legacy and convenience purpose. 
			 * The first parameter of the pair corresponds to the id and the second 
			 * parameter corresponds to the data. 
			 */
			Variable(const std::pair <Base::PortID, boost::any> &pair);

			/**
			 * @brief Returns the previously set id or an id with fmiTypeUnknown
			 * @details If the default C'tor is set, an fmiTypeUnknown typed id will 
			 * be returned. 
			 */
			Base::PortID getID() const;
			/**
			 * @brief Returns the previously set value.
			 * @details The function must not be called in case the value was not set
			 * properly or in case the type of the value doesn't correspond to the
			 * type in the id.
			 */
			boost::any getValue() const;

			/**
			 * @brief Returns the real-typed value of the variable
			 * @details The function assumes that the variable is valid and holds a
			 * real value.
			 */
			fmiReal getRealValue() const;

			/**
			 * @brief Returns the integer-typed value of the variable
			 * @details The function assumes that the variable is valid and holds an
			 * integer value.
			 */
			fmiInteger getIntegerValue() const;

			/**
			 * @brief Returns the boolean-typed value of the variable
			 * @details The function assumes that the variable is valid and holds a
			 * boolean value.
			 */
			fmiBoolean getBooleanValue() const;

			/**
			 * @brief Returns the string-typed value of the variable
			 * @details The function assumes that the variable is valid and holds a
			 * string value.
			 */
			const std::string getStringValue() const;


			/**
			 * @brief Sets the ID of the variable.
			 * @details The ID may not correspond to the previously set value. In 
			 * case the value type does not match the id type, no value must be 
			 * retrieved.
			 * @param id The variable id to set.
			 */
			void setID(const Base::PortID &id);

			/**
			 * @brief Sets the value of the variable.
			 * @details In case the value of the variable doesn't correspond to the 
			 * type of the id, the value must not be queried later on.
			 */
			void setValue(const boost::any &value);

			/**
			 * @brief Checks whether the type is known and corresponds to the value
			 * @details In case the variable is not valid, certain functions must not
			 * be used.
			 */
			bool isValid() const;

			/**
			 * @brief Returns whether the variable type is set in the port id.
			 * @details Unknown types are usually considered as a major error.
			 */
			bool isTypeUnknown() const;

			/**
			 * @brief Converts the Variable to a human readable string representation
			 */
			std::string toString() const;

			/**
			 * @brief Returns whether the variables are equal
			 * @details Two variables are considered equal iff both have the same id
			 * and the same value or both have the same id and are invalid.
			 */
			bool operator == (const Variable &other) const;

			/**
			 * @brief Returns whether both variables have the same content.
			 * @details It is assumed that both variables are valid.
			 */
			bool equalValue(const Variable &other) const;

		private:
			/** @brief The unique identifier of the variable */
			Base::PortID id_;
			/** @brief The value which is typed according to the identifier */
			boost::any data_;
		};

		/**
		 * @brief Prints the content of the variable
		 */
		// Must be declared in the namespace of the argument! 
		// -> Otherwise some boost macros cannot resolve it
		std::ostream& operator<<(std::ostream& stream, 
			const FMITerminalBlock::Timing::Variable& var);
	}
}


#endif
