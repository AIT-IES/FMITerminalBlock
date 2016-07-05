/* ------------------------------------------------------------------- *
 * Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.      *
 * All rights reserved. See file FMITerminalBlock_LICENSE for details. *
 * ------------------------------------------------------------------- */

/**
 * @file PredictingFMU.h
 * @author Michael Spiegel, michael.spiegel.fl@ait.ac.at
 */

#ifndef _FMITERMINALBLOCK_MODEL_PREDICTING_FMU
#define _FMITERMINALBLOCK_MODEL_PREDICTING_FMU

#include <import/utility/include/IncrementalFMU.h>

namespace FMITerminalBlock
{
	namespace Model
	{

		using namespace FMITerminalBlock;

		/**
		 * @brief Extends the incremental FMU by some convenient functions
		 * @details it maintains a copy of the time resolution variable. Since the
		 * object's features were reduced during refactoring, it may be deleted in
		 * future versions.
		 */
		class PredictingFMU: public IncrementalFMU
		{
		public:
			/**
			 * @brief C'tor passing the arguments to the super class
			 * @param fmuPath The URI of the folder which contains the extracted
			 * content of the FMU to include
			 * @param modelName The FMU's name as defined in the model description
			 */
			PredictingFMU(const std::string& fmuPath, const std::string& modelName, 
				const fmiReal timeDiffResolution = 1e-4, 
				const IntegratorType type = IntegratorType::dp);

			/**
			 * @brief C'tor passing the arguments to the super class
			 * @param xmlPath The URI of the model description file
			 * @param dllPath The URI to the model DLL or shared object
			 * @param modelName The FMU's name as defined in the model description
			 */
			PredictingFMU(const std::string& xmlPath, const std::string& dllPath, 
				const std::string& modelName, const fmiReal timeDiffResolution = 1e-4, 
				const IntegratorType type = IntegratorType::dp);

			/** @brief Frees allocated resources */
			~PredictingFMU();

			/**
			 * @brief Returns the maximum time difference of two equal time instants
			 * @return Returns the maximum time difference of two equal time instants
			 */
			fmiReal getTimeDiffResolution(void) const { return timeDiffResolution_; };

		protected:
			/** @brief The maximum time difference of two equal time instants */
			const fmiReal timeDiffResolution_;

		};

	}
}

#endif
