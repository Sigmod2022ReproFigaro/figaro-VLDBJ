from cgi import test
from faulthandler import disable
import evaluation.evaluator as eval
import numpy as np
import pandas as pd
import argparse
import os
from openpyxl.utils.cell import get_column_letter
import plots_and_results.real_data.performance_percent as perf_percent
import plots_and_results.real_data.performance_threads_join_orders as perf_join_ord
import plots_and_results.synthetic_data.synthetic_accuracy as syn_accur
import plots_and_results.synthetic_data.synthetic_performance as syn_perf
import logging
from data_management.query import Query
import argparse
import sys
import numpy as np
from evaluation.custom_logging import init_logging, set_logging_level
import matplotlib.pyplot as plt
import json


def test_perf_system_set(test_system_perf_path: str, is_figaro: bool):
    with open(test_system_perf_path, 'r') as tests_file_json:
        tests_system_perf_json = json.load(tests_file_json)

    tests_system_perf = tests_system_perf_json["systems"]
    for test_system_perf in tests_system_perf:
        disabled = True
        system_conf_path = test_system_perf["system_conf_path"]
        if is_figaro and "thin_diag" in system_conf_path and \
            test_system_perf["category"] == "figaro":
            disabled = False
        elif not is_figaro and test_system_perf["category"] == "postprocess" and \
            "lapack/col_major/system_test_only_r" in system_conf_path:
            disabled = False

        test_system_perf["disable"] = disabled
    with open(test_system_perf_path, 'w') as tests_file_json:
        tests_json = json.dump(tests_system_perf_json, tests_file_json, indent=4)


def test_perf_data_set_set(test_perf_data_set_path: str, data_set: str):
    with open(test_perf_data_set_path, 'r') as tests_file_json:
        tests_perf_data_set_json = json.load(tests_file_json)

    tests_perf_data_set = tests_perf_data_set_json["data_sets"]
    for test_perf_data_set in tests_perf_data_set:
        disabled = True
        database_conf_path = test_perf_data_set["database_conf_path"]
        if (data_set in database_conf_path) and ("20" in database_conf_path):
            disabled = False

        test_perf_data_set["disable"] = disabled

    with open(test_perf_data_set_path, 'w') as tests_file_json:
        tests_json = json.dump(tests_perf_data_set_json, tests_file_json, indent=4)


def test_perf_query_set(test_perf_query_path: str, skip_list: list, label: str,
    is_figaro: bool):

    with open(test_perf_query_path, 'r') as tests_file_json:
        tests_perf_query_json = json.load(tests_file_json)

    tests_perf_query = tests_perf_query_json["query"]
    tests_perf_query["evaluation_hint"]["skip_attributes"] = skip_list
    tests_perf_query["evaluation_hint"]["figaro"] = is_figaro
    tests_perf_query["evaluation_hint"]["label_name"] = label


    with open(test_perf_query_path, 'w') as tests_file_json:
        tests_json = json.dump(tests_perf_query_json, tests_file_json, indent=4)


def set_system(system_tests_path: str, is_figaro: bool):
    test_perf_path = os.path.join(system_tests_path, "test_real_data", "tests",
        "percent", "performance.conf")
    test_perf_system_set(test_perf_path, is_figaro)
    test_perf_anal_path = os.path.join(system_tests_path, "test_real_data", "tests",
        "percent", "performance_analysis.conf")
    test_perf_system_set(test_perf_anal_path, is_figaro)


def set_dataset(system_tests_path: str, data_set: str):
    test_perf_path = os.path.join(system_tests_path, "test_real_data", "dataset_conf",
        "dataset_percent.conf")
    test_perf_data_set_set(test_perf_path, data_set)


def set_features(system_tests_path: str, data_set: str, features: list, label: str,
    is_figaro: bool):
    query_dict = {
        "yelp": "query_specs_business_root.conf",
        "favorita": "query_specs_stores_root.conf",
        "retailer": "query_specs_location_root.conf"}
    data_set_name_dict = {
        "yelp": "yelp",
        "favorita": "favorita",
        "retailer": "usretailer"}

    feature_names_dict = {
        "yelp": ["ReviewId","CityId","StateId","Latitude","Longitude","StarsB","ReviewCountB","IsOpen","StarsR","ReviewYear","ReviewMonth","ReviewDay","Useful","Funny","Cool","ReviewCountU","YearJoined","UserUseful","UserFunny","UserCool","Fans","AverageStars","ComplimentHot","ComplimentMore","ComplimentProfile","ComplimentCute","ComplimentList","ComplimentNote","ComplimentPlain","ComplimentCool","ComplimentFunny","ComplimentWriter","ComplimentPhotos","CategoryId","DayOfWeekC","CheckinCount","DayOfWeekH","HoursH"],
        "favorita": ["UnitSales","OnPromotion","OilPrize","HolidayType","Locale","LocaleId","Transferred","Transactions","City","State","StoreType","Cluster","Family","ItemClass","Perishable"],
        "retailer": ["InventoryUnits","RgnCd","ClimZnNbr","TotalAreaSqFt","SellAreaSqFt","AvgHi","SuperTargetDistance","SuperTargetDriveTime","TargetDistance","TargetDriveTime","WalmartDistance","WalmartDriveTime","WalmartSuperCenterDistance","WalmartSuperCenterDriveTime","Rain","Snow","MaxTemp","MinTemp","MeanWind","Thunder","SubCategory","Category","CategoryCluster","Prize","Population","White","Asian","Pacific","Black","MedianAge","OccupiedHouseUnits","HouseUnits","Families","Households","HusbWife","Males","Females","HouseholdChildren","Hispanic"]}

    skip_set = feature_names_dict[data_set]
    for feature in features:
        #print("feature", feature)
        #print(skip_set)
        skip_set.remove(feature)

    skip_set.remove(label)

    test_perf_query_path = os.path.join(system_tests_path,
        "test_real_data", "queries", data_set_name_dict[data_set],
        query_dict[data_set])
    #print(test_perf_query_path)
    #print(skip_set)
    test_perf_query_set(test_perf_query_path, list(skip_set), label, is_figaro)



def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--system_tests_path", action="store",
                        dest="system_tests_path", required=True)
    parser.add_argument('--figaro', default=False, action='store_true')
    parser.add_argument("-d", "--data_set", action="store",
                        dest="data_set", required=True)
    parser.add_argument("--features", action="store",
                        dest="features",  nargs='*', required=False)
    parser.add_argument("--label", action="store",
                        dest="label", required=True)
    args = parser.parse_args(args)

    system_tests_path = args.system_tests_path
    is_figaro = bool(args.figaro)
    data_set = args.data_set
    features = args.features
    label = args.label

    set_system(system_tests_path, is_figaro)
    set_dataset(system_tests_path, data_set)
    set_features(system_tests_path, data_set, features, label, is_figaro)



if __name__ == "__main__":
    init_logging()
    set_logging_level(logging.INFO)
    main(sys.argv[1:])
