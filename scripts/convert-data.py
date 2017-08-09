"""Transcodes the data CSV file into another CSV file format"""

from argparse import ArgumentParser
from data.reader import Reader
from data.filtered_reader import ConstantInterpolationEventFilter
from data.filtered_reader import EmptyEventFilter
from data.writer import write_basic_csv

import sys
import csv

def main():
    """Runs the application"""
    
    args = parse_arguments()
    
    try:
        process_request(args)
    except ValueError as err:
        sys.stderr.write("Cannot convert the CSV file: {}".format(err))
        sys.exit(2)
    except IOError as err:
        sys.stderr.write("Cannot read or write CSV files: {}".format(err))
        sys.exit(1)
    
    sys.exit(0)

def parse_arguments():
    """Evaluates the commandline arguments and returns the argument object"""
    
    parser = ArgumentParser( \
        description="""Condenses FMITerminalBlock data CSV files into a more 
                        dense CSV format""", \
        epilog="""Copyright (c) 2017, AIT Austrian Institute of Technology GmbH.
                \nAll rights reserved.""")
    
    parser.add_argument("source", \
        help="The FMITerminalBlock CSV file to read and process")
    parser.add_argument("destination", \
        help="""The path to the output file. The file may not exist. Any 
                existing file may be overwritten without notice.""")
    
    init_opts = parser.add_mutually_exclusive_group()
    init_opts.add_argument("--sparse", "-s", action="store_true", \
        help = """Preserves empty fields within a single CSV row without 
                  interpolating any values. Per default, values which are not 
                  associated with a certain time-stamp will be set to their 
                  previous values. The flag prevents interpolation and outputs 
                  sparse events instead.""")
    init_opts.add_argument("--default", "-d", nargs=2, action="append", \
        metavar = ("variable-name","initial-value"), \
        help = """Sets the initial value of the variable. The value will be 
                  set in the corresponding column as long as no input CSV row 
                  does not set another value. It is advised to use the default 
                  values as set in the FMITerminalBlock simulation run.""")
                  
    parser.add_argument("--empty", "-e", action="store_true", \
        help = """Preserves rows which just contain a timestamp and no other 
                  values. If the flag is not present empty rows (events) will be
                  removed from the simulation output.""")
    
    return parser.parse_args()

def process_request(args):
    """Processes the conversion request by evaluating the arguments"""
    
    with open(args.source, 'r', newline='') as infile, \
        open(args.destination, 'w') as outfile:
        
        inreader = get_connected_reader(args, infile)
        write_basic_csv(inreader, outfile, out_dialect=csv.unix_dialect)

def get_connected_reader(args, infile):
    """Returns an abstract reader as described by args.
    
    The reader will take its data from infile. And the reader which is at the 
    end of the processing pipe, will be returned.
    """
    
    inreader = Reader(infile)
    
    if not args.empty:
        inreader = EmptyEventFilter(inreader)
    
    if not args.sparse:
        initial_values = get_default_variable_dict(args)
        inreader = ConstantInterpolationEventFilter(inreader, initial_values)
    
    return inreader

def get_default_variable_dict(args):
    """Generates the dictionary of default variables from the arguments"""
    
    if args.default:
        return dict(args.default)
    else:
        return {}

main()