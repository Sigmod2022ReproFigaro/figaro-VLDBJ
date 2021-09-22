import logging
from data_management.database import Database
from data_management.database_pandas import DatabasePandas
from data_management.database_psql import DatabasePsql
from data_management.query import Query
from evaluation.system_test.system_test import DecompConf, DumpConf, ExcecutableConf, LogConf, SystemTest
from evaluation.system_test.system_test import AccuracyConf
from evaluation.system_test.system_test import PerformanceConf
from evaluation.system_test.system_test_dbms import SystemTestDBMS
from evaluation.custom_logging import add_logging_file_handler, remove_logging_file_handler


class SystemTestPandas(SystemTestDBMS):
    def __init__(self, name: str, log_conf: LogConf, dump_conf: DumpConf,
            perf_conf: PerformanceConf, accur_conf: AccuracyConf,
            decomp_conf: DecompConf, exec_conf: ExcecutableConf,
            database: Database, query: Query,
            test_mode: SystemTest.TestMode,
            username: str, password: str, **kwargs):
        super().__init__(name, log_conf=log_conf, dump_conf=dump_conf,
                    perf_conf=perf_conf, accur_conf = accur_conf,
                    decomp_conf = decomp_conf, exec_conf = exec_conf,
                    database = database, query= query,
                    test_mode=test_mode)
        self.password = password
        self.username = username
        self.join_path = self.conf_dump.file_path


    def eval(self, dump: bool, performance: bool):
        database_pandas = DatabasePandas(database=self.database)
        num_repetitions = self.conf_perf.num_reps if performance else 1
        database_pandas.evaluate_join(self.query,
            num_repetitions=num_repetitions)
        if (dump):
            database_pandas.dump_join(self.query, self.join_path)


    def run_debug(self):
        self.eval()


    def run_info(self):
        self.eval()


    def run_dump(self):
        self.eval(dump=True, performance=False)


    def run_performance(self):
        self.eval(dump=False, performance=True)


    def is_dbms(self):
        return True

    def get_join_result_path(self):
        return self.join_path

