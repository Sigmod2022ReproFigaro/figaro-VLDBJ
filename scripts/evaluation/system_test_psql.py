from enum import Enum, auto
import subprocess
import os
import json
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from evaluation.system_test import SystemTest
from evaluation.system_test import PrecisionConf
from evaluation.system_test import PerformanceConf
from data_management.database_psql import JOIN_TABLE_NAME

class SystemTestPsql(SystemTest):
    def __init__(self, path_log: str, path_dump: str, 
            perf_conf: PerformanceConf, prec_conf: PrecisionConf, database: Database,
            test_mode: SystemTest.TestMode, 
            password: str, *args, **kwargs):
        super().__init__("PSQL", path_log=path_log, path_dump=path_dump, 
                    perf_conf=perf_conf, prec_conf = prec_conf, 
                    database = database, test_mode=test_mode)
        self.password = password
        self.join_path = os.path.join(self.path_dump, JOIN_TABLE_NAME) + ".csv"


    def eval(self):
        database_psql = DatabasePsql(host_name="",user_name="popina", 
        password=self.password, database_name=self.database.name)
        database_psql.drop_database()
        database_psql.create_database(self.database)
        
        database_psql.evaluate_join(self.database.get_relations())
        database_psql.dump_join(self.database.get_relations(), 
                                self.join_path)


    def run_debug(self):
        self.eval()

    
    def run_dump(self):
        self.eval()

    
    def run_performance(self):
        self.eval()

    
    def run_precision(self):
        self.eval()


    def is_dbms(self):
        return True

    def get_join_result_path(self):
        return self.join_path
