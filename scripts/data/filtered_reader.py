from data.abc.abstract_reader import AbstractReader

class EmptyEventFilter(AbstractReader):
    """Reader which removes all empty events from the source reader
    
    The source reader will be encapsulated in order to allow multiple source 
    types. Hence, no inheritance is used to implement new features.
    """
    
    def __init__(self, source_reader):
        """Initializes the object. 
        
        The source reader must be an abstract reader which deliverers a stream 
        of events.
        """
        
        self._source_reader = source_reader
    
    def get_header(self):
        """Returns the header of the source reader"""
        
        return self._source_reader.get_header()
    
    def __iter__(self):
        """Returns an iterator which yields all non-empty event instances
        
        Non-empty event instances from the source will be returned without any 
        modification. An event instance is considered to be empty iff it does 
        not have any associated event variables
        """
        
        return self._filter_events()
    
    def _filter_events(self):
        """Generator function which filters the event instances"""
        
        it = iter(self._source_reader)
        try:
            while True:
                ev = next(it)
                if len(ev) > 0:
                    yield ev
        except StopIteration:
            pass;
    