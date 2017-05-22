# -------------------------------------------------------------------
# Copyright (c) 2015, AIT Austrian Institute of Technology GmbH.
# All rights reserved. See file FMITerminalBlock_LICENSE for details.
# -------------------------------------------------------------------

cmake_minimum_required( VERSION 2.8.12 )

project( FMITerminalBlock )

# Include custom functions
include( cmake/CMakeAddSourceFile.txt )
include( cmake/ConfigureNetworkManager.cmake )

# add a target to generate documentation with Doxygen
# The statements were taken from FMI++
find_package( Doxygen )
if ( DOXYGEN_FOUND )
   configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile @ONLY )
   add_custom_target( FMITerminalBlock-doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generating API documentation with Doxygen" VERBATIM )
endif ()


# OS-specific compiler settings
# The if-statement was taken from FMI++
if ( WIN32 ) # windows-specific
   set( PATH_SEPARATOR ";" )
   set( CMAKE_SHARED_LIBRARY_PREFIX "" )
   set( CMAKE_SHARED_MODULE_PREFIX "" )
   add_definitions( -DWIN32 )
   if ( MINGW )
      add_definitions( -DMINGW -Wall ) # -O1
      if ( CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0" )
         set( CMAKE_CXX_FLAGS "-std=c++0x" )
      else ()
         set( CMAKE_CXX_FLAGS "-std=c++11" )
      endif ()
   endif ()
   if ( MSVC )
      # disable auto-linking and use cmake's dependency handling
      add_definitions( -DBOOST_ALL_NO_LIB /W3 )
   endif ()
else () # linux-specific
   set( PATH_SEPARATOR ":" )
   add_definitions( -Wall ) # -O1
      if ( CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0" )
         set( CMAKE_CXX_FLAGS "-std=c++0x" )
      else ()
         set( CMAKE_CXX_FLAGS "-std=c++11" )
      endif ()
endif ()

# Boost dependencies
option( Boost_USE_STATIC_LIBS "Specifies whether boost should be linked statically" ON)
find_package( Boost 1.56.0 COMPONENTS thread system filesystem date_time regex log log_setup REQUIRED )

if( NOT Boost_USE_STATIC_LIBS )
	# Workaround for boost namespace mangling. See 
	# http://www.boost.org/doc/libs/1_55_0/libs/log/doc/html/log/rationale/namespace_mangling.html
	# for more details
	option(BOOST_LOG_DYN_LINK "Specify whether boost log was compiled for dynamic linking" ON)
	if( BOOST_LOG_DYN_LINK )
		add_definitions( -DBOOST_LOG_DYN_LINK )
	endif()
endif()

# Get and check FMI++ dependencies
set( fmipp_PROJECT_DIR ../fmipp CACHE PATH "The path to FMI++'s source directory" )
get_filename_component( fmipp_PROJECT_DIR ${fmipp_PROJECT_DIR} ABSOLUTE )
if ( NOT IS_DIRECTORY ${fmipp_PROJECT_DIR} )
	message( FATAL_ERROR "${fmipp_PROJECT_DIR} is not a valid FMI++ source "
			"directory. Please set the variable fmipp_PROJECT_DIR "
			"accordingly." )
endif ()

#set( fmippim_LIBRARIES U:/Development/fmipp/build/import/Release/fmippim.lib )
add_subdirectory(${fmipp_PROJECT_DIR} fmipp)

# List of include directories
include_directories( ${FMITerminalBlock_SOURCE_DIR} )
include_directories( ${FMITerminalBlock_SOURCE_DIR}/include )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ${fmipp_PROJECT_DIR} )

# Configure Network Manager
AddNetworkManagerPublisher("CompactASN1UDPPublisher" "network/CompactASN1UDPPublisher.h")
AddNetworkManagerPublisher("CompactASN1TCPClientPublisher" "network/CompactASN1TCPClientPublisher.h")
ConfigureNetworkManager( ${CMAKE_CURRENT_BINARY_DIR}/src/network/NetworkManager.cpp )

# Declare source files per namespace
add_source_file(NETWORK src/network/ASN1Commons.cpp )
add_source_file(NETWORK src/network/CompactASN1Publisher.cpp )
add_source_file(NETWORK src/network/CompactASN1UDPPublisher.cpp )
add_source_file(NETWORK src/network/CompactASN1TCPClientPublisher.cpp )
add_source_file(NETWORK src/network/ConcurrentSubscriber.cpp )
add_source_file(NETWORK src/network/CompactASN1Subscriber.cpp )
add_source_file(NETWORK src/network/CompactASN1TCPClientSubscriber.cpp )
add_source_file(NETWORK src/network/PartialEvent.cpp )

add_source_file(BASE src/base/ApplicationContext.cpp )
add_source_file(BASE src/base/ChannelMapping.cpp )
add_source_file(BASE src/base/PortIDDrawer.cpp )
add_source_file(BASE src/base/TransmissionChannel.cpp )

add_source_file(MODEL src/model/EventPredictor.cpp )
add_source_file(MODEL src/model/LazyEvent.cpp )
add_source_file(MODEL src/model/PredictingFMU.cpp )

add_source_file(TIMING src/timing/Variable.cpp )
add_source_file(TIMING src/timing/Event.cpp )
add_source_file(TIMING src/timing/StaticEvent.cpp )
add_source_file(TIMING src/timing/EventDispatcher.cpp )
add_source_file(TIMING src/timing/TimedEventQueue.cpp )
add_source_file(TIMING src/timing/EventLogger.cpp )

# State the main executable file
add_executable(FMITerminalBlock src/main.cpp 
	${FMITerminalBlock_BASE_SOURCE} 
	${FMITerminalBlock_MODEL_SOURCE} 
	${FMITerminalBlock_TIMING_SOURCE} 
	${FMITerminalBlock_NETWORK_SOURCE}
	${CMAKE_CURRENT_BINARY_DIR}/src/network/NetworkManager.cpp )

target_link_libraries(FMITerminalBlock ${Boost_LIBRARIES} fmippim )

# Configure test cases
option(FMITerminalBlock_BUILD_TESTS 
	"Specify whether the unit tests should be generated for FMITerminalBlock" ON )

if (FMITerminalBlock_BUILD_TESTS)
	if (NOT BUILD_TESTS)
		message (FATAL_ERROR 
			"Require FMI++ test cases (BUILD_TESTS=ON) to build the FMITerminalBlock test cases")
	endif ()

	enable_testing()
	add_subdirectory(test)
	add_test( NAME ASN1Publisher COMMAND testASN1Publisher )
	add_test( NAME ASN1Subscriber COMMAND testASN1Subscriber )
	add_test( NAME NetworkManager COMMAND testNetworkManager )
	add_test( NAME EventPredictor COMMAND testEventPredictor )
	add_test( NAME EventHandling COMMAND testEventHandling )
	add_test( NAME ApplicationContext COMMAND testApplicationContext )
	add_test( NAME ChannelMapping COMMAND testChannelMapping )
endif ()


