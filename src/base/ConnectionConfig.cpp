/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file baseConnectionConfig.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "base/ConnectionConfig.h"

using namespace FMITerminalBlock::Base;

ConnectionConfig::ConnectionConfig(
	const boost::property_tree::ptree& configTree, const std::string& id):
			id_(id), config_(configTree)
{
}

