# -------------------------------------------------------------------
# Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.
# All rights reserved. See file FMITerminalBlock_LICENSE for details.
# -------------------------------------------------------------------

# Sets the default compiler settings for the given target. The function uses 
# target compile features. Hence, it is necessary to invoke the function for 
# each target separately.
macro(set_default_compiler_settings target)
    target_compile_features(${target} PRIVATE cxx_auto_type)
    target_compile_features(${target} PRIVATE cxx_lambdas)
endmacro ()
