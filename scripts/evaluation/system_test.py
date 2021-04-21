# Define a class that wraps a system test where system represents
# either competitors or Figaro method.
# paths: log,  - build logsalongside output
#        dump, - dump data
#        comp/perf - performance comparison
#        comp/prec - precision comparsion
#

from enum import IntEnum
from abc import ABC, abstractmethod
import abc
import logging
import os
import json
import typing
import shutil
from data_management.database import Database
from evaluation.performance.benchmark import gather_times

# Class that wraps performance parameters used in testing
class PerformanceConf:
    def __init__(self, glob_path: str, path: str, num_reps: int):
        self.path = path
        self.num_reps = num_reps
        self.glob_path = glob_path



# Class that wraps precisions elements
class AccuracyConf:
    def __init__(self, path: str, r_comp_file_path: str,
                errors_file_path: str, precision: float):
        self.path = path
        self.r_comp_file_path = r_comp_file_path
        self.errors_file_path = errors_file_path
        self.precision = precision


class LogConf:
    def __init__(self, path: str, file_path: str):
        self.path = path
        self.file_path = file_path


class DumpConf:
    def __init__(self, path: str, file_path: str):
        self.path = path
        self.file_path  = file_path


class QueryConf:
    def __init__(self, path: str):
        self.path = path

class SystemTest(ABC):
    # Debug test is only for debugging
    # Dump is used for accuracy where data is later compared.
    # Performance is used to run successive tests for comparisons.
    # Perofrmance analysis evaluates speed of the algorithm
    # Accuracy compares data from dumps
    class TestMode(IntEnum):
        DEBUG = 1
        DUMP = 2
        PERFORMANCE = 3
        ACCURACY = 4
        PERFORMANCE_ANALYSIS = 5

    map_mode_to_str = {TestMode.DEBUG : "DEBUG", TestMode.DUMP: "DUMP",
                    TestMode.PERFORMANCE: "PERFORMANCE",
                    TestMode.PERFORMANCE_ANALYSIS: "PERFORMANCE_ANALYSIS",
                    TestMode.ACCURACY: "ACCURACY"}

    @staticmethod
    def test_mode_to_str(test_mode: TestMode)->str:
        return SystemTest.map_mode_to_str[test_mode]


    def __init__(self, name, log_conf: LogConf, dump_conf: DumpConf,
    perf_conf: PerformanceConf, accur_conf: AccuracyConf, database: Database,
    query_conf: QueryConf, test_mode = TestMode.PERFORMANCE):
        self.name = name
        self.conf_accur = accur_conf
        self.conf_perf = perf_conf
        self.conf_log = log_conf
        self.conf_dump = dump_conf
        self.database = database
        self.conf_query = query_conf
        self.test_mode = test_mode
        self.system_test_paper = None


    @staticmethod
    def create_dir_with_name(path: str, dir_name: str)-> str:
        dir_abs_path = os.path.join(path, dir_name)

        if not os.path.exists(dir_abs_path):
            os.makedirs(dir_abs_path)

        return dir_abs_path


    @classmethod
    def from_specs_path(cls, system_test_specs_path: str,
        database: Database, query_conf: QueryConf, test_mode: TestMode, *args, **kwargs):
        with open(system_test_specs_path) as json_file:
            system_json = json.load(json_file)

        #TODO: Refactor to remove unnecessary clutter.
        log_json = system_json["system"]["log"]
        log_path = SystemTest.create_dir_with_name(
                    log_json["path"], database.name)
        log_file_path = os.path.join(log_path, log_json["file"])

        dump_json = system_json["system"]["dump"]
        path_dump = SystemTest.create_dir_with_name(
            dump_json["path"], database.name)
        dump_file_path = os.path.join(path_dump, dump_json["file"])

        perf_json = system_json["system"]["performance"]
        path_glob = perf_json["path"]
        path_perf = SystemTest.create_dir_with_name(
                        perf_json["path"], database.name)
        num_reps = perf_json["number_reps"]

        accuracy_json = system_json["system"]["accuracy"]
        path_accuracy = SystemTest.create_dir_with_name(
            accuracy_json["path"],  database.name)
        path_errors_file = os.path.join(path_accuracy,
                            accuracy_json["errors_file"])
        path_r_comp_file =os.path.join(path_accuracy,
                            accuracy_json["r_comp_file"])
        precision = accuracy_json["precision"]

        system_test = cls(
            LogConf(log_path, log_file_path),
            DumpConf(path_dump, dump_file_path),
            PerformanceConf(path_glob, path_perf, num_reps),
            AccuracyConf(path_accuracy, path_r_comp_file, path_errors_file, precision),
            database, query_conf, test_mode,
            *args, **kwargs)

        return system_test

    def run(self):
        logging.info("Cleaning old data")
        #self.clean_data(self.test_mode)
        logging.info("Old data cleaned")
        logging.info("Starting of test {}".format(self.name))
        if self.test_mode == SystemTest.TestMode.DEBUG:
            logging.info("Run debug")
            self.run_debug()
        elif self.test_mode == SystemTest.TestMode.DUMP:
            logging.info("Run dump")
            self.run_dump()
        elif self.test_mode == SystemTest.TestMode.ACCURACY:
            logging.info("Run accuracy")
            self.run_accuracy()
            logging.info("End accuracy")
        elif self.test_mode == SystemTest.TestMode.PERFORMANCE:
            logging.info("Run performance")
            self.run_performance()
        elif self.test_mode == SystemTest.TestMode.PERFORMANCE_ANALYSIS:
            logging.info("Run performance analysis")
            self.run_performance_analysis()
        else:
            logging.error('This type of system test does not exist')
        logging.info("End of test {}".format(self.name))


    @abstractmethod
    def run_debug(self):
        pass


    @abstractmethod
    def run_dump(self):
        pass


    @abstractmethod
    def run_accuracy(self):
        pass


    @abstractmethod
    def run_performance(self):
        pass


    def run_performance_analysis(self):
        path_log_file = self.conf_log.file_path
        path_times_file = os.path.join(self.conf_perf.path, "time.xlsx")
        path_glob_times_file = os.path.join(self.conf_perf.glob_path, "time.xlsx")
        gather_times(path_log_file, path_times_file, self.database.name)
        logging.info(path_glob_times_file)
        gather_times(path_log_file, path_glob_times_file, self.database.name)


    @abstractmethod
    def is_dbms(self):
        pass


    @abstractmethod
    def requires_dbms_result(self):
        pass


    #@abstractmethod
    def is_paper_algorithm(self):
        return False


    def set_paper_system_test(self, system_test_paper):
        self.system_test_paper = system_test_paper


    def delete_content_of_dir(self, dir):
        for file_name in os.listdir(dir):
            file_path = os.path.join(dir, file_name)
            if os.path.isfile(file_path):
                os.remove(file_path)
            elif os.path.isdir(file_path):
                shutil.rmtree(file_path)


    # Deletes all the auxilary data from the correpsonding path
    def clean_data(self, test_mode: TestMode):
        if test_mode == SystemTest.TestMode.ACCURACY:
            self.delete_content_of_dir(self.conf_accur.path)
        elif test_mode == SystemTest.TestMode.DUMP:
            self.delete_content_of_dir(self.path_dump)
        elif test_mode == SystemTest.TestMode.PERFORMANCE:
            self.delete_content_of_dir(self.conf_perf)

        self.delete_content_of_dir(self.path_log)


    def clean_all_data(self):
        for test_mode in SystemTest.TestMode:
            self.clean_data(test_mode)
