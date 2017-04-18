/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file EventPredictor.cpp
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#include "model/EventPredictor.h"
#include "base/BaseExceptions.h"
#include "base/ApplicationContext.h"
#include "model/LazyEvent.h"

#include <import/base/include/ModelManager.h>
#include <assert.h>
#include <algorithm>
#include <boost/any.hpp>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <stdexcept>
#include <string>
#include <utility>
#include <stdio.h>

using namespace FMITerminalBlock::Model;
using namespace FMITerminalBlock;

const std::string EventPredictor::PROP_FMU_PATH = "fmu.path";
const std::string EventPredictor::PROP_FMU_NAME = "fmu.name";
const std::string EventPredictor::PROP_FMU_INSTANCE_NAME = "fmu.instanceName";

EventPredictor::EventPredictor(Base::ApplicationContext &context):
	context_(context), solver_(NULL), description_(NULL), 
	outputIDs_(5,std::vector<Base::PortID>()), 
	lastPredictedEventTime_(0.0), currentTime_(0.0), outputEventVariables_(), 
	outputEventVariablesPopulated_(false)
{
	std::string path = context.getProperty<std::string>(PROP_FMU_PATH);
	std::string name = context.getProperty<std::string>(PROP_FMU_NAME);

	// Load The model description, the handler function won't be used directly.
	struct BareFMUModelExchange * modExBare = ModelManager::getModelManager().getModel(path, name, fmiTrue);
	
	if(NULL == modExBare){
		boost::format err("Can't load the ModelExchange FMU \"%1%\" in URL \"%2%\"");
		err % name % path;
		throw std::invalid_argument(err.str());
	}
	assert(modExBare->description != NULL);
	description_ = modExBare->description;

	solver_ = new PredictingFMU(path, name);
}

EventPredictor::~EventPredictor()
{
	if(solver_ != NULL)
		delete solver_;
}

void 
EventPredictor::init()
{
	// Fetch parameter. Set sensitive default values, if needed
	std::string instanceName = context_.getProperty<std::string>(
		PROP_FMU_INSTANCE_NAME, context_.getProperty<std::string>(PROP_FMU_NAME));
	fmiTime start = context_.getPositiveDoubleProperty(
		Base::ApplicationContext::PROP_START_TIME);
	fmiTime lookAheadHorizon = context_.getRealPositiveDoubleProperty(
		Base::ApplicationContext::PROP_LOOK_AHEAD_TIME);
	fmiTime lookAheadStepSize = context_.getRealPositiveDoubleProperty(
		Base::ApplicationContext::PROP_LOOK_AHEAD_STEP_SIZE, lookAheadHorizon/10);
	fmiTime integratorStepSize = context_.getRealPositiveDoubleProperty(
		Base::ApplicationContext::PROP_INTEGRATOR_STEP_SIZE, lookAheadStepSize/10);

	// Check values
	if(lookAheadHorizon < lookAheadStepSize)
	{
		throw Base::SystemConfigurationException("The look ahead step size exceeds "
			"the lookahead horizon", 
			Base::ApplicationContext::PROP_LOOK_AHEAD_STEP_SIZE,
			context_.getProperty<std::string>(
				Base::ApplicationContext::PROP_LOOK_AHEAD_STEP_SIZE));
	}
	if(lookAheadStepSize < integratorStepSize)
	{
		throw Base::SystemConfigurationException("The integrator step size exceeds "
			"the look ahead step size", 
			Base::ApplicationContext::PROP_INTEGRATOR_STEP_SIZE,
			context_.getProperty<std::string>(
				Base::ApplicationContext::PROP_INTEGRATOR_STEP_SIZE));
	}
	
	BOOST_LOG_TRIVIAL(debug) << "Try to initialize the FMU \"" << instanceName 
		<< "\": " << Base::ApplicationContext::PROP_START_TIME << "=" << start 
		<< ", "	<< Base::ApplicationContext::PROP_LOOK_AHEAD_TIME << "=" 
		<< lookAheadHorizon	<< ", " 
		<< Base::ApplicationContext::PROP_LOOK_AHEAD_STEP_SIZE 	<< "=" 
		<< lookAheadStepSize	<< ", " 
		<< Base::ApplicationContext::PROP_INTEGRATOR_STEP_SIZE << "=" 
		<< integratorStepSize;

	// Initialize global variables
	currentTime_ = start;
	lastPredictedEventTime_ = start;
	outputEventVariables_.clear();
	outputEventVariablesPopulated_ = false;

	defineOutputs(context_.getOutputChannelMapping());

	// initialize FMU
	int err = solver_->init(instanceName, NULL, NULL, 0, start, lookAheadHorizon,
		lookAheadStepSize, integratorStepSize);
	if(err != 1)
	{
		boost::format msg("Can't initialize the ModelExchange FMU (%1%)");
		msg % err;
		throw std::runtime_error(msg.str());
	}

}

Timing::Event *
EventPredictor::predictNext()
{
	fmiTime nextEventTime = solver_->predictState(currentTime_);
	if(nextEventTime == INVALID_FMI_TIME)
	{
		throw Base::SolverException("Can't predict the next event.", currentTime_);
	}

	// Instance will be managed by the calling function
	LazyEvent * ret = new LazyEvent(nextEventTime,*this);

	// Clear buffered variables.
	outputEventVariables_.clear();
	outputEventVariablesPopulated_ = false;
	lastPredictedEventTime_ = nextEventTime;
	return ret;
}

void 
EventPredictor::eventTriggered(Timing::Event * ev)
{
	//TODO: Update input variables, if an external event was received. Otherwise check if solver_->updateState has to be called.
}

std::vector<Timing::Event::Variable> &
EventPredictor::getOutputVariables(fmiTime time)
{
	assert(time >= 0.0);
	assert(solver_ != NULL);
	// Time corresponds to the current event's time
	assert(abs(lastPredictedEventTime_ - time) 
		< solver_->getTimeDiffResolution());
	// The current Event isn't outdated
	assert(currentTime_ <= time + solver_->getTimeDiffResolution());
	// Time was forwarded correctly
	assert(!outputEventVariablesPopulated_ || currentTime_ == lastPredictedEventTime_);

	// Set the same time, they are nearly equal anyway.
	time = lastPredictedEventTime_;

	if(!outputEventVariablesPopulated_)
	{
		assert(outputEventVariables_.empty());

		// fix time and populate outputEventVariables_
		BOOST_LOG_TRIVIAL(trace) << "Pandora's box opened at " << time 
			<< ". State will be setteled by querying event data.";

		fmiTime updTime = solver_->updateStateFromTheRight(time);
		// update may inhance the time on its own -> 2* timeDiffResolution
		if(abs(updTime - time) > 2*solver_->getTimeDiffResolution())
		{
			throw Base::SolverException("Can't update the model's state", time);
		}
		currentTime_ = time;
		fetchOutputs(outputEventVariables_, time);
		outputEventVariablesPopulated_ = true;
	}
	return outputEventVariables_;
}

void 
EventPredictor::defineOutputs(const Base::ChannelMapping *mapping)
{

	assert(mapping != NULL);

	defineOutput(mapping, fmiTypeReal);
	defineOutput(mapping, fmiTypeInteger);
	defineOutput(mapping, fmiTypeBoolean);
	defineOutput(mapping, fmiTypeString);

	if(!mapping->getVariableNames(fmiTypeUnknown).empty())
	{
		throw Base::SystemConfigurationException("Model variable of unknown type registered");
	}
}

void 
EventPredictor::defineOutput(const Base::ChannelMapping *mapping, FMIType type)
{
	assert(outputIDs_.size() >= 5);
	assert(solver_ != NULL);
	assert(mapping != NULL);
	assert((int) type >= 0);
	assert((int) type < 5);

	const std::vector<std::string> &names = mapping->getVariableNames(type);
	outputIDs_[type] = mapping->getVariableIDs(type);

	if(!names.empty())
	{
		std::string * nameBuffer = new std::string[names.size()];
		std::copy(names.begin(), names.end(), nameBuffer);
		switch(type)
		{
		case fmiTypeReal:
			solver_->defineRealOutputs(nameBuffer, names.size());
			break;
		case fmiTypeInteger:
			solver_->defineIntegerOutputs(nameBuffer, names.size());
			break;
		case fmiTypeBoolean:
			solver_->defineBooleanOutputs(nameBuffer, names.size());
			break;
		case fmiTypeString:
			solver_->defineStringOutputs(nameBuffer, names.size());
			break;
		default:
			assert(0);
		}
		delete [] nameBuffer;
	}

}

void 
EventPredictor::fetchOutputs(std::vector<Timing::Event::Variable> &values, fmiTime time)
{
	assert(solver_ != NULL);
	assert(outputIDs_.size() >= 5);

	Timing::Event::Variable element;

	const fmiReal * real = solver_->getRealOutputs();
	assert(outputIDs_[(int) fmiTypeReal].size() == 0 || real != NULL);
	for(unsigned i = 0; i < outputIDs_[(int) fmiTypeReal].size(); i++)
	{
		element.first = outputIDs_[(int)fmiTypeReal][i];
		element.second = real[i];
		values.push_back(element);
	}

	const fmiInteger * integer = solver_->getIntegerOutputs();
	assert(outputIDs_[(int) fmiTypeInteger].size() == 0 || integer != NULL);
	for(unsigned i = 0; i < outputIDs_[(int) fmiTypeInteger].size(); i++)
	{
		element.first = outputIDs_[(int)fmiTypeInteger][i];
		element.second = integer[i];
		values.push_back(element);
	}

	const fmiBoolean * boolean = solver_->getBooleanOutputs();
	assert(outputIDs_[(int) fmiTypeBoolean].size() == 0 || boolean != NULL);
	for(unsigned i = 0; i < outputIDs_[(int) fmiTypeBoolean].size(); i++)
	{
		element.first = outputIDs_[(int)fmiTypeBoolean][i];
		element.second = boolean[i];
		values.push_back(element);
	}
	
	const std::string * str = solver_->getStringOutputs();
	assert(outputIDs_[(int) fmiTypeString].size() == 0 || str != NULL);
	for(unsigned i = 0; i < outputIDs_[(int) fmiTypeString].size(); i++)
	{
		element.first = outputIDs_[(int)fmiTypeString][i];
		element.second = str[i];
		values.push_back(element);
	}

}
