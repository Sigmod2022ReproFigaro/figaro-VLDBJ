
from evaluation.precision.precision_workbook import PrecisionWorkbook
from decimal import *
import csv


def read_csv_as_list(path: str):
    rows = None
    with open(path, 'r') as csv_file:
        csv_reader = csv.reader(csv_file)
        rows = list(csv_reader)
        
    return rows


def compare_precision_r(figaro_path: str, competitor_path: str, precision_path: str, precision, operation: str):
    abs_err = Decimal(0)
    abs_err_comp = Decimal(0)
    comp_wb = PrecisionWorkbook(output_file=precision_path+"/out.xlsx", 
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
            #print(abs_err)
            abs_err_comp += comp_val * comp_val
            #print(abs_err_comp)
            comp_wb.save_entry(row_idx + 1, col_idx + 1, figaro_val, comp_val)
    comp_wb.save()
    '''
    with open(output_file +".txt", 'w') as file_prec:
        file_prec.write("Absolute error is: {}\n".format(abs_err.sqrt()))
        file_prec.write("Frobenius norm of comp is: {}\n".format(abs_err_comp.sqrt()))
        file_prec.write("Relative error is: {}\n".format((abs_err).sqrt() / abs_err_comp.sqrt()))
    '''

if __name__ == "__main__":
    figaro_path = "/home/popina/Figaro/figaro-code/dumps/figaro/DB3/R.csv"
    python_path = "/home/popina/Figaro/figaro-code/dumps/python/numpy/DB3/r.csv"
    prec_comp_path = "/home/popina/Figaro/figaro-code/comparisons/precision/python/numpy/DB3"
    compare_precision_r(figaro_path=figaro_path, competitor_path=python_path,
                        precision_path=prec_comp_path, 
                        precision=1e-5, operation='qr')