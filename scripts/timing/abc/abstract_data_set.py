""" Encapsulates all abstract base classes for the data set facilities
"""

import numpy as np
import math

from abc import ABCMeta, abstractmethod

class AbstractAxis(metaclass=ABCMeta):
    """Defines an abstract time series axis which provides some statistics
    
    Each axis must provide a series of time span values which is associated with
    a certain simulation time stamp. Each time span is also called delay. For 
    axis objects which encode certain points in time, delay values correspond to
    the deviation from simulation time. Other axis may encode processing times.
    Thereby delay values will correspond to the delay which was introduced by 
    the processing step.
    """
    
    @abstractmethod
    def get_delay(self):
        """Returns an one dimensional array of delay values.
        
        Each entry corresponds to the positional equivalent entry of the 
        simulation time vector.
        """
        return []
    
    @abstractmethod
    def get_simulation_time_data(self):
        """Returns a vector of nominal simulation time stamps
        
        For each delay value, the vector must contain the corresponding 
        simulation time stamp.
        """
        return []
    
    @abstractmethod
    def _get_cleaned_axis(self, permitted_row_indices):
        """Factory function which returns a cleaned version of the current axis
        
        The array permitted_row_indices of row indices lists all rows which 
        should reside in the new axis object. It can be assumed that the array 
        is ascendingly sorted.
        """
        return []
    
    def get_length(self):
        """Returns the number of samples included in the axis"""
        return len(self.get_delay())
    
    def get_mean_delay(self):
        """Returns the mean delay of all timing events"""
        return np.mean(self.get_delay())
    
    def get_variance_of_delay(self):
        """Returns the variance Sigma^2 of all delay values"""
        return np.var(self.get_delay(), ddof=1)
    
    def get_min_delay(self):
        """Returns the minimum delay of all timing events"""
        return np.amin(self.get_delay())
    
    def get_max_delay(self):
        """Returns the maximum delay of all timing events"""
        return np.amax(self.get_delay())
    
    def get_delay_cleaned_axis(self, outlier_factor=0.05):
        """Returns a cleaned version of the axis
        
        The cleaning operation will remove some timing entries with a very low 
        and very high delay. These entries are considered as outliers. The 
        outlier factor will give the share of removed samples. On subsequently 
        calling the function on returned objects (e.g. 
        axis.get_delay_cleaned_axis().get_delay_cleaned_axis()), the number of 
        entries may decrease and further entries may be considered as outliers.
        """
        
        assert(0 <= outlier_factor <= 1)
        
        ind_order = np.argsort(self.get_delay())
        num_removed = math.floor(outlier_factor / 2.0 * self.get_length())
        ind_order = ind_order[num_removed : (self.get_length() - num_removed)]
        ind_order = np.sort(ind_order) # Preserve order of other elements
        return self._get_cleaned_axis(ind_order)
    
