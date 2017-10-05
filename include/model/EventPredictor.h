/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file EventPredictor.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_MODEL_EVENT_PREDICTOR
#define _FMITERMINALBLOCK_MODEL_EVENT_PREDICTOR


#include "model/AbstractEventPredictor.h"
#include "model/ManagedLowLevelFMU.h"
#include "base/ChannelMapping.h"
#include "timing/Event.h"
#include "timing/EventListener.h"

// Fixes an include dependency flaw/feature(?) of ModelDescription.h
#include <common/fmi_v1.0/fmiModelTypes.h>
#include <import/base/include/ModelDescription.h>
#include <import/utility/include/IncrementalFMU.h>
#include <vector>
#include <memory>

namespace FMITerminalBlock
{
	namespace Model
	{

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

			/** @brief The name of the FMU instance name property */
			static const std::string PROP_FMU_INSTANCE_NAME;
			/** @brief The format string of the default input property */
			static const std::string PROP_DEFAULT_INPUT;

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
			virtual ~EventPredictor() {}

			/**
			 * @brief Implements 
			 * AbstractEventPredictor::configureDefaultApplicationContext(\
					Base::ApplicationContext)
			 */
			virtual void configureDefaultApplicationContext(
				Base::ApplicationContext *appContext);

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
			 * undesired behavior. In particular, it is assumed that the event handling 
			 * function is not called for any event which may be discarded later on.
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
			 * @brief Pointer to the low level FMU instance which is also used in 
			 * the solver_
			 * @details The variable is initialized at the C'tor and will remain 
			 * valid until the object is deleted.
			 */
			std::unique_ptr<ManagedLowLevelFMU> lowLevelFMU_;
			/**
			 * @brief Pointer to the predicting FMU which manages the simulation.
			 * @details It can be assumed that the pointer is valid. It will be 
			 * initialized in the C'tor and will not be reset.
			 */
			std::shared_ptr<IncrementalFMU> solver_;

			/**
			 * @brief Stores the ID of each registered output.
			 * @details The first index corresponds to the integer type of the
			 * type id and the second index corresponds to the index in the 
			 * output array.
			 */
			std::vector<std::vector<Base::PortID>> outputIDs_;

			/** 
			 * @brief The output values associated with the currently emitted event
			 * @details If the vector is empty, current output variables havn't been 
			 * queried.
			 */
			std::vector<Timing::Variable> outputEventVariables_;
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
			* @brief Stores the ID of each registered input.
			* @details The first index corresponds to the integer type of the
			* type id and the second index corresponds to the index in the
			* output array.
			*/
			std::vector<std::vector<Base::PortID>> inputIDs_;

			/**
			 * @brief vector which contains the values of all real inputs
			 * @details The vector has exactly inputIDs_[fmiTypeReal].size() elements. 
			 * The memory which holds the array must be allocated as soon as init() 
			 * is called.
			 */
			std::vector<fmiReal> realInputImage_;
			/**
			 * @brief vector which contains the values of all integer inputs
			 * @details The vector has exactly inputIDs_[fmiTypeInteger].size() elements.
			 * The memory which holds the array must be allocated as soon as init()
			 * is called.
			 */
			std::vector<fmiInteger> integerInputImage_;
			/**
			 * @brief vector which contains the values of all boolean inputs
			 * @details The vector has exactly inputIDs_[fmiTypeBoolean].size() elements.
			 * The memory which holds the array must be allocated as soon as init()
			 * is called.
			 */
			std::vector<fmiBoolean> booleanInputImage_;
			/**
			 * @brief vector which contains the values of all string inputs
			 * @details The vector has exactly inputIDs_[fmiTypeString].size() elements.
			 * The memory which holds the array must be allocated as soon as init()
			 * is called.
			 */
			std::vector<std::string> stringInputImage_;

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
			std::vector<Timing::Variable> & getOutputVariables(fmiTime time);

			/**
			 * @brief Initializes the solver
			 * @details The function assumes that all parameters are previously checked 
			 * and valid. It also assumes that the name, id and default process image 
			 * values are properly set and validated. If the solver returned with an 
			 * error, an std::runtime_exception is thrown. Please consider the FMI++ 
			 * documentation for more details on the parameters.
			 * @param instanceName The valid name of the FMU instance.
			 * @param startTime The first time instant of the simulation
			 * @param lookAheadHorizon A valid lookAheadHorizon value
			 * @param lookAheadStepSize A valid lookAheadStepSize value
			 * @param integratorStepSize A valid integrator Step size
			 */
			void initSolver(const std::string& instanceName, const fmiTime startTime,
				const fmiTime lookAheadHorizon, const fmiTime lookAheadStepSize,
				const fmiTime integratorStepSize);

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
			void defineOutput(const Base::ChannelMapping *mapping, FMIVariableType type);

			/**
			 * @brief Fetches the solver's outputs at the current instant of time and 
			 * appends it to the given vectors.
			 * @details The function assumes that the given references are still 
			 * valid. FMI String types will be converted to std::string to avoid
			 * memory issues.
			 * @param values The vector of fetched variable values.
			 * @param time The last event's time
			 */
			void fetchOutputs(std::vector<Timing::Variable> &values,
					fmiTime time);

			/**
			 * @brief Registers an input of a particular type
			 * @details The function also populates all corresponding data structures. 
			 * I.e. the input port IDs as well as the image are set to the default 
			 * value. In order to be generic enough to handle all input types, the 
			 * function defines several parameter and one generic type. It is assumed
			 * that the type of the function corresponds to the type code which is 
			 * passed in the type parameter.
			 * @param destinationImage A valid pointer to the vector in which the 
			 * default image is written.
			 * @param type The FMI type code which corresponds to the template arguments.
			 * @param defaultValue The value which is set if no default value was 
			 * presented in the configuration section.
			 * @param defineFunction The function of IncrementalFMU which is called 
			 * to register the inputs at the solver_ instance.
			 */
			template<typename InputType>
			void defineInputs(std::vector<InputType> *destinationImage, 
				FMIVariableType type, InputType defaultValue, 
				void(IncrementalFMU::*defineFunction)(const std::string *, std::size_t));

			/**
			 * @brief Registers all inputs of the event predictor.
			 * @details The function also populates all corresponding data structures.
			 */
			void defineInputs();

			/**
			 * @brief Updates the input image of a particular type.
			 * @param destinationImage The image valid vector pointer to populate
			 * @param ev The valid event pointer to query
			 * @param type The type code of the particular data type.
			 */
			template<typename InputType>
			bool updateInputImage(std::vector<InputType> *destinationImage,
				Timing::Event *ev, FMIVariableType type);

			/**
 			 * @brief Updates the input image variables
			 * @details Queries the event and checks whether an associated variable
			 * is managed as input variable. If it is managed, the input image will
			 * be updated accordingly.
			 * @param ev A valid reference to the event instance
			 * @returns <code>true</code> iff at least one variable is managed as
			 * input.
			 */
			bool updateInputImage(Timing::Event *ev);


		};

	}
}

#endif
