
from evaluation.system_test import *
from evaluation.accuracy.accuracy import compare_accuracy_r

class SystemTestCompetitor(SystemTest):
    def __init__(self, name, path_log: str, path_dump: str, 
    perf_conf: PerformanceConf, accur_conf: AccuracyConf, database: Database, test_mode):
        super().__init__(name, path_log, path_dump, perf_conf, accur_conf, database, test_mode)


    def is_paper_algorithm(self):
        return False


    def is_dbms(self):
        return False


    def run_accuracy(self):
        system_test_paper_path = os.path.join(
            self.system_test_paper.path_dump, "R.csv")
        competitor_path = os.path.join(self.path_dump, "R.csv")
        
        compare_accuracy_r(figaro_path=system_test_paper_path, 
            competitor_path=competitor_path, 
            accuracy_path=self.conf_accur.path, 
            precision=self.conf_accur.precision,
            operation='qr')


