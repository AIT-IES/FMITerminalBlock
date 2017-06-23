/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file OneStepEventPredictor.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_MODEL_ONE_STEP_EVENT_PREDICTOR
#define _FMITERMINALBLOCK_MODEL_ONE_STEP_EVENT_PREDICTOR

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include <import/base/include/FMUModelExchangeBase.h>
#include <import/base/include/ModelManager.h>

#include "model/AbstractEventPredictor.h"
#include "timing/StaticEvent.h"

namespace FMITerminalBlock
{
	namespace Model
	{

		/**
		 * @brief Implements a simulation strategy which calculates one 
		 * non-revertible step ahead.
		 * @details <p>In case an external event is triggered, the event is delayed 
		 * until the next synchronization point. The predictor supports both, 
		 * strictly periodic synchronization points and synchronization points as 
		 * soon as a model event is detected. Since the model is not reverted to a
		 * previous state, a broader range of FMI-Models can be simulated. 
		 * All input events will be delayed until the next  synchronization point.
		 * </p>
		 * <p> The event predictor holds a FMI model instance which is timed to the
		 * next synchronization point. Hence, any input event can be directly 
		 * applied to the model. From the definition of the event predictor, it 
		 * follows that every incoming event forwards the time to the time of the 
		 * issued event. Hence, predictNext() is called for every incoming event 
		 * too. To prevent any re-calculations and reset operations, an event 
		 * cache is implemented which stores the event at the next synchronization
		 * point.</p>
		 * <p>In order to reduce the network load, the event predictor checks which
		 * output variables were actually changed. Only if the output variable was 
		 * changed in the last period, it will be set in the event object.</p>
		 */
		class OneStepEventPredictor: public AbstractEventPredictor
		{
		public:

			/** @brief The name of the FMU path property */
			static const std::string PROP_FMU_PATH;
			/** @brief The name of the FMU name property */
			static const std::string PROP_FMU_NAME;
			/** @brief The name of the FMU instance name property */
			static const std::string PROP_FMU_INSTANCE_NAME;
			/** @brief The key of the default input property */
			static const std::string PROP_DEFAULT_INPUT;
			/** @brief The key of the variable step size flag */
			static const std::string PROP_VARIABLE_STEP_SIZE;

			/**
			 * @brief Constructs an uninitialized event predictor.
			 * @details The function will try to load the model but the model still
			 * stays uninitialized. The init() function must be called before the 
			 * prediction and event update functions can be used.
			 * In case a configuration error is encountered, a 
			 * Base::SystemConfigurationException is thrown.
			 * @param appContext The global configuration. It is assumed that the 
			 * given reference remains valid until the object gets destroyed. After 
			 * init() is called, the object may eventually query the channel mapping
			 * objects.
			 */
			OneStepEventPredictor(Base::ApplicationContext &appContext);

			/** @brief Frees all allocated resources */
			~OneStepEventPredictor();

			/**
			 * @brief Sets the default configuration parameters which can be derived
			 * from the previously constructed model.
			 */
			virtual void configureDefaultApplicationContext(
				Base::ApplicationContext *appContext);

			/**
			 * @brief Initializes the model
			 * @details After the model is initialized, the prediction and event 
			 * update functions are fully operational. Do not use any of these 
			 * functions unless the model is successfully initialized.
			 */
			virtual void init();
			
			/**
			 * @brief Returns the prediction of the next synchronization point.
			 * @details The prediction is based on the input variables of the current
			 * synchronization point. Any input variable update will be delayed until
			 * the next synchronization point. Hence, the prediction does not change 
			 * by any input variable update. Nevertheless, the prediction may be 
			 * queried multiple times.
			 * @return A reference to the newly created prediction. Ownership of the
			 * object is transferred to the calling instance. Hence, that instance 
			 * must delete the event object.
			 */
			virtual Timing::Event * predictNext();

			/**
			 * @brief Updates the state of the model according to the given event
			 * @details <p> In case the event is an input event, it will always be 
			 * delayed to the next synchronization point. Even if the time stamp of 
			 * the event dates beyond the last synchronization point, it will be 
			 * delayed. In case the event is an output event which was generated by 
			 * the model, predictNext will advance the model to the next 
			 * synchronization point. It is assumed that after an output event  is 
			 * received, predictNext() is immediately called. No other event may be 
			 * processed until predictNext() is executed. </p>
			 * 
			 */
			virtual void eventTriggered(Timing::Event * ev);

		private:
			/** @brief The application context which holds the configuration*/
			Base::ApplicationContext &appContext_;

			/** @brief Last values of each real output variable */
			std::vector<fmiReal> outputRealImage_;
			/** @brief Last values of each integer output variable */
			std::vector<fmiInteger> outputIntegerImage_;
			/** @brief Last values of each boolean output variable */
			std::vector<fmiBoolean> outputBooleanImage_;
			/** @brief Last values of each string output variable */
			std::vector<std::string> outputStringImage_;
			/**
			 * @brief Holds the output value reference for each output image variable
			 * @details The outer vector holds a vector of FMI references for each 
			 * fmi type. The type enum value is thereby casted to the appropriate 
			 * integer index value. It is asserted that the size of each inner vector
			 * corresponds to the size of the appropriate image vector.
			 */
			std::vector<std::vector<fmiValueReference>> outputValueReference_;

			/** @brief Holds the output mapping to query every output PortID */
			const Base::ChannelMapping * outputMapping_;

			/** 
			 * @brief Stores the value references for each input
			 * @details The map prevents additional lookup operation during the 
			 * simulation.
			 */
			std::unordered_map<Base::PortID, fmiValueReference, 
				Base::PortIDHashFunction> inputValueReference_;

			/** @brief The currently active prediction or a null pointer */
			std::unique_ptr<Timing::StaticEvent> currentPrediction_;

			/** @brief The model instance which will be managed */
			std::shared_ptr<FMUModelExchangeBase> fmu_;

			/**
			 * @brief Encapsulates several parameters which are needed at runtime.
			 * @details The structure is populated in the initialization step.
			 */
			struct
			{
				/// Maximum size of the singe look ahead step taken
				fmiReal lookAheadStepSize;
				/// Preferred size of one integrator step
				fmiReal integratorStepSize;
				/** 
				 * @brief Reduces the step size in case a model event is triggered. 
				 * @details External events will still be delayed until the next 
				 * synchronization point
				 */
				bool variableStepSizeOnModelEvent;
				/// The absolute precision to compare simulation time
				fmiTime timingPrecision;
			} simulationProperties_;

			/**
			 * @brief Initializes the output images and reference structures
			 * @details The function will not query any initial output value.
			 */
			void initOutputStructures(Base::ApplicationContext &appContext);

			/**
			 * @brief Initializes the output value references of the particular type
			 * @details The function assumes that the outputMapping_ variable is 
			 * properly initialized.
			 */
			void initOutputValueReferences(Base::ApplicationContext &appContext, 
				FMIVariableType type);

			/**
			 * @brief Initializes the inputValueReference_ variable
			 */
			void initInputValueReference(const Base::ChannelMapping *inputMapping);

			/**
			 * @brief Sets the simulation properties in the corresponding structure
			 * @details In case an invalid configuration is found, a 
			 * Base::SystemConfigurationException will be thrown.
			 */
			void initSimulationProperties(
				const Base::ApplicationContext &appContext);

			/**
			 * @brief loads a particular model
			 * @details In case a model cannot be loaded correctly, a 
			 * Base::SystemConfigurationException will be thrown. 
			 */
			static std::unique_ptr<FMUModelExchangeBase> loadModel(
				const Base::ApplicationContext &appContext);

			/**
			 * @brief Instantiates the previously loaded model
			 * @details The function assumes that the fmu_ variables was previously
			 * loaded but not instantiated. In case of an error, a 
			 * Base::SystemConfigurationException will be thrown.
			 */
			void instantiateModel(const Base::ApplicationContext &appContext);

			/** 
			 * @brief Converts the status code to a human readable error description.
			 */
			static std::string getErrorDescription(ModelManager::LoadFMUStatus err);
			/** @brief Returns a human readable string representation of the type */
			static std::string getFMUTypeString(FMUType type);

			/**
			 * @brief Sets the default input and parameter values which are 
			 * referenced in the ApplicationContext.
			 * @details The function assumes that the FMU is correctly loaded and 
			 * instantiated. In case an error is detected, a 
			 * Base::SystemConfigurationException will be thrown.
			 * @param appContext The configuration source to take the default 
			 * values from.
			 */
			void setDefaultValues(const Base::ApplicationContext &appContext);
			/**
			 * @brief Sets the default value of the referenced variable.
			 * @details The default value is taken from the application context. In 
			 * case an error is detected, a Base::SystemConfigurationException will 
			 * be thrown.
			 * @param appContext The configuration to query
			 * @praam varName The name of the variable to initialize
			 */
			void setDefaultValue(const Base::ApplicationContext &appContext, 
				const std::string &varName);

			/**
			 * @brief Advances the FMU by exactly one step.
			 * @details The function does not set any in- or outputs. The exact 
			 * behavior depends on the simulation properties. In case 
			 * variableStepSizeOnModelEvent is set, the function will return as 
			 * soon as an event is detected.
			 */
			void predictOneStep();

			/**
			 * @brief Fetches the model outputs of a particular type and updates the
			 * given image vector accordingly
			 * @return <code>true</code> iff the destinationImage was changed
			 * @param valType The type of each variable
			 * @param destinationImage the image vector to update. It is assumed that
			 * the given pointer is always valid.
			 * @param referenceVector The vector of FMI value references which is 
			 * used to fetch the variables of a particular type. The parameter should
			 * be constant but FMI++ prevents it from being constant.
			 */
			template<typename valType>
			bool updateOutputImage(std::vector<valType> *destinationImage, 
				std::vector<fmiValueReference> &referenceVector);

			/**
			 * @brief Queries all outputs and updates the image vectors accordingly.
			 * @returns <code>true</code> iff an image vector was changed
			 */
			bool updateOutputImage();

			/**
			 * @brief Constructs an event from the output images and the current time
			 * of the FMU
			 * @return The newly constructed event
			 */
			std::unique_ptr<Timing::StaticEvent> getOutputEvent();

			/**
			 * @brief Appends the output values of a particular type to the given 
			 * variable vector
			 * @param destination A valid pointer to the destination which will 
			 * contain the appended variables.
			 * @param ids A vector with all PortIDs of the particular variable type
			 * @param values The vector with all variable values. It must have the 
			 * same size as the ids vector. Each element must match the appropriate 
			 * element in the ids vector.
			 * @param valType The template parameter specifies the data type of the 
			 * variable and the data to append
			 */
			template<typename valType>
			void appendOutputVariables(std::vector<Timing::Variable> *destination, 
				const std::vector<Base::PortID> &ids, 
				const std::vector<valType> &values) const;

			/**
			 * @brief Sets the input variables of the event at the managed FMU
			 * @details It is assumed that the FMU as well as inputValueReference is 
			 * properly initialized. The function will not check or set the time of 
			 * the event nor the FMU.
			 * @return <code>true</code> iff at leas one input variable was set. In 
			 * case no input variable is set, the event is most likely an output 
			 * event triggered by the predictor itself.
			 */
			bool updateInputVariables(Timing::Event *ev);

			/**
			  * @brief Tries to update the given input variable
				* @param variable The variable to check. The given reference may point 
				* to an arbitrary variable. In case the variable is not an input 
				* variable <code>false</code> will be returned. Otherwise 
				* <code>true</code> is returned and the model is updated accordingly.
				* @return <code>true</code> if the variable is a known input variable.
			  */
			bool updateInputVariable(const Timing::Variable &variable);

		};
	}
}
#endif
