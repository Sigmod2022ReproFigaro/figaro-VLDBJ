
from enum import Enum, auto
import subprocess
import os
import json
import logging
from data_management.database import Database
from evaluation.system_test import LogConf, SystemTest
from evaluation.system_test import SystemTest
from evaluation.system_test import AccuracyConf
from evaluation.system_test import PerformanceConf

class SystemTestFigaro(SystemTest):
    def __init__(self, log_conf: LogConf, path_dump: str, 
            perf_conf: PerformanceConf, accur_conf: AccuracyConf, database: Database,
            test_mode = SystemTest.TestMode.PERFORMANCE, *args, **kwargs):
        super().__init__("FIGARO", log_conf, path_dump, perf_conf, 
            accur_conf, database, test_mode)
    

    def eval(self):
        pass


    def run_debug(self):
        test_mode = SystemTest.test_mode_to_str(self.test_mode)
        logging.error(test_mode)
        args = ["/bin/bash", "setup.sh", 
                "--log_file_path={}".format(self.conf_log.file_path),
                "--db_config_path={}".format(self.database.db_config_path),
                "--precision={}".format(self.conf_accur.precision),
                "--test_mode={}".format
                (SystemTest.test_mode_to_str(self.test_mode))]
        result = subprocess.run(
            args=args, cwd="/home/popina/Figaro/figaro-code/figaro", 
            capture_output=True, text=True, shell=False)
        logging.info(result.stdout)
        logging.error(result.stderr)


    def run_dump(self):
        test_mode = SystemTest.test_mode_to_str(self.test_mode)
        logging.error(test_mode)
        args = ["/bin/bash", "setup.sh", 
                "--log_file_path={}".format(self.conf_log.file_path),
                "--dump_path={}".format(self.path_dump),
                "--db_config_path={}".format(self.database.db_config_path),
                "--precision={}".format(self.conf_accur.precision),
                "--test_mode={}".format
                (SystemTest.test_mode_to_str(self.test_mode))]
        result = subprocess.run(
            args=args, cwd="/home/popina/Figaro/figaro-code/figaro", 
            capture_output=True, text=True, shell=False)
        logging.info(result.stdout)
        logging.error(result.stderr)


    def run_accuracy(self):
        pass

    
    def run_performance(self):
        pass

    def run_performance_analysis(self):
        super().run_performance_analysis()

    def is_paper_algorithm(self):
        return True
        

    def is_dbms(self):
        return False

    
    def requires_dbms_result(self):
        return False