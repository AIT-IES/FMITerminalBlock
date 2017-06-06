FMITerminalBlock: FMI - Fieldbus Interface
==========================================

The [FMI](http://fmi-standard.org/) is a standard which allows to export simulation models and tool functionality. It is basically split into two sub-specification, FMI for model exchange and FMI for co-simulation. The former allows to exchange simulation models which still need to be solved separately and the later targets simulator coupling in a co-simulation setup. FMITerminalBlock is capable of solving a FMI-based model in soft real-time and exposes the in- and outputs of the model via a fieldbus interface. A PLC and FMITerminalBlock may be connected in order couple virtual and real components.

## Features
The main features of the software are:
* Numerically solves models exported via FMI for model exchange.
* Implements an event-based operation.
* Uses a predictive approach to forecast future events triggered by the model.
* Sends a fieldbus message as soon as an event is triggered and the corresponding time is reached.
* Maps model variables to zero, one or many network variables.
* Implements the IEC 61499 ASN.1-based fieldbus protocol.
* Various (but currently not all) IEC 61499 types supported.
* Logs timing and timing deviations of soft real-time events.
* Continuous operation or end-of-simulation time supported.
* Provides a command line interface which starts the operation.

## Concepts
FMITerminalBlock focuses on event-based communication which does not restrict communication to fixed time intervals. Nevertheless, FMITerminalBlock also supports a periodic operation in case events are triggered within a fixed interval. Since IEC 61131-based PLCs do not directly support an event-base execution, FMITerminalBlock may only reveal its full capabilities when combined with a IEC 61499-based PLC such as [4diac](https://eclipse.org/4diac).

The accuracy and stability of real-time simulations highly depends on the delay which is introduced by the coupling tools. In order to reduce the delay, a predictive approach is implemented. In principle, the model is solved for future time instances until an event is detected. Each predicted event is calculated under the assumption that network variables remain constant. I.e. no external event is triggered. FMITerminalBlock now waits until the predicted event time is reached and sends the corresponding information to all connected devices. While FMITerminalBlock is waiting, an external event (e.g. from a PLC) may be triggered. Since the prediction was made under the assumption that all network variables remain constant, all predictions have to be outdated and deleted. As soon as the external event is processed, the time and state is rewound and the prediction step is repeated.

A continuous FMI-based model may never trigger an event on its own. FMITerminalBlock implements the concept of a prediction horizon which limits the time until the model is solved in advance. If the prediction horizon is reached without encountering an event, an event is artificially generated. Hence, the memory requirements needed to store intermediate, predicted states is kept within reasonable bounds. The prediction horizon has to be set according to the requirements of the particular application. 

## Development Status
Currently, the ASN.1-based communication protocol which is defined in the IEC 61499 is implemented. Via an intermediate software PLC other communication protocols such as Modbus and MQTT may already be used. Nevertheless, FMITerminalBlock is designed such that multiple communication protocols can be natively supported without the need of an intermediate PLC.

Please be aware that FMITerminalBlock is still in an early development stage. The main focus is currently put on implementing the core functionality and on evaluating the coupling concept. Several (convenience-) functions and some protocol details may not be implemented yet. However, some measures such as an extensive set of unit test are taken to keep the code quality of existing features as high as reasonable possible. If you find a flaw or if you want to propose a feature, we would be glad if you submit a ticket via the issue tracker or contact us directly.

## Further Reading

* [Usage Documentation](doc/user/usage.md)
* [Installation](doc/user/installation.md)

## Publications regarding FMITerminalBlock
* M. Spiegel, F. Leimgruber, E. Widl, G. Gridling:
  ["On using FMI-based models in IEC 61499 control applications"](https://doi.org/10.1109/MSCPES.2015.7115407); 
  Talk: 2015 IEEE Workshop on Modeling and Simulation of Cyber-Physical Energy Systems, Seattle, USA; 2015-04-13; in: "Proceedings of the 2015 Workshop on Modeling and Simulation of Cyber-Physical Energy Systems (MSCPES)", IEEE, Piscataway, NJ, USA (2015), ISBN: 978-1-4799-7357-6; 41 - 46.
* M. Spiegel: [Integrating the Functional Mockup Interface into IEC 61499-based components](https://www.auto.tuwien.ac.at/bib/pdf_TR/TR0175.pdf);
  Bachelor Thesis in Computer Engineering; Faculty of Informatics at the Vienna University of Technology

# License
Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.
All rights reserved. See file [FMITerminalBlock_LICENSE](FMITerminalBlock_LICENSE) for details.
