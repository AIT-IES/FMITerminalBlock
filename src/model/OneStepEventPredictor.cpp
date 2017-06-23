/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file OneStepEventPredictor.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "model/OneStepEventPredictor.h"

#include <cassert>
#include <cmath>

#include <boost/format.hpp>
#include <boost/log/trivial.hpp>

#include <import/base/include/FMUModelExchange_v1.h>
#include <import/base/include/FMUModelExchange_v2.h>

#include <base/BaseExceptions.h>

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Model;

const std::string OneStepEventPredictor::PROP_FMU_PATH = "fmu.path";
const std::string OneStepEventPredictor::PROP_FMU_NAME = "fmu.name";
const std::string OneStepEventPredictor::PROP_FMU_INSTANCE_NAME = "fmu.instanceName";
const std::string OneStepEventPredictor::PROP_DEFAULT_INPUT = Base::ApplicationContext::PROP_IN + ".default";
const std::string OneStepEventPredictor::PROP_VARIABLE_STEP_SIZE = "app.variableStepSize";

OneStepEventPredictor::OneStepEventPredictor(
	Base::ApplicationContext &appContext):
	appContext_(appContext),
	outputRealImage_(), outputIntegerImage_(), outputBooleanImage_(), 
	outputStringImage_(),
	outputValueReference_(4, std::vector<fmiValueReference>()),
	outputMapping_(NULL), 
	inputValueReference_(0, Base::hashPortID), 
	currentPrediction_(), fmu_()
{
	fmu_ = loadModel(appContext);
	instantiateModel(appContext);
}

OneStepEventPredictor::~OneStepEventPredictor()
{

}

void 
OneStepEventPredictor::configureDefaultApplicationContext(
	Base::ApplicationContext *appContext)
{
	assert(appContext);
	appContext->addSensitiveDefaultProperties(fmu_->getModelDescription());
}

void 
OneStepEventPredictor::init()
{
	assert(outputMapping_ == NULL);

	fmiTime start = appContext_.getPositiveDoubleProperty(
		Base::ApplicationContext::PROP_START_TIME);

	initOutputStructures(appContext_);
	initInputValueReference(appContext_.getInputChannelMapping());
	initSimulationProperties(appContext_);

	fmiStatus err;
	fmu_->setTime(start);
	setDefaultValues(appContext_);
	err = fmu_->initialize();

	if (err != fmiOK)
	{
		boost::format fmt("Error while initializing the model: %1%");
		fmt % (int) err;
		throw Base::SystemConfigurationException(fmt.str());
	}
	
	(void) updateOutputImage();
}

Timing::Event * 
OneStepEventPredictor::predictNext()
{
	assert(fmu_);

	if (!currentPrediction_)
	{
		predictOneStep();
		bool valuesChanged = updateOutputImage();
		if (valuesChanged)
		{
			currentPrediction_ = getOutputEvent();
		} else {
			// Set an empty event, nothing has changed significantly.
			std::vector<Timing::Variable> emptyVector;
			currentPrediction_ = std::unique_ptr<Timing::StaticEvent>(
				new Timing::StaticEvent(fmu_->getTime(), emptyVector));
		}
	}

	return new Timing::StaticEvent(*currentPrediction_);
}

void 
OneStepEventPredictor::eventTriggered(Timing::Event * ev)
{
	assert(ev);

	bool updated = updateInputVariables(ev);
	if (updated)
	{
		BOOST_LOG_TRIVIAL(debug) << "Event " << ev->toString() 
			<< " was applied to the model at time " << fmu_->getTime();
	} else {
		currentPrediction_.reset();
	}
}

void 
OneStepEventPredictor::initOutputStructures(
	Base::ApplicationContext &appContext)
{
	outputMapping_ = appContext.getOutputChannelMapping();

	// Create properly shaped image vectors
	outputRealImage_.resize(
		outputMapping_->getVariableIDs(fmiTypeReal).size(), 0.0);
	outputIntegerImage_.resize(
		outputMapping_->getVariableIDs(fmiTypeInteger).size(), 0);
	outputBooleanImage_.resize(
		outputMapping_->getVariableIDs(fmiTypeBoolean).size(), fmiFalse);
	outputStringImage_.resize(
		outputMapping_->getVariableIDs(fmiTypeString).size(), "");

	// Init output value reference
	initOutputValueReferences(appContext, fmiTypeReal);
	initOutputValueReferences(appContext, fmiTypeInteger);
	initOutputValueReferences(appContext, fmiTypeBoolean);
	initOutputValueReferences(appContext, fmiTypeString);

	if (outputMapping_->getVariableNames(fmiTypeUnknown).size() > 0)
	{
		boost::format fmt("An output variable (%1%) of unknown type was defined");
		fmt % outputMapping_->getVariableNames(fmiTypeUnknown)[0];
		throw Base::SystemConfigurationException(fmt.str());
	}
}

void 
OneStepEventPredictor::initOutputValueReferences(
	Base::ApplicationContext &appContext, FMIVariableType type)
{
	assert(outputMapping_);
	assert(((unsigned int) type) < outputValueReference_.size());

	auto varNames = outputMapping_->getVariableNames(type);
	for (auto it = varNames.begin(); it != varNames.end(); ++it)
	{
		fmiValueReference ref = fmu_->getValueRef(*it);
		if (ref == fmiUndefinedValueReference)
		{
			boost::format fmt("The output variable %1% is undefined.");
			fmt % *it;
			throw Base::SystemConfigurationException(fmt.str());
		}
		outputValueReference_[(unsigned int) type].push_back(ref);
	}
}

void 
OneStepEventPredictor::initInputValueReference(
	const Base::ChannelMapping *inputMapping)
{
	assert(inputMapping);

	auto allNames = inputMapping->getAllVariableNames();
	auto allIDs = inputMapping->getAllVariableIDs();
	assert(allNames.size() == allIDs.size());

	inputValueReference_.reserve(allNames.size());

	for (unsigned int i = 0; i < allNames.size(); i++)
	{
		fmiValueReference ref = fmu_->getValueRef(allNames[i]);
		if (ref == fmiUndefinedValueReference)
		{
			boost::format fmt("Undefined input variable: %1%");
			fmt % allNames[i];
			throw Base::SystemConfigurationException(fmt.str());
		}
		inputValueReference_[allIDs[i]] = ref;
	}
}

void 
OneStepEventPredictor::initSimulationProperties(
	const Base::ApplicationContext &appContext)
{
	simulationProperties_.lookAheadStepSize = 
		appContext.getRealPositiveDoubleProperty(
			Base::ApplicationContext::PROP_LOOK_AHEAD_TIME);
	simulationProperties_.integratorStepSize = 
		appContext.getRealPositiveDoubleProperty(
			Base::ApplicationContext::PROP_INTEGRATOR_STEP_SIZE, 
			simulationProperties_.lookAheadStepSize/10);

	if(simulationProperties_.lookAheadStepSize < 
		simulationProperties_.integratorStepSize)
	{
		throw Base::SystemConfigurationException("The integrator step size exceeds "
			"the look ahead step size", 
			Base::ApplicationContext::PROP_INTEGRATOR_STEP_SIZE,
			appContext.getProperty<std::string>(
				Base::ApplicationContext::PROP_INTEGRATOR_STEP_SIZE));
	}

	simulationProperties_.variableStepSizeOnModelEvent =
		appContext.getProperty<bool>(PROP_VARIABLE_STEP_SIZE, false);

	simulationProperties_.timingPrecision = 1e-4;
}

std::unique_ptr<FMUModelExchangeBase>
OneStepEventPredictor::loadModel(
	const Base::ApplicationContext &appContext)
{
	FMUType fmuType = invalid;
	ModelManager::LoadFMUStatus loadStatus;
	loadStatus = ModelManager::loadFMU(
		appContext.getProperty<std::string>(PROP_FMU_NAME), 
		appContext.getProperty<std::string>(PROP_FMU_PATH), fmiFalse, fmuType);

	if (loadStatus != ModelManager::success &&
		loadStatus != ModelManager::duplicate)
	{
		throw Base::SystemConfigurationException(
			std::string("The model instantiation failed: ") +
			getErrorDescription(loadStatus));
	}


	std::unique_ptr<FMUModelExchangeBase> ret;
	if (fmuType == fmi_1_0_me)
	{
		ret = std::unique_ptr<FMUModelExchangeBase>(new fmi_1_0::FMUModelExchange(
			appContext.getProperty<std::string>(PROP_FMU_NAME)));
	}
	else if (fmuType == fmi_2_0_me || fmuType == fmi_2_0_me)
	{
		ret = std::unique_ptr<FMUModelExchangeBase>(new fmi_2_0::FMUModelExchange(
			appContext.getProperty<std::string>(PROP_FMU_NAME)));
	}
	else
	{
		throw Base::SystemConfigurationException(
			std::string("Unsupported FMU type: ") = getFMUTypeString(fmuType));
	}

	if (ret->getLastStatus() != fmiOK)
	{
		throw Base::SystemConfigurationException("Could not create the model");
	}

	return ret;
}

void 
OneStepEventPredictor::instantiateModel(
	const Base::ApplicationContext &appContext)
{
	assert(fmu_);
	fmiStatus err;
	err = fmu_->instantiate(
		appContext.getProperty<std::string>(PROP_FMU_INSTANCE_NAME,
			appContext.getProperty<std::string>(PROP_FMU_NAME)));
	if (err != fmiOK)
	{
		boost::format fmt("Unable to instantiate the FMU (%1%)");
		fmt % (int) err;
		throw Base::SystemConfigurationException(fmt.str());
	}
}

std::string 
OneStepEventPredictor::getErrorDescription(ModelManager::LoadFMUStatus err)
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
			return "Unknown error";
	}
}

std::string
OneStepEventPredictor::getFMUTypeString(FMUType type)
{
	switch (type)
	{
		case fmi_1_0_cs: return "FMI 1.0 CS";
		case fmi_1_0_me: return "FMI 1.0 ME";
		case fmi_2_0_cs: return "FMI 2.0 CS";
		case fmi_2_0_me: return "FMI 2.0 ME";\
		case fmi_2_0_me_and_cs: return "FMI 2.0 CS and ME";
		default: return "Unknown type";
	}
}

void 
OneStepEventPredictor::setDefaultValues(
	const Base::ApplicationContext &appContext)
{
	if (!appContext.hasProperty(PROP_DEFAULT_INPUT)) return;
	const boost::property_tree::ptree &def = 
		appContext.getPropertyTree(PROP_DEFAULT_INPUT);
	for (auto defVal = def.begin(); defVal != def.end(); ++defVal)
	{
		std::string varName = defVal->first;
		setDefaultValue(appContext, varName);
	}
}

void 
OneStepEventPredictor::setDefaultValue(
	const Base::ApplicationContext &appContext, const std::string &varName)
{
	std::string varPath = PROP_DEFAULT_INPUT + "." + varName;

	FMIVariableType type = fmu_->getType(varName);
	if (type == fmiTypeUnknown)
	{
		throw Base::SystemConfigurationException(
			std::string("Unknown default variable in ") + varPath);
	}

	fmiStatus err = fmiFatal;
	switch (type)
	{
		case fmiTypeReal:
			err = fmu_->setValue(varName,
				appContext.getProperty<fmiReal>(varPath));
			break;
		case fmiTypeInteger:
			err = fmu_->setValue(varName,
				appContext.getProperty<fmiInteger>(varPath));
			break;
		case fmiTypeBoolean:
			err = fmu_->setValue(varName,
				appContext.getProperty<fmiBoolean>(varPath));
			break;
		case fmiTypeString:
			err = fmu_->setValue(varName,
				appContext.getProperty<std::string>(varPath));
			break;
		default: assert(0);
	}
	if (err != fmiOK)
	{
		boost::format fmt("Cannot set the default variable (%1%)");
		fmt % (int) err;
		throw Base::SystemConfigurationException(fmt.str(), varPath, 
			appContext.getProperty<std::string>(varPath));
	}
}

void 
OneStepEventPredictor::predictOneStep()
{
	assert(fmu_);

	fmiTime nextCompleteStep;
	nextCompleteStep = fmu_->getTime() + simulationProperties_.lookAheadStepSize;
	do {
		fmiTime nextTime = fmu_->integrate(nextCompleteStep, 
			simulationProperties_.integratorStepSize);
		if (isnan(nextTime) || fmu_->getLastStatus() != fmiOK)
		{
			boost::format fmt("Could not integrate FMU to %1% (%2%, %3%)");
			fmt % nextCompleteStep % nextTime % (int) fmu_->getLastStatus();
			throw Base::SolverException(fmt.str(), fmu_->getTime());
		}
	} while (!simulationProperties_.variableStepSizeOnModelEvent && 
			fabs(fmu_->getTime() - nextCompleteStep) > 
				simulationProperties_.timingPrecision);
}

template<typename valType>
bool 
OneStepEventPredictor::updateOutputImage(
	std::vector<valType> *destinationImage,
	std::vector<fmiValueReference> &referenceVector)
{
	assert(destinationImage);
	assert(referenceVector.size() == destinationImage->size());
	assert(fmu_);

	// Some FMUs issue a warning in case no output should be fetched
	if (destinationImage->size() <= 0) return false;

	// Fetch the outputs
	std::unique_ptr<valType[]> tmpVal(new valType[referenceVector.size()]);
	fmiStatus err = fmu_->getValue(referenceVector.data(), tmpVal.get(), 
		referenceVector.size());
	if (err != fmiOK)
	{
		boost::format fmt("Could not fetch the outputs of the model (%1%)");
		fmt % (int) err;
		throw Base::SolverException(fmt.str(), fmu_->getTime());
	}

	// Compare and update
	bool significantChange = false;
	for (unsigned int i = 0; i < referenceVector.size(); i++)
	{
		if (destinationImage->operator[](i) != tmpVal[i])
		{
			significantChange = true;
		}
		destinationImage->operator[](i) = tmpVal[i];
	}
	return significantChange;
}

bool 
OneStepEventPredictor::updateOutputImage()
{
	bool significantChange = false;

	significantChange |= updateOutputImage(&outputRealImage_, 
		outputValueReference_[fmiTypeReal]);
	significantChange |= updateOutputImage(&outputIntegerImage_, 
		outputValueReference_[fmiTypeInteger]);
	significantChange |= updateOutputImage(&outputBooleanImage_, 
		outputValueReference_[fmiTypeBoolean]);
	significantChange |= updateOutputImage(&outputStringImage_, 
		outputValueReference_[fmiTypeString]);

	return significantChange;
}

std::unique_ptr<Timing::StaticEvent>
OneStepEventPredictor::getOutputEvent()
{
	assert(outputMapping_);
	assert(fmu_);

	std::vector<Timing::Variable> vars;
	vars.reserve(outputMapping_->getTotalNumberOfVariables());
	
	appendOutputVariables(&vars, outputMapping_->getVariableIDs(fmiTypeReal), 
		outputRealImage_);
	appendOutputVariables(&vars, outputMapping_->getVariableIDs(fmiTypeInteger), 
		outputIntegerImage_);
	appendOutputVariables(&vars, outputMapping_->getVariableIDs(fmiTypeBoolean), 
		outputBooleanImage_);
	appendOutputVariables(&vars, outputMapping_->getVariableIDs(fmiTypeString), 
		outputStringImage_);

	return std::unique_ptr<Timing::StaticEvent>(
		new Timing::StaticEvent(fmu_->getTime(), vars));
}

template<typename valType>
void 
OneStepEventPredictor::appendOutputVariables(
	std::vector<Timing::Variable> *destination,
	const std::vector<Base::PortID> &ids,
	const std::vector<valType> &values) const
{
	assert(ids.size() == values.size());
	assert(destination);

	for (unsigned int i = 0; i < ids.size(); i++)
	{
		destination->push_back(Timing::Variable(ids[i], values[i]));
	}
}

bool 
OneStepEventPredictor::updateInputVariables(Timing::Event *ev)
{
	assert(ev);
	bool inputVariableSet = false;

	auto vars = ev->getVariables();
	for (auto it = vars.begin(); it != vars.end(); ++it)
	{
		inputVariableSet |= updateInputVariable(*it);
	}
	return inputVariableSet;
}

bool
OneStepEventPredictor::updateInputVariable(const Timing::Variable &variable)
{
	assert(variable.isValid());
	assert(fmu_);
	auto varID = variable.getID();
	if (inputValueReference_.count(varID) > 0)
	{
		fmiStatus err = fmiFatal;
		auto varRef = inputValueReference_[varID];
		switch (varID.first)
		{
			case fmiTypeReal:
			{
				auto rVal = variable.getRealValue();
				err = fmu_->setValue(varRef, rVal);
			}	break;
			case fmiTypeInteger:
			{
				auto iVal = variable.getIntegerValue();
				err = fmu_->setValue(varRef, iVal);
			}	break;
			case fmiTypeBoolean:
			{
				auto bVal = variable.getBooleanValue();
				err = fmu_->setValue(varRef, bVal);
			}	break;
			case fmiTypeString:
			{
				auto sVal = variable.getStringValue();
				err = fmu_->setValue(varRef, sVal);
			}	break;
			default: assert(0);
		}

		if (err != fmiOK)
		{
			boost::format fmt("Unable to set input value %1% (%2%)");
			fmt % variable.toString() % (int) err;
			throw Base::SolverException(fmt.str(), fmu_->getTime());
		}

		return true;
	} else {
		return false;
	}
}