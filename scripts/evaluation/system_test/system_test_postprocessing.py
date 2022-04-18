
from enum import Enum, auto
from os import path
import subprocess
import os
import json
import logging
from data_management.database import Database
from data_management.query import Query
from evaluation.system_test.system_test import DecompConf, DumpConf, ExcecutableConf, LogConf, SystemTest
from evaluation.system_test.system_test import SystemTest
from evaluation.system_test.system_test import AccuracyConf
from evaluation.system_test.system_test import PerformanceConf
from evaluation.system_test.system_test_competitor import SystemTestCompetitor

class SystemTestPostprocess(SystemTestCompetitor):
    def __init__(self, name: str, log_conf: LogConf, dump_conf: DumpConf,
            perf_conf: PerformanceConf, accur_conf: AccuracyConf,
            decomp_conf: DecompConf, exec_conf: ExcecutableConf, database: Database,
            query: Query, test_mode, root_path: str, *args, **kwargs):
        super().__init__(name, log_conf, dump_conf, perf_conf,
            accur_conf, decomp_conf, exec_conf, database, query, test_mode)
        self.figaro_path = os.path.join(root_path, "figaro")

    def eval(self):
        pass


    def get_database_json(self):
        non_join_attr_names = self.query.get_non_join_attr_names_ordered()
        non_join_cat_attr_names = self.query.get_non_join_cat_attr_names_ordered()
        attributes_json = []
        for non_join_attr_name in  non_join_attr_names:
            if non_join_attr_name in non_join_cat_attr_names:
                type_non_join_attr = "category"
            else:
                type_non_join_attr = "double"

            attribute_json = {"name": non_join_attr_name, "type": type_non_join_attr}
            attributes_json.append(attribute_json)

        relations_json =  [
            {"name": "JoinTable",
            "attributes": attributes_json,
            "primary_key": [],
            "data_path": self.join_path
            }]
        database_json = \
            {"database":
                {"name": self.database.get_name(), "relations": relations_json}
            }
        return database_json


    def get_query_json(self):
        query_json_s = """
        {
            "query":
            {
                "name": "FullJoin",
                "expression": "POSTPROCESS_QR(JoinTable)",
                "evaluation_hint":
                {
                    "operator": "POSTPROCESS_QR",
                    "operands":
                    [
                        {
                            "operator": "relation",
                            "relation": "JoinTable"
                        }
                    ],
                    "relation_order": ["JoinTable"],
                    "num_threads": 48
                }
            }
        }
        """
        query_json = json.loads(query_json_s)
        query_json["query"]["evaluation_hint"]["compute_all"] = \
            self.conf_decomp.compute_all
        return query_json


    def eval(self, dump = False):
        postprocess_str = DecompConf.postprocess_mode_to_str(self.conf_decomp.postprocessing)
        memory_layout = DecompConf.memory_layout_to_str(self.conf_decomp.memory_layout)
        compute_all = "true" if self.conf_decomp.compute_all else "false"

        # Generate database config
        database_json = self.get_database_json()
        # Generate query config
        query_json = self.get_query_json()

        dump_db_config_path = os.path.join(self.conf_dump.path,  "database_specs.conf")
        dump_query_config_path = os.path.join(self.conf_dump.path,  "query_specs.conf")
        with open(dump_db_config_path, 'w') as dump_db_config_file:
            logging.debug(dump_db_config_path)
            json.dump(database_json, dump_db_config_file, indent=4)

        with open(dump_query_config_path, 'w') as dump_query_config_file:
            logging.debug(dump_query_config_path)
            json.dump(query_json, dump_query_config_file, indent=4)
        args = ["/bin/bash", "setup.sh",
                "--root_path={}".format(self.figaro_path),
                "--log_file_path={}".format(self.conf_log.file_path),
                "--db_config_path={}".format(dump_db_config_path),
                "--query_config_path={}".format(dump_query_config_path),
                "--num_threads={}".format(self.conf_perf.num_threads),
                "--compute_all={}".format(compute_all),
                "--postprocess={}".format(postprocess_str),
                "--memory_layout={}".format(memory_layout),
                "--implementation={}".format("postprocess"),
                "--precision={}".format(self.conf_accur.precision),
                "--test_mode={}".format
                (SystemTest.test_mode_to_str(self.test_mode))]

        if dump:
            args.append("--dump_file_path={}".format(self.conf_dump.file_path))

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


    def run_performance(self):
        for rep in range(self.conf_perf.num_reps):
            logging.info("Run {} / {}".format(rep + 1, self.conf_perf.num_reps))
            self.eval()


    def run_performance_analysis(self):
        super().run_performance_analysis()


    def requires_dbms_result(self):
        return True