# -------------------------------------------------------------------
# Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.
# All rights reserved. See file FMITerminalBlock_LICENSE for details.
# -------------------------------------------------------------------

# Copies the dynamic libraries to the target file directory
#   target - The build target which requires the libraries
#   ARGN - (All the other arguments) - A list of source library file paths
#
# The variable DO_NOT_COPY_DYNAMIC_LIBRARIES may be defined to suppress the copy
# operation.
function(copy_dynamic_library target)

  if(NOT DO_NOT_COPY_DYNAMIC_LIBRARIES)
    foreach(library ${ARGN})
      add_custom_command(TARGET ${target} POST_BUILD 
                         COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                                 ${library}  $<TARGET_FILE_DIR:${target}> 
                         COMMENT "Make ${library} available" VERBATIM)
    endforeach()
  endif()
endfunction()
