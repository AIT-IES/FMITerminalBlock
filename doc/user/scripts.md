FMITerminalBlock Post Processing Scripts
========================================

The source code repository contains a series of [Python 3](https://www.python.org/) scripts and modules which target post processing of generated data. Please note that on some platform a dedicated call of the ```python3``` executable is required. All python scripts and packages are located in the [scripts](../../scripts) directory. The following sections briefly describe the usage of the conversion scripts and python APIs.

## CSV Data File Conversion

[FMITerminalBlock](usage.md) may write data files which record every event and its associated data. To gain a concise representation, a sparse format which omits several values in case they are not associated with a certain event is used. Nevertheless, several external programs and evaluation tasks require a dense format which contains every model variable value for every recorded time step. The script ```convert-data.py```  was written to convert the FMITerminalBlock output data file format to a dense CSV file format which may be processed more easily. Several options allow a customization of the output data. The ```-h``` option prints a summary of the command line interface. In particular ```$ python3 convert-data.py -h``` yields:

```
usage: convert-data.py [-h] [--sparse | --default variable-name initial-value]
                       [--empty]
                       source destination

Condenses FMITerminalBlock data CSV files into a CSV format which may be
processed more easily

positional arguments:
  source                The FMITerminalBlock CSV file to read and process
  destination           The path to the output file. The file may not exist.
                        Any existing file may be overwritten without notice.

optional arguments:
  -h, --help            show this help message and exit
  --sparse, -s          Preserves empty fields within a single CSV row without
                        interpolating any values. Per default, values which
                        are not associated with a certain time-stamp will be
                        set to their previous values. The flag prevents
                        interpolation and outputs sparse events instead.
  --default variable-name initial-value, -d variable-name initial-value
                        Sets the initial value of the variable. The value will
                        be present in the corresponding column as long as no
                        input CSV row contains a newer value of the variable.
                        It is advised to use the default values as set in the
                        FMITerminalBlock simulation run.
  --empty, -e           Preserves rows which just contain a timestamp and no
                        other values. If the flag is not present, empty rows
                        (events) will be removed from the simulation output.

Copyright (c) 2017, AIT Austrian Institute of Technology GmbH. All rights
reserved.

```

For instance, to convert an input file ```~/data.csv``` to an output file ```~/output.csv``` which records for each non-empty event the state of all model variables, ```$ python3 convert-data.py ~/data.csv ~/output.csv``` may be invoked. In case some default values were set during the execution of FMITerminalBlock, it is recommended to also set them via the ```--default``` option.

The output CSV file format will use a comma (",") as a field separator and will enclose all values within quotation marks. The first row names the model variable of each column and does not contain any simulation outcome. The first column always corresponds to the simulation time.

## Python Data Processing API

The [script/data](../../scripts/data) directory contains a Python package which may be used to read and process FMITerminalBlock data files directly from Python. (e.g. via a [Jupyther](https://jupyter.org/) notebook. Each event is represented by an [```data.event.Event```](../../scripts/data/event.py) object. A [```data.reader.Reader```](../../scripts/data/reader.py) object may be used to read streams of events from CSV files. Filters for removing empty events and for interpolating values which are not directly associated with events can be found in the [```data.filtered_reader```](../../scripts/data/filtered_reader.py) module. Since the filtered reader are also reader objects, it is possible to connect multiple reader objects to a processing pipe which handles a complex task. Please refer to the source code documentation for more details on using the API.

