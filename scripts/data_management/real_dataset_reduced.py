import os
import logging
import argparse
import sys
import data_management.database_full_reducer as dfr
from evaluation.custom_logging import init_logging

def percent_generator(data_path: str, system_tests_path: str, username: str, password: str):
    real_data_sets = ["retailer", "favorita", "yelp"]
    config_paths = {
        "retailer":
        {
            "db_config_path": "test_real_data/databases/usretailer/database_specs_usretailer.conf",
            "query_config_path": "test_real_data/queries/usretailer/query_specs_location_root.conf"
        },
        "favorita":
        {
            "db_config_path": "test_real_data/databases/favorita/database_specs_favorita.conf",
            "query_config_path": "test_real_data/queries/favorita/query_specs_oil_root.conf"
        },
        "yelp":
        {
            "db_config_path": "test_real_data/databases/yelp/database_specs_yelp.conf",
            "query_config_path": "test_real_data/queries/yelp/query_specs_user_root.conf"
        }
    }

    for real_data_set in real_data_sets:
        db_config_path = os.path.join(system_tests_path,
        config_paths[real_data_set]["db_config_path"])
        query_config_path = os.path.join(system_tests_path, config_paths[real_data_set]["query_config_path"])
        logging.info(real_data_set)
        logging.info(db_config_path)
        logging.info(query_config_path)
        args = ["-d", db_config_path, "-q", query_config_path,
                "-p", password, "-u", username, "--percent"]
        dfr.main(args)


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
    percent_generator(data_path, system_tests_path, username, password)


if __name__ == "__main__":
    init_logging()
    main(sys.argv[1:])