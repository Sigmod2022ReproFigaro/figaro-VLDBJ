from enum import Enum, auto
import subprocess
import os
import json
import logging
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from evaluation.system_test import LogConf, SystemTest
from evaluation.system_test import AccuracyConf
from evaluation.system_test import PerformanceConf
from evaluation.system_test_competitor import SystemTestCompetitor

class SystemTestPython(SystemTestCompetitor):
    def __init__(self, log_conf: LogConf, path_dump: str, 
    perf_conf: PerformanceConf, accur_conf: AccuracyConf, database: Database,
    test_mode = SystemTest.TestMode.PERFORMANCE, **kwargs):
        super().__init__("PYTHON", log_conf, path_dump, perf_conf, accur_conf, database, test_mode)


    def set_join_result_path(self, join_path):
        self.join_path = join_path
    

    def run_debug(self):
        pass

    
    def eval(self, dump: bool, performance: bool):
        args = ["python3", 
            "/home/popina/Figaro/figaro-code/competitors/python/qr.py", 
            "--data_path", self.join_path]
        if dump:
            args += [ "--dump_file", os.path.join(self.path_dump, "R.csv"),
                     "--precision", str(self.conf_accur.precision)]

        if performance:
            args += ["--num_repetitions", "5"]

        result = subprocess.run(args=args,  capture_output=True, text=True)
        path_log_file = self.conf_log.file_path
        with open(path_log_file, "w") as log_file: 
            log_file.write(result.stdout)
        logging.error(result.stderr)


    def run_dump(self):
        self.eval(dump=True, performance=False)


    def run_performance(self):
        self.eval(dump=False, performance=True)


    def requires_dbms_result(self):
        return True
