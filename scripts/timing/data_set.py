"""Contains some classes which hold and process timing data

The classes use numpy as a back end library and hold the complete data in 
memory.
"""

import numpy as np
from timing.timing_entry import TimingEntry
from timing.reader import Reader

class DataSet:
    """Encapsulates the whole timing data set of a timing file
    
    The data is kept in memory. Several functions are provided to access the 
    data via views and to calculate certain metrics.
    """
    
    _iSim = 0 # Simulation time index
    _iReg = 1 # Registration time index
    _iBgn = 2 # Begin distribution time index
    _iEnd = 3 # End distribution time index
    
    def __init__(self, source):
        """Initializes the data set via the given iterable data source
        
        source needs to provide a sequence of TimingEntry objects
        """
        
        entries = list(source)

        # simulation | registration | begin distribution | end distribution
        self._time_stamps = np.empty([len(entries),4])
        self._predicted = np.empty(len(entries), dtype='bool')
        self._triggered = np.empty(len(entries), dtype='bool')
        
        self._set_data_members(entries)
    
    def _set_data_members(self, entries):
        """Sets the data members according to the list of entries
        
        For performance reasons, it is assumed that the data members are already
        initialized to their correct size.
        """
        
        for i,entry in enumerate(entries):
            self._time_stamps[i,DataSet._iSim] = entry.get_simulation_time()
            self._time_stamps[i,DataSet._iReg] = entry.get_registration_time()
            self._predicted[i] = entry.is_predicted()
            self._triggered[i] = entry.is_triggered()
            
            if entry.is_triggered():
                self._time_stamps[i,DataSet._iBgn] = \
                    entry.get_begin_distribution_time()
                self._time_stamps[i,DataSet._iEnd] = \
                    entry.get_end_distribution_time()
    
    def get_registration_axis(self):
        """Returns a TimingAxis object of the registration processing stage
        
        The axis will include all registration events, i.e. predictions and 
        external registrations
        """
        return TimingAxis(self._time_stamps[:,[DataSet._iSim,DataSet._iReg]])
    
    def get_begin_distribution_axis(self):
        """Returns a TimingAxis of begin of distribution time stamps
        
        Events which do not have a distribution state will not be covered by the
        axis
        """
        all_rows = self._time_stamps[:,[DataSet._iSim,DataSet._iBgn]]
        return TimingAxis(all_rows[self._triggered])
    
    def get_end_distribution_axis(self):
        """Returns a TimingAxis of end of distribution time stamps
        
        Events which do not have a distribution state will not be covered by the
        axis
        """
        all_rows = self._time_stamps[:,[DataSet._iSim,DataSet._iEnd]]
        return TimingAxis(all_rows[self._triggered])
    
    

class TimingAxis:
    """Represents a series of certain timings
    
    The timing of each processing stage is represented as TimingAxix. TimingAxis
    objects allow to calculate and query certain parameters which may be useful
    for timing evaluations.
    """
    
    def __init__(self, timing):
        """Initializes the object based on the timing np.array
        
        It is assumed that the first column holds the simulation time and the 
        second column holds the real-time instant which is normalized to the 
        simulation time.
        """
        
        assert(timing.ndim == 2)
        assert(timing.shape[1] == 2)
        
        self._timing = timing
    
    def get_raw_data(self):
        """Returns the raw data representation of the axis
        
        The first column will be the simulation time and the second column the 
        corresponding real time instant. Do not modify any values.
        """
        return self._timing
    
    def get_delay(self):
        """Returns the delay of each timing event"""
        return self._timing[:,1] - self._timing[:,0]
    
    def get_length(self):
        """Returns the number of samples included in the axis"""
        return self._timing.shape[0]


def load_data_set_from_file(filename):
    """Loads and constructs a new DataSet from the given file name
    
    The function is a convenience function to quickly generate a new DataSet 
    object. The newly generated object is returned. All exceptions which may 
    occur are directly passed on to the caller.
    """
    
    with open(filename, 'r', newline='') as infile:
        reader = Reader(filename)
        ds = DataSet(reader)
        return ds
