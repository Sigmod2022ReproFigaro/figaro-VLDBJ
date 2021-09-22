
from shutil import ExecError
from evaluation.system_test.system_test import *
from evaluation.accuracy.accuracy import compare_accuracy_r

class SystemTestCompetitor(SystemTest):
    def __init__(self, name, log_conf: LogConf, dump_conf: DumpConf,
    perf_conf: PerformanceConf, accur_conf: AccuracyConf,
    decomp_conf: DecompConf, exec_conf: ExcecutableConf, database: Database,
    query: Query, test_mode):
        super().__init__(name, log_conf, dump_conf, perf_conf, accur_conf, decomp_conf,
            exec_conf, database, query, test_mode)


    def is_paper_algorithm(self):
        return False


    def is_dbms(self):
        return False


    def set_join_result_path(self, join_path):
        self.join_path = join_path


    def run_accuracy(self):
        for system_paper in self.system_test_papers:
            system_test_paper_path = system_paper.conf_dump.file_path
            competitor_path = self.conf_dump.file_path

            r_comp_file_path = self.conf_accur.r_comp_file_path
            errors_file_path = self.conf_accur.errors_file_path

            head_r, tail_r = os.path.split(r_comp_file_path)
            r_comp_file_path = os.path.join(head_r, system_paper.name+tail_r)

            head_e, tail_e = os.path.split(errors_file_path)
            errors_file_path = os.path.join(head_e, system_paper.name+ tail_e)

            compare_accuracy_r(figaro_path=system_test_paper_path,
                competitor_path=competitor_path,
                r_comp_file_path=r_comp_file_path,
                errors_file_path=errors_file_path,
                accuracy_path=self.conf_accur.path,
                precision=self.conf_accur.precision,
                operation='qr',
                generate_xlsx=self.conf_accur.generate_xlsx)


