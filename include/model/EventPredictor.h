/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file EventPredictor.h
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_MODEL_EVENT_PREDICTOR
#define _FMITERMINALBLOCK_MODEL_EVENT_PREDICTOR


#include "model/AbstractEventPredictor.h"
#include "model/PredictingFMU.h"
#include "base/ChannelMapping.h"
#include "timing/Event.h"
#include "timing/EventListener.h"

#include <import/base/include/ModelDescription.h>
#include <common/fmi_v1.0/fmiModelTypes.h>
#include <vector>

namespace FMITerminalBlock
{
	namespace Model
	{

		using namespace FMITerminalBlock;

		/**
		 * @brief Encapsulates and accesses the FMI-model
		 * @details The class contains the solver and a copy of the output
		 * variable's state. It provides functions to query the next event and to 
		 * register an incoming event at the current lookahead horizon. The
		 * EventPredictor doesn't implement the prediction logic itself but it
		 * interfaces FMI++. Although FMI++ implements the actual event prediction,
		 * some management functionality is still required.
		 */
		class EventPredictor: public AbstractEventPredictor
		{
		public:

			/** @brief Used to lazy load the event's data */
			friend class LazyEvent;

			/** @brief The name of the FMU path property */
			static const std::string PROP_FMU_PATH;
			/** @brief The name of the FMU name property */
			static const std::string PROP_FMU_NAME;
			/** @brief The name of the FMU instance name property */
			static const std::string PROP_FMU_INSTANCE_NAME;

			/**
			 * @brief C'tor loading a FMU
			 * @details <p>The application context has to contain the initial 
			 * parameters which are necessary to instantiate the solver. If the
			 * context object doesn't contain enough arguments to load the FMU, a
			 * std::invalid_argument will be thrown.</p>
			 * <p> The event predictor requires at least the following properties:
			 * <ul>
			 *	<li>EventPredictor::PROP_FMU_PATH</li>
			 *	<li>EventPredictor::PROP_FMU_NAME</li>
			 * </ul></p>
			 * @param context A reference to the partially populated application
			 * context
			 */
			EventPredictor(Base::ApplicationContext &context);

			/**
			 * @brief Frees allocated resources
			 * @details Deletes the FMU instance
			 */
			virtual ~EventPredictor();

			/**
			 * @brief Returns a pointer to the model's description
			 * @details The model's description contains the XML file's data. It is
			 * guaranteed that the pointer is valid as long as the object exist. Do
			 * not modify the model description! Any modification may result in
			 * undesired behavior.
			 * @return a pointer to the model's description
			 */
			const ModelDescription * getModelDescription() const
				{ return description_; }

			/**
			 * @brief Initializes the model
			 * @details <p>The application context object may not be fully populated
			 * but the given reference has to be valid during the object's life time.
			 * It may be used to retain information during the initialization. The two
			 * stage initialization process may be used to load some information 
			 * provided by an encapsulated model dynamically. The gathered 
			 * information may be fed into the application context after constructing
			 * the event predictor. Although the event predictor may be initialized 
			 * later, the application context may have to contain a minimal 
			 * configuration.</p>
			 * <p>The function will first check and set available properties. If the
			 * global context which was previously set contains an invalid
			 * configuration a Base::SystemConfigurationException will be thrown. If
			 * the solver can't be initialized correctly a std::runtime_error will be
			 * thrown.</p>
			 * <p> The function requires at least the following properties given by 
			 * the previously assigned application context:
			 * <ul>
			 *	<li>Base::ApplicationContext::PROP_START_TIME</li>
			 *	<li>Base::ApplicationContext::PROP_LOOK_AHEAD_TIME</li>
			 *  <li>Properties used to generate a valid channel mapping. Generating 
			 *      the channel mapping may be implicitly done by fetching the 
			 *      channel mapping.</li>
			 * </ul></p>
			 */
			virtual void init(void);

			/**
			 * @brief Predicts the next event and returns it.
			 * @details The function may only be called after initializing the event 
			 * predictor and after registering the last event. Any subsequent calls of
			 * this function is not allowed and may result in undesired behavior.
			 * If the end of the prediction horizon is reached without triggering an
			 * FMI event, the state of the last prediction will be returned. The 
			 * event will only contain output channels which were registered at the 
			 * time the init function is called.
			 * If some error during solving the next step occurs, a
			 * Base::SolverException will be thrown. The returned pointer will have to 
			 * be managed by the caller. The caller MUST delete the referenced object 
			 * after processing it.
			 * @return A pointer to a newly created event raised by the model. (Must
			 * be deleted by the caller!)
			 */
			virtual Timing::Event * predictNext(void);

			/**
			 * @brief Updates the simulation using the received event.
			 * @details The function will set every changed input but doesn't call the
			 * predictNext. After receiving the event it isn't possible to reset the
			 * FMU prior the given event's time. Any such attempt may lead to
			 * undesired behavior. The function is NOT fully implemented yet
			 * @param ev The reference to the triggered event.
			 */
			virtual void eventTriggered(Timing::Event * ev);

		private:
			/** 
			 * @brief Reference to the global application context.
			 * @details The context may not always be fully populated which includes
			 * missing properties and channel information.
			 */
			Base::ApplicationContext &context_;

			/**
			 * @brief Pointer to the predicting FMU which manages the simulation.
			 * @details It can be assumed that the pointer is valid. It will be 
			 * initialized in the C'tor and deleted if the object's lifetime expires
			 */
			PredictingFMU * solver_;

			/**
			 * @brief A pointer to the descriptive object returned by FMI++'s
			 * ModelManager
			 * @details The Object will be managed by the ModelManager singelton 
			 * instance and must not be freed by this object. The pointer will be
			 * populated in the C'tor and references the FMU which is also
			 * encapsulated in solver_.
			 */
			ModelDescription * description_;

			/**
			 * @brief Stores the number of registered outputs
			 * @details The index corresponds to the integer value to the fmiType
			 * enumeration
			 */
			std::vector<unsigned> outputSize_;

			/** 
			 * @brief The output values associated with the currently emitted event
			 * @details If the vector is empty, current output variables havn't been 
			 * queried.
			 */
			std::vector<Timing::Event::Variable> outputEventVariables_;
			/** 
			 * @brief Flag which indicates that the outputEventVariables_ vector is
			 * populated
			 * @details If the flag is set, the outputs were fixed and the 
			 * outputEventVariables_ vector is fully populated. In this case 
			 * currentTime_ = lastPredictedEventTime_. On predicting the next event, 
			 * previously issued events get outdated and the flag will be reset.
			 */
			bool outputEventVariablesPopulated_;

			/** @brief The solver's current time */
			fmiTime currentTime_;

			/**
			 * @brief The last predicted event's time which is used for consistency
			 * checks.
			 */
			fmiTime lastPredictedEventTime_;

			/**
			 * @brief Manages the output variables and returns a reference to them
			 * @details If the output event variables arn't populated before, the
			 * solver's update function will be called and the variables will be 
			 * fetched at the given time. After calling this function it isn't
			 * possible to reset the time again.
			 * The returned reference will be valid until the next event is predicted. 
			 * The function assumes that the predictNext function was called before.
			 * If the solver isn't able to update the predicted state an
			 * Base::SolverException will be thrown
			 * @param time The output event's time
			 * @return A reference to the output variable's state
			 */
			std::vector<Timing::Event::Variable> & getOutputVariables(fmiTime time);

			/**
			 * @brief Registers the output channels based on the given channel mapping
			 * @details The function will populate outputSize_ and call the
			 * appropriate functions of the solver.
			 * If an unknown FMI type is found, a Base::SystemConfigurationException
			 * will be thrown.
			 * @param mapping The channel mapping used to fetch needed output ports
			 */
			void defineOutputs(const Base::ChannelMapping *mapping);

			/**
			 * @brief Registers the output channel for a particular FMIType
			 * @details The function will populate the outputSize_ vector and call the
			 * appropriate functions of the solver
			 * @param mapping The channel mapping used to fetch needed output channels
			 * @param type The output variable's type
			 */
			void defineOutput(const Base::ChannelMapping *mapping, FMIType type);

			/**
			 * @brief Fetches the solver's outputs at the current instant of time and 
			 * appends it to the given vectors.
			 * @details The function assumes that the given references are still 
			 * valid. FMI String types will be converted to std::string to avoid
			 * memory issues.
			 * @param values The vector of fetched variable values.
			 * @param time The last event's time
			 */
			void fetchOutputs(std::vector<Timing::Event::Variable> &values,
					fmiTime time);

		};

	}
}

#endif
