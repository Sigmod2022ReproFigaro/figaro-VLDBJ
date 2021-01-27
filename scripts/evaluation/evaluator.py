import argparse
import json
import logging
import sys
import os
from os import listdir
from evaluation.system_test import SystemTest
from evaluation.system_test_figaro import SystemTestFigaro
from evaluation.system_test_psql import SystemTestPsql
from evaluation.system_test_python import SystemTestPython
from data_management.database import Database
from evaluation.custom_logging import init_logging
class SystemTestsEvaluator:
    map_category_to_class = {
        'psql': SystemTestPsql, 'python': SystemTestPython,
        'figaro': SystemTestFigaro}

    map_mode_to_enum = {
        'dump': SystemTest.TestMode.DUMP,
        'accuracy': SystemTest.TestMode.ACCURACY,
        'performance': SystemTest.TestMode.PERFORMANCE,
        'performance_analysis': SystemTest.TestMode.PERFORMANCE_ANALYSIS,
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
                database = Database(database_conf_path)
                data_set_enabled = not data_set_json.get("disable", False)
            else:
                logging.error("TODO")
            
            if data_set_enabled:
                test += self.load_system_tests(system_tests_json, database)
        
        return test


    def load_system_tests(self, system_tests_json, database: Database):
        join_result_path = None
        system_test_paper = None
        batch_of_tests = []
        for system_test_json in system_tests_json:
            system_test_enabled = True
            if "system_conf_path" in system_test_json:
                system_test_enabled = not system_test_json.get("disable", False)
                system_test = self.create_system_test(system_test_json, database)
                if system_test.is_dbms():
                    join_result_path = system_test.get_join_result_path()
                if system_test.is_paper_algorithm():
                    system_test_paper = system_test
            else: 
                logging.error("TODO")
            
            if system_test_enabled:
                batch_of_tests.append(system_test)
            
        for system_test in batch_of_tests:
            if system_test.requires_dbms_result():
                system_test.set_join_result_path(join_result_path)
            
            if not system_test.is_paper_algorithm():
                system_test.set_paper_system_test(system_test_paper)
        
        return batch_of_tests


    def create_system_test(self, system_test_json, database: Database):
        system_conf_path = system_test_json["system_conf_path"]
        system_test_cat = system_test_json["category"]
        system_test_mode = system_test_json["mode"]
        class_type = SystemTestsEvaluator.map_category_to_class[system_test_cat]
        test_mode = SystemTestsEvaluator.map_mode_to_enum[system_test_mode]
        system_test = class_type.from_specs_path(system_conf_path, database, 
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



def eval_tests(password: str, test_conf_path: str):
    system_tests_evaluator = SystemTestsEvaluator(test_conf_path, password)
    system_tests_evaluator.eval_tests()


def get_all_test_specs_paths(test_num):
    test_conf_paths = []
    test_specs_dir = "/home/popina/Figaro/figaro-code/system_tests/"
    if test_num is None:
        for test_path in listdir(test_specs_dir):
            test_specs_path = os.path.join(test_specs_dir, test_path, "tests_specs.conf")
            logging.info(test_specs_path)
            test_conf_paths.append(test_specs_path)
    else:
        test_specs_path = os.path.join(test_specs_dir, "test{}".format(test_num), "tests_specs.conf")
        test_conf_paths.append(test_specs_path)

    return test_conf_paths


if __name__ == "__main__":
    ROOT_PATH = "/home/popina/Figaro/figaro-code"

    init_logging()
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--password", action="store",  
                        dest="password", required=True)
    parser.add_argument("-t", "--test", action="store", 
                        dest="test", required=False)
    args = parser.parse_args()

    test_conf_paths = get_all_test_specs_paths(args.test)
    for test_conf_path in test_conf_paths:
        logging.info("Running test specified in the path {}".format(test_conf_path))
        eval_tests(args.password, test_conf_path)

