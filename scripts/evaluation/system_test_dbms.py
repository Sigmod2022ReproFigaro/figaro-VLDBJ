from evaluation.system_test import *

class SystemTestDBMS(SystemTest):
    def __init__(self, name, path_log: str, path_dump: str, 
    perf_conf: PerformanceConf, accur_conf: AccuracyConf, database: Database, test_mode):
        super().__init__(name, path_log, path_dump, perf_conf, accur_conf, database, test_mode)


    def is_paper_algorithm(self):
        return False


    def run_accuracy(self):
        pass


    def is_dbms(self):
        return True


    def requires_dbms_result(self):
        return False