import unittest
from data.filtered_reader import EmptyEventFilter
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
    
    
if __name__ == "__main__":
    unittest.main()

    
