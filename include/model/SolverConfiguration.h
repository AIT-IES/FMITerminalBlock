/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file SolverConfiguration.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_MODEL_SOLVER_CONFIGURATION
#define _FMITERMINALBLOCK_MODEL_SOLVER_CONFIGURATION

#include <string>

// Fixes an include dependency flaw/feature(?) of ModelDescription.h
#include <common/fmi_v1.0/fmiModelTypes.h>
#include <import/integrators/include/Integrator.h>

#include "base/ApplicationContext.h"

namespace FMITerminalBlock
{
	namespace Model
	{

		/**
		 * @brief Facility class which stores common parameters of a solver
		 * @details The class is generated from an ApplicationContext which is 
		 * also needed to populate contained variables. It is primarily intended to
		 * provide a unique configuration interface for solver related parameters.
		 */
		class SolverConfiguration
		{
		public:
			/** @brief The name of the FMU debugging property */
			static const std::string PROP_FMU_DEBUGGING;
			/** @brief The name of the event search precision property */
			static const std::string PROP_EVENT_SEARCH_PRECISION;
			/** @brief The name of the property which specifies the integrator type*/
			static const std::string PROP_INTEGRATOR_TYPE;
			/** @brief The name of the integration order property */
			static const std::string PROP_INTEGRATION_ORDER;
			/** @brief The name of the absolute tolerance */
			static const std::string PROP_ABSOLUTE_TOLERANCE;
			/** @brief The name of the relative tolerance */
			static const std::string PROP_RELATIVE_TOLERANCE;

			/**
			 * @brief Queries the given ApplicationContext and sets encapsulated 
			 * parameters.
			 * @details In case a configuration error is detected, a 
			 * Base::SystemConfigurationException will be thrown.
			 * @param appContext A valid reference to the configuration source.
			 */
			SolverConfiguration(const Base::ApplicationContext& configSource);

			/** @brief Frees allocated resources */
			virtual ~SolverConfiguration() {};

			/** @brief Returns the structure of integrator properties */
			Integrator::Properties getIntegratorProperties() { return props_; }

			/** 
			 * @brief returns true whenever the FMU should be operated in the 
			 * debugging mode
			 */
			bool getFMUDebuggingMode() { return fmuDebugging_; }
			/** @brief Returns the global event search precision in seconds */
			fmiTime getEventSearchPrecision() { return eventSerachPrecision_; }

			/**
			 * @brief Returns a human readable representation of any differences
			 * @details The given property set is compared to the internal 
			 * representation. In case any significant deviations are found, the 
			 * description is returned. The function is primarily intended to 
			 * describe automatic parameter adjustments in case invalid properties 
			 * are found.
			 */
			std::string getDiffString(const Integrator::Properties &refProps) const;

		private:
			/** @brief The configured integrator properties */
			Integrator::Properties props_;

			/** @brief Flag which enables FMU debug messages */
			bool fmuDebugging_;

			/** @brief The timely precision of event search processes */
			fmiTime eventSerachPrecision_;

			/** 
			 * @brief Compares the two double values and returns a human readable 
			 * string representation
			 * @param setValue The currently set value
			 * @param refValue The official reference value
			 * @details name An identifier to describe the value (e.g. the 
			 * application context property name)
			 */
			static std::string getDiffString(double setVal, double refVal, 
				const std::string &name);

			/**
			 * @brief Populates the Integrator::Properties structure and returns it
			 * @details In case some invalid values are found, an exception is 
			 * thrown.
			 * @param configSource The context to read the properties.
			 */
			static Integrator::Properties makeIntegratorProperties(
				const Base::ApplicationContext& configSource);

			/** 
			 * @brief Parses the given type id and returns it
			 * @details The type must follow the naming convention as introduced by 
			 * the enumeration. In case an error is detected, a 
			 * Base::SystemConfigurationException will be thrown.
			 */
			static IntegratorType toIntegratorType(const std::string &id);

			/** @brief Returns the sensitive default integrator name */
			static std::string getDefaultName(IntegratorType type);
			/** @brief Returns the sensitive default integrator order */
			static int getDefaultOrder(IntegratorType type);
			/** @brief Returns the sensitive absolute default tolerance */
			static double getDefaultAbsoluteTolerance(IntegratorType type);
			/** @brief Returns the sensitive relative default tolerance */
			static double getDefaultRelativeTolerance(IntegratorType type);
		};

	}
}

#endif
