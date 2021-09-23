import json
import os
import pandas as pd
import numpy as np
import shutil
import random

import logging
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from data_management.query import Query
import argparse
import sys
import numpy as np
from evaluation.custom_logging import init_logging


def reduce_column_size(t: pd.DataFrame, percent: float, column: str):
    unique_vals_s = set(t[column].unique().tolist())
    reduced_num_cols =  int(len(unique_vals_s) * percent)
    map_vals_to = random.sample(unique_vals_s, reduced_num_cols)

    dic_mappings = {}

    logging.debug(map_vals_to[:10])

    for idx, unique_value in enumerate(unique_vals_s):
        dic_mappings [unique_value] = map_vals_to[idx % reduced_num_cols]


    t[column].replace(to_replace=dic_mappings, inplace=True)
    logging.debug("Before: {}".format(len(unique_vals_s)))
    logging.debug("Reduced number: {}".format(len(set(t[column].unique().tolist()))))


def ohe_column_copy_real_dataset(real_dataset_path: str,
    real_data_set_rels_to_ohe):
    db_ohe_path = real_dataset_path + "_ohe"

    shutil.copytree(real_dataset_path, db_ohe_path)

    percents = np.linspace(0.01, 0.01, num=1)

    db_paths = [ os.path.join(db_ohe_path, str(round(per, 2))) for per in percents]

    rel_names = [real_to_ohe["name"] for real_to_ohe in real_data_set_rels_to_ohe]
    for db_path in db_paths:
        for file_name in os.listdir(db_path):
            # ".csv"
            file_name_without_csv = file_name[:-4]
            if file_name_without_csv in rel_names:
                full_path = os.path.join(db_path, file_name)
                logging.debug(full_path)
                t_original = pd.read_csv(full_path, header=None)
                rel_ohe_cur = {}
                for rel_ohe in real_data_set_rels_to_ohe:
                        if rel_ohe["name"] == file_name_without_csv:
                            rel_ohe_cur = rel_ohe
                            break

                if "column_size_red" in rel_ohe_cur:
                    percents_column = np.linspace(0.1, 1, num=10)
                    #percents_column = np.linspace(0.1, 0.1, num=1)
                    for percent_column in percents_column:
                        t = t_original.copy()
                        t.columns = rel_ohe_cur["schema"]
                        for join_attr in rel_ohe_cur["join_attrs"]:
                            join_attr_ohe = join_attr + "C"
                            t[join_attr_ohe] = t[join_attr]

                        random.seed(1)
                        reduce_column_size(t, percent_column, rel_ohe_cur["column_size_red"] + "C")
                        full_path_column = os.path.join(db_path, str(round(percent_column, 2)) + file_name)
                        logging.debug(full_path_column)
                        t.to_csv(full_path_column, index=False, header=None)
                else:
                    t_original.columns = rel_ohe_cur["schema"]
                    for join_attr in rel_ohe_cur["join_attrs"]:
                        join_attr_ohe = join_attr + "C"
                        t_original[join_attr_ohe] = t_original[join_attr]
                    t_original.to_csv(full_path, index=False, header=None)


def ohe_column_copy_real_datasets(data_path: str):
    real_data_sets = ["retailer", "favorita", "yelp"]
    relations_to_ohe = {
        "retailer":
        [
            {
                "name": "Inventory",
                "schema": ["Locn", "DateId", "Ksn", "InventoryUnits"],
                "join_attrs": ["Locn", "DateId", "Ksn"],
                "column_size_red": "Ksn"
                },
            {
                "name": "Census",
                "schema" : ["Zip", "Population", "White", "Asian", "Pacific", "Black", "MedianAge", "OccupiedHouseUnits", "HouseUnits", "Families", "Households", "HusbWife", "Males", "Females", "HouseholdChildren", "Hispanic"],
                "join_attrs": ["Zip"]
            }
        ]
        ,
        "favorita":
        [
            {
                "name": "Sales",
                "schema": ["Date","Store","Item","UnitSales","OnPromotion"],
                "join_attrs": ["Date", "Store", "Item"],
                "column_size_red": "Item"
            }
        ],
        "yelp":
        [
            {
                "name": "review",
                "schema": ["BusinessId","UserId","ReviewId","StarsR","ReviewYear","ReviewMonth","ReviewDay","Useful","Funny","Cool"],
                "join_attrs": ["BusinessId", "UserId", "ReviewId"],
                "column_size_red": "BusinessId"
            }
        ]
    }

    for real_data_set in real_data_sets:
        real_data_set_path = os.path.join(data_path, real_data_set)
        ohe_column_copy_real_dataset(real_data_set_path, relations_to_ohe[real_data_set])


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_path", action="store",
                        dest="data_path", required=True)
    args = parser.parse_args(args)

    data_path = args.data_path
    ohe_column_copy_real_datasets(data_path)

if __name__ == "__main__":
    init_logging()
    main(sys.argv[1:])