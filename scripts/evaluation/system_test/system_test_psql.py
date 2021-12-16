import logging
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from data_management.query import Query
from evaluation.system_test.system_test import DecompConf, DumpConf, ExcecutableConf, LogConf, SystemTest
from evaluation.system_test.system_test import AccuracyConf
from evaluation.system_test.system_test import PerformanceConf
from evaluation.system_test.system_test_dbms import SystemTestDBMS
from evaluation.custom_logging import add_logging_file_handler, remove_logging_file_handler, set_logging_level


class SystemTestPsql(SystemTestDBMS):
    def __init__(self, name: str, log_conf: LogConf, dump_conf: DumpConf,
            perf_conf: PerformanceConf, accur_conf: AccuracyConf,
            decomp_conf: DecompConf, exec_conf: ExcecutableConf, database: Database, query: Query,
            test_mode: SystemTest.TestMode,
            username: str, password: str, **kwargs):
        super().__init__(name, log_conf=log_conf, dump_conf=dump_conf,
                    perf_conf=perf_conf, accur_conf = accur_conf,
                    decomp_conf=decomp_conf, exec_conf=exec_conf,
                    database = database, query= query, test_mode=test_mode)
        self.password = password
        self.username = username
        self.join_path = self.conf_dump.file_path


    def eval(self, dump: bool, performance: bool):
        log_file_path = self.conf_log.file_path
        file_handler = add_logging_file_handler(log_file_path, debug_level=logging.INFO)

        database_psql = DatabasePsql(host_name="",user_name=self.username,
        password=self.password, database=self.database)
        database_psql.drop_database()
        database_psql.create_database(self.database)

        num_repetitions = self.conf_perf.num_reps if performance else 1
        database_psql.evaluate_join(self.query, num_repetitions=num_repetitions,
            order_by=self.conf_dump.order_by)
        join_size = database_psql.get_join_size(self.query)
        logging.info("Number of rows is {}".format(join_size))
        database_psql.log_relation_sizes(self.query)
        if (dump):
            database_psql.dump_join(self.query, self.join_path)
        # Need due to memory usage.
        database_psql.drop_database()
        remove_logging_file_handler(file_handler)


    def run_debug(self):
        set_logging_level(logging.DEBUG)
        self.eval(dump=False, performance=False)


    def run_info(self):
        set_logging_level(logging.INFO)
        self.eval(dump=False, performance=False)


    def run_dump(self):
        self.eval(dump=True, performance=False)


    def run_performance(self):
        self.eval(dump=False, performance=True)


    def is_dbms(self):
        return True

    def get_join_result_path(self):
        return self.join_path

