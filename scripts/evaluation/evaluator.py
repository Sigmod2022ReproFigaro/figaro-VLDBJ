import argparse
import json
from evaluation.system_test import SystemTest
from evaluation.system_test_figaro import SystemTestFigaro
from evaluation.system_test_psql import SystemTestPsql
from evaluation.system_test_python import SystemTestPython
from data_management.database import Database

def normal_test(password):
    SYSTEM_PY_TEST_PATH = "/home/popina/Figaro/figaro-code/system_tests/test1/systems/system_test_python.conf"
    SYSTEM_PSQL_TEST_PATH = "/home/popina/Figaro/figaro-code/system_tests/test1/systems/system_test_psql.conf"
    
    system_psql = SystemTestPsql.from_specs_path(SYSTEM_PSQL_TEST_PATH,
                        password=password)
    system_py = SystemTestPython.from_specs_path(SYSTEM_PY_TEST_PATH)
    
    system_psql.setTestDataType(SystemTest.TestDataType.DEBUG)
    system_py.setTestDataType(SystemTest.TestDataType.DEBUG)
    system_psql.run()
    system_py.set_join_path(system_psql.join_path)
    system_py.run()


def figaro_test():
     SYSTEM_FIGARO_TEST_PATH = "/home/popina/Figaro/figaro-code/system_tests/test1/systems/system_test_figaro.conf"
     system_figaro = SystemTestFigaro.from_specs_path(SYSTEM_FIGARO_TEST_PATH)
     system_figaro.run()
    
class SystemTestsEvaluator:
    def __init__(self, tests_conf: str, password: str = None):
        with open(tests_conf) as json_file:
            tests_json = json.load(json_file)
        
        self.password = password
        self.load_system_tests(tests_json)


    def load_system_tests(self, tests_json):
        tests = []
        for test_json in tests_json["tests"]:
            print(test_json.keys())
            system_tests_json = test_json["systems"]
            data_set_json = test_json["data_sets"]
            test = self.load_system_test_data(system_tests_json, data_set_json)
            tests += test
        
        self.tests = tests


    map_type_to_class = {'psql': SystemTestPsql, 'python': SystemTestPython,
                        'figaro': SystemTestFigaro}

    def load_system_test_data(self, system_tests_json, data_sets_json):
        test = []
        for data_set_json in  data_sets_json:
            if "database_conf_path" in data_set_json:
                database_conf_path = data_set_json["database_conf_path"]
            else:
                print("TODO")
            for system_test_json in system_tests_json:
                if "system_conf_path" in system_test_json:
                    system_conf_path = system_test_json["system_conf_path"]
                    system_test_type = system_test_json["type"]
                    class_type = SystemTestsEvaluator.map_type_to_class[system_test_type]
                    system_test = class_type.from_specs_path(system_conf_path,  database_conf_path, self.password)
                    test.append(system_test)
                else: 
                    print("TODO")
        return test


    def eval_tests(self):
        for system_test in self.tests:
            system_test.run()



def eval_tests(password):
    TEST_CONF_PATH = "/home/popina/Figaro/figaro-code/system_tests/test1/tests_specs.conf"
    system_tests_evaluator = SystemTestsEvaluator(TEST_CONF_PATH, password)
    system_tests_evaluator.eval_tests()




if __name__ == "__main__":
    ROOT_PATH = "/home/popina/Figaro/figaro-code"
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--password", action="store",  
                        dest="password", required=True)
    args = parser.parse_args()
    #figaro_test()
    #normal_test(args.password)
    eval_tests(args.password)

