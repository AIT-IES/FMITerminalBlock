import unittest
import numpy as np
from timing.data_set import DataSet, TimingAxis
from timing.reader import Reader

class TestDataSet(unittest.TestCase):
    """Tests the DataSet class and associated utility classes
    """
    
    def setUp(self):
        """Creates a sample data set on which several operations are performed
        """
        
        raw = ['-;-;-;-;1.0;1;0.0;', \
               '-;-;-;-;1.0;3;1.0;', \
               '-;-;-;-;1.0;2;1.5;', \
               '-;-;-;-;2.0;1;1.5;', \
               '-;-;-;-;3.0;0;1.6;', \
               '-;-;-;-;2.0;3;2.0;', \
               '-;-;-;-;2.0;2;2.2;', \
               '-;-;-;-;3.0;3;3.1;', \
               '-;-;-;-;3.0;2;3.3;', \
               '-;-;-;-;5.0;1;3.5;', \
               '-;-;-;-;4.0;0;3.8;', \
               '-;-;-;-;5.0;4;3.8;', \
               '-;-;-;-;4.0;3;4.2;', \
               '-;-;-;-;4.0;2;4.4;', \
               \
               '-;-;-;-;11.0;1;10.0;', \
               '-;-;-;-;11.0;3;11.0;', \
               '-;-;-;-;11.0;2;11.5;', \
               '-;-;-;-;12.0;1;11.5;', \
               '-;-;-;-;13.0;0;11.6;', \
               '-;-;-;-;12.0;3;12.0;', \
               '-;-;-;-;12.0;2;12.2;', \
               '-;-;-;-;13.0;3;13.1;', \
               '-;-;-;-;13.0;2;13.3;', \
               '-;-;-;-;15.0;1;13.5;', \
               '-;-;-;-;14.0;0;13.8;', \
               '-;-;-;-;15.0;4;13.8;', \
               '-;-;-;-;14.0;3;14.2;', \
               '-;-;-;-;14.0;2;14.4;', \
               ]
        # t_sim  t_reg  predicted  triggered  t_begin  t_end
        # 1.0     0.0   T          T           1.0       1.5
        # 2.0     1.5   T          T           2.0       2.2
        # 3.0     1.6   F          T           3.1       3.3
        # 5.0     3.5   T          F          -        -
        # 4.0     3.8   F          T           4.2       4.4
        # 11.0   10.0   T          T          11.0     11.5
        # 12.0   11.5   T          T          12.0     12.2
        # 13.0   11.6   F          T          13.1     13.3
        # 15.0   13.5   T          F          -        -
        # 14.0   13.8   F          T          14.2     14.4
        self._data_set = DataSet(Reader(raw, strict=True))
    
    def test_empty_data_set(self):
        """Test the data set instance with an empty data set"""
        
        ds = DataSet([])
        
        axis = ds.get_registration_axis()
        self.assertEqual(axis.get_length(), 0)
    
    def test_registration_axis(self):
        """Test the registration axis of the default data set"""
        
        axis = self._data_set.get_registration_axis()
        self.assertEqual(axis.get_length(), 10)
        
        raw_reference = np.array([ \
            [1.0, 0.0], \
            [2.0, 1.5], \
            [3.0, 1.6], \
            [5.0, 3.5], \
            [4.0, 3.8], \
            [11.0, 10.0], \
            [12.0, 11.5], \
            [13.0, 11.6], \
            [15.0, 13.5], \
            [14.0, 13.8] \
        ])
        self.assertTrue((raw_reference == axis.get_raw_data()).all(), \
            "{} == {}".format(raw_reference, axis.get_raw_data()))
        
        delay_reference = np.array([-1.0, -0.5, -1.4, -1.5, -0.2, \
                                    -1.0, -0.5, -1.4, -1.5, -0.2 ])
        self.assertTrue((delay_reference - axis.get_delay() <= 10e-7).all(), \
            "{} == {}".format(delay_reference, axis.get_delay()))
    
    def test_get_begin_distribution_axis(self):
        """Test the begin of distribution axis of the default data set"""
        
        axis = self._data_set.get_begin_distribution_axis()
        raw_reference = np.array([ \
            [1.0, 1.0], \
            [2.0, 2.0], \
            [3.0, 3.1], \
            [4.0, 4.2], \
            [11.0, 11.0], \
            [12.0, 12.0], \
            [13.0, 13.1], \
            [14.0, 14.2] \
        ])
        self.assertTrue((raw_reference == axis.get_raw_data()).all(), \
            "{} == {}".format(raw_reference, axis.get_raw_data()))
    
    def test_get_end_distribution_axis(self):
        """Test the end of distribution axis of the default data set"""
        
        axis = self._data_set.get_end_distribution_axis()
        raw_reference = np.array([ \
            [1.0, 1.5], \
            [2.0, 2.2], \
            [3.0, 3.3], \
            [4.0, 4.4], \
            [11.0, 11.5], \
            [12.0, 12.2], \
            [13.0, 13.3], \
            [14.0, 14.4] \
        ])
        self.assertTrue((raw_reference == axis.get_raw_data()).all(), \
            "{} == {}".format(raw_reference, axis.get_raw_data()))

    
    def test_mean_delay(self):
        """Test the average delay function of a timing axis object"""
        
        axis = self._data_set.get_registration_axis()
        self.assertAlmostEqual(axis.get_mean_delay(), -0.92)

    def test_min_delay(self):
        """Test the minimum delay function of a timing axis object"""
        
        axis = self._data_set.get_registration_axis()
        self.assertAlmostEqual(axis.get_min_delay(), -1.5)

    def test_max_delay(self):
        """Test the maximum delay function of a timing axis object"""
        
        axis = self._data_set.get_registration_axis()
        self.assertAlmostEqual(axis.get_max_delay(), -0.2)
        
if __name__ == "__main__":
    unittest.main()