from timing.timing_entry import TimingEntry
import csv

class Reader:
    """Parses FMITerminalBlock timing files
    
    The reader implements an iterable interface which may be used to query the
    sequence of timing entries. Each timing entry covers an event at various
    stages. Each event is identified by its timestamp. However, the timestamp is
    not guaranteed to be really unique. The reader may be configured to fain in
    case two timing entries with the same simulation time value exist. Note, 
    however, that two predicted events with the same time may be handled without
    any error in case the first predicted event gets deleted before the 
    prediction is renewed. (Which is the normal operation of FMITerminalBlock)
    """
    
    def __init__(self, csv_source, strict=False):
        """Creates a reader which uses the timing information from csv_source
        
        The strict flag specifies whether the reader should abort in case a 
        duplicate event is detected. Otherwise, both duplicates will be removed
        from the stream of timing entries and a warning message is logged.
        
        The source_file may be anything which implements the iterator protocol.
        Most common usages include passing file objects and list of line-strings
        which contain the CSV data output of FMITerminalBlock. In case 
        csv_source is a file object, it should be called with newline='' 
        parameter [1].
        
        [1] https://docs.python.org/3/library/csv.html
        """
        
        self._strict = strict
        # Fields in the timing file are usually not quoted. Any qote is part of
        # the debug output at the end of the line and will be gracefully 
        # ignored.
        self._csv_reader = csv.reader(csv_source, \
            delimiter=";", lineterminator='\n', strict=True, \
            quoting=csv.QUOTE_NONE)
    
    def __iter__(self):
        """Returns the iterator which sequentially returns all processed entries
        """
        return self._get_timing_entries()
    
    # TODO: Refactor generator: It is too long (e.g. open new entry function which handles exceptions
    def _get_timing_entries(self):
        """Processes the input stream of raw timing entries
        
        In case a raw timing entry is not completed e.g. by a deletion event or
        by distributing it, it will be ignored and woll not be returned by the 
        generator. Hence one can safely assume that all returned timing entries
        are completely populated. Nevertheless for predicted events not all 
        timings may be present although they are complete.
        """
        
        open_events = {} # t_sim -> TimingEntry
        duplicates = {} # t_sim -> int (Number of concurrent instantiations)
        
        for csv_row in self._csv_reader:
            (t_sim, t_real, action) = self._extract_timing_record_row(csv_row)
            
            if action == '0' or action == '1': # external event or prediction
                pre = action == '1'
                self._add_new_event(open_events, duplicates, t_sim, t_real, pre)
                
            elif action == '3' and t_sim in open_events: # Begin distribution
                open_events[t_sim].set_begin_distribution_time(t_real)
                
            elif action == '2' or action == '4': # End distribution or outdated
                if action == '2' and t_sim in open_events:
                    open_events[t_sim].set_end_distribution_time(t_real)
                
                entry = self._finalize_event(open_events, duplicates, t_sim)
                
                if entry is not None:
                    yield entry
        
    
    def _extract_timing_record_row(self, row):
        """Returns a tuple of (t_sim, t_real, action) from the given row
        
        The function performs some preliminary checks on the length of the row 
        and the number format of time values. Additionally, the processing 
        stage code is checked to be valid.
        """
        
        if len(row) < 8:
            raise ValueError("A row in the timing file does not contain " \
                "enough fields. Parsed {} of 8 fields in '{}'" \
                .format(len(row), str(row)))
        
        t_sim = float(row[4])
        t_real = float(row[6])
        action = row[5]
        
        if action not in ['0', '1', '2', '3', '4']:
            raise ValueError("Invalid processing stage code '{}' found at "\
                    "{} for simulation time {}".format(action, t_real, t_sim))
        
        return (t_sim, t_real, action)
    
    def _add_new_event(self, open_events, duplicates, t_sim, t_real, predicted):
        """Adds a new event to the list of open events or duplicates"""
        
        if t_sim in open_events or t_sim in duplicates:
            if self._strict:
                raise ValueError("An event with the simulation time {} was " \
                "already registered. Hence, the timing evaluation cannot " \
                "differentiate between them. Since events currently do not " \
                "have unique identifiers, the evaluation of the event cannot " \
                "be finished. Please set strict mode to False to parse the " \
                "file anyway.".format(t_sim))
            else:
                if t_sim in open_events:
                    del open_events[t_sim]
                    duplicates[t_sim] = 2
                else:
                    duplicates[t_sim] += 1
        else:
            open_events[t_sim] = TimingEntry(t_sim)
            open_events[t_sim].set_registration_time(t_real, predicted)
    
    def _finalize_event(self, open_events, duplicates, t_sim):
        """Removes the event at t_sim from the given event lists
        
        In case the event is no duplicate, the timing entry will be returned. 
        Otherwise None is returned
        """
        
        if t_sim in open_events:
            ret = open_events[t_sim]
            del open_events[t_sim]
            return ret
        else:
            duplicates[t_sim] -= 1
            if duplicates[t_sim] <= 0:
                del duplicates[t_sim]
            return None
    
