""" Generates data set objects from event streams

The module provides functions which construct in-memory data sets from event
readers which do not hold the entire data in memory.
"""

from data.abc.abstract_reader import AbstractReader
import numpy as np
import pandas as pd

def generate_data_frame(reader):
    """ Collects all events of the reader and constructs a pandas data frame
    
    In case the event doesn't hold a particular variable, the default missing 
    data marker will be used. Every row is indexed by the simulation time 
    instant of the particular event.
    """
    
    event_list = list(reader)
    index = [ev.get_time() for ev in event_list]
    return pd.DataFrame(event_list, index=index)
 
 