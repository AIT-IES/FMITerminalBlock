import unittest
from timing.timing_entry import TimingEntry

class TestTimingEntry(unittest.TestCase):
    
    def test_empty_entry(self):
        """Creates an empty entry"""
        
        entry = TimingEntry(0.2)
        self.assertEqual(entry.get_simulation_time(), 0.2)
    
    def test_predicted_taken_event(self):
        """Test an event which was predicted and actually scheduled"""
        entry = TimingEntry(0.3)
        entry.set_registration_time(0.15, True)
        entry.set_begin_distribution_time(0.35)
        entry.set_end_distribution_time(0.42)
        
        self.assertEqual(entry.get_simulation_time(), 0.3)
        self.assertEqual(entry.get_registration_time(), 0.15)
        self.assertEqual(entry.get_registration_delay(), -0.15)
        self.assertEqual(entry.is_predicted(), True)
        
        self.assertEqual(entry.get_begin_distribution_time(), 0.35)
        self.assertEqual(entry.get_begin_distribution_delay(), 0.35 - 0.3)
        self.assertEqual(entry.get_end_distribution_time(), 0.42)
        self.assertEqual(entry.get_end_distribution_delay(), 0.42 - 0.3)
        
        self.assertEqual(entry.is_triggered(), True)
    
    def test_predicted_not_taken_event(self):
        """Test an event which was predicted and prematurely deleted"""
        entry = TimingEntry(0.3)
        entry.set_registration_time(0.15, True)
        
        self.assertEqual(entry.get_simulation_time(), 0.3)
        self.assertEqual(entry.get_registration_time(), 0.15)
        self.assertEqual(entry.get_registration_delay(), -0.15)
        self.assertEqual(entry.is_predicted(), True)
                
        self.assertEqual(entry.is_triggered(), False)
    
    def test_external_event(self):
        """Test an external event timing entry"""
        entry = TimingEntry(0.3)
        entry.set_registration_time(0.15, False)
        entry.set_begin_distribution_time(0.35)
        entry.set_end_distribution_time(0.42)
        
        self.assertEqual(entry.get_simulation_time(), 0.3)
        self.assertEqual(entry.get_registration_time(), 0.15)
        self.assertEqual(entry.get_registration_delay(), -0.15)
        self.assertEqual(entry.is_predicted(), False)
        
        self.assertEqual(entry.get_begin_distribution_time(), 0.35)
        self.assertEqual(entry.get_begin_distribution_delay(), 0.35 - 0.3)
        self.assertEqual(entry.get_end_distribution_time(), 0.42)
        self.assertEqual(entry.get_end_distribution_delay(), 0.42 - 0.3)
        
        self.assertEqual(entry.is_triggered(), True)
    

if __name__ == "__main__":
    unittest.main()


