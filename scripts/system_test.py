# Define a class that wraps a system test where system represents 
# either competitors or Figaro method. 
# paths: log,  - build logsalongside output 
#        dump, - dump data 
#        comp/perf - performance comparison
#        comp/prec - precision comparsion
# 

from enum import Enum, auto

# Class that wraps performance parameters used in testing
class Performance:
    def __init__(self, path):
        pass


# Class that wraps precisions elements 
class Precision:
    def __init__(self):
        pass


class SystemTest:
    class TestDataType(Enum): 
        LOG = auto()
        DUMP = auto()
        PERFORMANCE = auto()
        PRECISION = auto()


    def __init__(self, path_log, path_dump, perf: Performance, prec: Precision):
        self.prec = prec
        self.perf = perf
        self.path_log = path_log
        self.path_dump = path_dump


    def run(self):
        pass 
    
    # Deletes all the auxilary data from the correpsonding path
    def clean_data(self, test_data_type: TestDataType):
        pass

    def clean_all_data(self):
        for test_data_type in TestDataType:
            self.clean_data(test_data_type)
    



