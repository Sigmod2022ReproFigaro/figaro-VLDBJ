from logging import log
import os
import argparse
import data_generators.database_generator as dg
import data_generators.relation_schema_generator as rsg
import numpy as np
import sys
from evaluation.custom_logging import init_logging


def generate_cart_prod_two_rel(db_idx, db_config_path, query_config_path, username,
    password, num_rows, num_cols, system_tests_path: str, data_path):
    db_config_path_t = db_config_path.replace("{db_idx}", str(db_idx))
    rsg.main(["--db_conf_path_output",
                db_config_path_t,
                "two_cart_prod", "--test_num", str(db_idx),
                "--r_domain_size", "1",
                "--s_domain_size", "1",
                "--r_relation_num_non_join_attrs", str(num_cols),
                "--s_relation_num_non_join_attrs", str(num_cols),
                "--r_relation_num_rows", str(num_rows),
                "--s_relation_num_rows", str(num_rows),
                "--data_path", data_path])
    full_query_join_conf = os.path.join(system_tests_path, "test_syn_perf/queries/query_full_join.conf")
    dg.main(["-d", db_config_path_t,
        "-q", full_query_join_conf,"-u", username, "-p", password])


def vary_row_and_col_num(username, password, system_tests_path: str, data_path: str):
    db_config_path = os.path.join(system_tests_path, "test_syn_perf/databases/database_specs{db_idx}.conf")
    query_config_path = os.path.join(system_tests_path, "test_syn_perf/queries/query_full_join.conf")
    db_idx = 1
    num_rows_a = [512, 1024, 2048, 4096, 8192]
    num_cols_a = [64, 256, 1024, 4096]
    for num_rows in num_rows_a:
        for num_cols in num_cols_a:
            # Reproducible generation of datasets.
            np.random.seed(1)
            generate_cart_prod_two_rel(db_idx, db_config_path, query_config_path,
                username, password, num_rows, num_cols, system_tests_path, data_path)
            db_idx += 1


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--system_tests_path", action="store",
                        dest="system_tests_path", required=True)
    parser.add_argument("-d", "--data_path", action="store",
                        dest="data_path", required=True)
    parser.add_argument("-p", "--password", action="store",
                        dest="password", required=True)
    parser.add_argument("-u", "--username", action="store",
                        dest="username", required=True)
    args = parser.parse_args(args)

    username = args.username
    password = args.password
    system_tests_path = args.system_tests_path
    data_path = args.data_path

    vary_row_and_col_num(username, password, system_tests_path, data_path)


if __name__ == "__main__":
    init_logging()
    main(sys.argv[1:])

