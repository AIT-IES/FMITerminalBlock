from data.abc.abstract_reader import AbstractReader
from data.simulation_data_type import SimulationDataType

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

class ConstantInterpolationEventFilter(AbstractReader):
    """Filteres model variables which are not present and interpolates them
    
    A constant interpolation function is used to fill missing model variables.
    I.e. previous values are taken. In case a model variable was not contained 
    in any event so far, an initial default value is taken.
    """
    
    default_values = {SimulationDataType.REAL: 0.0, \
                      SimulationDataType.INTEGER: 0, \
                      SimulationDataType.BOOLEAN: False, \
                      SimulationDataType.STRING: ""}
    
    def __init__(self, source_reader, initial_values={}):
        """Initializes the object. 
        
        The source reader must be an abstract reader which deliverers a stream 
        of events. The dict of initial values maps model variable names to their
        initial values. It is assumed that the types of the initial values can 
        be interpreted according to the model variable type. In case a model 
        variable is not present in the initial_values dict, a type-dependent 
        default value (0, 0.0, False, and "" respectively) is taken.
        """
        
        self._source_reader = source_reader
        self._initial_variable_image = self._get_initial_variable_image( \
                initial_values, source_reader.get_header() \
            )
    
    def _get_initial_variable_image(self, initial_values, header):
        """Returns a dictionary which maps all model variables to their values
        
        The initial_values dict overrides the default values which depend on the
        type of the variable. The header dict records the types of each model 
        variable as defined by AbstractReader.get_header().
        """
        
        ret = {}
        for varname, vartype in header.items():
            if varname in initial_values:
                ret[varname] = initial_values[varname]
            else:
                de = ConstantInterpolationEventFilter.default_values[vartype]
                ret[varname] = de
        return ret
    
    def get_header(self):
        """Returns the header of the source reader"""
        
        return self._source_reader.get_header()
    
    def __iter__(self):
        """Returns an iterator which interpolates all model variables"""
        
        return self._interpolate_variables()
    
    def _interpolate_variables(self):
        """Generator function which does the actual interpolation"""
        
        image = self._initial_variable_image
        it = iter(self._source_reader)
        try:
            while True:
                ev = next(it)
                for variable_name in self.get_header().keys():
                    self._sync_variable(variable_name, image, ev)
                yield ev
        except StopIteration:
            pass
    
    def _sync_variable(self, variable_name, image, event):
        """Synchronizes the given image dictionary and the event
        
        In case the event does not contain the variable_name, the event is 
        updated, otherwise, the image dictionary is updated.
        """
        
        if variable_name in event:
            image[variable_name] = event[variable_name]
        else:
            event[variable_name] = image[variable_name]
    
