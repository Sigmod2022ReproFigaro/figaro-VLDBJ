import argparse
import json
import logging
import sys
from evaluation.system_test import SystemTest
from evaluation.system_test_figaro import SystemTestFigaro
from evaluation.system_test_psql import SystemTestPsql
from evaluation.system_test_python import SystemTestPython
from data_management.database import Database
    
class SystemTestsEvaluator:
    map_category_to_class = {
        'psql': SystemTestPsql, 'python': SystemTestPython,
        'figaro': SystemTestFigaro}

    map_mode_to_enum = {
        'dump': SystemTest.TestMode.DUMP,
        'accuracy': SystemTest.TestMode.ACCURACY,
        'performance': SystemTest.TestMode.PERFORMANCE,
        'debug': SystemTest.TestMode.DEBUG}


    def __init__(self, tests_conf: str, password: str = None):
        with open(tests_conf) as json_file:
            tests_json = json.load(json_file)
        
        self.password = password
        self.load_tests(tests_json)


    def load_tests(self, tests_json):
        tests = []
        for test_json in tests_json["tests"]:
            print(test_json.keys())
            system_tests_json = test_json["systems"]
            data_set_json = test_json["data_sets"]
            test = self.load_system_tests_data_sets(
                        system_tests_json, data_set_json)
            tests += test
        
        self.tests = tests


    def load_system_tests_data_sets(self, system_tests_json, data_sets_json):
        test = []
        for data_set_json in  data_sets_json:
            if "database_conf_path" in data_set_json:
                database_conf_path = data_set_json["database_conf_path"]
            else:
                logging.error("TODO")
            
            test += self.load_system_tests(system_tests_json, 
                database_conf_path)
        
        return test


    def load_system_tests(self, system_tests_json, database_conf_path):
        join_result_path = None
        system_test_paper = None
        batch_of_tests = []
        for system_test_json in system_tests_json:
            if "system_conf_path" in system_test_json:
                system_test_disable = system_test_json.get("disable", False)
                if system_test_disable:
                    continue
                system_test = self.create_system_test(system_test_json, 
                                    database_conf_path)
                if system_test.is_dbms():
                    join_result_path = system_test.get_join_result_path()
                if system_test.is_paper_algorithm():
                    system_test_paper = system_test
            else: 
                logging.error("TODO")
            
            batch_of_tests.append(system_test)
            
        for system_test in batch_of_tests:
            if system_test.requires_dbms_result():
                system_test.set_join_result_path(join_result_path)
            
            if not system_test.is_paper_algorithm():
                system_test.set_paper_system_test(system_test_paper)
        
        return batch_of_tests


    def create_system_test(self, system_test_json, database_conf_path: str):
        system_conf_path = system_test_json["system_conf_path"]
        system_test_cat = system_test_json["category"]
        system_test_mode = system_test_json["mode"]
        class_type = SystemTestsEvaluator.map_category_to_class[system_test_cat]
        test_mode = SystemTestsEvaluator.map_mode_to_enum[system_test_mode]
        system_test = class_type.from_specs_path(system_conf_path,
                                         database_conf_path, 
        test_mode=test_mode, password=self.password)

        logging.debug("Category is{}".format(system_test_cat))
        logging.debug("Created category {}".format(type(system_test)))
        logging.debug("Mode is{}".format(system_test_mode))
        logging.debug("Create mode{}".format(system_test_mode))
        
        return system_test


    def eval_tests(self):
        for system_test in self.tests:
            logging.debug("system_test {}".format(system_test))
            system_test.run()



def eval_tests(password):
    TEST_CONF_PATH = "/home/popina/Figaro/figaro-code/system_tests/test3/tests_specs.conf"
    system_tests_evaluator = SystemTestsEvaluator(TEST_CONF_PATH, password)
    system_tests_evaluator.eval_tests()


def init_logging():
    formatter_str = '-- %(levelname)5s -- [%(filename)20s:%(lineno)3s - %(funcName)20s()] %(message)s'
    formatter = logging.Formatter(formatter_str)

    stdout_handler = logging.StreamHandler(sys.stdout)
    stdout_handler.setFormatter(formatter)
    stdout_handler.setLevel(logging.INFO)
    
    file_handler = logging.FileHandler('log.txt', mode='w')
    file_handler.setFormatter(formatter)
    file_handler.setLevel(logging.DEBUG)
    
    root = logging.getLogger()
    root.setLevel(logging.DEBUG)
    root.addHandler(stdout_handler)
    root.addHandler(file_handler)

    
if __name__ == "__main__":
    ROOT_PATH = "/home/popina/Figaro/figaro-code"
    init_logging()
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--password", action="store",  
                        dest="password", required=True)
    args = parser.parse_args()
    #figaro_test()
    #normal_test(args.password)
    eval_tests(args.password)

