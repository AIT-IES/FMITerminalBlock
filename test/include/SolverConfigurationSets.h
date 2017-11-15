/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file SolverConfigurationSets.h
 * @brief The file creates a bunch of valid solver configurations.
 * @details It is implemented as a header only variant
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCKTEST_MODEL_SOLVER_CONFIGURATION_SETS
#define _FMITERMINALBLOCKTEST_MODEL_SOLVER_CONFIGURATION_SETS

#include "RawTestData.h"

namespace FMITerminalBlockTest 
{
	namespace Model 
	{
		/// List of solver related CMD arguments
		typedef std::vector<std::string> SolverParams;

		/// Returns various solver test sets.
		std::list<SolverParams> createValidSolverParameterSet()
		{
			std::list<SolverParams> ret;

			ret.push_back({}); //dp
			ret.push_back({"app.integration.method=eu"});
			ret.push_back({"app.integration.method=rk"});
			ret.push_back({"app.integration.method=abm"});
			ret.push_back({"app.integration.method=ck"});
			ret.push_back({"app.integration.method=fe"});
			ret.push_back({"app.integration.method=bs"});
			ret.push_back({"app.integration.method=ro"});
#ifdef USE_SUNDIALS
			ret.push_back({"app.integration.method=bdf"});
			ret.push_back({"app.integration.method=abm2"});
#endif
			// TODO: Test further solver configurations
			return ret;
		}
	}
}

namespace std { // Workaround used for boost test
	/** @brief Prints a string representation of the data buffer */
	std::ostream& operator<<(std::ostream& stream,
		const FMITerminalBlockTest::Model::SolverParams& data)
	{
		stream << '[';
		for (unsigned int i = 0; i < data.size(); i++) {
			stream << data[i];
			if (i < data.size() - 1) stream << ", ";
		}
		stream << ']';
		return stream;
	}
}

#endif
