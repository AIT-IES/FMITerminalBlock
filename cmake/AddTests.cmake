# -------------------------------------------------------------------
# Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.
# All rights reserved. See file FMITerminalBlock_LICENSE for details.
# -------------------------------------------------------------------

# Defines the target which compiles the given test case. the test case name is
# given without a leading "test" string. That prefix will be appended to 
# define the target. It is assumed that all test cases require the boost 
# libraries in Boost_LIBRARIES, fmipp and the local Threads libraries. In case 
# the option FMITerminalBlockTest_XML_FILE_OUTPUT is set, the output of the XML
# files will be redirected to a dedicated output directory in an XML format.
#
# test_name: The name of the test set
# others: All arguments after the last named one list the source files which 
# are necessary to compile the test set.
macro(add_test_target test_name)

	# Generate the command line option for the test target
	if(FMITerminalBlockTest_XML_FILE_OUTPUT)
		get_filename_component( test_output_directory 
			"${CMAKE_BINARY_DIR}/Testing/DetailedOutput/" ABSOLUTE )
		file( MAKE_DIRECTORY ${test_output_directory} )
		set( test_args --log_format=XML --log_level=test_suite 
			--report_format=XML 
			"--report_sink=${test_output_directory}/${test_name}.report.xml"
			"--log_sink=${test_output_directory}/${test_name}.log.xml" )
	else()
		set( test_args "" )
	endif()

	# Create the test target
    add_executable( test${test_name} ${ARGN} )
    target_link_libraries( test${test_name} ${Boost_LIBRARIES} fmippim 
	                                        ${CMAKE_THREAD_LIBS_INIT} )
	set_default_compiler_settings( test${test_name} )
	add_custom_command( 
		TARGET test${test_name} POST_BUILD 
		COMMAND ${CMAKE_COMMAND} -E copy_if_different 
			$<TARGET_FILE:fmippim> $<TARGET_FILE_DIR:test${test_name}> 
		COMMENT "Make fmippim available" VERBATIM)
    add_test( NAME ${test_name} COMMAND test${test_name} 
		${test_args} )
endmacro()
