/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file EventDispatcher.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_TIMING_EVENT_DISPATCHER
#define _FMITERMINALBLOCK_TIMING_EVENT_DISPATCHER

#include "model/AbstractEventPredictor.h"
#include "base/ApplicationContext.h"
#include "timing/EventQueue.h"
#include "timing/EventListener.h"
#include "timing/EventLogger.h"

#include <common/fmi_v1.0/fmiModelTypes.h>
#include <list>
#include <memory>

namespace FMITerminalBlock 
{
	namespace Timing
	{
		using namespace FMITerminalBlock;

		/**
		 * @brief Class providing event management and distribution functionality.
		 * @details <P> The dispatcher provides functions to register real-world
		 * events which are issued in realtime and can't be predicted. Additionally
		 * it manages an event predictor which returns future events. The event
		 * dispatcher is designed to feature different time management strategies by
		 * instantiating a dedicated queue component. The queue component handles
		 * the timing of the event dispatcher. The EventDispacher class implements
		 * the main program cycle. It may return, if a stopping time is
		 * configured. Additionally the event dispatcher records the application's
		 * timing via an Base::EventLogger instance.</p>
		 */
		class EventDispatcher
		{
		public:

			/** @brief The name of the stop time property */
			static const std::string PROP_STOP_TIME;

			/**
			 * @brief C'tor initializing a ready-to-run event dispatcher.
			 * @details The given references are expected to be valid until the event
			 * dispatcher instance is destroyed. It is also expected that the 
			 * application context is fully populated and that the event predictor is
			 * initialized. The predictor instance mustn't be shared between two 
			 * individual event dispatchers. If the application context contains an
			 * invalid configuration, a Base::SystemConfigurationExeption will be
			 * thrown.
			 */
			EventDispatcher(Base::ApplicationContext &context, 
				Model::AbstractEventPredictor &predictor);

			/** @brief Frees allocated resources */
			~EventDispatcher();

			/**
			 * @brief Runs the FMU until theEnd_ is reached.
			 * @details The first event after theEnd_ will yet be processed and after
			 * that event, the function returns. The function may throw a
			 * std::logic_exception or a Base::SolverException if something bad
			 * happens.
			 */
			void run(void);

			/**
			 * @brief Adds the event listener reference.
			 * @details If an event is triggered every registered listener will be 
			 * informed about the the upcoming event. The passed reference must be
			 * valid until the object is destroyed.
			 * @param listener The event listener reference to add
			 */
			void addEventListener(EventListener & listener);

			/**
			 * @brief Adds the event listener reference
			 * @details If an event is triggered every registered listener will be 
			 * informed about the upcoming event. The passed pointer must be
			 * valid until the object is destroyed.
			 * @param listener The event listener pointer to add
			 */
			void addEventListener(EventListener * listener);

			/**
			 * @brief Returns a valid shared pointer to the managed event sink
			 * @details The event sink reverence may be used to register external 
			 * events. The returned pointer is always valid and mustn't be null.
			 */
			std::shared_ptr<EventSink> getEventSink();

		private:

			/** @brief The global ApplicationContext instance */
			Base::ApplicationContext &context_;

			/** @brief The oracle used to predict future events */
			Model::AbstractEventPredictor &predictor_;

			/** @brief Time horizon used to abort the simulation */
			fmiTime theEnd_;
			
			/**
			 * @brief The Pointer to the event queue in use
			 * @details A function can safely assume that this reference is not NULL
			 */
			std::shared_ptr<EventQueue> queue_;

			/** @brief The list of known event listeners */
			std::list<EventListener *> listener_;

			/** 
			 * @brief The dispatcher's event logger used to trace some timing
			 * parameters
			 */
			EventLogger timingLogger_;

			/**
			 * @brief Processes the given event and deletes it afterwards
			 * @details The function will redirect the events to their listener. After
			 * the event is delivered, the event object will be deleted by the
			 * dispatcher. Deleting the object contradicts the "clean your own
			 * garbage" policy but eases the event's memory management.
			 */
			void processEvent(Event * ev);

		};

	}
}

#endif
