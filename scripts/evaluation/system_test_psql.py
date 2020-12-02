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

class SystemTestPsql(SystemTest):
    def __init__(self, path_log: str, path_dump: str, 
    performance: Performance, precision: Precision, database: Database,
    password: str):
        super().__init__(path_log=path_log, path_dump=path_dump, 
                    performance=performance, precision = precision, 
                    database = database)
        self.password = password


    def run(self):
        if self.test_type == SystemTest.TestDataType.DEBUG or \
        self.test_type == SystemTest.TestDataType.DUMP or \
        self.test_type == SystemTest.TestDataType.PRECISION:
            database_psql = DatabasePsql(host_name="",user_name="popina", 
            password=self.password, database_name=self.database.name)
            database_psql.drop_database()
            database_psql.create_database(self.database)
            
            database_psql.evaluate_join(self.database.get_relations())
            dump_file_path = os.path.join(self.path_dump, JOIN_TABLE_NAME)+".csv"
            print("DMP", dump_file_path)
            database_psql.dump_join(self.database.get_relations(), dump_file_path)
            self.join_path = dump_file_path
