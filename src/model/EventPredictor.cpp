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
const std::string EventPredictor::PROP_DEFAULT_INPUT = Base::ApplicationContext::PROP_IN + ".default.%1%";

EventPredictor::EventPredictor(Base::ApplicationContext &context):
	context_(context), solver_(NULL), description_(NULL), 
	outputIDs_(5,std::vector<Base::PortID>()), 
	lastPredictedEventTime_(0.0), currentTime_(0.0), outputEventVariables_(), 
	outputEventVariablesPopulated_(false),
	inputIDs_(5,std::vector<Base::PortID>()), realInputImage_(), 
	integerInputImage_(), booleanInputImage_(), stringInputImage_()
{
	std::string path = context.getProperty<std::string>(PROP_FMU_PATH);
	std::string name = context.getProperty<std::string>(PROP_FMU_NAME);

	// Load The model description, the handler function won't be used directly.
	BareFMUModelExchangePtr modExBare;
	modExBare = ModelManager::getModelManager().getModel(path, name, fmiTrue);
	
	if(!modExBare){
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
	if (solver_ != NULL)
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
	defineInputs();

	// initialize FMU
	initSolver(instanceName, start, lookAheadHorizon, lookAheadStepSize, integratorStepSize);
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
	assert(ev != NULL);
	assert(solver_ != NULL);

	bool imageUpdated = updateInputImage(ev);
	if (imageUpdated) {
		// Update the model according to the received event
		fmiTime eventTime = ev->getTime();
		if (currentTime_ - solver_->getTimeDiffResolution() > eventTime) {
			BOOST_LOG_TRIVIAL(warning) << "Received external event is timed before "
				<< "the current time of the model. Changing event time from "
				<< eventTime << " to " << currentTime_ << ".";
			eventTime = currentTime_;
		}

		if (currentTime_ - solver_->getTimeDiffResolution() <= eventTime &&
			currentTime_ + solver_->getTimeDiffResolution() >= eventTime)
		{	// External event occurs at approximately the same time as the last event.
			BOOST_LOG_TRIVIAL(trace) << "Received an event which is timely aligned "
				<< "with the state of the model t=" << currentTime_;
			eventTime = currentTime_; // Align events
		}

		// Forward the time of the model, regardless of the current state time
		// The forward operation is needed to set IncrementalFMU and its FMU 
		// instance to a defined state.
		BOOST_LOG_TRIVIAL(trace) << "Update the model state to t=" << eventTime;
		fmiTime updTime = solver_->updateState(eventTime);
		assert(updTime == eventTime);
		currentTime_ = eventTime;

		// Set the inputs at the event time and execute any event handling functions.
		solver_->syncState(eventTime, realInputImage_.data(), 
			integerInputImage_.data(), booleanInputImage_.data(), 
			stringInputImage_.data());

		// Clear buffered variables.
		outputEventVariables_.clear();
		outputEventVariablesPopulated_ = false;
		lastPredictedEventTime_ = eventTime;
	}
}

std::vector<Timing::Variable> &
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
			<< ". State will be settled by querying event data.";

		fmiTime updTime = solver_->updateStateFromTheRight(time);
		// update may enhance the time on its own -> 2* timeDiffResolution
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

void EventPredictor::initSolver(const std::string& instanceName, 
	const fmiTime startTime, const fmiTime lookAheadHorizon, 
	const fmiTime lookAheadStepSize, const fmiTime integratorStepSize)
{
	const Base::ChannelMapping *inputMapping = context_.getInputChannelMapping();
	assert(solver_ != NULL);

	assert(inputMapping != NULL);
	assert(inputMapping->getVariableNames(fmiTypeReal).size() == 
		realInputImage_.size());
	assert(inputMapping->getVariableNames(fmiTypeInteger).size() == 
		integerInputImage_.size());
	assert(inputMapping->getVariableNames(fmiTypeBoolean).size() == 
		booleanInputImage_.size());
	assert(inputMapping->getVariableNames(fmiTypeString).size() == 
		stringInputImage_.size());

	assert(lookAheadHorizon >= lookAheadStepSize);
	assert(lookAheadStepSize >= integratorStepSize);


	// TODO: Add support for general initial conditions (#373)
	int err = solver_->init(instanceName, 
		inputMapping->getVariableNames(fmiTypeReal).data(), 
		realInputImage_.data(), 
		realInputImage_.size(),

		inputMapping->getVariableNames(fmiTypeInteger).data(),
		integerInputImage_.data(), 
		integerInputImage_.size(),

		inputMapping->getVariableNames(fmiTypeBoolean).data(), 
		booleanInputImage_.data(), 
		booleanInputImage_.size(),

		inputMapping->getVariableNames(fmiTypeString).data(), 
		stringInputImage_.data(), 
		stringInputImage_.size(),

		startTime, lookAheadHorizon, lookAheadStepSize, integratorStepSize);
	if (err != 1)
	{
		boost::format msg("Can't initialize the ModelExchange FMU (%1%)");
		msg % err;
		throw std::runtime_error(msg.str());
	}

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
		switch(type)
		{
		case fmiTypeReal:
			solver_->defineRealOutputs(names.data(), names.size());
			break;
		case fmiTypeInteger:
			solver_->defineIntegerOutputs(names.data(), names.size());
			break;
		case fmiTypeBoolean:
			solver_->defineBooleanOutputs(names.data(), names.size());
			break;
		case fmiTypeString:
			solver_->defineStringOutputs(names.data(), names.size());
			break;
		default:
			assert(0);
		}
	}

}

void 
EventPredictor::fetchOutputs(std::vector<Timing::Variable> &values, fmiTime time)
{
	assert(solver_ != NULL);
	assert(outputIDs_.size() >= 5);

	Timing::Variable element;

	const fmiReal * real = solver_->getRealOutputs();
	assert(outputIDs_[(int) fmiTypeReal].size() == 0 || real != NULL);
	for(unsigned i = 0; i < outputIDs_[(int) fmiTypeReal].size(); i++)
	{
		element.setID(outputIDs_[(int)fmiTypeReal][i]);
		element.setValue(real[i]);
		values.push_back(element);
	}

	const fmiInteger * integer = solver_->getIntegerOutputs();
	assert(outputIDs_[(int) fmiTypeInteger].size() == 0 || integer != NULL);
	for(unsigned i = 0; i < outputIDs_[(int) fmiTypeInteger].size(); i++)
	{
		element.setID(outputIDs_[(int)fmiTypeInteger][i]);
		element.setValue(integer[i]);
		values.push_back(element);
	}

	const fmiBoolean * boolean = solver_->getBooleanOutputs();
	assert(outputIDs_[(int) fmiTypeBoolean].size() == 0 || boolean != NULL);
	for(unsigned i = 0; i < outputIDs_[(int) fmiTypeBoolean].size(); i++)
	{
		element.setID(outputIDs_[(int)fmiTypeBoolean][i]);
		element.setValue(boolean[i]);
		values.push_back(element);
	}
	
	const std::string * str = solver_->getStringOutputs();
	assert(outputIDs_[(int) fmiTypeString].size() == 0 || str != NULL);
	for(unsigned i = 0; i < outputIDs_[(int) fmiTypeString].size(); i++)
	{
		element.setID(outputIDs_[(int)fmiTypeString][i]);
		element.setValue(str[i]);
		values.push_back(element);
	}

}

template<typename InputType>
void EventPredictor::defineInputs(std::vector<InputType> *destinationImage, 
	FMIType type, InputType defaultValue, 
	void(IncrementalFMU::*defineFunction)(const std::string *,std::size_t))
{
	assert(destinationImage != NULL);
	assert(solver_ != NULL);
	const Base::ChannelMapping *mapping = context_.getInputChannelMapping();
	assert(mapping != NULL);

	std::vector<std::string> names = mapping->getVariableNames(type);
	std::vector<Base::PortID> ids = mapping->getVariableIDs(type);
	boost::format fmt(PROP_DEFAULT_INPUT);

	// Add the default variable to the image
	destinationImage->clear();
	for (unsigned int i = 0; i < names.size(); i++)
	{
		fmt.clear();
		fmt % names[i];
		InputType init_value;
		init_value = context_.getProperty<InputType>(fmt.str(), defaultValue);
		destinationImage->push_back(init_value);
	}

	// Register the Inputs
	inputIDs_[type] = ids;
	(solver_->*defineFunction)(names.data(), names.size());
}

void EventPredictor::defineInputs()
{
	const Base::ChannelMapping *mapping = context_.getInputChannelMapping();
	assert(mapping != NULL);
	
	defineInputs<fmiReal>(&realInputImage_, fmiTypeReal, 0.0, 
		&IncrementalFMU::defineRealInputs);
	defineInputs<fmiInteger>(&integerInputImage_, fmiTypeInteger, 0, 
		&IncrementalFMU::defineIntegerInputs);
	defineInputs<fmiBoolean>(&booleanInputImage_, fmiTypeBoolean, false, 
		&IncrementalFMU::defineBooleanInputs);
	defineInputs<std::string>(&stringInputImage_, fmiTypeString, "", 
		&IncrementalFMU::defineStringInputs);

	if (!mapping->getVariableNames(fmiTypeUnknown).empty())
	{
		throw Base::SystemConfigurationException("Model input variable of unknown type registered");
	}
}

template<typename InputType>
bool EventPredictor::updateInputImage(std::vector<InputType> *destinationImage,
	Timing::Event *ev, FMIType type)
{
	assert(destinationImage != NULL);
	assert(ev != NULL);
	assert(((unsigned int) type) < inputIDs_.size());

	std::vector<Timing::Variable> &vars = ev->getVariables();
	std::vector<Base::PortID> &ports = inputIDs_[type];
	bool found = false;
	for (auto varIt = vars.begin(); varIt != vars.end(); ++varIt)
	{
		for (unsigned int i = 0; i < ports.size(); i++)
		{
			if (varIt->getID() == ports[i])
			{
				found = true;
				(*destinationImage)[i] = boost::any_cast<InputType>(varIt->getValue());
			}
		}
	}
	return found;
}

bool EventPredictor::updateInputImage(Timing::Event *ev) {
	bool found = false;
	found |= updateInputImage(&realInputImage_, ev, fmiTypeReal);
	found |= updateInputImage(&integerInputImage_, ev, fmiTypeInteger);
	found |= updateInputImage(&booleanInputImage_, ev, fmiTypeBoolean);
	found |= updateInputImage(&stringInputImage_, ev, fmiTypeString);
	return found;
}
