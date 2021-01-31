import itertools as it
import random 
import numpy as np
import pandas as pd
import argparse
import math

db_config_str = """
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
                "data_path": "/home/popina/Figaro/data/generated_databases/{test_num}/R.csv"
            },
            {
                "name": "S",
                "attributes": 
                [ {"name": "A","type": "int"}, 
                {"name": "B","type": "int"},
{S_non_k_attr_specs}
                ],
                "primary_key": ["A", "B"],
                "data_path": "/home/popina/Figaro/data/generated_databases/{test_num}/S.csv"
            },
            {
                "name": "T",
                "attributes": 
                [ {"name": "B","type": "int"}, 
                {"name": "C","type": "int"},
{T_non_k_attr_specs}
                ],
                "primary_key": ["B", "C"],
                "data_path": "/home/popina/Figaro/data/generated_databases/{test_num}/T.csv"
            },
            {
                "name": "U",
                "attributes": 
                [ {"name": "C","type": "int"}, 
{U_non_k_attr_specs}
                ],
                "primary_key": ["C"],
                "data_path": "/home/popina/Figaro/data/generated_databases/{test_num}/U.csv"
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
                        "num_tuples": {num_relation_rows}
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
                        "num_tuples": {num_relation_rows}
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

def str_non_pk_atributes(start_name: str, num: int):
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


def str_generate_non_pk_atributes(start_name: str, num: int):
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



if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--test_num", action="store", 
                        dest="test_num", required=True)                
    parser.add_argument("-j", "--num_join_rows", action="store",
                        dest="num_join_rows", required=True)
    parser.add_argument("-r", "--num_relation_rows", action="store",
                    dest="num_relation_rows", required=True)                        
    parser.add_argument("-c", "--num_non_pk_attrs", action="store",
                        dest="num_non_pk_attrs", required=True)                    
    args = parser.parse_args()

    num_non_pk_attrs = int(args.num_non_pk_attrs)
    num_join_rows = int(args.num_join_rows)
    num_relation_rows=  int(args.num_relation_rows)
    degree = num_join_rows / num_relation_rows
    domain_size = math.ceil(num_relation_rows / degree)

    test_num = args.test_num

    db_config_str = db_config_str.replace("{test_num}", str(test_num))
    db_config_str = db_config_str.replace("{domain_size}", str(domain_size))
    db_config_str = db_config_str.replace("{num_relation_rows}", str(num_relation_rows))
    relations = ["R", "S", "T", "U"]
    for relation_name in relations:
        str_gen_non_pk_attrs = str_generate_non_pk_atributes(relation_name, num_non_pk_attrs)
        str_to_rep = "{" + relation_name + "_gen_non_k_attr_specs}"
        db_config_str = db_config_str.replace(str_to_rep, str_gen_non_pk_attrs)

        str_non_pk_attrs = str_non_pk_atributes(relation_name, num_non_pk_attrs)
        str_to_rep = "{" + relation_name + "_non_k_attr_specs}"
        db_config_str = db_config_str.replace(str_to_rep, str_non_pk_attrs)

    print(db_config_str)