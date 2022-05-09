import evaluation.evaluator as eval
import numpy as np
import pandas as pd
import argparse
import os
from openpyxl.utils.cell import get_column_letter
import plots_and_results.real_data.performance_percent as perf_percent
import plots_and_results.real_data.performance_threads_join_orders as perf_join_ord
import plots_and_results.synthetic_data.synthetic_accuracy as syn_accur
import plots_and_results.synthetic_data.synthetic_performance as syn_perf
import logging
from data_management.query import Query
import argparse
import sys
import numpy as np
from evaluation.custom_logging import init_logging, set_logging_level
import matplotlib.pyplot as plt
import json

def test_set_all_to_true(tests_path: str):
    with open(tests_path, 'r') as tests_file_json:
        tests_json = json.load(tests_file_json)

    tests = tests_json["tests"]
    for test in tests:
        test["disable"] = True

    with open(tests_path, 'w') as tests_file_json:
        tests_json = json.dump(tests_json, tests_file_json, indent=4)


def test_set_to_true(tests_path: str, subtests_to_set: list):
    with open(tests_path, 'r') as tests_file_json:
        tests_json = json.load(tests_file_json)

    tests = tests_json["tests"]
    subtests_set = set(subtests_to_set)
    for test in tests:
        if test["name"] in subtests_to_set:
            test["disable"] = False

    with open(tests_path, 'w') as tests_file_json:
        tests_json = json.dump(tests_json, tests_file_json, indent=4)


def set_exp1_real(system_tests_path: str):
    test_real_data_path = os.path.join(system_tests_path, "test_real_data", "tests_specs.conf")
    test_set_all_to_true(test_real_data_path)
    subtests_to_set = ["percent_psql_dump", "percent_performance", "percent_perf_analysis"]
    test_set_to_true(test_real_data_path, subtests_to_set)

    test_real_data_ohe_path = os.path.join(system_tests_path, "test_real_data_ohe", "tests_specs.conf")
    test_set_all_to_true(test_real_data_ohe_path)
    test_set_to_true(test_real_data_ohe_path, subtests_to_set)


def set_exp1_synt(system_tests_path: str):
    test_syn_perf_data_path = os.path.join(system_tests_path, "test_syn_perf", "tests_specs.conf")
    test_set_all_to_true(test_syn_perf_data_path)
    subtests_to_set = ["synthetic_performance_figaro", "synthetic_performance_mkl"]
    test_set_to_true(test_syn_perf_data_path, subtests_to_set)


def run_exp1(username: str, password: str, root_path: str,
    system_tests_path: str):
    set_exp1_real(system_tests_path)
    set_exp1_synt(system_tests_path)

    args = ["-p", password, "-u", username, "-r", root_path,
               "-s", system_tests_path]
    args_real_data = args + ["--test", "_real_data"]
    args_real_data_ohe = args + ["--test", "_real_data_ohe"]
    args_perf_syn = args + ["--test", "_syn_perf"]

    eval.main(args_real_data)
    eval.main(args_real_data_ohe)
    eval.main(args_perf_syn)

    args_coll = ["--root_path", root_path,
        "--exp_names", "figaro_thin", "post_proc_mkl",
        "post_proc_thin", "--dump_results"]
    args_coll_ohe = args_coll + ["--ohe"]
    args_coll_syn = ["--root_path", root_path, "--dump_results"]

    perf_percent.main(args_coll)
    perf_percent.main(args_coll_ohe)
    syn_perf.main(args_coll_syn)



def set_exp2_thread(system_tests_path: str):
    test_real_data_path = os.path.join(system_tests_path, "test_real_data", "tests_specs.conf")
    test_set_all_to_true(test_real_data_path)
    subtests_to_set = ["thread_perf", "thread_perf_analysis"]
    test_set_to_true(test_real_data_path, subtests_to_set)


def run_exp2(username: str, password: str, root_path: str,
    system_tests_path: str):
    set_exp2_thread(system_tests_path)
    args = ["-p", password, "-u", username, "-r", root_path,
               "-s", system_tests_path]
    args_real_data = args + ["--test", "_real_data"]
    eval.main(args_real_data)


def set_exp3_real(system_tests_path: str):
    test_real_data_path = os.path.join(system_tests_path, "test_real_data", "tests_specs.conf")
    test_set_all_to_true(test_real_data_path)
    subtests_to_set = ["join_order_perf", "join_order_perf_analysis"]
    test_set_to_true(test_real_data_path, subtests_to_set)

    test_real_data_ohe_path = os.path.join(system_tests_path, "test_real_data_ohe", "tests_specs.conf")
    test_set_all_to_true(test_real_data_ohe_path)
    test_set_to_true(test_real_data_ohe_path, subtests_to_set)


def run_exp3(username: str, password: str, root_path: str,
    system_tests_path: str):
    set_exp3_real(system_tests_path)

    args = ["-p", password, "-u", username, "-r", root_path,
               "-s", system_tests_path]
    args_real_data = args + ["--test", "_real_data"]
    args_real_data_ohe = args + ["--test", "_real_data_ohe"]

    eval.main(args_real_data)
    eval.main(args_real_data_ohe)

    args_coll = ["--root_path", root_path,
        "--exp_name", "figaro_thin", "--dump_results"]
    args_coll_ohe = args_coll + ["--ohe"]
    perf_join_ord.main(args_coll)
    perf_join_ord.main(args_coll_ohe)


def set_exp4_synt(system_tests_path: str):
    test_syn_perf_data_path = os.path.join(system_tests_path, "test_syn_accur", "tests_specs.conf")
    test_set_all_to_true(test_syn_perf_data_path)
    subtests_to_set = ["synthetic_accuracy_figaro",
        "synthetic_accuracy_mkl"]
    test_set_to_true(test_syn_perf_data_path, subtests_to_set)


def run_exp4(username: str, password: str, root_path: str,
    system_tests_path: str):
    set_exp4_synt(system_tests_path)

    args = ["-p", password, "-u", username, "-r", root_path,
               "-s", system_tests_path]
    args_synt_accur = args + ["--test", "_syn_accur"]

    eval.main(args_synt_accur)

    args_coll = ["--root_path", root_path,
        "--dump_results"]
    syn_accur.main(args_coll)


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
    parser.add_argument("-e", "--experiment", action="store",
                        dest="experiment", required=False)
    args = parser.parse_args(args)

    root_path = args.root_path
    system_tests_path = args.system_tests_path
    username = args.username
    password = args.password
    exp = int(args.experiment)

    logging.info("Running set of experiments {}".format(str(exp)))
    if exp == 1:
        run_exp1(username, password, root_path, system_tests_path)
    elif exp == 2:
        run_exp2(username, password, root_path, system_tests_path)
    elif exp == 3:
        run_exp3(username, password, root_path, system_tests_path)
    elif exp == 4:
        run_exp4(username, password, root_path, system_tests_path)


if __name__ == "__main__":
    init_logging()
    set_logging_level(logging.INFO)
    main(sys.argv[1:])
