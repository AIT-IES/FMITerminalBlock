"""Contains some classes which hold and process timing data

The classes use numpy as a back end library and hold the complete data in 
memory.
"""

import numpy as np

from timing.timing_entry import TimingEntry
from timing.reader import Reader
from timing.abc.abstract_data_set import AbstractAxis

class DataSet:
    """Encapsulates the whole timing data set of a timing file
    
    The data is kept in memory. Several functions are provided to access the 
    data via views and to calculate certain metrics.
    """
    
    _iSim = 0 # Simulation time index
    _iReg = 1 # Registration time index
    _iBgn = 2 # Begin distribution time index
    _iEnd = 3 # End distribution time index
    
    @staticmethod
    def filter_include_all(ds):
        """Simply includes all elements
        
        The DataSet ds will be used to create a filter expression which covers 
        all elements.
        """
        return slice(len(ds._time_stamps))
    
    @staticmethod
    def filter_include_predicted(ds):
        """Includes all predicted rows of the DataSet ds"""
        return ds._predicted
    
    @staticmethod
    def filter_include_external(ds):
        """Includes all externally registered timing records"""
        return np.logical_not(ds._predicted)
    
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
    
    def _get_filtered_array(self, filter, input_data):
        """Filters the given array and returns the result
        
        In the filter function which is passed in the filter variable is None, 
        the DataSet.filter_include_all function will be used to select filtered
        elements. The input_data array my contain an arbitrary number of columns
        but must contain one row for each timing entry. Only rows determined by
        the given filter expression will be returned. The number and of columns
        will not be altered."""
        
        selection_function = filter or DataSet.filter_include_all
        return input_data[selection_function(self)]
    
    def get_registration_axis(self, filter=None):
        """Returns a TimingAxis object of the registration processing stage
        
        The axis will include all registration events, i.e. predictions and 
        external registrations. The optional filter object must be a function 
        which takes the current data source object (self) and returns an 
        expression which indexes all rows which are to be included in the 
        timing axis.
        """
        
        selected_data = self._time_stamps[:, [DataSet._iSim,DataSet._iReg]]
        selected_data = self._get_filtered_array(filter, selected_data)
        return TimingAxis(selected_data)

    
    def get_begin_distribution_axis(self, filter=None):
        """Returns a TimingAxis of begin of distribution time stamps
        
        Events which do not have a distribution state will not be covered by the
        axis. The optional filter object must be a function which takes the 
        current data source object (self) and returns an expression which 
        indexes all rows which are to be included in the timing axis.
        """
        
        selected_data = self._time_stamps[:,[DataSet._iSim,DataSet._iBgn]]
        selected_data = self._get_filtered_array(filter, selected_data)
        is_triggered = self._get_filtered_array(filter, self._triggered)
        return TimingAxis(selected_data[is_triggered])
    
    def get_end_distribution_axis(self, filter=None):
        """Returns a TimingAxis of end of distribution time stamps
        
        Events which do not have a distribution state will not be covered by the
        axis. The optional filter object must be a function which takes the 
        current data source object (self) and returns an expression which 
        indexes all rows which are to be included in the timing axis.
        """
        
        selected_data = self._time_stamps[:,[DataSet._iSim,DataSet._iEnd]]
        selected_data = self._get_filtered_array(filter, selected_data)
        is_triggered = self._get_filtered_array(filter, self._triggered)
        return TimingAxis(selected_data[is_triggered])
    
    def get_distribution_delay_axis(self, filter=None):
        """Returns the distribution delays of all scheduled events
        
        The delay values will be encapsulated in a DelayAxis object. The 
        optional filter object must be a function which takes the current data 
        source object (self) and returns an expression which indexes all rows 
        which are to be included in the timing axis.
        """
        
        # Select and filter
        sel_data = self._time_stamps[:,\
            [DataSet._iSim, DataSet._iBgn, DataSet._iEnd]]
        sel_data = self._get_filtered_array(filter, sel_data)
        is_triggered = self._get_filtered_array(filter, self._triggered)
        sel_data = sel_data[is_triggered,:] # Only triggered samples
        
        return DelayAxis(sel_data[:,0], sel_data[:,2] - sel_data[:,1])
    
    def get_triggered_prediction_delay_axis(self, filter=None):
        """For each scheduled event, the following prediction delay is returned
        
        Each distribution triggers a prediction phase. The returned axis will 
        hold the duration of the prediction phase following the distribution. 
        Hence, the simulation time stamps do not correspond to the predicted 
        event but to the event which was scheduled before. Hence, for each event
        except the very last one, a prediction delay can be assigned. The very 
        last event will not be present in the returned delay axis.
        
        The given filter is applied to all entries which trigger the prediction.
        """
        
        prediction_timing = self._get_prediction_timing()
        prediction_timing = self._get_filtered_array(filter, prediction_timing)
        is_triggered = self._get_filtered_array(filter, self._triggered)
        trig_pred_timing =  prediction_timing[is_triggered, :]
        return DelayAxis(trig_pred_timing[:, 0], trig_pred_timing[:, 4])
    
    def _get_prediction_timing(self):
        """Returns an array of prediction timing data
        
        The following columns will be returned:
            0: Simulation time of event which triggers the prediction
            1: Real-time instant the prediction is triggered
            2: Simulation time instant of the next predicted event
            3: Real-time instant when the next predicted event is registered
            4: Duration of the prediction operation
        Since the very last event does not trigger a 
        prediction, the simulation time and the delay will be set to NaN. The 
        function will not remove the last entry in order to ease subsequent 
        filtering. Additionally, entries which does not contain a scheduled 
        event will also contain NaN entries as they do not trigger a subsequent
        prediction.
        """
        
        ret = np.full([self._time_stamps.shape[0],5], np.nan)
        ret[:,[0,1]] = self._time_stamps[:,[DataSet._iSim, DataSet._iEnd]]
        
        # Iterate through the array and match the corresponding predictions
        last_trigger_index = -1
        for i,predicted in enumerate(self._predicted):
            if predicted and last_trigger_index >= 0:
                ret[last_trigger_index, 2] = self._time_stamps[i,DataSet._iSim]
                ret[last_trigger_index, 3] = self._time_stamps[i,DataSet._iReg]
            if self._triggered[i]:
                last_trigger_index = i
        
        # Calculate the prediction duration
        ret[:,4] = ret[:,3] - ret[:,1]
        return ret

class TimingAxis(AbstractAxis):
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
    
    def get_simulation_time_data(self):
        """Returns an array of all simulation time stamps
        
        Shorthand for get_raw_data()[:,0]
        """
        return self.get_raw_data()[:,0]
    
    def get_real_time_data(self):
        """Returns the real-time stamps in terms of simulation time
        
        Shorthand for get_raw_data()[:,1]
        """
        return self.get_raw_data()[:,1]
    
    def get_delay(self):
        """Returns the delay of each timing event"""
        return self._timing[:,1] - self._timing[:,0]
    
    def _get_cleaned_axis(self, permitted_row_indices):
        """Factory function which returns a cleaned version of the current axis
        """
        return TimingAxis(self._timing[permitted_row_indices,:])
    

class DelayAxis(AbstractAxis):
    """Encapsulates a time series of time span.
    
    Each time span (delay) values is directly associated with a simulation time
    stamp. In contrast to TimingAxis, DelayAxis does not hold an absolute 
    real-time instance of time. Just delay values are stored.
    """
    
    def __init__(self, simulation_time, delay):
        """Sets the given data vectors
        
        It is assumed that both one dimensional data vectors have the same size.
        """
        assert(len(simulation_time) == len(delay))
        
        self._simulation_time = simulation_time
        self._delay = delay
    
    def get_delay(self):
        """Returns the vector of delay values"""
        return self._delay
    
    def get_simulation_time_data(self):
        """Returns the vector of simulation time stamps"""
        return self._simulation_time
    
    def _get_cleaned_axis(self, permitted_row_indices):
        """Factory function which returns a cleaned version of the current axis
        """
        
        clean_sim_time = self._simulation_time[permitted_row_indices]
        clean_delay = self._delay[permitted_row_indices]
        return DelayAxis(clean_sim_time, clean_delay)
    

def load_data_set_from_file(filename):
    """Loads and constructs a new DataSet from the given file name
    
    The function is a convenience function to quickly generate a new DataSet 
    object. The newly generated object is returned. All exceptions which may 
    occur are directly passed on to the caller.
    """
    
    with open(filename, 'r', newline='') as infile:
        reader = Reader(infile)
        ds = DataSet(reader)
        return ds
