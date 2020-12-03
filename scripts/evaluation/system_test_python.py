from enum import Enum, auto
import subprocess
import os
import json
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from evaluation.system_test import SystemTest
from evaluation.system_test import Precision
from evaluation.system_test import Performance

class SystemTestPython(SystemTest):
    def __init__(self, path_log: str, path_dump: str, 
    performance: Performance, precision: Precision, database: Database,
    test_type = SystemTest.TestDataType.PERFORMANCE, *args, **kwargs):
        super().__init__(path_log, path_dump, performance, precision, database, test_type)


    def set_join_result_path(self, join_path):
        self.join_path = join_path
    

    def eval(self):
        args = ["python3", 
            "/home/popina/Figaro/figaro-code/competitors/python/qr.py", 
            "--data_path", self.join_path,
            "--dump_file", os.path.join(self.path_dump, "r.csv")]
        print(self.join_path)
        result = subprocess.run(args=args,  capture_output=True, text=True)
        print(result.stdout)
        print(result.stderr)


    def run_debug(self):
        pass


    def run_dump(self):
        pass


    def run_precision(self):
        pass

    
    def run_performance(self):
        pass


    def run(self):
        args = ["python3", 
            "/home/popina/Figaro/figaro-code/competitors/python/qr.py", 
            "--data_path", self.join_path,
            "--dump_file", os.path.join(self.path_dump, "r.csv")]
        print(self.join_path)
        result = subprocess.run(args=args,  capture_output=True, text=True)
        print(result.stdout)
        print(result.stderr)