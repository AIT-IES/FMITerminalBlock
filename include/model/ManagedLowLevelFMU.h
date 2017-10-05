/* ------------------------------------------------------------------- *
 * Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file ManagedLowLevelFMU.h
 * @author Michael Spiegel, michael.spiegel@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_MODEL_MANAGED_LOW_LEVEL_FMU
#define _FMITERMINALBLOCK_MODEL_MANAGED_LOW_LEVEL_FMU

#include <string>
#include <memory>

#include <common/FMUType.h>

#include "base/ApplicationContext.h"

namespace FMITerminalBlock
{
	namespace Model
	{

		/**
		 * @brief Identifies a single FMU in the shared ModelManager (FMI++) object
		 * @details The class manages common tasks such as loading the FMU and 
		 * parsing the application context. It stores the model identification and 
		 * provides some basic information regarding the model. It also ensures 
		 * that the ModelManager may not remove the BareFMU by holding a shared 
		 * pointer instance. In order to unload the FMU at the model manager, the 
		 * object first has to be deleted. 
		 */
		class ManagedLowLevelFMU
		{
		public:
			/** @brief The name of the FMU path property */
			static const std::string PROP_FMU_PATH;
			/** @brief The name of the FMU name property */
			static const std::string PROP_FMU_NAME;

			/**
			 * @brief Creates a new instance and loads the FMU
			 * @details In case an error is encountered, a std::invalid_argument will
			 * be thrown. After the C'tor returns, the BareFMU may be accessed via 
			 * the given name.
			 * @param context The context object to read the user configuration. It
			 * will be queried to obtain the path of the URL and optionally the name 
			 * of it.
			 */
			ManagedLowLevelFMU(const Base::ApplicationContext &context);

			/**
			 * @brief Frees allocated resources but does not unload the FMU
			 * @details After the object is deleted, the FMU may be unloaded. For 
			 * performance reasons and to avoid spurious unload operations, the 
			 * function will not unload the FMU in the FMI++ ModelManager 
			 * automatically. 
			 */
			~ManagedLowLevelFMU() {}


			/**
			 * @brief Returns the model identifier of the FMU
			 * @details The model identifier may be used to query the corresponding 
			 * BareFMU. It will remain constant as long as the object exists.
			 * @return The unique identifier of the FMU.
			 */
			std::string getModelIdentifier() const { return modelIdentifier_; }

			/**
			 * @brief Returns the path URL of the FMU
			 * @details The path may be mainly used for debugging purpose. It will 
			 * not change during the execution either.
			 * @return The URL of the FMU directory
			 */
			std::string getPath() const { return path_; }

			/**
			 * @brief Returns the type code of the FMU
			 * @details The type code may be required to construct the appropriate 
			 * FMU object manually. It will also remain constant during the existence
			 * of the object.
			 * @return The type code of the FMU
			 */
			FMUType getType() const { return type_; }

		private:
			/** 
			 * @brief The FMUs model identifier
			 * @details The identifier must not be altered after initialization.
			 */
			std::string modelIdentifier_;

			/** 
			 * @brief The path URL of the FMU directory
			 * @details The identifier must not be altered after initialization.
			 */
			std::string path_;

			/** 
			 * @brief The type code of the FMU
			 * @details The identifier must not be altered after initialization.
			 */
			FMUType type_;

			/** 
			 * @brief Pointer to the BareFMU object.
			 * @details The pointer is used to avoid removal of the BareFMU object 
			 * from the FMI++ ModelManager. A void pointer is used since BareFMUs 
			 * currently lack a common base class.
			 */
			std::shared_ptr<void> fmuLock_;

			/**
			 * Initializes the modelIdentifier_, path_, and type_ and loads the FMU
			 */
			void initVarsAndLoadFMU(const Base::ApplicationContext &context);

			/**
			 * @brief Queries the correct bare FMU and populates the fmuLock_ object
			 * @details It is assumed that the type_, modelIdentifier, and path_ 
			 * object are correctly initialized and that the FMU has been loaded 
			 * before.
			 */
			void lockFMU();
		};
	}
}

#endif
