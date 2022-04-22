
from enum import Enum, Flag, auto
from os import path
import subprocess
import os
import shutil
import json
import logging
from data_management.database import Database
from data_management.query import Query
from evaluation.system_test.system_test import DecompConf, DumpConf, ExcecutableConf, LogConf, SystemTest
from evaluation.system_test.system_test import SystemTest
from evaluation.system_test.system_test import AccuracyConf
from evaluation.system_test.system_test import PerformanceConf

class SystemTestFigaro(SystemTest):
    def __init__(self, name: str, log_conf: LogConf, dump_conf: DumpConf,
            perf_conf: PerformanceConf, accur_conf: AccuracyConf,
            decomp_conf: DecompConf, exec_conf: ExcecutableConf, database: Database,
            query: Query, test_mode, root_path: str, *args, **kwargs):
        super().__init__(name, log_conf, dump_conf, perf_conf,
            accur_conf, decomp_conf, exec_conf, database, query, test_mode)
        self.figaro_path = os.path.join(root_path, "figaro")

    def eval(self):
        pass


    def eval(self, dump = False, profiler = False):
        compute_all = "true" if self.conf_decomp.compute_all else "false"
        postprocess_str = DecompConf.postprocess_mode_to_str(self.conf_decomp.postprocessing)
        memory_layout = DecompConf.memory_layout_to_str(self.conf_decomp.memory_layout)
        args = ["/bin/bash", "setup.sh",
                "--root_path={}".format(self.figaro_path),
                "--log_file_path={}".format(self.conf_log.file_path),
                "--db_config_path={}".format(self.database.db_config_path),
                "--query_config_path={}".format(self.query.get_conf_path()),
                "--num_threads={}".format(self.conf_perf.num_threads),
                "--postprocess={}".format(postprocess_str),
                "--compute_all={}".format(compute_all),
                "--memory_layout={}".format(memory_layout),
                "--implementation={}".format("figaro"),
                "--precision={}".format(self.conf_accur.precision),
                "--test_mode={}".format
                (SystemTest.test_mode_to_str(self.test_mode))]

        if dump:
            args.append("--dump_file_path={}".format(self.conf_dump.file_path))

        if profiler:
            profiler_path = os.path.join(self.conf_dump.path, SystemTest.test_mode_to_str(self.test_mode))
            if os.path.exists(profiler_path):
                shutil.rmtree(profiler_path)
            args.append("--profiler_dump_path={}".format(profiler_path))

        result = subprocess.run(
            args=args, cwd=self.figaro_path,
            capture_output=True, text=True, shell=False)
        logging.info(result.stdout)
        logging.error(result.stderr)


    def run_debug(self):
        self.eval()


    def run_info(self):
        self.eval()


    def run_dump(self):
        self.eval(dump=True)


    def run_accuracy(self):
        pass


    def run_performance(self):
        for rep in range(self.conf_perf.num_reps):
            logging.info("Run {} / {}".format(rep + 1, self.conf_perf.num_reps))
            self.eval()


    def run_performance_analysis(self):
        super().run_performance_analysis()

    def is_paper_algorithm(self):
        return True


    def is_dbms(self):
        return False


    def requires_dbms_result(self):
        return False


    def run_profiler(self):
        self.eval(dump=False, profiler=True)