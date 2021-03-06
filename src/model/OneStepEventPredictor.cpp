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

#include "base/BaseExceptions.h"
#include "model/SolverConfiguration.h"

using namespace FMITerminalBlock;
using namespace FMITerminalBlock::Model;

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
	lowLevelFMU_ = std::make_shared<ManagedLowLevelFMU>(appContext);
	fmu_ = loadModel(appContext, lowLevelFMU_);
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

	initModel(start);
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
		// The event handling function is now called for every input event. Hence,
		// it may be called multiple times until the next continuous time step is
		// made. If this is an issue, consider calling the event handling function
		// just before the next step is calculated. (Probably requires the storage 
		// of an update flag)
		fmu_->handleEvents();

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
	const Base::ApplicationContext &appContext, 
	const std::shared_ptr<ManagedLowLevelFMU> lowLevelFMU)
{
	assert(lowLevelFMU);
	
	FMUType fmuType = lowLevelFMU->getType();
	std::unique_ptr<FMUModelExchangeBase> ret;

	SolverConfiguration solverConfig(appContext);
	if (fmuType == fmi_1_0_me)
	{
		ret = std::unique_ptr<FMUModelExchangeBase>(new fmi_1_0::FMUModelExchange(
			lowLevelFMU->getModelIdentifier(), solverConfig.getFMUDebuggingMode(), 
			fmiFalse, solverConfig.getEventSearchPrecision()));
	}
	else if (fmuType == fmi_2_0_me || fmuType == fmi_2_0_me)
	{
		ret = std::unique_ptr<FMUModelExchangeBase>(new fmi_2_0::FMUModelExchange(
			lowLevelFMU->getModelIdentifier(), solverConfig.getFMUDebuggingMode(), 
			fmiFalse, solverConfig.getEventSearchPrecision()));
	}
	else
	{
		throw Base::SystemConfigurationException(
			std::string("Unsupported FMU type: ") + lowLevelFMU->getTypeString());
	}

	if (ret->getLastStatus() != fmiOK)
	{
		throw Base::SystemConfigurationException("Could not create the model");
	}

	// Set integrator properties
	Integrator::Properties intProp = solverConfig.getIntegratorProperties();
	ret->setIntegratorProperties(intProp);
	if (intProp != solverConfig.getIntegratorProperties())
	{
		boost::format err("The integration configuration was rejected: %1%");
		err % solverConfig.getDiffString(intProp);
		throw Base::SystemConfigurationException(err.str());
	}

	return ret;
}

void 
OneStepEventPredictor::instantiateModel(
	const Base::ApplicationContext &appContext)
{
	assert(fmu_);
	assert(lowLevelFMU_);

	fmiStatus err;
	std::string instanceName = appContext.getProperty<std::string>(
		PROP_FMU_INSTANCE_NAME, lowLevelFMU_->getModelIdentifier() );
	err = fmu_->instantiate(instanceName);
	if (err != fmiOK)
	{
		boost::format fmt("Unable to instantiate the FMU (%1%)");
		fmt % (int) err;
		throw Base::SystemConfigurationException(fmt.str());
	}
}

void 
OneStepEventPredictor::initModel(fmiReal startTime)
{
	assert(fmu_);

	fmiStatus err;
	fmu_->setTime(startTime);
	setDefaultValues(appContext_);
	err = fmu_->initialize(false, 0.0); // Do not use tolerance, yet

	if (err != fmiOK)
	{
		boost::format fmt("Error while initializing the model: %1%");
		fmt % (int) err;
		throw Base::SystemConfigurationException(fmt.str());
	}
	
	// Currently, the very first time event has to be handled by the caller to 
	// overcome a limitation of FMI++
	if (fmu_->checkTimeEvent() && fmu_->getTimeEvent() <= startTime)
	{
		fmu_->handleEvents();
		if (fmu_->getLastStatus() != fmiOK)
		{
			boost::format fmt("Error while handling an initial time event: %1%");
			fmt % (int) fmu_->getLastStatus();
			throw Base::SystemConfigurationException(fmt.str());
		}
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
		if (std::isnan(nextTime) || fmu_->getLastStatus() != fmiOK)
		{
			boost::format fmt("Could not integrate FMU to %1% (%2%, %3%)");
			fmt % nextCompleteStep % nextTime % (int) fmu_->getLastStatus();
			throw Base::SolverException(fmt.str(), fmu_->getTime());
		}
	} while (!simulationProperties_.variableStepSizeOnModelEvent && 
			fmu_->getTime() < nextCompleteStep - 
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
