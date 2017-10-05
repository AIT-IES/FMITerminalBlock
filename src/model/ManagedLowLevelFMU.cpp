/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file ManagedLowLevelFMU.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "model/ManagedLowLevelFMU.h"

#include <stdexcept>

#include <boost/format.hpp>
#include <boost/log/trivial.hpp>

using namespace FMITerminalBlock::Model;
using namespace FMITerminalBlock;

const std::string ManagedLowLevelFMU::PROP_FMU_PATH = "fmu.path";
const std::string ManagedLowLevelFMU::PROP_FMU_NAME = "fmu.name";

ManagedLowLevelFMU::ManagedLowLevelFMU(const Base::ApplicationContext &context)
{
	initVarsAndLoadFMU(context);
	lockFMU();
}

void ManagedLowLevelFMU::initVarsAndLoadFMU(
	const Base::ApplicationContext &context)
{
	path_ = context.getProperty<std::string>(PROP_FMU_PATH);
	modelIdentifier_ = "Unknown";
	type_ = FMUType::invalid;

	ModelManager mgr = ModelManager::getModelManager();
	ModelManager::LoadFMUStatus status;

	if (context.hasProperty(PROP_FMU_NAME)) {

		modelIdentifier_ = context.getProperty<std::string>(PROP_FMU_NAME);
		status = mgr.loadFMU(modelIdentifier_, path_, fmiTrue, type_);
		if (!(status == ModelManager::success 
			|| status == ModelManager::duplicate)) {
			boost::format err("Can't load the FMU at URL \"%1%\" with the "
				"specified name \"%2%\". %3%.");
			err % path_ % modelIdentifier_ % getErrorDescription(status);
			throw std::invalid_argument(err.str());
		}
	} else {

		status = mgr.loadFMU(path_, fmiTrue, type_, modelIdentifier_);
		if (!(status == ModelManager::success 
			|| status == ModelManager::duplicate)) {
			boost::format err("Can't load the FMU at URL \"%1%\" with the "
				"deduced name \"%2%\". %3%.");
			err % path_ % modelIdentifier_ % getErrorDescription(status);
			throw std::invalid_argument(err.str());
		}
		BOOST_LOG_TRIVIAL(debug) << "Take the default FMU model identifier \"" 
			<< modelIdentifier_ << "\" for FMU at \"" << path_ << "\"";
	}
}

void ManagedLowLevelFMU::lockFMU()
{
	ModelManager mgr = ModelManager::getModelManager();
	std::shared_ptr<void> lock; // To increase readability below
	switch (type_) {
		case FMUType::fmi_1_0_cs:
			lock = std::static_pointer_cast<void>(mgr.getSlave(modelIdentifier_));
			break;
		case FMUType::fmi_1_0_me:
			lock = std::static_pointer_cast<void>(mgr.getModel(modelIdentifier_));
			break;
		case FMUType::fmi_2_0_cs:
		case FMUType::fmi_2_0_me:
		case FMUType::fmi_2_0_me_and_cs:
			lock = std::static_pointer_cast<void>(mgr.getInstance(modelIdentifier_));
			break;
		default:
			boost::format err("The FMU at '%1%' has an unsupported FMI type (%2%)");
			err % path_ % getFMUTypeString(type_);
			throw std::invalid_argument(err.str());
	}
	fmuLock_ = lock;
}

std::string 
ManagedLowLevelFMU::getErrorDescription(ModelManager::LoadFMUStatus err)
{
	switch (err)
	{
		case ModelManager::success:
			return "Successful operation";
		case ModelManager::duplicate:
			return "The FMU was loaded before";
		case ModelManager::shared_lib_invalid_uri:
			return "The FMU shared library URL is invalid";
		case ModelManager::shared_lib_load_failed:
			return "The shared library of the FMU cannot be loaded correctly";
		case ModelManager::description_invalid_uri:
			return "The URL of the description is invalid";
		case ModelManager::description_invalid:
			return "The model description is invalid";
		case ModelManager::failed:
			return "Unable to load and instantiate the FMU";
		default:
			boost::format errmsg("Unknown error (code %1%)");
			errmsg % err;
			return errmsg.str();
	}
}

std::string
ManagedLowLevelFMU::getFMUTypeString(FMUType type)
{
	switch (type)
	{
		case fmi_1_0_cs: return "FMI 1.0 CS";
		case fmi_1_0_me: return "FMI 1.0 ME";
		case fmi_2_0_cs: return "FMI 2.0 CS";
		case fmi_2_0_me: return "FMI 2.0 ME";\
		case fmi_2_0_me_and_cs: return "FMI 2.0 CS and ME";
		default: 
			boost::format err("Unknown FMI type (code %1%)");
			err % type;
			return err.str();
	}
}