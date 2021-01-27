from enum import Enum, auto
import logging
import subprocess
import os
import json
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from evaluation.system_test import SystemTest
from evaluation.system_test import AccuracyConf
from evaluation.system_test import PerformanceConf
from data_management.database_psql import JOIN_TABLE_NAME
from evaluation.system_test_dbms import SystemTestDBMS
from evaluation.custom_logging import add_logging_file_handler, remove_logging_file_handler
class SystemTestPsql(SystemTestDBMS):
    def __init__(self, path_log: str, path_dump: str, 
            perf_conf: PerformanceConf, accur_conf: AccuracyConf, 
            database: Database,
            test_mode: SystemTest.TestMode, 
            password: str, **kwargs):
        super().__init__("PSQL", path_log=path_log, path_dump=path_dump, 
                    perf_conf=perf_conf, accur_conf = accur_conf, 
                    database = database, test_mode=test_mode)
        self.password = password
        self.join_path = os.path.join(self.path_dump, JOIN_TABLE_NAME) + ".csv"


    def eval(self):
        log_file_path = os.path.join(self.path_log, "log.txt")
        file_handler = add_logging_file_handler(log_file_path, debug_level=logging.INFO)

        database_psql = DatabasePsql(host_name="",user_name="popina", 
        password=self.password, database_name=self.database.name)
        database_psql.drop_database()
        database_psql.create_database(self.database)
        
        database_psql.evaluate_join(self.database.get_relations())
        join_size = database_psql.get_join_size()
        logging.info("Number of rows is {}".format(join_size))
        database_psql.dump_join(self.database.get_relations(), 
                                self.join_path)

        remove_logging_file_handler(file_handler)
        


    def run_debug(self):
        self.eval()

    
    def run_dump(self):
        self.eval()

    
    def run_performance(self):
        self.eval()

    
    def run_performance_analysis(self):
        self.eval()

    
    def is_dbms(self):
        return True

    def get_join_result_path(self):
        return self.join_path

