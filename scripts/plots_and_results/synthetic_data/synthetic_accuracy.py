""""
Figaro times collection times script.
"""
import csv
import math
import numpy as np
import pandas as pd
import argparse
import os
from openpyxl import load_workbook, Workbook
from openpyxl.utils.cell import get_column_letter
import logging
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from data_management.query import Query
import argparse
import sys
import numpy as np
from evaluation.custom_logging import init_logging, set_logging_level
import matplotlib.pyplot as plt
import seaborn as sns
from matplotlib.colors import LogNorm
import matplotlib.pyplot as plt

def arrange_up_triang(n: int):
    A  = np.zeros( (n, n))
    idx = 1
    for row_idx in range(n):
        for col_idx in range(row_idx, n):
            A[row_idx, col_idx] = idx
            idx += 1
    return A


def compare_rs(R: np.array, num_rows: int, num_cols: int):
    R_expX = arrange_up_triang(num_cols) * math.sqrt(num_rows)
    diff = R[:num_cols, :num_cols] - R_expX
    rel_fro = np.linalg.norm(diff, ord='fro') / np.linalg.norm(R_expX, ord='fro')
    return rel_fro



def collect_times(root_path: str, exp_names: list, exp_paths: dict,
    skip_dbs: list, column_nums: list, row_col_nums: list):
    CSV_NAME = "R.csv"
    JOIN_ORDER = "FullJoin"
    exp_rel_errors = {}

    for exp_name in exp_names:
        #Initialize paths for the current experiment
        exp_path = exp_paths[exp_name]
        accur_path = os.path.join(root_path, exp_path)
        df_measurement = pd.DataFrame(columns=column_nums)
        dump_r_path = os.path.join(accur_path, CSV_NAME)


        for db_idx, db_num in enumerate(range(1, len(row_col_nums) + 1)):
            if db_num in skip_dbs:
                continue
            db_name = "DBCartesianProductAccuracy{}".format(db_num)
            path_csv = os.path.join(accur_path, db_name, JOIN_ORDER,
                CSV_NAME)

            # Writng header files
            row_num = row_col_nums[db_idx][0]
            col_num = row_col_nums[db_idx][1]
            r_mat = np.genfromtxt(path_csv, delimiter=',')
            rel_error = compare_rs(r_mat, row_num, col_num)
            df_measurement.at[row_num, col_num] = rel_error
            df_measurement = df_measurement.astype(float)
            #df_measurement.to_csv(exp_name+"comp.csv")
        exp_rel_errors[exp_name] = df_measurement

    return exp_rel_errors


def df_to_latex(measurement: pd.DataFrame):
    measurement = measurement.drop(columns=[4096])
    df_measurement_lat = measurement.rename(index={512:" $2^9$", 1024: "$2^{10}$",
        2048: "$2^{11}$", 4096: "$2^{12}$", 8192: "$2^{13}$"},
        columns={64:"$2^6$", 128:"$2^7$", 256: "$2^8$", 512: "$2^9$", 1024:"$2^{10}$", 4096: "$2^{12}$"})
    return df_measurement_lat


def dump_results_to_latex(exp_names: list, figaro_impls: list,
        exp_rel_errors: dict):

    root_dir = "results/synthetic"
    out_path = os.path.join(root_dir, "synthetic_accur.tex")
    os.makedirs(root_dir, exist_ok=True)
    out_file = open(out_path, "w")

    for exp_name in exp_names:
        df_measurement = exp_rel_errors[exp_name]
        df_measur_lat = df_to_latex(df_measurement)
        out_file.write("Relative error {}\n".format(exp_name))
        out_file.write(df_measur_lat.to_latex(float_format="%.2g",escape=False, na_rep=" "))
        out_file.write("\n")

    for figaro_impl in figaro_impls:
        df_error = exp_rel_errors["postprocess_lapack"] / exp_rel_errors[figaro_impl]
        df_error_lat = df_to_latex(df_error)
        out_file.write("Relative error comparison {}\n".format(figaro_impl))
        out_file.write(df_error_lat.to_latex(float_format="%.2g",escape=False, na_rep=" "))
        out_file.write("\n")

    out_file.close()


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("--root_path", action="store",
                        dest="root_path", required=True)
    parser.add_argument("--dump_results", action="store_true",
        dest="dump_results", required=False)
    args = parser.parse_args(args)

    root_path = args.root_path
    dump_results = args.dump_results

    figaro_impls = ["figaro_lapack", "figaro_thin"]
    exp_names = [ "figaro_lapack", "figaro_thin", "postprocess_lapack"]
    exp_paths = {"figaro_lapack": "dumps/figaro/only_r/lapack/thread48", "postprocess_lapack": "dumps/postprocess/lapack/col_major/only_r/thread48", "figaro_thin": "dumps/figaro/only_r/thin_diag/thread48"}

    row_nums = [512, 1024, 2048, 4096, 8192]
    column_nums = [16, 64, 256, 1024, 4096]
    row_col_nums = []
    skip_dbs = [14]
    np.set_printoptions(threshold=sys.maxsize, precision=14)


    for row_num in row_nums:
        for col_num in column_nums:
            if (row_num > col_num):
                row_col_nums.append((row_num, col_num))

    row_col_nums = row_col_nums[:16]

    df_measurement_exps = collect_times(root_path, exp_names,
            exp_paths, skip_dbs, column_nums, row_col_nums)
    if dump_results:
        pass
        dump_results_to_latex(exp_names, figaro_impls, df_measurement_exps)


if __name__ == "__main__":
    init_logging()
    set_logging_level(logging.INFO)
    main(sys.argv[1:])