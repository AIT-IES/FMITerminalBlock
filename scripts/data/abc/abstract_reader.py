from abc import ABCMeta, abstractmethod

class AbstractReader(metaclass=ABCMeta):
    """Defines and documents an abstract reader which processes event streams
    
    Each event stream is characterized by a static stream header which lists 
    available data fields. Each event contains a subset of all available data 
    fields. In particular, the header lists available model variable and the 
    datatypes thereof. A reader must not dynamically alter the data types. I.e.
    each value of one model variable must be interpretable as the defined data
    type. The static typing requirement was introduced to ease processing via
    dedicated mathematical libraries.
    The reader functionality is implemented by a generator. Each reader must
    provide an __iter__() function which returns a generator or an iterator 
    which yields processed events. A reader must be able to return an iterator
    at least once. Nevertheless, it is advised to support multiple iterators.
    """
    
    @abstractmethod
    def get_header(self):
        """Returns the header map of the reader
        
        The header map lists each registered model variable as a key and maps it
        to the appropriate ..simulation_data_type.SimulationDataType.
        """
        
        return {}
    
    @abstractmethod
    def __iter__(self):
        """Returns the event iterator which returns all processed events
        
        The iterator must return a timely ordered sequence of data.event.Event 
        objects. I.e. each event time must be greater or equal to the previous 
        event time.
        """
        
        return iter({})
    