"""Defines some helper functions which convert event streams

In particular, each writer function expects an AbstractReader instance as event
source and converts it to a specific output format.
"""

import csv

def write_basic_csv(in_reader, out_file, out_dialect=csv.excel):
    """Reads all events from in_reader and converts them to a simple CSV format
    
    The output is written to the given out_file file object. Excep CSV dialect 
    is used to represent values. Alternatively, a csv.Dialect object may be 
    passed to format the output.
    The function always writes a single header line which describes all model 
    variable names. The time of each event is always recorded in the first 
    column of the CSV file.
    """
    
    fields = ["time"]
    fields.extend([modelvar for modelvar in in_reader.get_header().keys()])
    
    writer = csv.DictWriter(out_file, fieldnames=fields, dialect=out_dialect)
    
    writer.writeheader()
    
    for event in in_reader:
        row = dict(event)
        row["time"] = event.get_time()
        writer.writerow(row)
