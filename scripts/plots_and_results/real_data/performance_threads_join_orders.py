""""
Figaro times collection times script.
"""
import numpy as np
import pandas as pd
import os
from openpyxl import load_workbook, Workbook
from openpyxl.utils.cell import get_column_letter
import argparse
import matplotlib.pyplot as plt
from matplotlib import ticker
import csv
import logging
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from data_management.query import Query
import argparse
import sys
import numpy as np
from evaluation.custom_logging import init_logging
import matplotlib.pyplot as plt



def collect_times(root_path: str, exp_names: list, db_names: list,
        exp_paths: dict, join_orders: dict, thread_nums_exp: dict,
            start_per: int, end_per: int, per_inc: int,
            num_measurement: int):
    XLSX_NAME = "time.xlsx"
    df_measurement_exp_dbs = {}
    for exp_name in exp_names:
        exp_path = exp_paths[exp_name]
        perf_path = os.path.join(root_path, exp_path)

        for db_name in db_names:
            gather_times_path = os.path.join(perf_path, db_name + XLSX_NAME)
            print(gather_times_path)
            out_workbook = Workbook()
            out_workbook.remove(out_workbook.active)
            out_sheet = out_workbook.create_sheet("Times")
            df_measurement = pd.DataFrame(columns=join_orders[db_name])

            for join_idx, join_order in enumerate(join_orders[db_name]):
                for thread_idx, thread_num in enumerate(thread_nums_exp[exp_name]):
                    for db_idx, percent in enumerate(range(start_per, end_per + 1, per_inc)):
                        db_name_full = "{} {} {}".format(db_name, join_order, thread_num)
                        path_xlsx = os.path.join(perf_path, db_name, join_order+str(thread_num), XLSX_NAME)
                        if os.path.isfile(path_xlsx):
                            print("PATH", path_xlsx)
                            workbook = load_workbook(filename=path_xlsx, data_only=True)
                            sheet = workbook.active
                            row_count = sheet.max_row
                            col_idx = join_idx * thread_nums_exp[exp_name].__len__() + thread_idx + 1
                            col_letter = get_column_letter(col_idx)

                            out_sheet.cell(row=1, column=col_idx).value = db_name_full

                            start_row_idx = 2
                            np_measures = np.zeros(num_measurement)
                            for row_idx_dst in range(start_row_idx, start_row_idx + num_measurement):
                                offset_idx = row_idx_dst - start_row_idx
                                row_idx_src = row_count - num_measurement  + offset_idx
                                val = sheet.cell(row=row_idx_src, column=2).value
                                np_measures[offset_idx] = float(val)
                                #print(idx_shift, db_idx)
                                out_sheet.cell(row=row_idx_dst, column=col_idx).value = val

                            out_sheet.cell(row=start_row_idx + num_measurement, column=col_idx).value = '=AVERAGE({}{}:{}{})'.format(col_letter, start_row_idx + 1,
                                    col_letter,6)
                            time_avg = np.mean(np_measures[1:])
                        else:
                            time_avg = float("inf")
                        df_measurement.at[thread_num, join_order] = time_avg

            out_workbook.save(gather_times_path)
            df_measurement_exp_dbs[(exp_name, db_name)] = df_measurement.astype(np.float64)


def dump_results_to_dat(db_names: list, df_measurement_exps: dict,
    thread_nums_exp: dict):
    exp_name = "figaro"
    exp_dat_name = "exp2cores.dat"
    exp_dat_names = ["#cores", "Retailer", "Favorita", "Yelp"]
    dbs_results = []
    for db_name in db_names:
        df_measurement = df_measurement_exps[(exp_name, db_name)]
        print(df_measurement)
        min_idx_cols = df_measurement.idxmin(axis="columns")
        max_num_threads = thread_nums_exp[exp_name][-1]
        min_join_order = min_idx_cols[max_num_threads]
        dbs_results.append(df_measurement[min_join_order])
        print(df_measurement[min_join_order])
    df_db_results = pd.concat(dbs_results, axis=1)
    df_db_results = df_db_results.reset_index().rename(columns={df_db_results.index.name:'index'})
    df_db_results.columns = exp_dat_names
    df_db_results.to_csv(exp_dat_name, float_format='%.2f', sep='\t', index=False, quoting=csv.QUOTE_NONE,  escapechar=" ")


def plot_performance(db_names: list, exp_names: list,
    df_measurement_exp_dbs: dict, thread_nums_exp: dict):
    plt.figure("name", figsize=(16, 8), dpi=80)
    plt.xlabel("Number of threads")
    plt.ylabel("wall-clock-time[s]")

    plt.title("Runtime performance comparison of Figaro and competitors on real-world datasets")


    plt.yscale('log', base=10)
    plt.xscale('log', base=2)
    #plt.locator_params(axis='x', nbins=6)
    #plt.locator_params(axis='y', nbins=20)
    db_marker =  {"DBFavorita": "^", "DBYelp": "s", "DBRetailer": "x"}
    db_colour =  {"DBFavorita": "r", "DBYelp": "c", "DBRetailer": "g"}
    exp_colour = {"figaro": "r", "mkl": "b"}

    for exp_name in exp_names:
        if exp_name == "mkl":
            continue
        for db_name in db_names:
            df_measurement = df_measurement_exp_dbs[(exp_name, db_name)]
            print(df_measurement)
            min_idx_cols = df_measurement.idxmin(axis="columns")
            max_num_threads = thread_nums_exp[exp_name][-1]
            min_join_order = min_idx_cols[max_num_threads]

            print(min_idx_cols[thread_nums_exp["figaro"][-1]])
            #for join_order in join_orders[db_name]:
            plt.plot(df_measurement[min_join_order], "-" + db_colour[db_name] + db_marker[db_name], label="{} {} {}".format(exp_name, db_name, min_join_order), markersize=10)

    ax = plt.gca()
    def myLogFormat(y,pos):
        # Find the number of decimal places required
        decimalplaces = int(np.maximum(-np.log10(y),0))     # =0 for numbers >=1
        # Insert that number into a format string
        formatstring = '{{:.{:1d}f}}'.format(decimalplaces)
        # Return the formatted tick label
        return formatstring.format(y)

    ax.yaxis.set_major_formatter(ticker.FuncFormatter(myLogFormat))

    #formatter = ticker.LogFormatter(base=10, labelOnlyBase=False)
    #formatter.set_scientific(True)
    #formatter.set_powerlimits((-1,1))
    #ax.yaxis.set_major_formatter(formatter)

    formatter = ticker.LogFormatter(base=2)
    #formatter.set_scientific(True)
    #formatter.set_powerlimits((0,48))
    ax.xaxis.set_major_formatter(formatter)
    plt.legend(loc="upper right")
    plt.savefig("exp2threads.pdf",bbox_inches='tight')

    plt.show()

def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("--root_path", action="store",
                        dest="root_path", required=True)
    parser.add_argument("-e", "--exp_names", dest="exp_names", nargs='*', required=True)
    parser.add_argument("--dump_results", action="store_true",
        dest="dump_results", required=False)
    args = parser.parse_args(args)


    root_path = args.root_path
    exp_names = args.exp_names
    dump_results = args.dump_results

    db_names = ["DBRetailer", "DBFavorita", "DBYelp"]
    figaro_threads = [1, 2, 4, 8, 16, 24, 32, 48]
    thread_nums_exp = {"mkl": [48], "figaro": figaro_threads}

    join_orders = {"DBFavorita": ["HolidaysRoot", "ItemsRoot", "OilRoot", "SalesRoot", "StoresRoot", "TransactionsRoot"],
                "DBYelp": ["BusinessRoot", "CategoryRoot", "CheckinRoot", "HoursRoot", "ReviewRoot", "UserRoot"],
                "DBRetailer": ["CensusRoot", "InventoryRoot", "ItemRoot", "LocationRoot", "WeatherRoot"]}


    exp_paths = {"figaro_thin": "comparisons/performance/figaro/thin_diag",
    "figaro": "comparisons/performance/figaro/thin_diag",
    "mkl": "comparisons/performance/python/mkl",
    "figaro_lapack": "comparisons/performance/figaro/lapack",
    "openblas": "comparisons/performance/python/openblas",
    "post_proc_thin": "comparisons/performance/postprocess/thin_diag",
    "post_proc_mkl": "comparisons/performance/postprocess/lapack"}

    db_names = ["DBRetailer", "DBFavorita", "DBYelp"]

    start_per = 10
    end_per = 100
    per_inc = 10

    num_measurement = 5

    df_measurement_exps = collect_times(root_path, exp_names, db_names,
        exp_paths, join_orders, thread_nums_exp, start_per, end_per, per_inc,
        num_measurement)
    if dump_results:
        dump_results_to_dat(db_names, df_measurement_exps, thread_nums_exp)
    plot_performance(db_names, exp_names, df_measurement_exps,  thread_nums_exp)

if __name__ == "__main__":
    init_logging()
    main(sys.argv[1:])