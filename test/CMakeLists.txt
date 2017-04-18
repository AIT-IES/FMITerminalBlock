# -------------------------------------------------------------------
# Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.
# All rights reserved. See file FMITerminalBlock_LICENSE for details.
# -------------------------------------------------------------------

cmake_minimum_required( VERSION 2.8.12 )

project( FMITerminalBlockTest )

find_package( Boost 1.56.0 
	COMPONENTS 
		thread system filesystem date_time regex log log_setup 
		unit_test_framework 
	REQUIRED )

add_definitions( -DBOOST_TEST_DYN_LINK )
add_definitions( -DFMU_URI_PRE="${FMU_URI_PRE}" )
add_definitions( -DEPS_TIME=${EPS_TIME} )
add_definitions( -DEPS_VALUE=${EPS_TIME} )

# Include common directories
include_directories( ${FMITerminalBlock_SOURCE_DIR} )
include_directories( ${FMITerminalBlock_SOURCE_DIR}/include )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ${fmipp_INCLUDE_DIRS} )

# Generate test programs
add_executable( testASN1 src/testASN1.cpp 
	${FMITerminalBlock_BASE_SOURCE} 
	${FMITerminalBlock_MODEL_SOURCE} 
	${FMITerminalBlock_TIMING_SOURCE} 
	${FMITerminalBlock_NETWORK_SOURCE} )
target_link_libraries( testASN1 PUBLIC ${Boost_LIBRARIES} fmippim )

add_executable( testEventPredictor src/testEventPredictor.cpp 
	${FMITerminalBlock_BASE_SOURCE} 
	${FMITerminalBlock_MODEL_SOURCE} 
	${FMITerminalBlock_TIMING_SOURCE} )
target_link_libraries( testEventPredictor ${Boost_LIBRARIES} fmippim )

add_executable( testEventHandling src/testEventHandling.cpp 
	${FMITerminalBlock_BASE_SOURCE} 
	${FMITerminalBlock_MODEL_SOURCE} 
	${FMITerminalBlock_TIMING_SOURCE} )
target_link_libraries( testEventHandling ${Boost_LIBRARIES} fmippim )

add_executable( testApplicationContext src/testApplicationContext.cpp 
	${FMITerminalBlock_BASE_SOURCE} )
target_link_libraries( testApplicationContext ${Boost_LIBRARIES} fmippim )

