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
    
    def test_registration_axis_0(self):
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
    
    def test_registration_axis_1(self):
        """Test the registration axis of the default data set
        
        An "include all" filter will be explicitly applied
        """
        
        axis = self._data_set.get_registration_axis(DataSet.filter_include_all)
        
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
        self.assertArrayEqual(raw_reference, axis.get_raw_data())
    
    def test_registration_axis_2(self):
        """Test the registration axis of the default data set
        
        An "predicted" filter will be explicitly applied
        """
        
        axis = self._data_set.get_registration_axis( \
            DataSet.filter_include_predicted)
        
        raw_reference = np.array([ \
            [1.0, 0.0], \
            [2.0, 1.5], \
            [5.0, 3.5], \
            [11.0, 10.0], \
            [12.0, 11.5], \
            [15.0, 13.5] \
        ])
        self.assertArrayEqual(raw_reference, axis.get_raw_data())
    
    def test_registration_axis_3(self):
        """Test the registration axis of the default data set
        
        An "include external" filter will be explicitly applied
        """
        
        axis = self._data_set.get_registration_axis( \
            DataSet.filter_include_external)
        
        raw_reference = np.array([ \
            [3.0, 1.6], \
            [4.0, 3.8], \
            [13.0, 11.6], \
            [14.0, 13.8] \
        ])
        self.assertArrayEqual(raw_reference, axis.get_raw_data())
    
    def test_get_begin_distribution_axis_0(self):
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
        self.assertArrayEqual(raw_reference, axis.get_raw_data())
    
    def test_get_begin_distribution_axis_1(self):
        """Test the begin of distribution axis of the default data set
        
        A filter expression will be applied
        """
        
        axis = self._data_set.get_begin_distribution_axis( \
            DataSet.filter_include_external)
        raw_reference = np.array([ \
            [3.0, 3.1], \
            [4.0, 4.2], \
            [13.0, 13.1], \
            [14.0, 14.2] \
        ])
        self.assertArrayEqual(raw_reference, axis.get_raw_data())
    
    def test_get_end_distribution_axis_0(self):
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
        self.assertArrayEqual(raw_reference, axis.get_raw_data())
    
    def test_get_end_distribution_axis_1(self):
        """Test the end of distribution axis of the default data set
        
        A filter expression will be applied
        """
        
        axis = self._data_set.get_end_distribution_axis( \
            DataSet.filter_include_external)
        raw_reference = np.array([ \
            [3.0, 3.3], \
            [4.0, 4.4], \
            [13.0, 13.3], \
            [14.0, 14.4] \
        ])
        self.assertArrayEqual(raw_reference, axis.get_raw_data())
    
    def test_get_distribution_delay_axis_0(self):
        """Test the distribution delay axis generation"""
        axis = self._data_set.get_distribution_delay_axis()
        raw_reference = np.array([ \
            [1.0, 0.5], \
            [2.0, 0.2], \
            [3.0, 0.2], \
            [4.0, 0.2], \
            [11.0, 0.5], \
            [12.0, 0.2], \
            [13.0, 0.2], \
            [14.0, 0.2] \
        ])
        self.assertArrayEqual(raw_reference[:,0], \
            axis.get_simulation_time_data())
        self.assertArrayAlmostEqual(raw_reference[:,1], axis.get_delay())
    
    def test_get_distribution_delay_axis_1(self):
        """Test the filtered distribution delay axis generation"""
        
        axis = self._data_set.get_distribution_delay_axis(\
            DataSet.filter_include_external)
        raw_reference = np.array([ \
            [3.0, 0.2], \
            [4.0, 0.2], \
            [13.0, 0.2], \
            [14.0, 0.2] \
        ])
        self.assertArrayEqual(raw_reference[:,0], \
            axis.get_simulation_time_data())
        self.assertArrayAlmostEqual(raw_reference[:,1], axis.get_delay())
    
    def test_get_wait_delay_axis_0(self):
        """Test the wait delay axis generation"""
        axis = self._data_set.get_wait_delay_axis()
        raw_reference = np.array([ \
            [1.0, 1.0], \
            [2.0, 0.5], \
            [3.0, 1.5], \
            [4.0, 0.4], \
            [11.0, 1.0], \
            [12.0, 0.5], \
            [13.0, 1.5], \
            [14.0, 0.4] \
        ])
        self.assertArrayEqual(raw_reference[:,0], \
            axis.get_simulation_time_data())
        self.assertArrayAlmostEqual(raw_reference[:,1], axis.get_delay())
    
    def test_get_wait_delay_axis_1(self):
        """Test the filtered wait delay axis generation"""
        axis = self._data_set.get_wait_delay_axis( \
            DataSet.filter_include_external)
        raw_reference = np.array([ \
            [3.0, 1.5], \
            [4.0, 0.4], \
            [13.0, 1.5], \
            [14.0, 0.4] \
        ])
        self.assertArrayEqual(raw_reference[:,0], \
            axis.get_simulation_time_data())
        self.assertArrayAlmostEqual(raw_reference[:,1], axis.get_delay())
    
    def test_get_triggered_prediction_delay_axis_0(self):
        """Test the corresponding axis generation function"""
        
        axis = self._data_set.get_triggered_prediction_delay_axis()
        raw_reference = np.array([ \
            [1.0, 0.0], \
            [2.0, np.nan], \
            [3.0, 0.2], \
            [4.0, 5.6], \
            [11.0, 0.0], \
            [12.0, np.nan], \
            [13.0, 0.2], \
            [14.0, np.nan] \
        ])
        self.assertArrayEqual(raw_reference[:,0], \
            axis.get_simulation_time_data())
        self.assertArrayAlmostEqual(raw_reference[:,1], axis.get_delay())
    
    def test_get_triggered_prediction_delay_axis_1(self):
        """Test the corresponding filtered axis generation function"""
        
        axis = self._data_set.get_triggered_prediction_delay_axis( \
            DataSet.filter_include_external)
        raw_reference = np.array([ \
            [3.0, 0.2], \
            [4.0, 5.6], \
            [13.0, 0.2], \
            [14.0, np.nan] \
        ])
        self.assertArrayEqual(raw_reference[:,0], \
            axis.get_simulation_time_data())
        self.assertArrayAlmostEqual(raw_reference[:,1], axis.get_delay())
    
    def test_mean_delay(self):
        """Test the average delay function of a timing axis object"""
        
        axis = self._data_set.get_registration_axis()
        self.assertAlmostEqual(axis.get_mean_delay(), -0.92)
    
    def test_variance_of_delay(self):
        """Test the variance calculation of all delay values"""
        
        axis = self._data_set.get_registration_axis()
        self.assertAlmostEqual(axis.get_variance_of_delay(), 0.2817777777777778)
    
    def test_min_delay(self):
        """Test the minimum delay function of a timing axis object"""
        
        axis = self._data_set.get_registration_axis()
        self.assertAlmostEqual(axis.get_min_delay(), -1.5)

    def test_max_delay(self):
        """Test the maximum delay function of a timing axis object"""
        
        axis = self._data_set.get_registration_axis()
        self.assertAlmostEqual(axis.get_max_delay(), -0.2)
    
    def test_get_simulation_time_data(self):
        """Test the simulation time series function"""
        
        axis = self._data_set.get_registration_axis()
        raw_reference = np.array([ \
                1.0, 2.0, 3.0, 5.0, 4.0, 11.0, 12.0, 13.0, 15.0, 14.0 \
            ])
            
        time_data = axis.get_simulation_time_data()
        self.assertArrayEqual(raw_reference, time_data)
    
    def test_get_real_time_data(self):
        """Test the real time series function"""
        
        axis = self._data_set.get_registration_axis()
        raw_reference = np.array([ \
                0.0, 1.5, 1.6, 3.5, 3.8, 10.0, 11.5, 11.6, 13.5, 13.8 \
            ])
        
        time_data = axis.get_real_time_data()
        self.assertArrayEqual(raw_reference, time_data)
    
    def test_get_delay_cleaned_axis(self):
        """Test the outlier removal functionality
        """
        
        axis = self._data_set.get_registration_axis()
        
        # No cleanup
        axis = axis.get_delay_cleaned_axis(0.0)
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
        self.assertArrayEqual(raw_reference, axis.get_raw_data())

        # Remove four items
        axis = axis.get_delay_cleaned_axis(0.4)
        raw_reference = np.array([ \
            [1.0, 0.0], \
            [2.0, 1.5], \
            [3.0, 1.6], \
            [11.0, 10.0], \
            [12.0, 11.5], \
            [13.0, 11.6] \
        ])
        self.assertArrayEqual(raw_reference, axis.get_raw_data())
            
        # Remove all items
        axis = axis.get_delay_cleaned_axis(1.0)
        self.assertEqual(axis.get_length(), 0)
    
    def assertArrayEqual(self, a, b):
        """Asserts that both numpy arrays are equal"""
        
        self.assertTrue((a == b).all(), "{} == {}".format(a, b))
    
    def assertArrayAlmostEqual(self, a, b, tolerance=10e-7):
        """Asserts that both numpy arrays are equal up to tolerance"""
        
        assert(tolerance >= 0.0)
        almost_equal = np.absolute(a - b) <= tolerance
        both_nan = np.logical_and(np.isnan(a), np.isnan(b))
        self.assertTrue(np.logical_or(almost_equal, both_nan).all(), \
            "{} == {}".format(a, b))

if __name__ == "__main__":
    unittest.main()