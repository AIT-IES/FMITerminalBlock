from enum import Enum

class SimulationDataType(Enum):
    """Enumerates available variable data types
     
    The enumeration uses the same variable naming convention as the 
    corresponding fmi variables for the enumerated values
    """
    
    REAL = "fmiReal"
    INTEGER = "fmiInteger"
    BOOLEAN = "fmiBoolean"
    STRING = "fmiString"

def generate_simulation_data_types(source_sequence):
    """Tries to generate a sequence of SimulationDataType objects
    
    The source_sequence must implement the iterator protocol. Each element must
    be comparable with string objects or otherwise usable in a 
    SimulationDataType C'tor.
    """
    
    for element in source_sequence:
        yield SimulationDataType(element)
    
