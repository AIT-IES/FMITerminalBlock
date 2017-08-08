import unittest
from data.event import Event

class TestEvent(unittest.TestCase):
    
    def test_get_time(self):
        """Test the get_time function on empty events"""
        
        ev = Event(0.0)
        self.assertEqual(ev.get_time(), 0.0)
        
        ev = Event(-1.0)
        self.assertEqual(ev.get_time(), -1.0)
        
        self.assertRaises(ValueError, Event, "No Number")
    
    def test_set_and_get_value(self):
        """Check the value getter and setter functions"""
        
        ev = Event(0.0)
        self.assertNotIn("x", ev)
        self.assertEqual(len(ev), 0)
        
        ev["x"] = 1.0
        self.assertIn("x", ev)
        self.assertEqual(len(ev), 1)
        self.assertEqual(ev["x"], 1.0)

        ev["x"] = "two"
        self.assertIn("x", ev)
        self.assertEqual(len(ev), 1)
        self.assertEqual(ev["x"], "two")
        
        del ev["x"]
        self.assertNotIn("x", ev)
        self.assertEqual(len(ev), 0)
    
    def test_iterator(self):
        """Test the key iterator of the Event objects"""
        
        ev = Event(0.2)
        ev["x"] = 1.0
        
        it = iter(ev)
        self.assertEqual(next(it), "x")
        self.assertRaises(StopIteration, next, it)
    

if __name__ == "__main__":
    unittest.main()
