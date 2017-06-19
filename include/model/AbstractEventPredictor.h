/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file AbstractEventPredictor.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_MODEL_ABSTRACT_EVENT_PREDICTOR
#define _FMITERMINALBLOCK_MODEL_ABSTRACT_EVENT_PREDICTOR

#include "timing/EventListener.h"
#include "timing/Event.h"
#include "base/ApplicationContext.h"

namespace FMITerminalBlock
{
	namespace Model
	{

		using namespace FMITerminalBlock;

		/**
		 * @brief Defines an abstract interface used to provide future events.
		 * @details The event predictor interface offers a prediction function which
		 * returns the next upcoming event. The prediction may be based on the
		 * system's current state. Each event which triggers a state change or which
		 * fixes a predicted state must be offered using the event listener
		 * interface. After constructing an event predictor, it has to be
		 * initialized in order to use any other provided function. Without
		 * successfully calling the init() function, any other function except 
		 * configureDefaultApplicationContext(...) may return undesired results.
		 */
		class AbstractEventPredictor: public Timing::EventListener
		{
		public:
			/** @brief Frees allocated resources */
			virtual ~AbstractEventPredictor() {}

			/**
			 * @brief Configures the model dependent default properties
			 * @details The predictor should set any default property which depends 
			 * on the meta-data of the model or any other event source. The function 
			 * is called before the init() function is called. The predictor must not
			 * rely on the previously set values. Every value which is set in the 
			 * application context may be altered until the init() function is 
			 * called.
			 * @param appContext The destination application context. It is 
			 * guaranteed that the pointer is valid. Nevertheless, the pointer may 
			 * differ from any application context which is previously set or which 
			 * will be set by future function invocations.
			 */
			virtual void configureDefaultApplicationContext(
				Base::ApplicationContext *appContext) = 0;

			/**
			 * @brief Initializes the event predictor
			 * @details <p> The function has to be called successfully before any
			 * other function except configureDefaultApplicationContext(...) is used.
			 * It may utilize the previously set information to perform the 
			 * initialization.</p>
 			 * <p>The function may throw std::invalid_argument or a
			 * Base::SystemConfigurationException if the predictors
			 * initial state is invalid</p>
			 */
			virtual void init(void) = 0;

			/**
			 * @brief Predicts the next event and returns it
			 * @details <p> The function may only be called after initializing the
			 * event predictor and after registering the last event. Any subsequent
			 * calls of this function is not allowed and may result in undesired
			 * behavior. If some error during solving the next step occurred, a
			 * Base::SolverException will be thrown. The returned pointer will have to 
			 * be managed by the caller. The caller MUST delete the referenced object 
			 * after processing it.</p>
			 * <p> The returned event may be linked to the predictor instance and must
			 * be destroyed before the event predictor instance gets deleted. Reading
			 * the event's variables (not it's time) may settle the model's state.
			 * After the variable values are read and the state is settled, the 
			 * predictor mustn't receive external events which are timed before the 
			 * predicted event's time.</p>
			 * @return A pointer to a newly created event raised by the model. (Must
			 * be deleted by the caller!)
			 */
			virtual Timing::Event * predictNext(void) = 0;

		};
	}
}
#endif
