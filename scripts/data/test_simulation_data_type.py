import unittest
import data.simulation_data_type as simdat

class TestSimulationDataType(unittest.TestCase):
    """Tests the functions related to the SimulationDataType enum"""
    
    def test_enum_value(self):
        """Test the enum values"""
        
        type = simdat.SimulationDataType.REAL
        self.assertEqual(type.value, "fmiReal")
        type = simdat.SimulationDataType.INTEGER
        self.assertEqual(type.value, "fmiInteger")
        type = simdat.SimulationDataType.BOOLEAN
        self.assertEqual(type.value, "fmiBoolean")
        type = simdat.SimulationDataType.STRING
        self.assertEqual(type.value, "fmiString")
    
    def test_enum_from_value(self):
        """Test the generation of enums by their values"""
        
        type = simdat.SimulationDataType("fmiReal")
        self.assertEqual(type, simdat.SimulationDataType.REAL)
        type = simdat.SimulationDataType("fmiInteger")
        self.assertEqual(type, simdat.SimulationDataType.INTEGER)
        type = simdat.SimulationDataType("fmiBoolean")
        self.assertEqual(type, simdat.SimulationDataType.BOOLEAN)
        type = simdat.SimulationDataType("fmiString")
        self.assertEqual(type, simdat.SimulationDataType.STRING)
        
        self.assertRaises(ValueError, simdat.SimulationDataType, "nope")

     
    def test_generate_simulation_data_types(self):
        """Test the generator for simulation data type sequences"""
        
        seq = list(simdat.generate_simulation_data_types(["fmiString", \
            simdat.SimulationDataType.INTEGER, "fmiBoolean", "fmiReal"]))
        ref = [simdat.SimulationDataType.STRING, \
            simdat.SimulationDataType.INTEGER, \
            simdat.SimulationDataType.BOOLEAN, simdat.SimulationDataType.REAL]
            
        self.assertSequenceEqual(seq, ref)
        
        try:
            it = iter((simdat.generate_simulation_data_types(["nope"])))
            next(it)
            self.assertTrue(False)
        except ValueError:
            pass
    

if __name__ == "__main__":
    unittest.main()
