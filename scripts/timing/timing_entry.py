class TimingEntry:
    """Encapsulates and manages the timing of a single event
    
    Each event goes through various processing stages and will be time-stamped 
    multiple times. The TimingEntry encapsulates the timestamps and delays of 
    these processing stages. It may be used to directly access several delay 
    values.
    
    Most of the time stamps may be set after the object was constructed. In 
    case a value is queried which was not previously set, an exception will be 
    raised.
	"""
    
    def __init__(self, simulation_time):
        """Initializes the object and sets the nominal (simulation) time
        
        The simulation time of each event is taken to calculate various delay 
        values. All other time stamps may be set after the TimingEntry was 
        constructed.
        """
        self._simulation_time = simulation_time
    
    def get_simulation_time(self):
        """Returns the previously set simulation time"""
        return self._simulation_time
    
    def set_registration_time(self, registration_time, is_predicted):
        """Sets the timestamp where the event was registered at the event queue
        
        registration_time corresponds to the same time scale as the 
        simulation_time bug may have an associated offset value. The 
        is_predicted flag indicates whether the event is an external one or a 
        predicted one.
        """
        
        self._is_predicted = is_predicted
        self._registration_time = registration_time
        self._is_triggered = False
    
    def get_registration_time(self):
        """Returns the previously set absolute registration time
        """
        return self._registration_time
    
    def get_registration_delay(self):
        """Returns the delay on registering the event
        """
        return self.get_registration_time() - self.get_simulation_time()
    
    def is_predicted(self):
        """Returns whether the corresponding event is a predicted event
        """
        return self._is_predicted
    
    def set_begin_distribution_time(self, begin_distribution_time):
        """Sets the timestamp when the distribution phase started
        """
        self._is_triggered = True
        self._begin_distribution_time = begin_distribution_time
    
    def get_begin_distribution_time(self):
        """Returns the time when the distribution starts"""
        return self._begin_distribution_time
    
    def get_begin_distribution_delay(self):
        """Returns the delay when the event distribution starts"""
        return self.get_begin_distribution_time() - self.get_simulation_time()
    
    def set_end_distribution_time(self, end_distribution_time):
        """Sets the time when the distribution of the event ended"""
        self._is_triggered = True
        self._end_distribution_time = end_distribution_time
    
    def get_end_distribution_time(self):
        """Returns the time when the distribution of the event finished"""
        return self._end_distribution_time
    
    def get_end_distribution_delay(self):
        """Returns the delay when the distribution of the event finished"""
        return self.get_end_distribution_time() - self.get_simulation_time()
    
    def is_triggered(self):
        """Returns whether the event was actually triggered (or out dates)
        
        The function may only be used after all parameters are set.
        """
        
        assert(self._is_triggered or self._is_predicted)
        return self._is_triggered
    
