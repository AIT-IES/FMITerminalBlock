# -------------------------------------------------------------------
# Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.
# All rights reserved. See file FMITerminalBlock_LICENSE for details.
# -------------------------------------------------------------------

# Generates the NetworkManager source file and writes it to outputFile. The 
# functions ResetNetworkManagerConfig, AddNetworkManagerPublisher, and 
# AddNetworkManagerSubscriber may be used to adjust the configuration of the 
# network manager.
# 
# outputFile     The destination of the configured file.
#
# The function expects the following variables to be populated:
# - FMITerminalBlock_NetworkManager_SUBSCRIBERS:
#   The code snipped which selects the subscriber instances
# - FMITerminalBlock_NetworkManager_PUBLISHER:
#   The code snipped which selects the publisher instances
# - FMITerminalBlock_NetworkManager_INCLUDES
#   The code snipped which contains the include directives
function( ConfigureNetworkManager outputFile )
	configure_file(
		${FMITerminalBlock_SOURCE_DIR}/src/network/NetworkManager.cpp.in
		${outputFile} )
endfunction ()


# Resets any configuration of the NetworkManager
function( ResetNetworkManagerConfig )
	set(FMITerminalBlock_NetworkManager_SUBSCRIBERS "" PARENT_SCOPE)
	set(FMITerminalBlock_NetworkManager_PUBLISHER "" PARENT_SCOPE)
	set(FMITerminalBlock_NetworkManager_INCLUDES "" PARENT_SCOPE)
endfunction ()

# Registers a publisher type for the network manager.
# name         The name including all namesapces
# includefile  The relative path to the includefile
function( AddNetworkManagerPublisher name includefile)
	set(FMITerminalBlock_NetworkManager_PUBLISHER 
	# -------------------->8--------------------------
	"${FMITerminalBlock_NetworkManager_PUBLISHER} \n\
	if(id == ${name}::PUBLISHER_ID) \n\
	{ \n\
		return std::make_shared<${name}>(); \n\
	}else " 
	#-------------------------------------------------
		PARENT_SCOPE)
	set(FMITerminalBlock_NetworkManager_INCLUDES
		"${FMITerminalBlock_NetworkManager_INCLUDES}\n\#include \"${includefile}\""
		PARENT_SCOPE)
endfunction ()

# Registers a subscriber type for the network manager.
# name         The name including all namesapces
# includefile  The relative path to the includefile
function( AddNetworkManagerSubscriber name includefile)
	set(FMITerminalBlock_NetworkManager_SUBSCRIBERS 
	# -------------------->8--------------------------
	"${FMITerminalBlock_NetworkManager_SUBSCRIBERS} \n\
	if(id == ${name}::SUBSCRIBER_ID) \n\
	{ \n\
		return std::make_shared<${name}>(); \n\
	}else " 
	#-------------------------------------------------
		PARENT_SCOPE)
	set(FMITerminalBlock_NetworkManager_INCLUDES
		"${FMITerminalBlock_NetworkManager_INCLUDES}\n\#include \"${includefile}\""
		PARENT_SCOPE)
endfunction ()
