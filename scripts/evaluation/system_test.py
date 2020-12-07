# Define a class that wraps a system test where system represents 
# either competitors or Figaro method. 
# paths: log,  - build logsalongside output 
#        dump, - dump data 
#        comp/perf - performance comparison
#        comp/prec - precision comparsion
# 

from enum import IntEnum, auto
from abc import ABC, abstractmethod
import abc
import subprocess
import logging
import os
import json
import argparse
import typing
import shutil
from  collections import deque
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from data_management.database_psql import JOIN_TABLE_NAME

# Class that wraps performance parameters used in testing
class PerformanceConf:
    def __init__(self, path: str):
        self.path = path


# Class that wraps precisions elements 
class AccuracyConf:
    def __init__(self, path: str, precision: float):
        self.path = path
        self.precision = precision


class SystemTest(ABC):
    # Log test is only for debugging 
    # Dump is used for performance where data is later compared
    # PerformanceConf evaluates speed of the algorithm 
    # AccuracyConf compares data from dumps 
    class TestMode(IntEnum): 
        DEBUG = 1
        DUMP = 2
        PERFORMANCE = 3
        ACCURACY = 4

    map_mode_to_str = {TestMode.DEBUG : "DEBUG", TestMode.DUMP: "DUMP", 
                    TestMode.PERFORMANCE: "PERFORMANCE", 
                    TestMode.ACCURACY: "ACCURACY"}

    @staticmethod
    def test_mode_to_str(test_mode: TestMode)->str: 
        return SystemTest.map_mode_to_str[test_mode]


    def __init__(self, name, path_log: str, path_dump: str, 
    perf_conf: PerformanceConf, accur_conf: AccuracyConf, database: Database,
    test_mode = TestMode.PERFORMANCE):
        self.name = name
        self.conf_accur = accur_conf
        self.conf_perf = perf_conf
        self.path_log = path_log
        self.path_dump = path_dump
        self.database = database
        self.test_mode = test_mode
        self.system_test_paper = None
        self.clean_data(test_mode)


    @staticmethod
    def create_dir_with_name(path: str, dir_name: str)-> str:
        dir_abs_path = os.path.join(path, dir_name)
        
        if not os.path.exists(dir_abs_path):
            os.makedirs(dir_abs_path)

        return dir_abs_path


    @classmethod
    def from_specs_path(cls, system_test_specs_path: str,
        database: Database, test_mode: TestMode, *args, **kwargs):
        with open(system_test_specs_path) as json_file:
            system_json = json.load(json_file)
        
        #TODO: Refactor to remove unnecesary clutter. 
        path_log = SystemTest.create_dir_with_name(
            system_json["system"]["log"]["path"], database.name)
        path_dump = SystemTest.create_dir_with_name(
            system_json["system"]["dump"]["path"], database.name)
        accuracy_json = system_json["system"]["accuracy"]
        path_accuracy = SystemTest.create_dir_with_name(
            accuracy_json["path"], database.name)
        precision = accuracy_json["precision"]
        system_test = cls(path_log, path_dump, 
                PerformanceConf(""), 
                AccuracyConf(path_accuracy, precision), 
                database, test_mode, 
                *args, **kwargs)

        return system_test

    def run(self):
        logging.info("Starting of test {}".format(self.name))
        if self.test_mode == SystemTest.TestMode.DEBUG:
            logging.info("Run debug")
            self.run_debug()
        elif self.test_mode == SystemTest.TestMode.DUMP:
            logging.info("Run dump")
            self.run_dump()
        elif self.test_mode == SystemTest.TestMode.ACCURACY:
            logging.info("Run precision")
            self.run_accuracy()
            logging.info("End precision")
        elif self.test_mode == SystemTest.TestMode.PERFORMANCE:
            logging.info("Run performance")
            self.run_performance()
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
