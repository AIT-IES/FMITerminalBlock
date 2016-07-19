/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file PredictingFMU.cpp
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#include "model/PredictingFMU.h"

#include <assert.h>
#include <boost/log/trivial.hpp>
#include <stdio.h>
#include <math.h>
#include <string>

using namespace FMITerminalBlock::Model;

PredictingFMU::PredictingFMU(const std::string& fmuPath, 
														 const std::string& modelName,
														 const fmiBoolean loggingOn,
														 const fmiReal timeDiffResolution, 
														 const IntegratorType type): 
	IncrementalFMU(fmuPath, modelName, loggingOn, timeDiffResolution, type),
	timeDiffResolution_(timeDiffResolution)

{
}


PredictingFMU::PredictingFMU(const std::string& xmlPath, 
														 const std::string& dllPath, 
														 const std::string& modelName,
														 const fmiBoolean loggingOn,
														 const fmiReal timeDiffResolution, 
														 const IntegratorType type):
	IncrementalFMU(xmlPath, dllPath, modelName, loggingOn, timeDiffResolution, type),
	timeDiffResolution_(timeDiffResolution)
{
}

PredictingFMU::~PredictingFMU()
{
}

