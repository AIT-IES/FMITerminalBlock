# -------------------------------------------------------------------
# Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.
# All rights reserved. See file FMITerminalBlock_LICENSE for details.
# -------------------------------------------------------------------

# Defines the target which compiles the given test case. the test case name is
# given without a leading "test" string. That prefix will be appended to 
# define the target. It is assumed that all test cases require the boost 
# libraries in Boost_LIBRARIES and fmipp
#
# test_name: The name of the test set
# others: All arguments after the last named one list the source files which 
# are necessary to compile the test set.
macro(add_test_target test_name)
    add_executable( test${test_name} ${ARGN} )
    target_link_libraries( test${test_name} PUBLIC ${Boost_LIBRARIES} fmippim )
    add_test( NAME ${test_name} COMMAND test${test_name} )
endmacro()
