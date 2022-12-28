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
from data_management.query import Query
from evaluation.performance.benchmark import gather_times

# Class that wraps performance parameters used in testing
class PerformanceConf:
    def __init__(self, glob_path: str, path: str, num_reps: int, num_threads: int):
        self.path = path
        self.num_reps = num_reps
        self.glob_path = glob_path
        self.num_threads = num_threads



# Class that wraps precisions elements
class AccuracyConf:
    def __init__(self, path: str, r_comp_file_path: str,
                errors_file_path: str, precision: float,
                generate_xlsx: bool):
        self.path = path
        self.r_comp_file_path = r_comp_file_path
        self.errors_file_path = errors_file_path
        self.precision = precision
        self.generate_xlsx = generate_xlsx


class LogConf:
    def __init__(self, path: str, file_path: str):
        self.path = path
        self.file_path = file_path


class DumpConf:
    class OrderRelation(IntEnum):
        RANDOM = 1
        JOIN_ATTRIBUTE = 2


    def __init__(self, path: str, file_path: str, order_by: str):
        self.path = path
        self.file_path  = file_path
        self.order_by = self.map_order_by_to_enum[order_by]


    map_order_by_to_enum = {
        'random': OrderRelation.RANDOM,
        'join_attribute': OrderRelation.JOIN_ATTRIBUTE,
    }


class DecompConf:
    class Name(IntEnum):
        QR = 1
        LU = 2
        SVD = 3


    class Method(IntEnum):
        GIV_THIN_DIAG = 1
        HOUSEHOLDER = 2
        JACOBI = 3
        POWER_ITER = 4
        EIGEN_DECOMP_DIV_AND_CONQ = 5
        EIGEN_DECOMP_QR_ITER = 6
        EIGEN_DECOMP_RRR = 7
        QR = 8


    class SparsityMode(IntEnum):
        SPARSE = 1
        DENSE = 2


    class MemoryLayout(IntEnum):
        ROW_MAJOR = 1
        COL_MAJOR = 2


    map_memory_layout_to_str = {
        MemoryLayout.ROW_MAJOR : "ROW_MAJOR",
        MemoryLayout.COL_MAJOR : "COL_MAJOR"
    }

    map_method_to_enum = {
        'giv_thin_diag': Method.GIV_THIN_DIAG,
        'householder': Method.HOUSEHOLDER,
        'jacobi': Method.JACOBI,
        'power_iter': Method.POWER_ITER,
        'eigen_decomp_div_and_conq': Method.EIGEN_DECOMP_DIV_AND_CONQ,
        'eigen_decomp_qr_iter': Method.EIGEN_DECOMP_QR_ITER,
        'eigen_decomp_rrr': Method.EIGEN_DECOMP_RRR,
        'qr': Method.QR
    }

    map_method_to_str = {
        Method.GIV_THIN_DIAG: 'giv_thin_diag',
        Method.HOUSEHOLDER: 'householder',
        Method.JACOBI: 'jacobi',
        Method.POWER_ITER: 'power_iter',
        Method.EIGEN_DECOMP_DIV_AND_CONQ: 'eigen_decomp_div_and_conq',
        Method.EIGEN_DECOMP_QR_ITER: 'eigen_decomp_qr_iter',
        Method.EIGEN_DECOMP_RRR: 'eigen_decomp_rrr',
        Method.QR: 'qr'
    }

    map_name_to_enum  = {
        'qr': Name.QR,
        'lu': Name.LU,
        'svd': Name.SVD
    }


    map_sparsity_to_enum = {
        'sparse': SparsityMode.SPARSE,
        'dense': SparsityMode.DENSE
    }

    map_memory_layout_to_enum = {
        'row_major': MemoryLayout.ROW_MAJOR,
        'col_major': MemoryLayout.COL_MAJOR
    }


    @staticmethod
    def memory_layout_to_str(memory_layout: MemoryLayout)->str:
        return DecompConf.map_memory_layout_to_str[memory_layout]


    @staticmethod
    def method_to_str(method: Method)->str:
        return DecompConf.map_method_to_str[method]


    def __init__(self, sparsity: str,
        memory_layout: str, compute_all: bool,
        name: str, method: str):
        self.sparsity = DecompConf.map_sparsity_to_enum[sparsity]
        self.memory_layout = DecompConf.map_memory_layout_to_enum[memory_layout]
        self.compute_all = compute_all
        self.method  = DecompConf.map_method_to_enum[method]
        self.name = DecompConf.map_name_to_enum[name]



class ExcecutableConf:
    def __init__(self, interpreter_path: str = None):
        self.interpreter_path = interpreter_path


class SystemTest(ABC):
    # Debug test is only for debugging
    # Dump is used for accuracy where data is later compared.
    # Performance is used to run successive tests for comparisons.
    # Perofrmance analysis evaluates speed of the algorithm
    # Accuracy compares data from dumps
    class TestMode(IntEnum):
        DEBUG = 1
        INFO = 2
        DUMP = 3
        PERFORMANCE = 4
        ACCURACY = 5
        PERFORMANCE_ANALYSIS = 6
        CLEAN = 7
        MICRO_BENCHMARK = 8
        PROFILER_MEMORY = 9
        PROFILER_THREADS = 10
        PROFILER_HOTSPOTS = 11


    map_mode_to_str = {TestMode.DEBUG : "DEBUG",
                    TestMode.INFO : "INFO",
                    TestMode.DUMP: "DUMP",
                    TestMode.PERFORMANCE: "PERFORMANCE",
                    TestMode.PERFORMANCE_ANALYSIS: "PERFORMANCE_ANALYSIS",
                    TestMode.ACCURACY: "ACCURACY",
                    TestMode.CLEAN: "CLEAN",
                    TestMode.MICRO_BENCHMARK: "MICRO_BENCHMARK",
                    TestMode.PROFILER_MEMORY: "PROFILER_MEMORY",
                    TestMode.PROFILER_THREADS: "PROFILER_THREADS",
                    TestMode.PROFILER_HOTSPOTS: "PROFILER_HOTSPOTS"}


    @staticmethod
    def test_mode_to_str(test_mode: TestMode)->str:
        return SystemTest.map_mode_to_str[test_mode]


    def __init__(self, name, log_conf: LogConf, dump_conf: DumpConf,
    perf_conf: PerformanceConf, accur_conf: AccuracyConf, decomp_conf: DecompConf,
    exec_conf: ExcecutableConf,  database: Database,
    query: Query, test_mode = TestMode.PERFORMANCE):
        self.name = name
        self.conf_accur = accur_conf
        self.conf_perf = perf_conf
        self.conf_log = log_conf
        self.conf_dump = dump_conf
        self.conf_decomp = decomp_conf
        self.conf_exec = exec_conf
        self.database = database
        self.query = query
        self.test_mode = test_mode
        self.system_test_papers = []


    @staticmethod
    def create_dir_with_name(path: str, dir_name: str, sub_dir_name: str)-> str:
        dir_abs_path = os.path.join(path, dir_name, sub_dir_name)

        if not os.path.exists(dir_abs_path):
            os.makedirs(dir_abs_path)

        return dir_abs_path


    @classmethod
    def from_specs_path(cls, system_test_specs_path: str,
        database: Database, query: Query, test_mode: TestMode, *args, **kwargs):
        with open(system_test_specs_path) as json_file:
            system_json = json.load(json_file)

        name = system_json["system"]["name"]

        #TODO: Refactor to remove unnecessary clutter.
        log_json = system_json["system"]["log"]
        log_path = SystemTest.create_dir_with_name(
                    log_json["path"], database.get_name(), query.get_name())
        log_file_path = os.path.join(log_path, log_json["file"])

        dump_json = system_json["system"]["dump"]
        path_dump = SystemTest.create_dir_with_name(
            dump_json["path"], database.get_name(), query.get_name())
        dump_file_path = os.path.join(path_dump, dump_json["file"])
        order_by = dump_json.get("order_by", "join_attribute")

        perf_json = system_json["system"]["performance"]
        path_glob = perf_json["path"]
        path_perf = SystemTest.create_dir_with_name(
                        perf_json["path"], database.get_name(), query.get_name())
        num_reps = perf_json["number_reps"]
        num_threads = perf_json.get("num_threads", 1)

        accuracy_json = system_json["system"]["accuracy"]
        path_accuracy = SystemTest.create_dir_with_name(
            accuracy_json["path"],  database.get_name(), query.get_name())
        path_errors_file = os.path.join(path_accuracy,
                            accuracy_json["errors_file"])
        path_r_comp_file =os.path.join(path_accuracy,
                            accuracy_json["r_comp_file"])
        precision = accuracy_json["precision"]
        generate_xlsx = accuracy_json.get("generate_xlsx", True)

        decomp_json = system_json["system"]["decomposition"]
        decomp_name = decomp_json.get("name", "qr")
        method = decomp_json.get("method", "lapack")
        memory_layout = decomp_json.get("memory_layout", "row_major")
        sparsity = decomp_json.get("sparsity", "dense")
        compute_all = bool(decomp_json.get("compute_all", False))

        executable_json = system_json["system"].get("executable", {})
        interpreter = executable_json.get("interpreter", "")


        system_test = cls(
            name,
            LogConf(log_path, log_file_path),
            DumpConf(path_dump, dump_file_path, order_by),
            PerformanceConf(path_glob, path_perf, num_reps, num_threads),
            AccuracyConf(path_accuracy, path_r_comp_file, path_errors_file, precision, generate_xlsx),
            DecompConf(sparsity,
                memory_layout, compute_all, decomp_name, method),
            ExcecutableConf(interpreter),
            database, query, test_mode,
            *args, **kwargs)

        return system_test

    def run(self):
        if (self.conf_decomp.name == DecompConf.Name.QR):
            q_str = "Q and R" if (self.conf_decomp.compute_all) else "R"
            decomp_str = q_str + " in a QR decomposition"
        elif (self.conf_decomp.name == DecompConf.Name.LU):
            decomp_str = "L and U in an LU decomposition"
        elif (self.conf_decomp.name == DecompConf.Name.SVD):
            q_str = "U, Sigma and V " if (self.conf_decomp.compute_all) else "Sigma and V"
            decomp_str = q_str + " in an SVD decomposition"
        info_str = "Run {{mode}} for database {db_name} and query {query_name} and {decomp_str}".format(
            db_name=self.database.get_name(), query_name=self.query.get_name(),
            decomp_str=decomp_str)

        logging.info("Starting cleaning old data")
        self.clean_data(self.test_mode)
        logging.info("End cleaning old data")

        logging.info("Starting of test {}".format(self.name))
        if self.test_mode == SystemTest.TestMode.DEBUG:
            logging.info(info_str.format(mode="debug"))
            self.run_debug()
        elif self.test_mode == SystemTest.TestMode.INFO:
            logging.info(info_str.format(mode="info"))
            self.run_info()
        elif self.test_mode == SystemTest.TestMode.DUMP:
            logging.info(info_str.format(mode="dump"))
            self.run_dump()
        elif self.test_mode == SystemTest.TestMode.ACCURACY:
            logging.info(info_str.format(mode="accuracy"))
            self.run_accuracy()
            logging.info("End accuracy")
        elif self.test_mode == SystemTest.TestMode.PERFORMANCE:
            logging.info(info_str.format(mode="performance"))
            self.run_performance()
        elif self.test_mode == SystemTest.TestMode.PERFORMANCE_ANALYSIS:
            logging.info(info_str.format(mode="performance analysis"))
            self.run_performance_analysis()
        elif self.test_mode == SystemTest.TestMode.CLEAN:
            logging.info(info_str.format(mode="cleaning"))
            self.clean_data(SystemTest.TestMode.DUMP)
        elif self.test_mode == SystemTest.TestMode.MICRO_BENCHMARK:
            logging.info(info_str.format(mode="micro benchmarks"))
            self.run_microbenchmark()
        elif self.test_mode == SystemTest.TestMode.PROFILER_MEMORY:
            logging.info(info_str.format(mode="profiler memory"))
            self.run_profiler()
        elif self.test_mode == SystemTest.TestMode.PROFILER_THREADS:
            logging.info(info_str.format(mode="profiler threads"))
            self.run_profiler()
        elif self.test_mode == SystemTest.TestMode.PROFILER_HOTSPOTS:
            logging.info(info_str.format(mode="profiler hotspots"))
            self.run_profiler()
        else:
            logging.error('This type of system test does not exist')
        logging.info("End of test {}".format(self.name))


    @abstractmethod
    def run_debug(self):
        pass


    @abstractmethod
    def run_info(self):
        pass


    @abstractmethod
    def run_dump(self):
        pass


    @abstractmethod
    def run_accuracy(self):
        pass


    @abstractmethod
    def run_microbenchmark(self):
        pass


    @abstractmethod
    def run_performance(self):
        pass


    def run_performance_analysis(self):
        path_log_file = self.conf_log.file_path
        path_times_file = os.path.join(self.conf_perf.path, "time.xlsx")
        path_glob_db_times_file = os.path.join(self.conf_perf.glob_path, self.database.get_name(), "time.xlsx")
        path_glob_times_file = os.path.join(self.conf_perf.glob_path, "time.xlsx")
        #db_query_name = self.database.get_name() + self.query.get_name()
        # Gathes all times in local xlsx
        gather_times(path_log_file, path_times_file, self.query.get_name())
        logging.info(path_glob_times_file)
        # Gathers all times in globac xlsx
        gather_times(path_log_file, path_glob_db_times_file, self.query.get_name())


    @abstractmethod
    def is_dbms(self):
        pass


    @abstractmethod
    def requires_dbms_result(self):
        pass


    #@abstractmethod
    def is_paper_algorithm(self):
        return False


    def set_paper_system_test(self, system_test_papers: list):
        self.system_test_papers = system_test_papers


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
            self.delete_content_of_dir(self.conf_dump.path)
        elif test_mode == SystemTest.TestMode.PERFORMANCE_ANALYSIS:
            self.delete_content_of_dir(self.conf_perf.path)

        if (test_mode == SystemTest.TestMode.PERFORMANCE) or\
            (test_mode == SystemTest.TestMode.DUMP) or \
                (test_mode == SystemTest.TestMode.MICRO_BENCHMARK):
            self.delete_content_of_dir(self.conf_log.path)



    def clean_all_data(self):
        for test_mode in SystemTest.TestMode:
            self.clean_data(test_mode)

    def run_profiler(self):
        pass