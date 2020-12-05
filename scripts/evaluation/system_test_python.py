from enum import Enum, auto
import subprocess
import os
import json
import logging
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from evaluation.system_test import SystemTest
from evaluation.system_test import PrecisionConf
from evaluation.system_test import PerformanceConf

class SystemTestPython(SystemTest):
    def __init__(self, path_log: str, path_dump: str, 
    perf_conf: PerformanceConf, prec_conf: PrecisionConf, database: Database,
    test_type = SystemTest.TestMode.PERFORMANCE, **kwargs):
        super().__init__("PYTHON", path_log, path_dump, perf_conf, prec_conf, database, test_type)


    def set_join_result_path(self, join_path):
        self.join_path = join_path
    

    def run_debug(self):
        pass


    def run_dump(self):
        args = ["python3", 
            "/home/popina/Figaro/figaro-code/competitors/python/qr.py", 
            "--data_path", self.join_path,
            "--dump_file", os.path.join(self.path_dump, "r.csv")]
        result = subprocess.run(args=args,  capture_output=True, text=True)
        
        path_log_file = os.path.join(self.path_log, "log.txt")
        logging.info(path_log_file)
        with open(path_log_file, "w") as log_file: 
            log_file.write(result.stdout)
        logging.error(result.stderr)


    def run_precision(self):
        # TODO: call precision for the corresponding test. 
        pass
    
    def run_performance(self):
        pass


    def requires_dbms_result(self):
        return True
