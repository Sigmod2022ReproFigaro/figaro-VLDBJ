import argparse
import json
import logging
import sys
import os
from os import listdir
from evaluation.system_test.system_test import SystemTest
from evaluation.system_test.system_test_dbms_pandas import SystemTestPandas
from evaluation.system_test.system_test_postprocessing import SystemTestPostprocess
from evaluation.system_test.system_test_figaro import SystemTestFigaro
from evaluation.system_test.system_test_psql import SystemTestPsql
from evaluation.system_test.system_test_python import SystemTestPython
from data_management.database import Database
from data_management.query import Query
from evaluation.custom_logging import init_logging
class SystemTestsEvaluator:
    map_category_to_class = {
        'psql': SystemTestPsql, 'python': SystemTestPython,
        'figaro': SystemTestFigaro, 'pandas': SystemTestPandas,
        'postprocess': SystemTestPostprocess}

    map_mode_to_enum = {
        'debug': SystemTest.TestMode.DEBUG,
        'info': SystemTest.TestMode.INFO,
        'dump': SystemTest.TestMode.DUMP,
        'accuracy': SystemTest.TestMode.ACCURACY,
        'performance': SystemTest.TestMode.PERFORMANCE,
        'performance_analysis': SystemTest.TestMode.PERFORMANCE_ANALYSIS,
        'clean': SystemTest.TestMode.CLEAN,
        'profiler_memory': SystemTest.TestMode.PROFILER_MEMORY,
        'profiler_threads': SystemTest.TestMode.PROFILER_THREADS,
        'profiler_hotspots': SystemTest.TestMode.PROFILER_HOTSPOTS
        }


    def __init__(self, tests_conf: str, root_path: str, username: str, password: str = None, subtest: str = None):
        with open(tests_conf) as json_file:
            tests_json = json.load(json_file)

        self.root_path = root_path
        self.username = username
        self.password = password
        self.subtest = subtest
        self.load_tests(tests_json)


    def load_tests(self, tests_json):
        tests = []
        for test_json in tests_json["tests"]:
            if test_json.get('disable', False):
                continue
            if (self.subtest is not None) and \
                (test_json["name"] !=  self.subtest):
                continue

            test_path_conf = test_json["test_path_conf"]
            sub_tests = self.load_test(test_path_conf)
            tests += sub_tests

        self.tests = tests


    def load_test(self, test_path_conf: str):
        with open(test_path_conf, "r") as test_conf_file:
            test_json = json.load(test_conf_file)

        logging.debug("Loaded subtest{}".format(test_path_conf))
        system_tests_json = test_json["systems"]
        data_sets_json_conf = test_json["data_sets"]
        data_sets_json = []
        with open(data_sets_json_conf["dataset_conf_path"],
            "r") as ds_file:
            data_sets_json = json.load(ds_file)["data_sets"]

        tests = self.load_system_tests_data_sets(
                    system_tests_json, data_sets_json)
        return tests


    def load_system_tests_data_sets(self, system_tests_json, data_sets_json):
        test = []
        for data_set_json in  data_sets_json:
            if "database_conf_path" in data_set_json:
                database_conf_path = data_set_json["database_conf_path"]
                database = Database(database_conf_path, "")
                query_conf_path = data_set_json["query_conf_path"]
                query = Query(query_conf_path, database)
                data_set_enabled = not data_set_json.get("disable", False)
            else:
                logging.error("TODO")

            if data_set_enabled:
                test += self.load_system_tests(system_tests_json, database, query)

        return test


    def load_system_tests(self, system_tests_json, database: Database,
            query: Query):
        join_result_path = None
        dict_name_system_test_papers = {}
        batch_of_tests = []
        for system_test_json in system_tests_json:
            system_test_enabled = True
            if "system_conf_path" in system_test_json:
                system_test_enabled = not system_test_json.get("disable", False)
                system_test = self.create_system_test(system_test_json, database, query)
                if system_test.is_dbms():
                    join_result_path = system_test.get_join_result_path()
                if system_test.is_paper_algorithm():
                    dict_name_system_test_papers[system_test.name] = system_test
            else:
                logging.error("TODO")

            if system_test_enabled:
                batch_of_tests.append(system_test)

        for system_test in batch_of_tests:
            if system_test.requires_dbms_result():
                system_test.set_join_result_path(join_result_path)

            if not system_test.is_paper_algorithm():
                system_test.set_paper_system_test(list(dict_name_system_test_papers.values()))

        return batch_of_tests


    def create_system_test(self, system_test_json, database: Database,
         query: Query):
        system_conf_path = system_test_json["system_conf_path"]
        system_test_cat = system_test_json["category"]
        system_test_mode = system_test_json["mode"]
        class_type = SystemTestsEvaluator.map_category_to_class[system_test_cat]
        test_mode = SystemTestsEvaluator.map_mode_to_enum[system_test_mode]
        system_test = class_type.from_specs_path(system_conf_path, database, query,
            test_mode=test_mode, root_path = self.root_path, username=self.username,
            password=self.password)

        logging.debug("Category is{}".format(system_test_cat))
        logging.debug("Created category {}".format(type(system_test)))
        logging.debug("Mode is{}".format(system_test_mode))
        logging.debug("Create mode{}".format(system_test_mode))

        return system_test


    def eval_tests(self):
        for system_test in self.tests:
            logging.debug("system_test {}".format(system_test))
            system_test.run()



def eval_tests(root_path: str, username: str, password: str, test_conf_path: str, subtest: str):
    system_tests_evaluator = SystemTestsEvaluator(test_conf_path, root_path,
        username, password, subtest)
    system_tests_evaluator.eval_tests()


def get_all_test_specs_paths(system_tests_path: str, test_num):
    test_conf_paths = []
    test_specs_dir = system_tests_path
    if test_num is None:
        for test_path in listdir(test_specs_dir):
            test_specs_path = os.path.join(test_specs_dir, test_path, "tests_specs.conf")
            logging.info(test_specs_path)
            test_conf_paths.append(test_specs_path)
    else:
        test_specs_path = os.path.join(test_specs_dir, "test{}".format(test_num), "tests_specs.conf")
        test_conf_paths.append(test_specs_path)

    return test_conf_paths


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--password", action="store",
                        dest="password", required=True)
    parser.add_argument("-u", "--username", action="store",
                        dest="username", required=True)
    parser.add_argument("-r", "--root", action="store",
                        dest="root_path", required=False)
    parser.add_argument("-s", "--system_tests_path", action="store",
                        dest="system_tests_path", required=True)
    parser.add_argument("-t", "--test", action="store",
                        dest="test", required=False)
    parser.add_argument("--subtest", action="store",
                        dest="subtest", required=False)
    args = parser.parse_args(args)
    root_path = args.root_path
    system_tests_path = args.system_tests_path

    test_conf_paths = get_all_test_specs_paths(system_tests_path, args.test)
    for test_conf_path in test_conf_paths:
        logging.info("Running test specified in the path {}".format(test_conf_path))
        eval_tests(root_path, args.username, args.password, test_conf_path, args.subtest)


if __name__ == "__main__":
    init_logging()
    main(sys.argv[1:])

