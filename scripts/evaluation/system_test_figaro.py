
from enum import Enum, auto
from os import path
import subprocess
import os
import json
import logging
from data_management.database import Database
from data_management.query import Query
from evaluation.system_test import DumpConf, LogConf, SystemTest
from evaluation.system_test import SystemTest
from evaluation.system_test import AccuracyConf
from evaluation.system_test import PerformanceConf

class SystemTestFigaro(SystemTest):
    def __init__(self, log_conf: LogConf, dump_conf: DumpConf,
            perf_conf: PerformanceConf, accur_conf: AccuracyConf, database: Database, query: Query,
            test_mode, root_path: str, *args, **kwargs):
        super().__init__("FIGARO", log_conf, dump_conf, perf_conf,
            accur_conf, database, query, test_mode)
        self.figaro_path = os.path.join(root_path, "figaro")

    def eval(self):
        pass


    def run_debug(self):
        test_mode = SystemTest.test_mode_to_str(self.test_mode)
        logging.error(test_mode)
        args = ["/bin/bash", "setup.sh",
                "--root_path={}".format(self.figaro_path),
                "--log_file_path={}".format(self.conf_log.file_path),
                "--db_config_path={}".format(self.database.db_config_path),
                "--query_config_path={}".format(self.query.get_conf_path()),
                "--precision={}".format(self.conf_accur.precision),
                "--test_mode={}".format
                (SystemTest.test_mode_to_str(self.test_mode))]
        result = subprocess.run(
            args=args, cwd=self.figaro_path,
            capture_output=True, text=True, shell=False)
        logging.info(result.stdout)
        logging.error(result.stderr)


    def run_dump(self):
        test_mode = SystemTest.test_mode_to_str(self.test_mode)
        logging.error(test_mode)
        args = ["/bin/bash", "setup.sh",
                "--root_path={}".format(self.figaro_path),
                "--log_file_path={}".format(self.conf_log.file_path),
                "--dump_file_path={}".format(self.conf_dump.file_path),
                "--db_config_path={}".format(self.database.db_config_path),
                "--query_config_path={}".format(self.query.get_conf_path()),
                "--precision={}".format(self.conf_accur.precision),
                "--test_mode={}".format
                (SystemTest.test_mode_to_str(self.test_mode))]
        result = subprocess.run(
            args=args, cwd=self.figaro_path,
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