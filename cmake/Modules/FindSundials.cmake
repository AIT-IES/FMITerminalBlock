# -------------------------------------------------------------------
# Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.
# All rights reserved. See file FMITerminalBlock_LICENSE for details.
# -------------------------------------------------------------------

# Try to find the sundials libraries
#
# The file will try to find the Sundials installation. It therefore uses the 
# following options:
#   - SUNDIALS_INCLUDEDIR: The include directory of the Sundials installation
#   - SUNDIALS_LIBRARYDIR: The Library directory which contains all the shared 
#                          binary files.
#   - SUNDIALS_BASE: The base directory of the sundials installation.
# 
# Once done, the following variables will be defined:
#   - SUNDIALS_FOUND: All necessary files were successfully located.
#   - SUNDIALS_INCLUDE_DIR
#   - SUNDIALS_LIBRARIES: The linked libraries which are needed to use Sundials
#   - SUNDIALS_SHARED_FOUND: All shared libraries were found. In case Sundials 
#                            was compiled for static linkage and no dll/so files
#                            are present, the variable is set to false
#   - SUNDIALS_SHARED_LIBRARIES: All Shared library files which are needed to 
#                                execute the binaries.
#
# Author: Michael Spiegel <michael.spiegel@ait.ac.at>
#
# Parts are taken from:
#   - https://cmake.org/Wiki/CMake:How_To_Find_Libraries
#   - https://stackoverflow.com/questions/17225121/how-to-use-external-dlls-in-cmake-project

# Save old library suffixes
set( OLD_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES} )

find_path(SUNDIALS_INCLUDE_DIR 
			NAMES cvode/cvode.h 
			HINTS ${SUNDIALS_INCLUDEDIR} ${SUNDIALS_BASE}/include
			)

# Tries to find the sundials library libname and stores the result in variable
macro(find_sundials_library variable libname)
	find_library(${variable}
			NAMES ${libname}
			HINTS ${SUNDIALS_LIBRARYDIR} ${SUNDIALS_BASE}/lib
			)
endmacro()

# --- Find static libraries ---
if(WIN32)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")  
else()
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".so")
endif()

find_sundials_library(SUNDIALS_CVODE_LIBRARY sundials_cvode)
find_sundials_library(SUNDIALS_NVECSERIAL_LIBRARY sundials_nvecserial)

# --- Find shared libraries ---
if(WIN32)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")  
else()
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".so")
endif()

find_sundials_library(SUNDIALS_CVODE_SHARED_LIBRARY sundials_cvode)
find_sundials_library(SUNDIALS_NVECSERIAL_SHARED_LIBRARY sundials_nvecserial)

# --- Register Results ---
include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set PAHO_MQTT_C_FOUND to TRUE
# if all listed variables are TRUE

find_package_handle_standard_args(SUNDIALS DEFAULT_MSG
			SUNDIALS_INCLUDE_DIR 
			SUNDIALS_CVODE_LIBRARY SUNDIALS_NVECSERIAL_LIBRARY )

# Check whether the shared libraries could be found
set(SUNDIALS_SHARED_FOUND 
	$<BOOL:SUNDIALS_CVODE_SHARED_LIBRARY AND SUNDIALS_NVECSERIAL_SHARED_LIBRARY>
	)

			
mark_as_advanced(SUNDIALS_INCLUDE_DIR 
			SUNDIALS_CVODE_LIBRARY SUNDIALS_NVECSERIAL_LIBRARY
			SUNDIALS_CVODE_SHARED_LIBRARY SUNDIALS_NVECSERIAL_SHARED_LIBRARY)

set(SUNDIALS_INCLUDE_DIRS ${SUNDIALS_INCLUDE_DIR})
set(SUNDIALS_LIBRARIES ${SUNDIALS_CVODE_LIBRARY} ${SUNDIALS_NVECSERIAL_LIBRARY})
set(SUNDIALS_SHARED_LIBRARIES 
		${SUNDIALS_CVODE_SHARED_LIBRARY} ${SUNDIALS_NVECSERIAL_SHARED_LIBRARY})

# Restore library suffixes
set( CMAKE_FIND_LIBRARY_SUFFIXES ${OLD_CMAKE_FIND_LIBRARY_SUFFIXES} )
unset( OLD_CMAKE_FIND_LIBRARY_SUFFIXES )
