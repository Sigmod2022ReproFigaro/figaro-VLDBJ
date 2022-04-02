import os
import logging
import argparse
import sys
import data_management.database_full_reducer as dfr
from evaluation.custom_logging import init_logging




import os
import pandas as pd
import numpy as np
import shutil
import random
import json

import sys
import numpy as np
import pandas as pd
from ctypes import CDLL
from  argparse import ArgumentParser
from timeit import default_timer as timer
from sklearn.base import BaseEstimator, TransformerMixin
from sklearn.preprocessing import OneHotEncoder
from sklearn.compose import ColumnTransformer
import threadpoolctl
from threadpoolctl import threadpool_limits

class DummyEncoder(BaseEstimator, TransformerMixin):
    def __init__(self, sparse):
        self.sparse = sparse

    def transform(self, X):
        ohe = OneHotEncoder(handle_unknown='ignore', sparse=self.sparse)
        return ohe.fit_transform(X)[:,1:]

    def fit(self, X, y=None, **fit_params):
        return self


class IdentityTransformer(BaseEstimator, TransformerMixin):
    def transform(self, input_array):
        return input_array

    def fit(self, X, y=None, **fit_params):
        return self



def dump_qr(dump_file_path, r):
    np.savetxt(dump_file_path, np.asarray(r), delimiter=',')


def transform_data(data, columns, cat_columns, sparse):
    transformer_a = []
    for column in columns:
        if column in cat_columns:
            transf_name = column + "_onehot"
            transformer_a.append((transf_name, DummyEncoder(sparse), [column]))
        else:
            transf_name = column + "_identity"
            transformer_a.append((transf_name, IdentityTransformer(), [column]))

    preprocessor = ColumnTransformer(transformers=transformer_a)
    one_hot_a = preprocessor.fit_transform(data)
    return one_hot_a


def ohe_and_dump_database(db_config):
    db_config_path = db_config["db_config_path"]
    db_ohe_path = db_config["db_ohe_path"]
    skip_attrs = db_config["skip_attrs"]
    if os.path.exists(db_ohe_path):
        shutil.rmtree(db_ohe_path)
    os.makedirs(db_ohe_path)
    with open(db_config_path, 'r') as db_config_file:
        db_config_json = json.load(db_config_file)

    rels_json = db_config_json["database"]["relations"]

    os.rmdir(db_ohe_path)
    os.makedirs(db_ohe_path)

    for rel_json in rels_json:
        rel_name = rel_json["name"]
        attrs_json = rel_json["attributes"]
        data_path = rel_json["data_path"]
        print(rel_name)
        attrs = []
        attrs_without_dropped = []
        attrs_cat = []
        drop_attrs = []
        for attr_json in attrs_json:
            attr_name = attr_json["name"]
            attr_type = attr_json["type"]
            attrs.append(attr_name)
            if attr_name in skip_attrs:
                drop_attrs.append(attr_name)
            else:
                attrs_without_dropped.append(attr_name)
                if attr_type == "category":
                    attrs_cat.append(attr_name)

        print("attrs", attrs)
        print("cat_attrs", attrs_cat)
        print("drop_attrs",  drop_attrs)
        print("attrs_without_dropped",  attrs_without_dropped)

        table = pd.read_csv(data_path, names=attrs,
            delimiter=",", header=None)
        print("dropping columns")
        table = table.drop(drop_attrs, axis="columns")
        print("ohe data")
        table_np = transform_data(table, attrs_without_dropped, attrs_cat, False)
        data_out_path = os.path.join(db_ohe_path, rel_name +".csv")
        print(data_out_path)
        np.savetxt(data_out_path, np.asarray(table_np), delimiter=',')


def ohe_and_dump_databases(real_dataset_path: str, system_tests_path: str):
    db_names =["retailer_ohe", "favorita_ohe", "yelp_ohe"]
    db_configs=  {
        "retailer":
        {
            "db_config_path" : "test_real_data/databases/usretailer/database_specs_usretailer_100.conf",
            "skip_attrs":  ["Population", "OccupiedHouseUnits", "SubCategory", "CategoryCluster"]
        },
        "favorita":
        {
            "db_config_path" : "test_real_data/databases/favorita/database_specs_favorita_100.conf",
            "skip_attrs": ["OnPromotion", "HolidayType", "Locale", "LocaleId", "State", "Cluster", "Family", "Perishable"]
        },
        "yelp":
        {
            "db_config_path" : "test_real_data/databases/yelp/database_specs_yelp_100.conf",
            "skip_attrs": ["ComplimentFunny"]
        },
        "retailer_ohe":
        {
            "db_config_path" : "test_real_data_ohe/databases/usretailer/database_specs_usretailer_1_100.conf",
            "skip_attrs":  [ "Population", "White",  "Asian", "Pacific", "Black",  "MedianAge", "OccupiedHouseUnits", "HouseUnits", "Families", "Households", "HusbWife", "Males", "Females", "HouseholdChildren", "ZipC", "SubCategory", "Category", "CategoryCluster", "LocnC",  "DateIdC", "MaxTemp", "MinTemp", "Thunder"]
        },
        "favorita_ohe":
        {
            "db_config_path" : "test_real_data_ohe/databases/favorita/database_specs_favorita_1_100.conf",
            "skip_attrs":  ["OnPromotion", "HolidayType", "Locale", "LocaleId", "State", "Cluster", "Family", "Perishable"]
        },
        "yelp_ohe":
        {
            "db_config_path" : "test_real_data_ohe/databases/yelp/database_specs_yelp_1_100.conf",
            "skip_attrs":  [
                    "CityId", "StateId", "Latitude", "Longitude", "StarsB", "ReviewCountB", "IsOpen",
                    "StarsR", "Useful", "Funny", "Cool","UserIdC", "ReviewIdC","ComplimentFunny"]
        }
    }

    for db_name in db_configs.keys():
        db_configs[db_name]["db_ohe_path"] = os.path.join(
            real_dataset_path, db_name + "_size_on_disk")
        db_configs[db_name]["db_config_path"] = os.path.join(
            system_tests_path, db_configs[db_name]["db_config_path"],
            )

    for db_name in db_names:
        ohe_and_dump_database(db_configs[db_name])


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("--data_path", action="store",
                        dest="data_path", required=True)
    parser.add_argument("-s", "--system_tests_path", action="store",
                        dest="system_tests_path", required=True)
    parser.add_argument("-p", "--password", action="store",
                        dest="password", required=True)
    parser.add_argument("-u", "--username", action="store",
                        dest="username", required=True)
    args = parser.parse_args(args)

    data_path = args.data_path
    username = args.username
    password = args.password
    system_tests_path = args.system_tests_path
    ohe_and_dump_databases(data_path, system_tests_path)


if __name__ == "__main__":
    init_logging()
    main(sys.argv[1:])