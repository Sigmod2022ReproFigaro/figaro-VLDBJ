
from evaluation.accuracy.accuracy_workbook import AccuracyWorkbook
import math
import csv
import os
import pandas as pd

def read_csv_as_list(path: str):
    rows = None
    #df = pd.read_csv(path, header=None)
    #df.
    with open(path, 'r') as csv_file:
        csv_reader = csv.reader(csv_file)
        rows = list(csv_reader)
        
    return rows


def compare_accuracy_r(figaro_path: str, competitor_path: str, 
        accuracy_path: str, r_comp_file_path: str, errors_file_path: str, 
        precision: float, operation: str):
    abs_err = 0.0
    abs_err_comp = 0.0
    precision = math.pow(10, -precision)
    comp_wb = AccuracyWorkbook(output_file=r_comp_file_path, 
                precision=precision, operation=operation)
                
    figaro_rows = read_csv_as_list(figaro_path)
    comp_rows = read_csv_as_list(competitor_path)

    for row_idx, figaro_row in enumerate(figaro_rows):
        for col_idx, figaro_col in enumerate(figaro_row):
            figaro_val = float(figaro_col)
            comp_val = float(comp_rows[row_idx][col_idx])
            #print("figaro_val ", figaro_val, " comp_val", comp_val)
            diff = figaro_val - comp_val
            abs_err += diff * diff
            abs_err_comp += comp_val * comp_val
            comp_wb.save_entry(row_idx + 1, col_idx + 1, figaro_val, comp_val)
    comp_wb.save()
    

    relative_frob_norm = math.sqrt(abs_err) / math.sqrt(abs_err_comp)
    with open(errors_file_path, 'w') as file_errors:
        file_errors.write("Absolute error is: {}\n".format(math.sqrt(abs_err)))
        file_errors.write("Frobenius norm of comp is: {}\n".format(math.sqrt(abs_err_comp)))
        file_errors.write("Relative error is: {}\n".format(relative_frob_norm))

    if relative_frob_norm > precision:
        print("Figaro and competitor have different precision: {}".format(relative_frob_norm))
    else:
        print("Accuracy excellent")


if __name__ == "__main__":
    figaro_path = "/home/popina/Figaro/figaro-code/dumps/figaro/DB3/R.csv"
    python_path = "/home/popina/Figaro/figaro-code/dumps/python/numpy/DB3/r.csv"
    prec_comp_path = "/home/popina/Figaro/figaro-code/comparisons/precision/python/numpy/DB3"
    compare_accuracy_r(figaro_path=figaro_path, competitor_path=python_path,
                        accuracy_path=prec_comp_path, 
                        precision=1e-5, operation='qr')