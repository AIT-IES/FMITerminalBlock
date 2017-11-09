FMITerminalBlock Usage
=======================

FMITerminalBlock provides a very simple command line interface. The tool will be configured by passing options via command line arguments. Since often several options are required to configure the connections, it is advised to place the program invocation in a batch or shell script. Future improvements regarding usability of the interface are planned but current development focus is put on methodological aspects.

The configuration will be checked before the actual simulation run is started. Hence, the number of runtime errors is reduced to a minimum. In case errors are encountered, FMITerminalBlock will print an error message and will terminate. Additionally, FMITerminalBlock implements sensitive default values for several parameters. Please refer to the following parameter documentation for further information. Each parameter is passed on to FMITerminalBlock in the format ```<key>.<subkey>.<...>=<value>```. In case a value contains spaces, the whole argument including the key part must be quoted according to the requirements of the used shell.

To give a first impression, the following invocation shows a simulation run which executes an FMU called "Ramp":
```sh
./FMITerminalBlock.exe \
	"fmu.path=file:/C:/My Unzipped FMUs/Ramp.fmu.dir" \
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

**app.lookAheadTime**: The size of the lookahead horizon in seconds. In general, FMITerminalBlock executes the model until the *app.lookAheadTime* is reached. Nevertheless, the exact behavior of FMITerminalBlock depends on the configured simulation mode. See the [Simulation Method](#simulation-method-specific-parameters) section for more details.

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

### IEC 61499 ASN.1 Specifics
Input channels which implement the CompactASN.1 protocol will convert received IEC 61499 types (REAL, LREAL, DINT, ...) to FMI types in a best effort approach. For instance a received integer values will be converted to a numerical string representation if the model expects a string. CompactASN.1 outputs choose the default IEC 61499 type according to the FMI type of the variable. A corresponding IEC 61499 type which is capable of representing the content of the FMI variable without loss of information is chosen. In particular, the following sensitive default values apply.

| FMI Type | IEC 61499 Default Type |
|----------|------------------------|
| Real     | LREAL                  |
| Integer  | DINT                   |
| Boolean  | BOOL                   |
| String   | STRING                 |

An alternative output encoding may be specified with the optional **out.-nr-.-nr-.encoding** property. The values of the property correspond to the names (in uppercase) of the IEC 61499 types.

The order of encoded model variable corresponds to the number of the network port. Port number 0 is sent or received first, followed by port number 1 and so on. Each output event is sent in a single packet which holds all output ports in the particular order. Input packets may be split into several packets but the total order of network ports must remain. I.e. Although the first and the second input network port may be sent in different packets, they must not be received in reversed order. Please note that while TCP guarantees the condition, UDP may not. (UDP receivers are currently unsupported anyway. If you need UDP support for receiving, please open an issue.)

## Simulation Method Specific Parameters

FMITerminalBlock supports multiple modes of operation. Each mode implements a different simulation method. Please note that due to some restrictions in the FMI 1.0 specification and possibly reduced capabilities of the included FMU, not all modes of operation lead to reliable results. The mode of operation is set with the optional **app.simulationMethod** parameter. Currently FMITerminalBlock supports two simulation modes, *multistep-prediction* which is the default value and *singlestep-delayed*.

### Multistep Prediction (Default)

In multistep prediction mode, FMITerminalBlock projects the result several steps (lookahead steps) ahead until the end of the prediction horizon is reached or an event is triggered by the model. The state after each lookahead step is saved to enable FMITerminalBlock to easily revert some predictions to an arbitrary time within the lookahead horizon. If no event is triggered by the model until the lookAheadTime is reached, an event which outputs the current outputs of the model will be triggered by FMITerminalBlock. In case an external event is detected, the state of the model is reverted to the time of the external event. Hence, the external event can be applied at the correct point in simulation time and it does not have to be artificially delayed. As soon as the external event is successfully applied, the prediction starts anew.

The multistep prediction requires the FMU to be able to completely reset its state to a previous one. Some FMUs are known to be incompatible with the multistep prediction method. For instance, some delay blocks do not expose all internal states and produce invalid results in case they are reseted. Please consider using the [Singlestep Delayed Operation](#singlestep-delayed-operation), in case multistep prediction fails to produce correct results.

The simulation is controlled by the following parameters:

**app.lookAheadTime**: The mandatory parameter specifies the maximum time in which preliminary results are computed. In case no model event is detected, an event is triggered at *app.lookAheadTime* from the previous event. The event will contain all exposed model variables. Please note that continuous changes of real-typed outputs do not trigger an event. In case they need to passed regularly to connected devices, the look ahead time needs to be set accordingly.

**app.lookAheadStepSize**: An internal, optional parameter which specifies the time until a predicted state is saved. The smaller the look ahead step size is chosen, the more accurate a state can be interpolated. Per default, a value of app.lookAheadTime/10 is chosen.

**app.directOutputDependency**: Flag (```0``` or ```1```) which controls the output event generation in case an input event was triggered. Per default, the direct output event generation is disabled (```0```). In case the flag is enabled, an output event will be generates as soon as an input event is triggered. Direct output events may be useful in case in case the output is directly changed by the input event. Nevertheless, direct output events may decrease real-time performance and may case cyclic dependency issues. Since the output event will be time-stamped with the same time as the input event, it will always be late. 

### Singlestep Delayed Operation

In singlestep delayed operation, FMITerminalBlock performs a series of micro prediction steps. After each prediction step was finished, results are synchronized and external events may be applied. Notably, the state of the FMU will not be reset to any previous value. Instead, external inputs are delayed to the end of the current prediction step. The operation introduces an artificial delay which slightly biases the result but allows to include FMUs which could not be included otherwise. It is important to carefully choose the maximum time of each simulation step such that the bias is kept to a reasonable limit and that the model can still be solved in real-time.

**app.lookAheadTime**: In singlestep delayed operation, the mandatory parameter specifies the size of each simulation step. External events are delayed at maximum by the given amount of time.

Since usually smaller look ahead time values are chosen in the singlestep delayed mode than in the multistep prediction mode, FMITerminalBlock filters outgoing events to avoid overloading the network infrastructure. If no value was changed since the last prediction step, the outgoing event will be suppressed and no message will be sent.

**app.variableStepSize**: Optional flag ("true"/"false" or "0"/"1") which indicates whether the lookAheadTime should be adopted to model-generated events. In case the parameter is set to *true*, events which are triggered by the model will trigger an immediate output event. Please note that incoming external events will still be delayed until the end of the look ahead horizon or the next upcoming model event. Since the variable step size reduces the predictability of the results, it is set to *false* per default.

## Optional Parameters
The operation of the solver and the prediction logic may be adjusted by the following parameters:

**fmu.name**: The actual model identifier of the FMU as defined in the model description. Usually, the identifier is specified during the export procedure and can be automatically deduced from the files in the FMU directory. In case the FMU simultaneously provides co-simulation and a model exchange (FMI 2.0 only) and in case both variants use different identifiers, the name property should be set to ensure the proper variant is taken. A debug message can be displayed (log level *debug*) which prints the actually taken model identifier. One may use *fmu.name* to change the default behavior, if necessary.

**app.integratorStepSize**: The time interval of a single integrator step. The default value is app.lookAheadStepSize/10.

**app.startTime**: The initial time of the simulation. In real-time mode (currently the only model of operation), the program starts processing events which are associated with the given start time, immediately. The default value which is set in the FMU description may be used. In case the FMU does not contain any default start time property, a value must be given.

**app.stopTime**: The time until the simulation is performed. The program will terminate after the first event which exceeds the stop time is triggered. Hence, the actual simulation may run longer as *app.stopTime* seconds.

**app.timingFile**: If the parameter is set, a timing file will be written which may be used to analyze the performance of FMITerminalBlock and its latencies. It is advised to use the [JuPyther notebook](../../scripts/basic-timing-evaluation.ipynb) and provided [python facilities](scripts.md) to analyze the timing of a simulation run.

In a nutshell, the timing file is a CSV-like text file which contains one timing record on each row. A timing record lists the current instance of real-time of a single event at various processing stages. For instance, it is recorded when an event is predicted or received, when it is scheduled and when it gets deleted. Real-time in a timing record is expressed as absolute time and in terms of simulation time. Similar to data files, fields are separated by semicolon characters. The first four fields of a timing record list the absolute real-time instant (weekday number, hour, minute, and seconds) and the seventh field lists the real-time instant in terms of simulation time. Please not that both real-time representation do not directly correspond to each other since they are generated at different times. The real-time instance in terms of simulation time is generated first and will usually contain less jitter. After the absolute real-time fields, a number which encodes the processing stage is appended in the sixth field. The following table summarizes the magic numbers.

| Processing Stage Number | Description                                     |
|-------------------------|-------------------------------------------------|
| 0                       | Registration by an external (real-time) source  | 
| 1                       | Prediction of the event finished                |
| 2                       | Distribution via the network finished           |
| 3                       | Distribution via the network started            |
| 4                       | The predicted event was considered as outdated  |

The simulation time of each event is present in the fifth field of the timing record. For each event, its simulation time remains constant. At the end of each timing records one or more fields may be appended which contain some debug information. In particular the informal string representation of each event. Please note that the string representation may not be properly escaped. It is advised to ignore all fields after the last non-debug field. The following list summarizes the files of each timing record in order of their appearance.

1. Weekday number of the real-time instance
2. Hour of the real-time instance
3. Minute of the real-time instance
4. Second (as real value) of the real-time instance
5. Simulation time instant of the event
6. Processing stage number
7. Real-time instant of the record expressed in simulation time
8. Debug information

**app.dataFile**: If the parameter is set, a CSV data file will be written which stores the actually scheduled data. Each field of a single row is separated by a semicolon (```;```) character. Each column of the CSV file corresponds to a single model variable. The first two rows are headers which describe the variables and the data types respectively. The first row lists the variable names of each exposed variable and the second row lists the FMI data type of that variable. The FMI data type variable is included in order to ease post processing of exposed results. For some purposes and experiment specific post processing, the second header row may be safely ignored. All other rows describe a single event. I.e. they correspond to a single point in time where an exposed model variable may change. The first column of the CSV file is always the current simulation time instant of the event. In case an event does not cover all variables, the corresponding field will be left empty. Hence, often the data file contains lots of empty fields/variables (e.g. ```0.2;;;;;;;1;```). Depending on the simulation method, a row may even contain no value except the current time. Such an empty row indicates that no value was changed. All variables which are left empty are not associated with the event and remain constant. Sometimes, the concept of events is not needed to evaluate results and a fully populated time series table is more appropriate. Please consider the [CSV data file conversion script](scripts.md) in case a dense CSV format is needed.

The following table shows an exemplary content of a data file. For better readability, the column separators (```;```) are replaced by the tabular representation.

| "time"    | "x"       | "y"          | "new"        | "message"   |
|-----------|-----------|--------------|--------------|-------------|
| "fmiReal" | "fmiReal" | "fmiInteger" | "fmiBoolean" | "fmiString" |
| 0.0       |           |              |              |             |
| 0.1       |           |              |              |             |
| 0.2       |           |              |              |             |
| 0.3       | 3.0       |              |              |             |
| 0.4       |           |  42          | 1            | "One"       |
| 0.5       |           |  42          | 0            | "Two"       |

The first time a variable is actually changed is at *0.3 s*. The first change only covers the variable *x* and no other value. Subsequently, all other model variables are changed at simulation time instant *0.4 s* and *0.5 s*. At these time instants the variable *x* remains constant with a value of *3.0*. 

**in.default.-variable-name-**: The initial value of the input variable with the name *-variable-name-*. The value will be interpreted according to the variable type. I.e. if the variable is an fmiInteger, it will be converted to an integer variable. Unknown or unused inputs will be gracefully ignored.

**app.logLevel**: Specifies the number of log messages which will be displayed. A default log level of *debug* is assumed. The following table summarizes available log levels.

|  Log-Level  | Displayed Messages                                         |
|-------------|------------------------------------------------------------|
| *error*     | error messages only                                        |
| *warning*   | errors and warnings                                        |
| *info*      | some informational messages, errors, warnings              |
| *debug*     | all messages which are intended to debug test setups       |
| *trace*     | all available messages (intended to debug the application) |
