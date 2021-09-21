import itertools as it
import logging
import sys
import numpy as np
import pandas as pd
import argparse
import math
from evaluation.custom_logging import init_logging

gl_db_config_str = """
{
    "database":
    {
        "name": "DB{test_num}",
        "relations":
        [
            {
                "name": "R",
                "attributes":
                [ {"name": "A","type": "int"},
{R_non_k_attr_specs}
                ],
                "primary_key": ["A"],
                "data_path": "{data_path}/generated_databases/{test_num}/R.csv"
            },
            {
                "name": "S",
                "attributes":
                [ {"name": "A","type": "int"},
                {"name": "B","type": "int"},
{S_non_k_attr_specs}
                ],
                "primary_key": ["A", "B"],
                "data_path": "{data_path}/generated_databases/{test_num}/S.csv"
            },
            {
                "name": "T",
                "attributes":
                [ {"name": "B","type": "int"},
                {"name": "C","type": "int"},
{T_non_k_attr_specs}
                ],
                "primary_key": ["B", "C"],
                "data_path": "{data_path}/generated_databases/{test_num}/T.csv"
            },
            {
                "name": "U",
                "attributes":
                [ {"name": "C","type": "int"},
{U_non_k_attr_specs}
                ],
                "primary_key": ["C"],
                "data_path": "{data_path}/generated_databases/{test_num}/U.csv"
            }
        ],
        "stats":
        {
            "distribution_specs":
            {
                "relations":
                [
                    {
                        "name": "R",
                        "distribution": "uniform",
                        "attribute_domains":
                        [
                            {"name": "A", "start": 1, "end": 1000},
{R_gen_non_k_attr_specs}
                        ],
                        "num_tuples": 1000
                    },
                    {
                        "name": "S",
                        "distribution": "uniform",
                        "attribute_domains":
                        [
                            {"name": "A", "start": 1, "end": 1000},
                            {"name": "B", "start": 1, "end": {domain_size}},
{S_gen_non_k_attr_specs}
                        ],
                        "num_tuples": {relation_num_rows}
                    },
                    {
                        "name": "T",
                        "distribution": "uniform",
                        "attribute_domains":
                        [
                            {"name": "B", "start": 1, "end": {domain_size}},
                            {"name": "C", "start": 1, "end": 1000},
{T_gen_non_k_attr_specs}
                        ],
                        "num_tuples": {relation_num_rows}
                    },
                     {
                        "name": "U",
                        "distribution": "uniform",
                        "attribute_domains":
                        [
                            {"name": "C", "start": 1, "end": 1000},
{U_gen_non_k_attr_specs}
                        ],
                        "num_tuples": 1000
                    }
                ]
            },
            "generate_data": true
        }
    }
}
"""

gl_db_config_cart_prod_str = """

{
    "database":
    {
        "name": "DBCartesianProduct{test_num}",
        "relations":
        [
            {
                "name": "R",
                "attributes":
                [
                    {"name": "A", "type": "int"},
{R_non_join_attr_specs}
                ],
                "primary_key": [],
                "data_path": "{data_path}/generated_databases/cartesian_product/{test_num}/R.csv"
            },
            {
                "name": "S",
                "attributes":
                [
                    {"name": "A", "type": "int"},
{S_non_join_attr_specs}
                ],
                "primary_key": [],
                "data_path": "{data_path}/generated_databases/cartesian_product/{test_num}/S.csv"
            }
        ],
        "stats":
        {
            "distribution_specs":
            {
                "relations":
                [
                    {
                        "name": "R",
                        "distribution": "uniform",
                        "attribute_domains":
                        [
                            {"name": "A", "start": 1, "end": {R_domain_size}},
{R_gen_non_join_attr_specs}
                        ],
                        "num_tuples": {R_relation_num_rows}
                    },
                    {
                        "name": "S",
                        "distribution": "uniform",
                        "attribute_domains":
                        [
                            {"name": "A", "start": 1, "end": {S_domain_size}},
{S_gen_non_join_attr_specs}
                        ],
                        "num_tuples": {S_relation_num_rows}
                    }
                ]
            },
            "generate_data": true
        }
    }
}
"""

def str_non_join_atributes(start_name: str, num: int):
    str_gen = ""
    attribute_config_str = "{\"name\": \"{start_name}{num}\",\"type\": \"float\"},"
    for idx in range(1, num + 1):
        conf_str = attribute_config_str.replace("{start_name}", start_name).replace("{num}", str(idx))
        if idx == num:
            conf_str = conf_str[:-1]
        else:
            conf_str += "\n"

        conf_str = " " * 18 + conf_str
        str_gen += conf_str

    return str_gen


def str_generate_non_join_atributes(start_name: str, num: int):
    str_gen = ""
    attribute_config_str = "{\"name\": \"{start_name}{num}\", \"start\": -3, \"end\": 3},"
    for idx in range(1, num + 1):
        conf_str = attribute_config_str.replace("{start_name}", start_name).replace("{num}", str(idx))
        if idx == num:
            conf_str = conf_str[:-1]
        else:
            conf_str += "\n"
        conf_str = " " * 28 + conf_str
        str_gen += conf_str

    return str_gen


def four_path_add_cmd_args(subparser):
    subparser.add_argument("-t", "--test_name", action="store",
                        dest="test_name", required=True)
    subparser.add_argument("-j", "--num_join_rows", action="store",
                        dest="num_join_rows", required=True)
    subparser.add_argument("-r", "--relation_num_rows", action="store",
                    dest="relation_num_rows", required=True)
    subparser.add_argument("-c", "--num_non_pk_attrs", action="store",
                        dest="num_non_pk_attrs", required=True)


def path_four_config_gen(args):
    global db_config_str
    num_non_pk_attrs = int(args.num_non_pk_attrs)
    num_join_rows = int(args.num_join_rows)
    relation_num_rows=  int(args.relation_num_rows)
    degree = num_join_rows / relation_num_rows
    domain_size = math.ceil(relation_num_rows / degree)

    test_num = args.test_num

    db_config_str = gl_db_config_str
    db_config_str = db_config_str.replace("{test_num}", str(test_num))
    db_config_str = db_config_str.replace("{domain_size}", str(domain_size))
    db_config_str = db_config_str.replace("{relation_num_rows}", str(relation_num_rows))
    relations = ["R", "S", "T", "U"]
    for relation_name in relations:
        str_gen_non_pk_attrs = str_generate_non_join_atributes(relation_name, num_non_pk_attrs)
        str_to_rep = "{" + relation_name + "_gen_non_k_attr_specs}"
        db_config_str = db_config_str.replace(str_to_rep, str_gen_non_pk_attrs)

        str_non_pk_attrs = str_non_join_atributes(relation_name, num_non_pk_attrs)
        str_to_rep = "{" + relation_name + "_non_k_attr_specs}"
        db_config_str = db_config_str.replace(str_to_rep, str_non_pk_attrs)

    print(db_config_str)


def two_cart_prod_add_cmd_args(subparser):
    subparser.add_argument("-t", "--test_num", action="store",
                        dest="test_num", required=True)

    subparser.add_argument("--r_domain_size", action="store",
                    dest="r_domain_size", required=True)
    subparser.add_argument("--s_domain_size", action="store",
                dest="s_domain_size", required=True)

    subparser.add_argument("--r_relation_num_non_join_attrs", action="store",
                    dest="r_relation_num_non_join_attrs", required=True)
    subparser.add_argument("--s_relation_num_non_join_attrs", action="store",
                    dest="s_relation_num_non_join_attrs", required=True)

    subparser.add_argument("-r", "--r_relation_num_rows", action="store",
                    dest="r_relation_num_rows", required=True)
    subparser.add_argument("-s", "--s_relation_num_rows", action="store",
                    dest="s_relation_num_rows", required=True)
    subparser.add_argument("-d", "--data_path", action="store",
                    dest="data_path", required=True)


def two_cart_prod_config_gen(args):
    global gl_db_config_cart_prod_str

    r_domain_size = int(args.r_domain_size)
    s_domain_size = int(args.s_domain_size)

    r_relation_num_non_join_attrs = int(args.r_relation_num_non_join_attrs)
    s_relation_num_non_join_attrs = int(args.s_relation_num_non_join_attrs)

    r_relation_num_rows = int(args.r_relation_num_rows)
    s_relation_num_rows = int(args.s_relation_num_rows)

    test_num = int(args.test_num)

    db_config_cart_prod_str = gl_db_config_cart_prod_str
    db_config_cart_prod_str = db_config_cart_prod_str.replace("{data_path}", str(args.data_path))
    db_config_cart_prod_str = db_config_cart_prod_str.replace("{test_num}", str(test_num))

    db_config_cart_prod_str = db_config_cart_prod_str.replace("{R_domain_size}", str(r_domain_size))
    db_config_cart_prod_str = db_config_cart_prod_str.replace("{S_domain_size}", str(s_domain_size))


    db_config_cart_prod_str = db_config_cart_prod_str.replace("{R_relation_num_rows}", str(r_relation_num_rows))
    db_config_cart_prod_str = db_config_cart_prod_str.replace("{S_relation_num_rows}", str(s_relation_num_rows))
    db_config_path = args.db_conf_path_output

    relations = ["R", "S"]
    num_non_join_attrs = {"R": r_relation_num_non_join_attrs, "S": s_relation_num_non_join_attrs}
    for relation_name in relations:
        str_gen_non_pk_attrs = str_generate_non_join_atributes(relation_name,
        num_non_join_attrs[relation_name])
        str_to_rep = "{" + relation_name + "_gen_non_join_attr_specs}"
        db_config_cart_prod_str = db_config_cart_prod_str.replace(str_to_rep, str_gen_non_pk_attrs)

        str_non_pk_attrs = str_non_join_atributes(relation_name,
        num_non_join_attrs[relation_name])
        str_to_rep = "{" + relation_name + "_non_join_attr_specs}"
        db_config_cart_prod_str = db_config_cart_prod_str.replace(str_to_rep, str_non_pk_attrs)


    with open(db_config_path, "w") as db_config_file:
        db_config_file.write(db_config_cart_prod_str)

    logging.debug(db_config_cart_prod_str)


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("--db_conf_path_output", action="store", dest="db_conf_path_output", required=True)
    subparsers = parser.add_subparsers(dest="option")
    four_path_parser = subparsers.add_parser("four_path")
    two_cart_prod_parser = subparsers.add_parser("two_cart_prod")

    two_cart_prod_add_cmd_args(two_cart_prod_parser)
    four_path_add_cmd_args(four_path_parser)

    args = parser.parse_args(args)

    option = args.option
    if option == "four_path":
        path_four_config_gen(args)
    elif option == "two_cart_prod":
        two_cart_prod_config_gen(args)


if __name__ == "__main__":
    init_logging()
    main(sys.argv[1:])


