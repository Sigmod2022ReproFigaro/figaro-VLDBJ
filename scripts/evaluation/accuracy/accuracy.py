
from evaluation.accuracy.accuracy_workbook import AccuracyWorkbook
from decimal import Decimal
import csv
import os


def read_csv_as_list(path: str):
    rows = None
    with open(path, 'r') as csv_file:
        csv_reader = csv.reader(csv_file)
        rows = list(csv_reader)
        
    return rows


def compare_accuracy_r(figaro_path: str, competitor_path: str, accuracy_path: str, precision, operation: str):
    abs_err = Decimal(0)
    abs_err_comp = Decimal(0)
    comp_wb = AccuracyWorkbook(output_file=accuracy_path+"/R_comp.xlsx", 
                precision=precision, operation=operation)
                
    figaro_rows = read_csv_as_list(figaro_path)
    comp_rows = read_csv_as_list(competitor_path)

    for row_idx, figaro_row in enumerate(figaro_rows):
        for col_idx, figaro_col in enumerate(figaro_row):
            figaro_val = Decimal(figaro_col)
            comp_val = Decimal(comp_rows[row_idx][col_idx])
            print("figaro_val ", figaro_val, " comp_val", comp_val)
            diff = figaro_val - comp_val
            abs_err += diff * diff
            abs_err_comp += comp_val * comp_val
            comp_wb.save_entry(row_idx + 1, col_idx + 1, figaro_val, comp_val)
    comp_wb.save()
    
    errors_path = os.path.join(accuracy_path, "error.txt")
    with open(errors_path, 'w') as file_errors:
        file_errors.write("Absolute error is: {}\n".format(abs_err.sqrt()))
        file_errors.write("Frobenius norm of comp is: {}\n".format(abs_err_comp.sqrt()))
        file_errors.write("Relative error is: {}\n".format((abs_err).sqrt() / abs_err_comp.sqrt()))


if __name__ == "__main__":
    figaro_path = "/home/popina/Figaro/figaro-code/dumps/figaro/DB3/R.csv"
    python_path = "/home/popina/Figaro/figaro-code/dumps/python/numpy/DB3/r.csv"
    prec_comp_path = "/home/popina/Figaro/figaro-code/comparisons/precision/python/numpy/DB3"
    compare_accuracy_r(figaro_path=figaro_path, competitor_path=python_path,
                        accuracy_path=prec_comp_path, 
                        precision=1e-5, operation='qr')