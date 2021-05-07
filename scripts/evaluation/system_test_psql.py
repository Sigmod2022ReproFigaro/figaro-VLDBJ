import logging
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from data_management.query import Query
from evaluation.system_test import DumpConf, LogConf, SystemTest
from evaluation.system_test import AccuracyConf
from evaluation.system_test import PerformanceConf
from evaluation.system_test_dbms import SystemTestDBMS
from evaluation.custom_logging import add_logging_file_handler, remove_logging_file_handler


class SystemTestPsql(SystemTestDBMS):
    def __init__(self, log_conf: LogConf, dump_conf: DumpConf,
            perf_conf: PerformanceConf, accur_conf: AccuracyConf,
            database: Database, query: Query,
            test_mode: SystemTest.TestMode,
            username: str, password: str, **kwargs):
        super().__init__("PSQL", log_conf=log_conf, dump_conf=dump_conf,
                    perf_conf=perf_conf, accur_conf = accur_conf,
                    database = database, query= query,
                    test_mode=test_mode)
        self.password = password
        self.username = username
        self.join_path = self.conf_dump.file_path


    def eval(self, dump: bool, performance: bool):
        log_file_path = self.conf_log.file_path
        file_handler = add_logging_file_handler(log_file_path, debug_level=logging.INFO)

        database_psql = DatabasePsql(host_name="",user_name=self.username,
        password=self.password, database_name=self.database.get_name())
        database_psql.drop_database()
        database_psql.create_database(self.database)

        relation_order = self.query.get_relation_order()
        skip_attrs = self.query.get_skip_attrs()
        self.database.sort_relations(relation_order)
        self.database.set_join_attrs()
        logging.info(relation_order)
        logging.info(skip_attrs)


        num_repetitions = self.conf_perf.num_reps if performance else 1
        database_psql.evaluate_join(self.database.get_relations(), num_repetitions=num_repetitions, drop_attributes=skip_attrs)
        join_size = database_psql.get_join_size()
        logging.info("Number of rows is {}".format(join_size))
        database_psql.log_relation_sizes(self.database.get_relation_names())
        if (dump):
            database_psql.dump_join(self.database.get_relations(),
                skip_attrs, self.join_path)

        remove_logging_file_handler(file_handler)



    def run_debug(self):
        self.eval()


    def run_dump(self):
        self.eval(dump=True, performance=False)


    def run_performance(self):
        self.eval(dump=False, performance=True)


    def is_dbms(self):
        return True

    def get_join_result_path(self):
        return self.join_path

