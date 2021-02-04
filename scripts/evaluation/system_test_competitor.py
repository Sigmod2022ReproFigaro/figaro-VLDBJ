
from evaluation.system_test import *
from evaluation.accuracy.accuracy import compare_accuracy_r

class SystemTestCompetitor(SystemTest):
    def __init__(self, name, log_conf: LogConf, dump_conf: DumpConf, 
    perf_conf: PerformanceConf, accur_conf: AccuracyConf, database: Database, test_mode):
        super().__init__(name, log_conf, dump_conf, perf_conf, accur_conf, database, test_mode)


    def is_paper_algorithm(self):
        return False


    def is_dbms(self):
        return False


    def run_accuracy(self):
        system_test_paper_path = self.system_test_paper.conf_dump.file_path
        competitor_path = self.conf_dump.file_path
        
        compare_accuracy_r(figaro_path=system_test_paper_path, 
            competitor_path=competitor_path, 
            r_comp_file_path=self.conf_accur.r_comp_file_path,
            errors_file_path=self.conf_accur.errors_file_path,
            accuracy_path=self.conf_accur.path, 
            precision=self.conf_accur.precision,
            operation='qr')


