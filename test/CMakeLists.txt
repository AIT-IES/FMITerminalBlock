# -------------------------------------------------------------------
# Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.
# All rights reserved. See file FMITerminalBlock_LICENSE for details.
# -------------------------------------------------------------------

cmake_minimum_required( VERSION 3.1.0 )

project( FMITerminalBlockTest )

option( FMITerminalBlockTest_XML_FILE_OUTPUT 
     "Specifies whether to output all test results in an XML format." OFF)

find_package( Boost 1.61.0 
	COMPONENTS 
		thread system filesystem date_time regex log log_setup 
		unit_test_framework 
	REQUIRED )

add_definitions( -DBOOST_TEST_DYN_LINK )
add_definitions( -DFMU_URI_PRE="${FMU_URI_PRE}" )
add_definitions( -DEPS_TIME=${EPS_TIME} )
add_definitions( -DEPS_VALUE=${EPS_TIME} )

if( INCLUDE_SUNDIALS )
	add_definitions( -DUSE_SUNDIALS )
endif()

# Include common directories
include_directories( ${FMITerminalBlockTest_SOURCE_DIR}/include )
include_directories( ${FMITerminalBlock_SOURCE_DIR} )
include_directories( ${FMITerminalBlock_SOURCE_DIR}/include )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ${fmipp_INCLUDE_DIRS} )

# Generate test programs
add_test_target( ASN1Publisher src/testASN1Publisher.cpp 
	$<TARGET_OBJECTS:FMITerminalBlock_BASE_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_MODEL_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_TIMING_OBJ>
	$<TARGET_OBJECTS:FMITerminalBlock_NETWORK_OBJ> )

add_test_target( ASN1Subscriber src/testASN1Subscriber.cpp 
	$<TARGET_OBJECTS:FMITerminalBlock_BASE_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_MODEL_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_TIMING_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_NETWORK_OBJ> 
	${CMAKE_CURRENT_SOURCE_DIR}/src/RawTCPServerTestDataSource.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/src/RawTestData.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/src/RawTestDataCollection.cpp )

ResetNetworkManagerConfig()
AddNetworkManagerSubscriber(
	"FMITerminalBlockTest::Network::ConcurrentMockupSubscriber"
	"ConcurrentMockupSubscriber.h")
AddNetworkManagerPublisher(
	"FMITerminalBlockTest::Network::MockupPublisher"
	"MockupPublisher.h")

ConfigureNetworkManager( ${CMAKE_CURRENT_BINARY_DIR}/src/network/NetworkManager.cpp )
add_test_target( NetworkManager src/testNetworkManager.cpp
	${CMAKE_CURRENT_BINARY_DIR}/src/network/NetworkManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ConcurrentMockupSubscriber.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MockupPublisher.cpp
	$<TARGET_OBJECTS:FMITerminalBlock_BASE_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_MODEL_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_TIMING_OBJ>
	$<TARGET_OBJECTS:FMITerminalBlock_NETWORK_OBJ> )

add_test_target( EventPredictor src/testEventPredictor.cpp 
	$<TARGET_OBJECTS:FMITerminalBlock_BASE_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_MODEL_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_TIMING_OBJ> )
	
add_test_target( OneStepEventPredictor src/testOneStepEventPredictor.cpp 
	$<TARGET_OBJECTS:FMITerminalBlock_BASE_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_MODEL_OBJ>
	$<TARGET_OBJECTS:FMITerminalBlock_TIMING_OBJ> )

add_test_target( SolverConfiguration src/testSolverConfiguration.cpp 
	$<TARGET_OBJECTS:FMITerminalBlock_BASE_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_MODEL_OBJ>
	$<TARGET_OBJECTS:FMITerminalBlock_TIMING_OBJ> )
	
add_test_target( EventPredictorFactory src/testEventPredictorFactory.cpp 
	$<TARGET_OBJECTS:FMITerminalBlock_BASE_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_MODEL_OBJ>
	$<TARGET_OBJECTS:FMITerminalBlock_TIMING_OBJ> )

add_test_target( ManagedLowLevelFMU src/testManagedLowLevelFMU.cpp 
	$<TARGET_OBJECTS:FMITerminalBlock_BASE_OBJ>
	$<TARGET_OBJECTS:FMITerminalBlock_MODEL_OBJ>
	$<TARGET_OBJECTS:FMITerminalBlock_TIMING_OBJ> )
	
add_test_target( EventHandling src/testEventHandling.cpp 
	$<TARGET_OBJECTS:FMITerminalBlock_BASE_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_MODEL_OBJ>
	$<TARGET_OBJECTS:FMITerminalBlock_TIMING_OBJ> )
	
add_test_target( ApplicationContext src/testApplicationContext.cpp 
	$<TARGET_OBJECTS:FMITerminalBlock_BASE_OBJ> )

add_test_target( ChannelMapping src/testChannelMapping.cpp 
	$<TARGET_OBJECTS:FMITerminalBlock_BASE_OBJ> )

add_test_target( CSVDataLogger src/testCSVDataLogger.cpp 
	$<TARGET_OBJECTS:FMITerminalBlock_BASE_OBJ> 
	$<TARGET_OBJECTS:FMITerminalBlock_TIMING_OBJ> )
