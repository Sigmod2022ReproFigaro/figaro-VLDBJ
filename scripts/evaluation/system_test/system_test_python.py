from enum import Enum, auto
import subprocess
import os
import json
import logging
from data_management.database import Database
from data_management.query import Query
from evaluation.system_test.system_test import DecompConf, DumpConf, ExcecutableConf, LogConf, SystemTest
from evaluation.system_test.system_test import AccuracyConf
from evaluation.system_test.system_test import PerformanceConf
from evaluation.system_test.system_test_competitor import SystemTestCompetitor

class SystemTestPython(SystemTestCompetitor):
    def __init__(self, name: str, log_conf: LogConf, dump_conf: DumpConf,
    perf_conf: PerformanceConf, accur_conf: AccuracyConf,
    decomp_conf: DecompConf, exec_conf: ExcecutableConf, database: Database,
    query: Query, test_mode: SystemTest.TestMode, root_path: str,
    **kwargs):
        super().__init__(name, log_conf, dump_conf, perf_conf, accur_conf, decomp_conf,
            exec_conf, database, query, test_mode)
        self.root_path = root_path



    def run_debug(self):
        pass


    def run_info(self):
        pass


    def eval(self, dump: bool, performance: bool):

        script_path = os.path.join(self.root_path, "competitors/python/qr.py")

        non_join_attr_names = self.query.get_non_join_attr_names_ordered()
        non_join_cat_attr_names = self.query.get_non_join_cat_attr_names_ordered()

        query_num_threads = self.query.get_num_threads()

        num_threads = query_num_threads if query_num_threads is not None else self.conf_perf.num_threads

        logging.debug(self.conf_exec.interpreter_path)
        args = [self.conf_exec.interpreter_path, script_path,
                 "--data_path", self.join_path,
                "--columns", *non_join_attr_names,
                "--cat_columns", *non_join_cat_attr_names,
                "--num_threads", str(num_threads)]
        if dump:
            args += [ "--dump_file", self.conf_dump.file_path,
                     "--precision", str(self.conf_accur.precision)]
        if self.conf_decomp.compute_all:
            args += ["--compute_all"]

        if self.conf_decomp.memory_layout == self.conf_decomp.MemoryLayout.COL_MAJOR:
            args += ["--column_major"]


        num_reps = self.conf_perf.num_reps if performance else 1
        for rep in range(num_reps):
            logging.info("Run {} / {}".format(rep + 1, num_reps))
            result = subprocess.run(args=args,  capture_output=True, text=True)
            path_log_file = self.conf_log.file_path
            with open(path_log_file, "a") as log_file:
                log_file.write(result.stdout)
            logging.error(result.stderr)


    def run_dump(self):
        self.eval(dump=True, performance=False)


    def run_performance(self):
        self.eval(dump=False, performance=True)


    def requires_dbms_result(self):
        return True
