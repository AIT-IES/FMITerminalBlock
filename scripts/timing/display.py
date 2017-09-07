"""Encapsulates several helper functions which format and display timings
"""

from timing.data_set import DataSet, TimingAxis

def to_html_statistics(ds):
    """Returns an HTML table which outputs the major timing parameters
    
    The ds parameter lists the data source which will be evaluated and printed
    """
    
    ret = "<table>"
    ret += "<tr> <th>Axis</th><th>Samples</th><th>Mean Delay</th>"
    ret += "<th>Delay Variance</th><th>Min. Delay</th><th>Max. Delay</th> </tr>"
    
    ret += _to_table_entry(ds.get_registration_axis(), "Registration")
    ret += _to_table_entry(ds.get_begin_distribution_axis(), "Begin Distribution")
    ret += _to_table_entry(ds.get_end_distribution_axis(), "End Distribution")
    
    ret += "</table>"
    return ret

def _to_table_entry(axis, name):
    """Formats all statistics a a particular axis.
    
    The statistics will include the original axis as well as a cleaned one."""
    
    ret = _to_table_row(axis, name)
    ret += _to_table_row(axis.get_delay_cleaned_axis(), \
        "{} (Cleaned)".format(name))
    return ret

def _to_table_row(axis, name):
    """Returns a string which encodes one table row of the statistics table
    
    The name string will not be escaped.
    """
    
    ret = "<tr> "
    ret += "<td> {} </td> ".format(name)
    ret += "<td> {} </td> ".format(axis.get_length())
    ret += "<td> {:.4} </td> ".format(axis.get_mean_delay())
    ret += "<td> {:.4f} </td> ".format(axis.get_variance_of_delay())
    ret += "<td> {:.4f} </td> ".format(axis.get_min_delay())
    ret += "<td> {:.4f} </td> ".format(axis.get_max_delay())
    ret += "</tr> "
    return ret
    