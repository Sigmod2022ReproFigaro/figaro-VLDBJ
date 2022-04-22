import os
import logging
import argparse
from pathlib import Path
import sys
import pandas as pd
import numpy as np
import shutil
import json
from evaluation.custom_logging import init_logging, set_logging_level
from sklearn.base import BaseEstimator, TransformerMixin
from sklearn.preprocessing import OneHotEncoder
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from data_management.query import Query
from evaluation.system_test.system_test import DumpConf
from sklearn.compose import ColumnTransformer

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


def convert_bytes_to_unit_str(size: int) -> str:
    for unit in ("B", "K", "M", "G", "T"):
        if size < 1024:
            break
        size /= 1024
    return f"{size:.1f}{unit}"


def get_dir_size(folder: str) -> int:
    dir_size = sum(p.stat().st_size for p in Path(folder).rglob('*'))
    return dir_size


def get_dir_size_str(path: str) -> str:
    size = get_dir_size(path)
    return convert_bytes_to_unit_str(size)


def get_file_size(file_path: str) -> int:
    file_size = Path(file_path).stat().st_size
    return file_size


def get_file_size_str(file_path: str) -> int:
    size = get_file_size(file_path)
    return convert_bytes_to_unit_str(size)


def ohe_and_dump_database(db_config, query: Query, database: Database):
    db_config_path = db_config["db_config_path"]
    db_ohe_path = db_config["db_ohe_path"]
    if os.path.exists(db_ohe_path):
        shutil.rmtree(db_ohe_path)
    os.makedirs(db_ohe_path)
    with open(db_config_path, 'r') as db_config_file:
        db_config_json = json.load(db_config_file)

    num_rows = 0

    rels_json = db_config_json["database"]["relations"]

    shutil.rmtree(db_ohe_path, ignore_errors=True)
    os.makedirs(db_ohe_path)
    logging.info("OHE and dumping relations from {}".format(database.name))

    for rel_json in rels_json:
        rel_name = rel_json["name"]
        data_path = rel_json["data_path"]
        logging.debug(rel_name)
        attrs = database.get_all_attribute_names(rel_name=rel_name)
        attrs_cat = database.get_attribute_names(rel_name=rel_name, is_cat=True)
        attrs_without_dropped = query.get_join_attrs(rel_name) \
            +  query.get_non_join_attrs(rel_name)
        skip_attrs_set = set(query.get_skip_attrs())
        drop_attrs = []
        for attr in attrs:
            if (attr in skip_attrs_set):
                drop_attrs.append(attr)

        logging.debug("attrs {}".format(attrs))
        logging.debug("cat_attrs {}".format(attrs_cat))
        logging.debug("drop_attrs {}".format(drop_attrs))
        logging.debug("attrs_without_dropped {}".format(attrs_without_dropped))

        table = pd.read_csv(data_path, names=attrs,
            delimiter=",", header=None)
        num_rows += len(table)


        logging.debug("dropping columns")
        table = table.drop(drop_attrs, axis="columns")
        logging.debug("ohe data")
        logging.debug("Dimensions before ohe row {} col {}".format(table.shape[0], table.shape[1]))
        table_np = transform_data(table, attrs_without_dropped, attrs_cat , False)
        logging.debug("Dimensions after ohe row {} col {}".format(table_np.shape[0], table_np.shape[1]))
        data_out_path = os.path.join(db_ohe_path, rel_name +".csv")
        logging.debug(data_out_path)
        np.savetxt(data_out_path, np.asarray(table_np), delimiter=',')

    logging.info("Size of database on disk of the database {} {} ".
        format(database.name, get_dir_size_str(db_ohe_path)))
    logging.info("Number of rows: {} ".
        format(num_rows))

    # Needed for memory usage
    shutil.rmtree(db_ohe_path, ignore_errors=True)


def eval_ohe_and_dump_join(username: str, password: str,
    database: Database, query: Query, dump_path: str):
    join_path = os.path.join(dump_path, "join.csv")
    join_ohe_path = os.path.join(dump_path, "join.csv")
    logging.debug("join path {}".format(join_path))

    logging.info("OHE and dumping join from {}".format(database.name))

    database_psql = DatabasePsql(host_name="",user_name=username,
        password=password, database=database)

    database_psql.drop_database()
    database_psql.create_database(database)

    database_psql.evaluate_join(query, num_repetitions=1,
        order_by=DumpConf.OrderRelation.JOIN_ATTRIBUTE)
    database_psql.dump_join(query, join_path)

    # Need due to memory usage.
    database_psql.drop_database()

    table = pd.read_csv(join_path, names=query.get_non_join_attr_names_ordered(),
            delimiter=",", header=None)

    table_np = transform_data(table, query.get_non_join_attr_names_ordered(),
        query.get_non_join_cat_attr_names_ordered(), False)
    data_out_file_path = join_ohe_path
    logging.debug(data_out_file_path)
    logging.info("Dimensions row {} col {}".format(table_np.shape[0], table_np.shape[1]))
    np.savetxt(data_out_file_path, np.asarray(table_np), delimiter=',')

    logging.info("Size of database on disk of the database {} {} ".
        format(database.name, get_file_size_str(data_out_file_path)))
    os.remove(data_out_file_path)


def ohe_and_dump_join_and_databases(real_dataset_path: str, system_tests_path: str, username: str, password: str,
    database_size: bool, join_size: bool):
    db_names = ["retailer", "favorita", "yelp",
        "retailer_ohe", "favorita_ohe", "yelp_ohe"]
    db_configs =  {
        "retailer":
        {
            "db_config_path" : "test_real_data/databases/usretailer/database_specs_usretailer_100.conf",
            "query_config_path": "test_real_data/queries/usretailer/query_specs_location_root.conf"
        },
        "favorita":
        {
            "db_config_path" : "test_real_data/databases/favorita/database_specs_favorita_100.conf",
            "query_config_path": "test_real_data/queries/favorita/query_specs_stores_root.conf"
        },
        "yelp":
        {
            "db_config_path" : "test_real_data/databases/yelp/database_specs_yelp_100.conf",
            "query_config_path": "test_real_data/queries/yelp/query_specs_business_root.conf"
        },
        "retailer_ohe":
        {
            "db_config_path" : "test_real_data_ohe/databases/usretailer/database_specs_usretailer_1_100.conf",
            "query_config_path": "test_real_data_ohe/queries/usretailer/query_specs_item_root.conf"
        },
        "favorita_ohe":
        {
            "db_config_path" : "test_real_data_ohe/databases/favorita/database_specs_favorita_1_100.conf",
            "query_config_path": "test_real_data_ohe/queries/favorita/query_specs_items_root.conf"
        },
        "yelp_ohe":
        {
            "db_config_path" : "test_real_data_ohe/databases/yelp/database_specs_yelp_1_100.conf",
            "query_config_path": "test_real_data_ohe/queries/yelp/query_specs_user_root.conf"
        }
    }

    for db_name in db_configs.keys():
        db_configs[db_name]["db_ohe_path"] = os.path.join(
            real_dataset_path, db_name + "_size_on_disk")
        db_configs[db_name]["db_config_path"] = os.path.join(
            system_tests_path, db_configs[db_name]["db_config_path"],
            )
        db_configs[db_name]["dump_path"] = os.path.join(
            real_dataset_path, db_name)
        db_configs[db_name]["query_config_path"] = os.path.join(system_tests_path, db_configs[db_name]["query_config_path"])

    for db_name in db_names:
        db_config = db_configs[db_name]
        db_config_path = db_config["db_config_path"]
        query_config_path = db_config["query_config_path"]

        database = Database(db_config_path, "")
        query = Query(query_config_path=query_config_path, database=database)
        if database_size:
            ohe_and_dump_database(db_configs[db_name], query, database)

        if join_size:
            eval_ohe_and_dump_join(username, password, database,
                query, db_config["dump_path"])


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
    parser.add_argument("--database_size", default=False, action='store_true')
    parser.add_argument("--join_size", default=False, action='store_true')
    args = parser.parse_args(args)

    data_path = args.data_path
    username = args.username
    password = args.password
    system_tests_path = args.system_tests_path
    compute_database_size = args.database_size
    compute_join_size = args.join_size
    ohe_and_dump_join_and_databases(data_path, system_tests_path,
        username, password, compute_database_size, compute_join_size)


if __name__ == "__main__":
    init_logging()
    set_logging_level(logging.INFO)
    main(sys.argv[1:])
