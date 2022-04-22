import evaluation.evaluator as eval
import numpy as np
import pandas as pd
import argparse
import os
from openpyxl.utils.cell import get_column_letter
import data_management.real_dataset_reduced as real_dataset_reduced
import data_management.real_dataset_reduced_ohe as real_dataset_reduced_ohe
import data_generators.cartesian_product_generator  as syn_perf_generator
import data_generators.accuracy_cart_prod_generator  as syn_accur_generator
import logging
from data_management.query import Query
import argparse
import sys
import gdown
import shutil
import numpy as np
from evaluation.custom_logging import init_logging, set_logging_level
import matplotlib.pyplot as plt

WEB_DATA_SET_PATH = "https://drive.google.com/u/0/uc?id=1xlGxL8q8EVliKsshgBS94y2Ej_siXYio&export=download&confirm=t"

def generate_data(username: str, password: str, system_tests_path: str,
    data_path: str, data_type: str):
    args = ["-s", system_tests_path, "-u", username, "-p", password,
            "--data_path", data_path]
    args_ohe = ["--data_path", data_path]
    args_syn_accur = ["-s", system_tests_path, "--data_path", data_path]
    if data_type == "download_real_data":
        download_zip_path = os.path.join(data_path, "test.zip")
        gdown.download(WEB_DATA_SET_PATH, download_zip_path)
        shutil.unpack_archive(download_zip_path, data_path)
        os.remove(download_zip_path)
    if data_type == "real_data":
        real_dataset_reduced.main(args)
    elif data_type == "real_data_ohe":
        real_dataset_reduced_ohe.main(args_ohe)
    elif data_type == "syn_perf":
        syn_perf_generator.main(args)
    elif data_type == "syn_accur":
        syn_accur_generator.main(args_syn_accur)



def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--password", action="store",
                        dest="password", required=True)
    parser.add_argument("-u", "--username", action="store",
                        dest="username", required=True)
    parser.add_argument("-d", "--data_path", action="store",
                        dest="data_path", required=True)
    parser.add_argument("-s", "--system_tests_path", action="store",
                        dest="system_tests_path", required=True)
    parser.add_argument("-t", "--data_type", action="store",
                        dest="data_type", required=True)
    args = parser.parse_args(args)

    system_tests_path = args.system_tests_path
    username = args.username
    password = args.password
    data_type = args.data_type
    data_path = args.data_path

    logging.info("Generating data: {}".format(data_type))

    data_types = ["real_data", "real_data_ohe", "syn_perf", "syn_accur"]
    if data_type == "all":
        for data_type in data_types:
            generate_data(username, password, system_tests_path, data_path, data_type)
    else:
        generate_data(username, password, system_tests_path, data_path, data_type)



if __name__ == "__main__":
    init_logging()
    set_logging_level(logging.INFO)
    main(sys.argv[1:])