import unittest
from data.filtered_reader import EmptyEventFilter
from data.filtered_reader import ConstantInterpolationEventFilter

from data.reader import Reader

class TestEmptyEventFilter(unittest.TestCase):
    """Tests the empty event filter class instance"""
    
    def test_get_header(self):
        """Fetch the header of the data stream"""
        
        csv_data = ['"time";"x"', '"fmiReal";"fmiReal"']
        source_reader = Reader(csv_data)
        filtered_reader = EmptyEventFilter(source_reader)
        self.assertDictEqual(filtered_reader.get_header(), \
            source_reader.get_header())
    
    def test_filter_sequence(self):
        """Test the filtering functionality"""
        
        csv_data = ['"time";"x"', \
                    '"fmiReal";"fmiReal"', \
                    '0.0;0.0', \
                    '0.1;', \
                    '0.2;-0.1', \
                    '0.3;', \
                    '0.4;', \
                    '0.5;0.1']
        source_reader = Reader(csv_data)
        filtered_reader = EmptyEventFilter(source_reader)
        
        it = iter(filtered_reader)
        
        ev = next(it)
        self.assertEqual(ev.get_time(), 0.0)
        self.assertEqual(ev["x"], 0.0)
        
        ev = next(it)
        self.assertEqual(ev.get_time(), 0.2)
        self.assertEqual(ev["x"], -0.1)
        
        ev = next(it)
        self.assertEqual(ev.get_time(), 0.5)
        self.assertEqual(ev["x"], 0.1)
        
        self.assertRaises(StopIteration, next, it)

class TestConstantInterpolationEventFilter(unittest.TestCase):
    """Tests the interpolation event filter class instance"""
    
    def test_get_header(self):
        """Fetch the header of the data stream"""
        
        csv_data = ['"time";"x"', '"fmiReal";"fmiReal"']
        source_reader = Reader(csv_data)
        filtered_reader = ConstantInterpolationEventFilter(source_reader)
        self.assertDictEqual(filtered_reader.get_header(), \
            source_reader.get_header())
    
    def test_default_filter_sequence(self):
        """Test the filtering functionality without any start values"""
        
        csv_dat = ['"time";"a";"b";"c";"d"', \
                   '"fmiReal";"fmiReal";"fmiInteger";"fmiBoolean";"fmiString"',\
                   '0.0;;;;', \
                   '0.1;0.1;;;', \
                   '0.2;;42;;', \
                   '0.3;;;1;', \
                   '0.4;;;;"Hi!"', \
                   '0.5;;;;', \
                   '0.6;-2.0;5;0;""']
        source_reader = Reader(csv_dat)
        filtered_reader = ConstantInterpolationEventFilter(source_reader)
        
        it = iter(filtered_reader)
        
        ev = next(it)
        self.assertEqual(ev.get_time(), 0.0)
        ref = {"a":0.0, "b":0, "c":False, "d":""}
        self.assertDictEqual(dict(ev), ref)
        
        ev = next(it)
        self.assertEqual(ev.get_time(), 0.1)
        ref = {"a":0.1, "b":0, "c":False, "d":""}
        self.assertDictEqual(dict(ev), ref)
        
        ev = next(it)
        self.assertEqual(ev.get_time(), 0.2)
        ref = {"a":0.1, "b":42, "c":False, "d":""}
        self.assertDictEqual(dict(ev), ref)
        
        ev = next(it)
        self.assertEqual(ev.get_time(), 0.3)
        ref = {"a":0.1, "b":42, "c":True, "d":""}
        self.assertDictEqual(dict(ev), ref)
        
        ev = next(it)
        self.assertEqual(ev.get_time(), 0.4)
        ref = {"a":0.1, "b":42, "c":True, "d":"Hi!"}
        self.assertDictEqual(dict(ev), ref)
        
        ev = next(it)
        self.assertEqual(ev.get_time(), 0.5)
        ref = {"a":0.1, "b":42, "c":True, "d":"Hi!"}
        self.assertDictEqual(dict(ev), ref)
        
        ev = next(it)
        self.assertEqual(ev.get_time(), 0.6)
        ref = {"a":-2.0, "b":5, "c":False, "d":""}
        self.assertDictEqual(dict(ev), ref)
        
        self.assertRaises(StopIteration, next, it)
    
    def test_initialized_filter_sequence_1(self):
        """Test the filtering functionality with start values"""
        csv_dat = ['"time";"a";"b";"c";"d"', \
                   '"fmiReal";"fmiReal";"fmiInteger";"fmiBoolean";"fmiString"',\
                   '0.0;;;;']
        start={"a":0.5, "b":2, "c":True, "d":"Big Bang"}
        source_reader = Reader(csv_dat)
        filtered_reader = ConstantInterpolationEventFilter(source_reader, start)
        
        it = iter(filtered_reader)
        
        ev = next(it)
        self.assertEqual(ev.get_time(), 0.0)
        ref = {"a":0.5, "b":2, "c":True, "d":"Big Bang"}
        self.assertDictEqual(dict(ev), ref)

    def test_invalid_real_init_value_type(self):
        """Test passing an invalid initialization value"""
        csv_dat = ['"time";"a";"b";"c";"d"', \
                   '"fmiReal";"fmiReal";"fmiInteger";"fmiBoolean";"fmiString"',\
                   '0.0;;;;']
        start={"a":"not-real", "b":2, "c":True, "d":"Big Bang"}
        source_reader = Reader(csv_dat)
        self.assertRaises(ValueError, ConstantInterpolationEventFilter, \
            source_reader, start)
    
    def test_invalid_integer_init_value_type(self):
        """Test passing an invalid initialization value"""
        csv_dat = ['"time";"a";"b";"c";"d"', \
                   '"fmiReal";"fmiReal";"fmiInteger";"fmiBoolean";"fmiString"',\
                   '0.0;;;;']
        start={"a":0.1, "b":"no-int", "c":True, "d":"Big Bang"}
        source_reader = Reader(csv_dat)
        self.assertRaises(ValueError, ConstantInterpolationEventFilter, \
            source_reader, start)
    
    def test_invalid_init_variable_name(self):
        """Test passing an invalid initialization value"""
        csv_dat = ['"time";"a";"b";"c";"d"', \
                   '"fmiReal";"fmiReal";"fmiInteger";"fmiBoolean";"fmiString"',\
                   '0.0;;;;']
        start={"a":0.1, "b":1, "c":True, "d":"Big Bang", "e":0}
        source_reader = Reader(csv_dat)
        self.assertRaises(ValueError, ConstantInterpolationEventFilter, \
            source_reader, start)
        

if __name__ == "__main__":
    unittest.main()

    
