
from enum import Enum, auto
import subprocess
import os
import json
from data_management.database import Database
from evaluation.system_test import SystemTest
from evaluation.system_test import SystemTest
from evaluation.system_test import Precision
from evaluation.system_test import Performance

class SystemTestFigaro(SystemTest):
    def __init__(self, path_log: str, path_dump: str, 
            performance: Performance, precision: Precision, database: Database,
            test_type = SystemTest.TestDataType.PERFORMANCE, *args, **kwargs):
        super().__init__(path_log, path_dump, performance, 
            precision, database, test_type)


    def run_debug(self):
        pass


    def run_dump(self):
        pass


    def run_precision(self):
        pass

    
    def run_performance(self):
        pass


    def run(self):
        args = ["/bin/bash", "setup.sh", "--log_path={}".format(self.path_log)]
        print(args)
        result = subprocess.run(
            args=args, cwd="/home/popina/Figaro/figaro-code/figaro", 
            capture_output=True, text=True, shell=False)
        print(result.stdout)
        print(result.stderr)