import unittest
import numpy as np
import pandas as pd
import numbers

from data.reader import Reader
from data.simulation_data_type import SimulationDataType
import data.data_set_generator as dsg

class TestDataSetGeneraot(unittest.TestCase):
    """Tests the functions of the data_set_generator functions"""
    
    def test_empty_data_frame_1(self):
        """Test the generation of an empty data frame"""
        
        header = ['"time";','"fmiReal";']
        file_reader = Reader(header)
        
        df = dsg.generate_data_frame(file_reader)
        self.assertEqual(df.size, 0)
    
    def test_empty_data_frame_2(self):
        """Test the generation of an empty data frame"""
        
        header = ['"time";"w";"x";"y";"z"', \
            '"fmiReal";"fmiReal";"fmiInteger";"fmiBoolean";"fmiString"']
        file_reader = Reader(header)
        
        df = dsg.generate_data_frame(file_reader)
        self.assertEqual(df.size, 0)
    
    def test_sparse_data_frame(self):
        """Test the data frame generation which contains some missing values"""
        
        data = ['"time";"w";"x";"y";"z"', \
            '"fmiReal";"fmiReal";"fmiInteger";"fmiBoolean";"fmiString"', \
            '0.0;;42;1;"Ok"', '0.1;0.5;;0;"Still Ok"', \
            '0.1;0.6;41;;"Wrong Value"', '0.2;0.4;42;1;', \
            '0.2;0.4;42;1;"Ok Again"']
        file_reader = Reader(data)
        
        df = dsg.generate_data_frame(file_reader)
        
        self.assertEqual(df.shape, (5,4))
        self._assertSeriesEqual(df["w"], [np.nan, 0.5, 0.6, 0.4, 0.4])
        self._assertSeriesEqual(df["x"], [42, np.nan, 41, 42, 42])
        self._assertSeriesEqual(df["y"], [1, 0, np.nan, 1, 1])
        self._assertSeriesEqual(df["z"], ["Ok", "Still Ok", "Wrong Value", \
                                          np.nan, "Ok Again"])
        self._assertSeriesEqual(df.index, [0.0, 0.1, 0.1, 0.2, 0.2])
        
    def _assertSeriesEqual(self, ser, ref):
        """Compares the series ser to the given reference ref"""
        
        self.assertEqual(len(ser), len(ref))
        for (a, b) in zip(ser, ref):
            if not isinstance(b, numbers.Number):
                self.assertEqual(a, b)
            else:
                self.assertEqual(np.isnan(a), np.isnan(b))
                if not np.isnan(a):
                    self.assertEqual(a, b)
    

if __name__ == "__main__":
    unittest.main()