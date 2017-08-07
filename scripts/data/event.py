from collections.abc import MutableMapping

class Event(MutableMapping):
    """Encodes a single event which encapsulates data at a given instant of time
    
    Each event is precisely timed. E.i. the instant of time may always be 
    quireied. It contains a map of variables which will be changed by the 
    particular event. As key elements, the unique name of the variable is used.
    The new value will be associated with the variable name. The event overloads
    the usual dictionary access functions to access variable values.
    """
    
    def __init__(self, event_time):
        """Initializes an empty event at the given instant of time
        
        event_time corresponds to the current insutant of simulation time.
        """
        
        self._event_time = float(event_time)
        self._event_dict = {}
     
    def get_time(self):
        """Returns the instant of simulation time at which the event occurred"""
        
        return self._event_time
    
    # Functions to emulate the behavior of dictionaries
    def __getitem__(self, key):
        return self._event_dict[key]
    
    def __setitem__(self, key, value):
        self._event_dict[key] = value
    
    def __delitem__(self, key):
        del self._event_dict[key]
    
    def __iter__(self):
        return iter(self._event_dict)
    
    def __len__(self):
        return len(self._event_dict)
    
    