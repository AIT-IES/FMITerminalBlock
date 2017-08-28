import unittest
from timing.timing_entry import TimingEntry
from timing.reader import Reader

class TestReader(unittest.TestCase):
    
    def test_minimal_reader(self):
        """Instantiate a minimal Reader object and read no line"""
        
        raw = []
        reader = Reader(raw)
        it = iter(reader)
        self.assertRaises(StopIteration, next, it)
    
    def test_one_predicted_event(self):
        """Test the timing trace of a single predicted event"""
        
        raw = ['-;-;-;-;0.3;1;0.4;"some malformed; debug output', \
               '-;-;-;-;0.3;3;0.5;""', \
               '-;-;-;-;0.3;2;0.6;']
        reader = Reader(raw)
        it = iter(reader)
        
        entry = next(it)
        self.assertEqual(entry.get_simulation_time(), 0.3)
        self.assertEqual(entry.get_registration_time(), 0.4)
        self.assertEqual(entry.is_predicted(), True)
        self.assertEqual(entry.is_triggered(), True)
        self.assertEqual(entry.get_begin_distribution_time(), 0.5)
        self.assertEqual(entry.get_end_distribution_time(), 0.6)
        
        self.assertRaises(StopIteration, next, it)
    
    def test_one_external_event_1(self):
        """Test the timing trace of a single external event"""
        
        raw = ['-;-;-;-;0.3;1;0.4;', \
               '-;-;-;-;0.3;4;0.5;', \
               '-;-;-;-;0.3;0;0.4;', \
               '-;-;-;-;0.3;3;0.5;', \
               '-;-;-;-;0.3;2;0.6;']
        reader = Reader(raw)
        it = iter(reader)

        entry = next(it) # Outdated entry
        self.assertEqual(entry.get_simulation_time(), 0.3)
        self.assertEqual(entry.get_registration_time(), 0.4)
        self.assertEqual(entry.is_predicted(), True)
        self.assertEqual(entry.is_triggered(), False)
        
        entry = next(it) # Predicted entry
        self.assertEqual(entry.get_simulation_time(), 0.3)
        self.assertEqual(entry.get_registration_time(), 0.4)
        self.assertEqual(entry.is_predicted(), False)
        self.assertEqual(entry.is_triggered(), True)
        self.assertEqual(entry.get_begin_distribution_time(), 0.5)
        self.assertEqual(entry.get_end_distribution_time(), 0.6)
        
        self.assertRaises(StopIteration, next, it)
    
    def test_one_external_event_2(self):
        """Test the timing trace of a single external event"""
        
        raw = ['-;-;-;-;0.5;1;0.4;', \
               '-;-;-;-;0.3;0;0.4;', \
               '-;-;-;-;0.5;4;0.5;', \
               '-;-;-;-;0.3;3;0.5;', \
               '-;-;-;-;0.3;2;0.6;']
        reader = Reader(raw)
        it = iter(reader)

        entry = next(it) # Outdated entry
        self.assertEqual(entry.get_simulation_time(), 0.5)
        self.assertEqual(entry.get_registration_time(), 0.4)
        self.assertEqual(entry.is_predicted(), True)
        self.assertEqual(entry.is_triggered(), False)
        
        entry = next(it) # Predicted entry
        self.assertEqual(entry.get_simulation_time(), 0.3)
        self.assertEqual(entry.get_registration_time(), 0.4)
        self.assertEqual(entry.is_predicted(), False)
        self.assertEqual(entry.is_triggered(), True)
        self.assertEqual(entry.get_begin_distribution_time(), 0.5)
        self.assertEqual(entry.get_end_distribution_time(), 0.6)
        
        self.assertRaises(StopIteration, next, it)
    
    def test_one_outdated_event(self):
        """Test the timing trace of a single external event"""
        
        raw = ['-;-;-;-;0.3;0;0.4;"some malformed; debug output', \
               '-;-;-;-;0.3;3;0.5;""', \
               '-;-;-;-;0.3;2;0.6;']
        reader = Reader(raw)
        it = iter(reader)
        
        entry = next(it)
        self.assertEqual(entry.get_simulation_time(), 0.3)
        self.assertEqual(entry.get_registration_time(), 0.4)
        self.assertEqual(entry.is_predicted(), False)
        self.assertEqual(entry.is_triggered(), True)
        self.assertEqual(entry.get_begin_distribution_time(), 0.5)
        self.assertEqual(entry.get_end_distribution_time(), 0.6)
        
        self.assertRaises(StopIteration, next, it)
    
    def test_interleaved_events(self):
        """Apply multiple interleaving event records"""

        raw = ['-;-;-;-;1.0;1;0.0;"First predicted event"', \
               '-;-;-;-;0.5;0;0.0;"First external event"', \
               '-;-;-;-;1.5;0;0.0;"Second external event"', \
               '-;-;-;-;1.0;4;0.1;"Delete predicted"', \
               '-;-;-;-;0.5;3;0.5;"Schedule first external"', \
               '-;-;-;-;0.5;2;0.6;""', \
               '-;-;-;-;1.5;3;1.5;"Schedule second external"', \
               '-;-;-;-;1.5;2;1.6;']
        reader = Reader(raw)
        it = iter(reader)
        
        entry = next(it)
        self.assertEqual(entry.get_simulation_time(), 1.0)
        self.assertEqual(entry.get_registration_time(), 0.0)
        self.assertEqual(entry.is_predicted(), True)
        self.assertEqual(entry.is_triggered(), False)
        
        entry = next(it)
        self.assertEqual(entry.get_simulation_time(), 0.5)
        self.assertEqual(entry.get_registration_time(), 0.0)
        self.assertEqual(entry.is_predicted(), False)
        self.assertEqual(entry.is_triggered(), True)
        self.assertEqual(entry.get_begin_distribution_time(), 0.5)
        self.assertEqual(entry.get_end_distribution_time(), 0.6)
        
        entry = next(it)
        self.assertEqual(entry.get_simulation_time(), 1.5)
        self.assertEqual(entry.get_registration_time(), 0.0)
        self.assertEqual(entry.is_predicted(), False)
        self.assertEqual(entry.is_triggered(), True)
        self.assertEqual(entry.get_begin_distribution_time(), 1.5)
        self.assertEqual(entry.get_end_distribution_time(), 1.6)
        
        self.assertRaises(StopIteration, next, it)
    
    def test_duplicate_message_1(self):
        """Apply a duplicate timing and expect an exception"""
        
        raw = ['-;-;-;-;0.3;1;0.4;', \
               '-;-;-;-;0.3;0;0.5;']
        reader = Reader(raw, strict=True)
        it = iter(reader)
        
        self.assertRaises(ValueError, next, it)

    def test_duplicate_message_2(self):
        """Apply a duplicate timing and expects to ignore it"""
        
        raw = ['-;-;-;-;0.3;1;0.3;', \
               '-;-;-;-;0.3;0;0.3;', \
               '-;-;-;-;0.3;3;0.3;', \
               '-;-;-;-;0.3;2;0.3;', \
               '-;-;-;-;0.3;4;0.3;', \
               '-;-;-;-;0.4;1;0.4;"Follow-up event"', \
               '-;-;-;-;0.4;3;0.5;', \
               '-;-;-;-;0.4;2;0.6;']
        reader = Reader(raw, strict=False)
        it = iter(reader)
        
        entry = next(it)
        self.assertEqual(entry.get_simulation_time(), 0.4)
        self.assertEqual(entry.get_registration_time(), 0.4)
        self.assertEqual(entry.is_predicted(), True)
        self.assertEqual(entry.is_triggered(), True)
        self.assertEqual(entry.get_begin_distribution_time(), 0.5)
        self.assertEqual(entry.get_end_distribution_time(), 0.6)
        
        self.assertRaises(StopIteration, next, it)
    
    def apply_invalid_raw_string(self, raw_string):
        """Constructs a reader and applies the given string
        
        It is expected that a value error is raised in case some records should
        be queried
        """
        raw = [raw_string]
        reader = Reader(raw, strict=True)
        it = iter(reader)
        
        self.assertRaises(ValueError, next, it)
    
    def test_malformed_simulation_time(self):
        """Test an invalid simulation time value"""
        self.apply_invalid_raw_string('-;-;-;-;0,3;1;0.4;')
    
    def test_malformed_real_time(self):
        """Test an invalid real time value"""
        self.apply_invalid_raw_string('-;-;-;-;0.3;1;0,0-4;')
    
    def test_malformed_stage_value(self):
        """Test an invalid processing stage value"""
        self.apply_invalid_raw_string('-;-;-;-;0.3;42;0.4;')
    
    def test_missing_columns_1(self):
        """Test a timing entry which does not contain all columns"""
        self.apply_invalid_raw_string('-;-;-;-;0.3;0;0.4')
    
    def test_missing_columns_2(self):
        """Test a timing entry which does not contain all columns"""
        self.apply_invalid_raw_string('-;-;-;-;0.3;0')
    
    def test_missing_columns_3(self):
        """Test a timing entry which does not contain all columns"""
        self.apply_invalid_raw_string('-;-;-;-;0.3')
    
    def test_missing_columns_4(self):
        """Test a timing entry which does not contain all columns"""
        self.apply_invalid_raw_string('-;-;-;-')
    

if __name__ == "__main__":
    unittest.main()