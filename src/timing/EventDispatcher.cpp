/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file EventDispatcher.cpp
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#include "timing/EventDispatcher.h"
#include "timing/TimedEventQueue.h"

#include <assert.h>
#include <boost/log/trivial.hpp>
#include <float.h>

using namespace FMITerminalBlock::Timing;

const std::string EventDispatcher::PROP_STOP_TIME = "app.stopTime";

EventDispatcher::EventDispatcher(Base::ApplicationContext &context, 
																 Model::AbstractEventPredictor &predictor):
	context_(context), predictor_(predictor), theEnd_(0.0), queue_(NULL), 
	listener_(NULL), timingLogger_()
{
	
	// The default value may take a time but that ok. In this case the program 
	// has to be terminated manually.
	theEnd_ = context.getRealPositiveDoubleProperty(PROP_STOP_TIME, DBL_MAX);

	// Eventually loaded dynamically in future versions.
	queue_ = new TimedEventQueue();

	// Notify the predictor via the common event listener interface.
	addEventListener(predictor);

}

EventDispatcher::~EventDispatcher()
{
	assert(queue_ != NULL);
	delete queue_;
}

void 
EventDispatcher::run()
{
	assert(queue_ != NULL);
	fmiTime currentTime;
	do{
		Event * prediction = predictor_.predictNext();
		timingLogger_.logEvent(prediction, ProcessingStage::prediction);
		queue_->add(prediction, true);

		Event* nextEvent = queue_->get();
		assert(nextEvent != NULL);

		currentTime = nextEvent->getTime();

		// Consume Event
		processEvent(nextEvent);

	}while(currentTime < theEnd_);
	
}

void
EventDispatcher::addEventListener(EventListener & listener)
{
	addEventListener(&listener);
}

void
EventDispatcher::addEventListener(EventListener * listener)
{
	assert(listener != NULL);
	listener_.push_back(listener);
}


void
EventDispatcher::processEvent(Event * ev)
{
		BOOST_LOG_TRIVIAL(debug) << "Process next Event: " << ev->toString();
		timingLogger_.logEvent(ev, ProcessingStage::beginOfDistribution);

		for(std::list<EventListener *>::iterator it = listener_.begin(); 
			it != listener_.end(); ++it)
		{
			(*it)->eventTriggered(ev);
		}

		timingLogger_.logEvent(ev, ProcessingStage::endOfDistribution);
		BOOST_LOG_TRIVIAL(debug) << "Delete processed Event: " << ev->toString();
		delete ev;
}
