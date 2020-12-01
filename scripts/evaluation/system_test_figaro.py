
from enum import Enum, auto
import subprocess
import os
import json
from data_management.database import Database
from evaluation.system_test import SystemTest

class SystemTestFigaro(SystemTest):
    def run(self):
        args = ["/bin/bash", "setup.sh", "--log_path={}".format(self.path_log)]
        print(args)
        result = subprocess.run(
            args=args, cwd="/home/popina/Figaro/figaro-code/figaro", 
            capture_output=True, text=True, shell=False)
        print(result.stdout)
        print(result.stderr)