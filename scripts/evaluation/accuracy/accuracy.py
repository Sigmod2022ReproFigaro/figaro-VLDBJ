
from evaluation.accuracy.accuracy_workbook import AccuracyWorkbook
import math
import csv
import os
import pandas as pd
import numpy as np

def compare_accuracy_r(figaro_path: str , competitor_path: str,
        accuracy_path: str, r_comp_file_path: str, errors_file_path: str,
        precision: float, operation: str, generate_xlsx: bool):
    abs_err = 0.0
    abs_err_comp = 0.0
    precision = math.pow(10, -precision)

    pd_figaro = pd.read_csv(figaro_path, header=None).astype(float)
    pd_comp = pd.read_csv(competitor_path, header=None).astype(float)
    np_figaro = pd_figaro.to_numpy()
    np_comp = pd_comp.to_numpy()
    num_rows_fig, num_cols_fig = np_figaro.shape

    if generate_xlsx:
        comp_wb = AccuracyWorkbook(output_file=r_comp_file_path,
                    precision=precision, operation=operation)
        for row_idx in range(num_rows_fig):
            for col_idx in range(num_cols_fig):
                figaro_val = np_figaro[row_idx][col_idx]
                comp_val = np_comp[row_idx][col_idx]
                comp_wb.save_entry(row_idx + 1, col_idx + 1, figaro_val, comp_val)
        comp_wb.save()

    np_diff = np_figaro - np_comp
    abs_err = np.linalg.norm(np_diff, ord='fro')
    abs_err_comp  = np.linalg.norm(np_comp, ord='fro')
    relative_frob_norm = abs_err / abs_err_comp


    with open(errors_file_path, 'w') as file_errors:
        file_errors.write("Absolute error is: {}\n".format(abs_err))
        file_errors.write("Frobenius norm of comp is: {}\n".format(abs_err_comp))
        file_errors.write("Relative error is: {}\n".format(relative_frob_norm))

    if relative_frob_norm > precision:
        print("Figaro and competitor have different precision: {}".format(relative_frob_norm))
    else:
        print("Accuracy excellent")


if __name__ == "__main__":
    figaro_path = "/home/username/Figaro/figaro-code/dumps/figaro/DB3/R.csv"
    python_path = "/home/username/Figaro/figaro-code/dumps/python/numpy/DB3/r.csv"
    prec_comp_path = "/home/username/Figaro/figaro-code/comparisons/precision/python/numpy/DB3"
    compare_accuracy_r(figaro_path=figaro_path, competitor_path=python_path,
                        accuracy_path=prec_comp_path,
                        precision=1e-5, operation='qr')