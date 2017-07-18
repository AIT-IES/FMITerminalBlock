Quick Start Tutorial
====================

The following tutorial demonstrates the usage of FMITerminalBlock in a Controller Hardware in the Loop (CHIL) and Hardware in the Loop (HIL) setup. It is intended to quickly outline the operation of all involved software components but does not describe them in details. The model is created and exported via [OpenModelica](https://www.openmodelica.org/), the controller is implemented in [Eclipse 4diac](http://www.eclipse.org/4diac/) and a hardware is emulated via [Modbus simulator](https://sourceforge.net/projects/modrssim/). Please make sure that all tools including [FMITerminalBlock](installation.md) are available beforehand. Please refer to the documentation of the software tools in general and the [FMITerminalBlock usage](usage.md) documentation in particular for a detailed description.

## Model and Pure Virtual Simulation

At the beginning, the behavior of interest is modeled. For the current tutorial, a use case which does not introduce too many technical details was chosen. The mythological character [Sisyphos (lat. Sisyphus)](https://en.wikipedia.org/wiki/Sisyphus), king of Ephyra is forced to roll a bolder up a hill. Nearly on top, the bolder comes back and the action is repeated. The following model simulates the situation. The Sisyphus block takes the vertical speed and outputs a flag as soon as the bottom or top is nearly reached. Additionally, the current height is presented. The Hades block reads the flags and changes the vertical speed of Sisyphus accordingly.

![Model: Sisyphus and Hades](tutorial-data/img/underworld.png)

The model files are included in the [model](tutorial-data/model) directory. One can simulate the pure virtual simulation in order to check the functionality of the blocks. The output of the pure virtual simulation is printed in the following figure.

![Pure Virtual Simulation Output](tutorial-data/img/simulation-plot.png)

## Model Export as FMU

In the next steps, the Hades block should be substituted by a controller implementation and the Sisyphus block is executed as virtual component. Regardless of what tool is used to model the virtual component, it needs to be exported as FMU for model exchange. OpenModelica, for instance, offers a menu entry for exporting models as FMU. Make sure that the platform of the FMU corresponds to the platform of FMITerminalBlock. For instance, if FMITerminalBlock was compiled as 32-bit windows application, the FMU must directly support 32-bit windows systems. For 32-bit windows systems, the Sisyphus FMU is also [available](tutorial-data/model/Sisyphus.fmu).

![Export FMU](tutorial-data/img/export-fmu.png)

FMUs are zip compressed archives which contain all files necessary to include the model. FMITerminalBlock requires a FMU to be available in extracted form. Hence, one needs to extract the .fmu file via her/his favourite archive tool.

## Controller Design

The controller is designed in Eclipse 4diac, an IEC 61499-based software PLC. FMITerminalBlock will read the exported model and connect to the PLC. Therefore, FMITerminalBlock needs to synchronize the time with the computer clock and needs to map the in- and outputs of the FMU to the out- and inputs of the controller. The following figure illustrates the experimental setup. 

![Experimental Setup](tutorial-data/img/experimental-setup.png)

Eclipse 4diac provides a system centric view of automation systems which possibly include several controllers with distributed control logic. For the sake of simplicity, only a single PLC device is configured. Please consider the [Eclipse 4diac](http://www.eclipse.org/4diac/en_help.php) documentation for detailed information on how to create new systems and control logic. In the tutorial, the functionality of the Hades block is configured via predefined function blocks. A function block in IEC 61499-based systems has two types of in- and outputs. The first kind are variable outputs which are similar to in- and outputs of IEC 61131-based systems. The other kind of outputs are event outputs which control the execution of a function block. Event in- and outputs are drawn at the top of a function block and variable in- and outputs are drawn at the bottom. The following figure shows a screenshot of the implemented control logic. The control system may also be directly [downloaded from the repository](tutorial-data/controller/4diac-tutorial-underworld.zip).

![IEC 61499-based Control Logic](tutorial-data/img/control-logic.png)

The server function block on the left hand side of the screenshot receives all exported values from FMITerminalBlock. The first value is set at RD_1, the second value on RD_2 and so on. In particular, RD_1 is set when Sisyphus nearly reaches the top and RD_2 is set when he nearly reaches the bottom. RD_3 holds the current height for debugging purpose. Conversion function blocks such as LREAL2LREAL and REAL2REAL set the in- and output types in case Eclipse 4diac cannot automatically deduce them. The RS flip-flop called UpDownStorage keeps track of the current direction and the following selector sets the actual speed set-point. Finally, the server function block on the right hand side of the graphic waits for incoming connection of FMITerminalBlock and sends the computational results to the model. Since the standard function block library does not contain any server function block with just one input, the output RD_1 is left unconnected. Note that every server function block handles a separate connection to FMITerminalBlock. FMITerminalBlock is able to handle an arbitrary amount of connections concurrently. Also simultaneous connections to different devices are feasible. Nevertheless, every connection currently handles only one data flow direction.

## Start and Program the Controller 

First, the Eclipse 4diac Project needs to be imported into the IDE. One way of importing external projects is to press the file menu item "Import ..." and select General/"Existing Projects Into Workspace". One may directly select the archive file or the extracted directory for import. As soon as the project is imported, the entities of the project will be displayed in the "System Explorer" view.

![Project in System Explorer](tutorial-data/img/system-explorer-project.png)

The application which is preceded by the blue icon can be opened by double-clicking on the entry called Hades. Eclipse 4diac IDE provides several perspectives which group available view elements. One can switch the perspective by pressing the buttons at the top right corner of the window. In order to start and program a software PLC one needs to switch to the deployment perspective.

![Switch to Deployment Perspective](tutorial-data/img/switch-to-deployment.png)

In the deployment perspective, the PLC or project needs to be selected for programming. Software download is started via the download button in the download selection view. In order to program the software PLC, Eclipse 4diac IDE connects to the management port of the PLC instance and writes the function block configuration. It does not start a new PLC instance not erases any previous configuration. Hence, in most cases the PLC instance has to be started beforehand. The "Launch Local FORTE" button directly starts the configured default instance of the Eclipse 4diac FORTE software PLC. Another implementation which may be used to create HMIs is also available via the "Launch FBRT" button. The current tutorial only uses Eclipse 4diac FORTE instances.

![Download Selection View](tutorial-data/img/download-selection.png)

The program was downloaded successfully if the "Deployment Console" view prints a wall of text (XML) without highlighting any error messages. Some error messages may be shown in case the PLC instance was already programmed. A programmed PLC is usually indicated by error messages such as ```<Response ID="..." Reason="INVALID_STATE"/>``` and may be fixed by restarting the PLC instance. A locally launched PLC instance may be stopped via the terminate button of the "Console" view.

![Deployment Error and Terminate Button](tutorial-data/img/deployment-error.png)

Alternatively, a "Clean Device" entry in the context menu of the PLC in the "Download Selection" view also wipes the configuration and allows a rapid re-programming. 

![Clean Device Entry](tutorial-data/img/clean-device.png)

Eclipse 4diac FORTE PLC may be compiled to read a given startup configuration file. Per default, the file is called ```forte.fboot```. In case the file is present and contains the correct version of the PLC configuration, no manual download is necessary. If the configuration was changed, the device needs to be cleaned and the updated configuration needs to be downloaded again.

## Debug the Controller

Eclipse 4diac IDE provides an option to view the current status of all function block variables. The debug option may be used to assess the functionality of the controller and to find any configuration flaw. Before the variable values can be queried, the IDE needs to connect to the programmed controller. Therefore, monitoring needs to be enabled via the "Debug"/"Monitor System"/[project-name] menu entry or the corresponding "Online" button. 

![Enable Online Mode](tutorial-data/img/online-mode.png)

Each variable provides a "Watch" and "Remove Watch" entry in the associated context menu which controls the debug output. Alternatively all variables of a function block may be debugged by the "Watch All" entry of the context menu of the function block. 

![Watch All Variables](tutorial-data/img/watch-all.png)

The following figure shows the debug output during a successful experiment.

![Debug Output](tutorial-data/img/debug-output.png)

## Configure and Start FMITerminalBlock

FMITerminalBlock uses a command line interface to read all [configuration options](usage.md). Since external configuration files are currently not supported, it is advised to start FMITerminalBlock from a simple batch file or shell script. The following batch file executes the virtual component Sisyphus and connects th the previously configured PLC. Please make sure that the PLC is already configured before starting FMITerminalBlock. Otherwise the program may exit with an error message which states that a connection is not feasible.

```Batch
FMITerminalBlock.exe ^
	"fmu.path=file:/C:/<path-to-FMU-Directory>/Sisyphus.fmu.dir" ^
	fmu.name=Sisyphus ^
	^
	out.0.0=nearlyOnTop out.0.0.type=2 ^
	out.0.1=nearlyOnBottom out.0.1.type=2 ^
	out.0.2=height out.0.2.type=0 ^
	out.0.protocol=CompactASN.1-TCP ^
	out.0.addr=localhost:1499 ^
	^
	in.0.0=verticalSpeed in.0.0.type=0 ^
	in.0.protocol=CompactASN.1-TCP ^
	in.0.addr=localhost:1500 ^
	^
	app.lookAheadTime=10 ^
	app.dataFile=./data.csv ^
	app.logLevel=debug
```

Please make sure to adapt the path the directory of the extracted FMU (```fmu.path```). It is also vital that the name of the FMU as indicated by the exporting program matches the configured ```fmu.name``` directive. In case the URL to the FMU directory or the name is invalid, the following error message will be displayed.

```
fatal: Invalid command line argument detected: Can't load the incremental FMU "Thanatos" in URL "file:/C:/<path-to-FMU-Directory>/Sisyphus.fmu.dir" Got status 4
```

As indicated in the [usage documentation](usage.md), the ```out.``` and ```in.``` directives configure the variables which will be sent to and read from the controllers respectively. ```app.lookAheadTime``` specifies the time in seconds until the prediction of upcoming events is performed. In case no model event was found within that interval, an artificial event will be sent to the controller. ```app.logLevel``` specifies the amount of data which will be printed. In debug mode each communication event will be listed by entries such as the following ones.

```
[2017-07-18 11:06:40.312559] [0x000003e4] debug: Processed event: PartialEvent: Event: time=41.8849 variables={Variable: <fmiReal, id:1>=100.000000} -- 1 of 1 variables registered
[2017-07-18 11:06:50.717492] [0x000003e4] debug: Processed event: Event: time=51.8696,  variables={Variable: <fmiReal, id:0>=1000.009768, Variable: <fmiTypeBoolean, id:0>=1, Variable: <fmiTypeBoolean, id:1>=0}
```

Events may only change a subset of all registered model variables. Listed variables record the associated model variables of each events. The internal identifier ```<fmiReal, id:1>``` may be resolved by the channel mapping debug lines which are printed at the beginning. A more concise format is given in the data CSV file which also records each event and all associated variables. ```app.dataFile``` specifies the CSV file which will contain events and the associated data. The following snipped shows an example output of the experiment. In case a variable is not associated with a triggered event, the field in the CSV field is left empty.

```csv
"time";"verticalSpeed";"height";"nearlyOnTop";"nearlyOnBottom"
"fmiReal";"fmiReal";"fmiReal";"fmiBoolean";"fmiBoolean"
10;;0;0;1
10.2801;100;;;
10.2801;;0.01;0;0
10.2801;100;;;
20.28;;1000.01;1;0
20.5291;-200;;;
20.6536;;999.98;0;0
20.6851;-200;;;
25.6535;;-0.0156161;0;1
25.8017;100;;;
26.0983;;0.00939047;0;0
26.1293;100;;;
36.0982;;1000.01;1;0
36.2847;-200;;;
36.3779;;999.981;0;0
36.3783;-200;;;
```

## Connect to External Hardware

Based on the CHIL application, a HIL simulation is performed. Instead of the logic which controls the speed of Sisyphus, a Modbus device which sets the speed is used. To emulate the Modbus device (Slave), an [emulator program](https://sourceforge.net/projects/modrssim/) is deployed. An Eclipse 4diac forte PLC is still used to translate between the ASN.1-based protocol which is implemented in FMITerminalBlock and Modbus. Please note that the PLC needs to be compiled manually in order to support Modbus. The documentation of Eclipse 4diac provides further information on how to compile Eclipse 4diac forte. The following graphic illustrates the experimental HIL setup.

![HIL Experiment Setup](tutorial-data/img/interactive-experimental-setup.png)

Another Eclipse 4diac system was created which implements the protocol handling. Before executing the setup, it needs to be imported into the IDE and downloaded to the controller as described above. The following graphic illustrates the translation logic. The system may also be [downloaded from the repository](tutorial-data/controller/4diac-tutorial-interactive-underworld.zip).

![Translation Logic](tutorial-data/img/translation-logic.png)

The server function blocks on the left and right hand side connect to FMITerminalBlock again. Since the same variable mapping as in the previous CHIL setup is used, the very same FMITerminalBlock configuration can be set. The client function block connects to the Modbus TCP server (slave device). One may note the ID parameter of the client function block which is set to ```modbus[127.0.0.1:502:2000:3:1:0:1..3]```. The ID encodes, among others, the remote IP address, the polling interval (2000 ms), and the addresses of the variable to read and write. In particular, holding register 40001 is used to read the vertical speed. Holding Registers 40002 and 40003 are used to set the nearlyOnTop and nearlyOnBottom values respectively and register 40004 will contain the height at the last event. The following screenshot shows the an exemplary Modbus simulator output.

![Modbus Simulator Output](tutorial-data/img/modbus-simulator.png)

One may note that the modbus client function block sends an event after each polling cycle, even if the read values are constant. Consequently, many events are triggered in the simulation environment which may have negative impact on the simulation performance. In case small polling intervals of few hundred milliseconds or below are used, an IEC 61499-based filtering logic which relays only change events is advised. Another observation is that the continuous height parameter is not frequently updated. In the default simulation mode, FMITErminalBlock only outputs discrete events. I.e. if the top or bottom is reached, an event is triggered. Hence, the overhead which is introduced by the simulation is reduced. One may adjust the default behavior to meet the requirements of the particular use case.

