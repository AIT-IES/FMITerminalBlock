import unittest
from data.reader import Reader
from data.simulation_data_type import SimulationDataType
class TestReader(unittest.TestCase):
    
    def test_minimal_file(self):
        """Test a valid input file without any event"""
        
        header = ['"time";','"fmiReal";']
        file_reader = Reader(header)
        
        self.assertEqual(len(file_reader.get_header()), 0)
        it = iter(file_reader)
        self.assertRaises(StopIteration, next, it)
    
    def test_one_element_file(self):
        """Test a file header which just contains a single model variable"""
        
        csv_data = ['"time";"x"','"fmiReal";"fmiInteger"','0.0;2']
        file_reader = Reader(csv_data)
        
        reference_header = {"x":SimulationDataType.INTEGER}
        self.assertDictEqual(file_reader.get_header(), reference_header)
        
        it = iter(file_reader)
        event = next(it)
        self.assertEqual(event.get_time(), 0.0)
        self.assertEqual(len(event), 1)
        self.assertEqual(event["x"], 2)
        
        self.assertRaises(StopIteration, next, it)
    
    def test_all_data_types(self):
        """Tests a CSV file which contains all data types"""
        
        csv_in = ['"time";"a""";"b;";"c";"d"',\
                  '"fmiReal";"fmiReal";"fmiInteger";"fmiBoolean";"fmiString"',\
                  '0.0;0.2;;1;',\
                  '0.1;;42;;"""h;"";;"""";i!"',
                  '0.2;-1e2;;0;',\
                  '0.3;;-2;;""']
        
        file_reader = Reader(csv_in)
        
        reference_header = {'a"':SimulationDataType.REAL, \
                            "b;":SimulationDataType.INTEGER, \
                            "c":SimulationDataType.BOOLEAN, \
                            "d":SimulationDataType.STRING}
        self.assertDictEqual(file_reader.get_header(), reference_header)
        
        it = iter(file_reader)
        event = next(it)
        self.assertEqual(event.get_time(), 0.0)
        self.assertEqual(len(event), 2)
        self.assertEqual(event['a"'], 0.2)
        self.assertEqual(event["c"], True)

        event = next(it)
        self.assertEqual(event.get_time(), 0.1)
        self.assertEqual(len(event), 2)
        self.assertEqual(event["b;"], 42)
        self.assertEqual(event["d"], '"h;";;"";i!')
        
        event = next(it)
        self.assertEqual(event.get_time(), 0.2)
        self.assertEqual(len(event), 2)
        self.assertEqual(event['a"'], -1e2)
        self.assertEqual(event["c"], False)

        event = next(it)
        self.assertEqual(event.get_time(), 0.3)
        self.assertEqual(len(event), 2)
        self.assertEqual(event["b;"], -2)
        self.assertEqual(event["d"], "")
        
        self.assertRaises(StopIteration, next, it)
    
    def test_invalid_time_1(self):
        """Apply an invalid time header"""
        
        header = ['"no time";','"fmiReal";']
        self.assertRaises(ValueError, Reader, header)
    
    def test_invalid_time_2(self):
        """Apply an invalid time header"""
        
        header = ['"time";','"fmiString";']
        self.assertRaises(ValueError, Reader, header)
    
    def test_missing_type_header_1(self):
        """Apply an missing type header field"""
        
        header = ['"time";"x"','"fmiString";']
        self.assertRaises(ValueError, Reader, header)
    
    def test_missing_type_header_2(self):
        """Apply an missing type header field"""
        
        header = ['"time";"x"','"fmiString"']
        self.assertRaises(ValueError, Reader, header)
    
    def test_invalid_type_header(self):
        """Tests an invalid type string in the type header"""
        
        header = ['"time";"x"','"fmiString";"fmiArray"']
        self.assertRaises(ValueError, Reader, header)
    
    def test_invalid_string_value_1(self):
        """Apply an invalid string literal"""
        
        header = ['"time";"x"','"fmiReal";"fmiString"', '0.0;"""']
        file_reader = Reader(header)
        
        self.assertEqual(len(file_reader.get_header()), 1)
        it = iter(file_reader)
        self.assertRaises(ValueError, next, it)
    
    def test_invalid_string_value_2(self):
        """Apply an invalid string literal"""
        
        header = ['"time";"x"','"fmiReal";"fmiString"', '0.0;"']
        file_reader = Reader(header)
        
        self.assertEqual(len(file_reader.get_header()), 1)
        it = iter(file_reader)
        self.assertRaises(ValueError, next, it)
    
    def test_invalid_string_value_3(self):
        """Apply an invalid string literal"""
        
        header = ['"time";"x"','"fmiReal";"fmiString"', '0.0;no-escape']
        file_reader = Reader(header)
        
        self.assertEqual(len(file_reader.get_header()), 1)
        it = iter(file_reader)
        self.assertRaises(ValueError, next, it)
    
    def test_invalid_real_value(self):
        """Apply an invalid real literal"""
        
        header = ['"time";"x"','"fmiReal";"fmiReal"', '0.0;"no-real-example"']
        file_reader = Reader(header)
        
        self.assertEqual(len(file_reader.get_header()), 1)
        it = iter(file_reader)
        self.assertRaises(ValueError, next, it)
        
    def test_invalid_integer_value(self):
        """Apply an invalid integer literal"""
        
        header = ['"time";"x"','"fmiReal";"fmiInteger"', '0.0;1.0']
        file_reader = Reader(header)
        
        self.assertEqual(len(file_reader.get_header()), 1)
        it = iter(file_reader)
        self.assertRaises(ValueError, next, it)
    
    def test_invalid_boolean_value(self):
        """Apply an invalid boolean literal"""
        
        header = ['"time";"x"','"fmiReal";"fmiBoolean"', '0.0;"no-truth"']
        file_reader = Reader(header)
        
        self.assertEqual(len(file_reader.get_header()), 1)
        it = iter(file_reader)
        self.assertRaises(ValueError, next, it)


if __name__ == "__main__":
    unittest.main()

