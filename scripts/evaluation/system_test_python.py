from enum import Enum, auto
import subprocess
import os
import json
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from evaluation.system_test import SystemTest
from evaluation.system_test import Precision
from evaluation.system_test import Performance
from data_management.database_psql import JOIN_TABLE_NAME

class SystemTestPython(SystemTest):
    def set_join_path(self, join_path):
        self.join_path = join_path

    def run(self):
        args = ["python3", 
            "/home/popina/Figaro/figaro-code/competitors/python/qr.py", 
            "--data_path", self.join_path,
            "--dump_file", os.path.join(self.path_dump, "r.csv")]
        result = subprocess.run(args=args,  capture_output=True, text=True)
        print(result.stdout)
        print(result.stderr)