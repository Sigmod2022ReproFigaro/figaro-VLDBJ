from evaluation.system_test import *

class SystemTestDBMS(SystemTest):
    def __init__(self, name, log_conf: LogConf, dump_conf: DumpConf, 
    perf_conf: PerformanceConf, accur_conf: AccuracyConf, database: Database, test_mode):
        super().__init__(name, log_conf, dump_conf, perf_conf, accur_conf, database, test_mode)


    def is_paper_algorithm(self):
        return False


    def run_accuracy(self):
        pass


    def is_dbms(self):
        return True


    def requires_dbms_result(self):
        return False