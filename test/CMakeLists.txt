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
include_directories( ${FMITerminalBlockTest_SOURCE_DIR}/include )
include_directories( ${FMITerminalBlock_SOURCE_DIR} )
include_directories( ${FMITerminalBlock_SOURCE_DIR}/include )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ${fmipp_INCLUDE_DIRS} )

# Generate test programs
add_executable( testASN1Publisher src/testASN1Publisher.cpp 
	${FMITerminalBlock_BASE_SOURCE} 
	${FMITerminalBlock_MODEL_SOURCE} 
	${FMITerminalBlock_TIMING_SOURCE} 
	${FMITerminalBlock_NETWORK_SOURCE} )
target_link_libraries( testASN1Publisher PUBLIC ${Boost_LIBRARIES} fmippim )

add_executable( testASN1Subscriber src/testASN1Subscriber.cpp 
	${FMITerminalBlock_BASE_SOURCE} 
	${FMITerminalBlock_MODEL_SOURCE} 
	${FMITerminalBlock_TIMING_SOURCE} 
	${FMITerminalBlock_NETWORK_SOURCE} 
	${CMAKE_CURRENT_SOURCE_DIR}/src/RawTCPServerTestDataSource.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/src/RawTestData.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/src/RawTestDataCollection.cpp )
target_link_libraries( testASN1Subscriber PUBLIC ${Boost_LIBRARIES} fmippim )


ResetNetworkManagerConfig()
AddNetworkManagerSubscriber(
	"FMITerminalBlockTest::Network::ConcurrentMockupSubscriber"
	"ConcurrentMockupSubscriber.h")
AddNetworkManagerPublisher(
	"FMITerminalBlockTest::Network::MockupPublisher"
	"MockupPublisher.h")

ConfigureNetworkManager( ${CMAKE_CURRENT_BINARY_DIR}/src/network/NetworkManager.cpp )
add_executable( testNetworkManager src/testNetworkManager.cpp
	${CMAKE_CURRENT_BINARY_DIR}/src/network/NetworkManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ConcurrentMockupSubscriber.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/MockupPublisher.cpp
	${FMITerminalBlock_BASE_SOURCE} 
	${FMITerminalBlock_MODEL_SOURCE} 
	${FMITerminalBlock_TIMING_SOURCE} 
	${FMITerminalBlock_NETWORK_SOURCE} )
target_link_libraries( testNetworkManager PUBLIC ${Boost_LIBRARIES} fmippim )

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

add_executable( testChannelMapping src/testChannelMapping.cpp 
	${FMITerminalBlock_BASE_SOURCE} )
target_link_libraries( testChannelMapping ${Boost_LIBRARIES} fmippim )