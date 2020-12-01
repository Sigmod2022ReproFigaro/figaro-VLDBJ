# Define a class that wraps a system test where system represents 
# either competitors or Figaro method. 
# paths: log,  - build logsalongside output 
#        dump, - dump data 
#        comp/perf - performance comparison
#        comp/prec - precision comparsion
# 

from enum import Enum, auto
import subprocess
import os
import json
import argparse
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from data_management.database_psql import JOIN_TABLE_NAME

# Class that wraps performance parameters used in testing
class Performance:
    def __init__(self, path: str):
        self.path = path


# Class that wraps precisions elements 
class Precision:
    def __init__(self, path: str):
        self.path = path


class SystemTest:
    # Log test is only for debugging 
    # Dump is used for performacne where data is later compared
    # Performance evaluate speed of the algorithm 
    # Precision compare data from dumps 
    class TestDataType(Enum): 
        DEBUG = auto()
        DUMP = auto()
        PERFORMANCE = auto()
        PRECISION = auto()


    def __init__(self, path_log: str, path_dump: str, 
    performance: Performance, precision: Precision, database: Database,
    test_type = TestDataType.PERFORMANCE):
        self.prec = precision
        self.perf = performance
        self.path_log = path_log
        self.path_dump = path_dump
        self.database = database
        self.test_type = test_type


    @staticmethod
    def create_dir_with_name(path, dir_name):
        dir_abs_path = os.path.join(path, dir_name)
        if not os.path.exists(dir_abs_path):
            os.makedirs(dir_abs_path)

        return dir_abs_path


    @classmethod
    def from_specs_path(cls, system_test_specs_path: str, *args, **kwargs):
        database = None
        with open(system_test_specs_path) as json_file:
            system_json = json.load(json_file)
            #print(system_json)
            #print(system_json["data"])
            for key in system_json:
                print(key)
            if "database_conf_path" in system_json["data"]:
                print("HEJ")
                database_conf_path = system_json["data"]["database_conf_path"]
                database = Database(database_conf_path)
            else:
                print("NO HEY")
                #TODO:  Add
                pass
        
        path_log = SystemTest.create_dir_with_name(
            system_json["system"]["log"]["path"], database.name)
        path_dump = SystemTest.create_dir_with_name(
            system_json["system"]["dump"]["path"], database.name)


        return cls(path_log, path_dump, Precision(""), 
                Performance(""), database, 
                *args, **kwargs)



    def setTestDataType(self, test_type: TestDataType):
        self.test_type = test_type

    def run(self):
        if self.test_type == SystemTest.TestDataType.DEBUG:
            pass
        elif self.test_type == SystemTest.TestDataType.DUMP:
            pass
        elif self.test_type == SystemTest.TestDataType.PRECISION:
            pass
        elif self.test_type == SystemTest.TestDataType.PERFORMANCE:
            pass

        result = subprocess.run(["ls", "-l"], capture_output=True, text=True)
        print(result.stdout)
        
    
    # Deletes all the auxilary data from the correpsonding path
    def clean_data(self, test_data_type: TestDataType):
        pass

    def clean_all_data(self):
        pass
        #for test_data_type in TestDataType:
        #    self.clean_data(test_data_type)


    def load_database(self, database_specs):
        pass
    

class SystemTestPython(SystemTest):
    def set_join_path(self, join_path):
        self.join_path = join_path

    def run(self):
        if self.test_type == SystemTest.TestDataType.DEBUG:
            result = subprocess.run(["python3", 
            "/home/popina/Figaro/figaro-code/competitors/python/qr.py", 
            "--data_path", self.join_path,
            "--dump_file", "dsfdfs"], 
            capture_output=True, text=True)
            print(result.stdout)
            print(result.stderr)
        

class SystemTestPsql(SystemTest):
    def __init__(self, path_log: str, path_dump: str, 
    performance: Performance, precision: Precision, database: Database,
    password: str):
        super().__init__(path_log=path_log, path_dump=path_dump, 
                    performance=performance, precision = precision, 
                    database = database)
        self.password = password


    def run(self):
        print("FUCK")
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


#TODO: Define order of columns by order of relations joined

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--password", action="store",  
                        dest="password", required=True)
    args = parser.parse_args()

    SYSTEM_PY_TEST_PATH = "/home/popina/Figaro/figaro-code/system_tests/test1/systems/system_python.conf"
    SYSTEM_PSQL_TEST_PATH = "/home/popina/Figaro/figaro-code/system_tests/test1/systems/system_psql.conf"
    ROOT_PATH = "/home/popina/Figaro/figaro-code"
    
    system_psql = SystemTestPsql.from_specs_path(SYSTEM_PSQL_TEST_PATH,
    password=args.password)
    system_py = SystemTestPython.from_specs_path(SYSTEM_PY_TEST_PATH)
    
    system_psql.setTestDataType(SystemTest.TestDataType.DEBUG)
    system_py.setTestDataType(SystemTest.TestDataType.DEBUG)
    system_psql.run()
    system_py.set_join_path(system_psql.join_path)
    system_py.run()
