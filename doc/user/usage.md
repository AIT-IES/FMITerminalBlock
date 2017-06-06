FMITerminalBlock Usage
=======================

FMITerminalBlock provides a very simple command line interface. The tool will be configured by passing options via command line arguments. Since often several options are required to configure the connections, it is advised to place the program invocation in a batch or shell script. Future improvements regarding usability of the interface are planned but current development focus is put on methodological aspects.

The configuration will be checked before the actual simulation run is started. Hence, the number of runtime errors is reduced to a minimum. In case errors are encountered, FMITerminalBlock will print an error message and will terminate. Additionally, FMITerminalBlock implements sensitive default values for several parameters. Please refer to the following parameter documentation for further information. Each parameter is passed on to FMITerminalBlock in the format ```<key>.<subkey>.<...>=<value>```. In case a value contains spaces, the whole argument including the key part must be quoted according to the requirements of the used shell.

To give a first impression, the following invocation shows a simulation run which executes an FMU called "Ramp":
```sh
./FMITerminalBlock.exe \
	"fmu.path=file:/C:/My Unzipped FMUs/Ramp.fmu.dir" \
	fmu.name=Ramp \
	app.lookAheadTime=8.0 \
	in.0.protocol=CompactASN.1-TCP \
	in.0.addr=localhost:1500 \
	in.0.0=slope \
	in.0.0.type=0 \
	out.0.protocol=CompactASN.1-TCP \
	out.0.addr=localhost:1499 \
	out.0.0=value \
	out.0.0.type=0 \
	out.0.1=top \
	out.0.1.type=0
```

## Mandatory Parameters
The following parameters must be passed on to FMITerminalBlock:

**fmu.path**: An *URL* to a local directory which holds the content of the FMU. Each .fmu file is a zip compressed archive which contains all files to run the simulation. FMITerminalBlock is currently not able to decompress the fmu itself. Hence, the location of directory which holds all extracted files and subdirectories needs to be specified.

**fmu.name**: The actual name of the FMU as defined in the model description. Usually, the name is specified during the export procedure. Future versions may directly query the name from the model description file.

**app.lookAheadTime**: The size of the lookahead horizon in seconds. If no event is triggered by the model until the lookAheadTime is reached, an event which outputs the current outputs of the model will be triggered by FMITerminalBlock.

## Network Channel Parameters
The network connections of FMITerminalBlock are organized into input and output channels. Each channel connects to a certain destination and uses a certain protocol. For instance, the first channel may connect to one PLC using UDP and the second channel may connect to another PLC using TCP. Each channel contains a nonempty list of model variables which should be sent or received. A model variable within a channel is also called (network-) port. Each channel and each port within a channel is configured by an index starting from zero. In the following, *-nr-* refers to the index number used in the configuration. FMITerminalBlock determines the number of channels and ports by assuming consecutive index numbers. Whenever a gap in the numbering is encountered, following indexes will be ignored.

**in.-nr-.protocol** and **out.-nr-.protocol**: A string which specifies the protocol to be used to send and receive data. Currently, the following protocols are supported:
* *CompactASN.1-TCP*: A TCP client which connects to a server and encodes the data according to the CompactASN.1 format.
* *CompactASN.1-UDP*: Currently only for output channels. Encapsulates the data in UDP packets

**in.-nr-.addr** and **out.-nr-.addr**: The address of the remote end point to connect to. CompactASN.1 protocols expect an address format according following the ```<hostname>:<port>``` scheme. For instance, ```localhost:1499``` Connects to a local PLC on port 1499.

**in.-nr-.-nr-** and **out.-nr-.-nr-**: Specifies the name of the FMI model variable of a particular port. In case the channel is an input channel, values which are received from the connected device will trigger an event and update the inputs of the model. Likewise, output channels send out information as soon as an event is triggered.

**in.-nr-.-nr-.type** and **out.-nr-.-nr-.type**: Specifies the FMI type id of the output. Currently, the parameter is required to match the variable name. It may be automatically determined from the model description file in future versions. The following type ids are supported:
* *0*: FMI Real typed variable
* *1*: FMI Integer typed variable
* *2*: FMI Boolean typed variable
* *3*: FMI String typed variable

Input channels which implement the CompactASN.1 protocol will convert received IEC 61499 types (REAL, LREAL, DINT, ...) to FMI types in a best effort approach. For instance a received integer values will be converted to a numerical string representation if the model expects a string. CompactASN.1 outputs chose the default IEC 61499 type according to the FMI type of the variable. A corresponding IEC 61499 type which is capable of representing the content of the FMI variable without loss of information is chosen. An alternative encoding may be specified with the optional **out.-nr-.-nr-.encoding** property. The values of the property correspond to the names (in uppercase) of the IEC 61499 types.

## Optional Parameters
The operation of the solver and the prediction logic may be adjusted by the following parameters:

**app.lookAheadStepSize**: An internal parameter which specifies the time until a predicted state is saved. The smaller the look ahead step size is chosen, the more accurate a state can be interpolated. Per default, a value of app.lookAheadTime/10 is chosen.

**app.integratorStepSize**: The time interval of a single integrator step. The default value is app.lookAheadStepSize/10.

**app.startTime**: The initial time of the simulation. The value defaults to 0.0 s

**app.stopTime**: The time until the simulation is performed. The program will terminate after the first event which exceeds the stop time is triggered. Hence, the actual simulation may run longer as *app.stopTime* seconds.

**app.timingFile**: If the parameter is set, a timing file will be written which may be used to analyze the performance of FMITerminalBlock and its latencies.

