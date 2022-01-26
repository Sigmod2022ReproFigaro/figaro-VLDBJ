import logging
import numpy as np
import pandas as pd
import math
from evaluation.custom_logging import init_logging
import os
import argparse
import json
import sys

def arrange_up_triang(n: int):
    A  = np.zeros( (n, n))
    idx = 1
    for row_idx in range(n):
        for col_idx in range(row_idx, n):
            A[row_idx, col_idx] = idx
            idx += 1
    return A


def generate_cart_prod_accur(m_1: int, n_1: int, m_2: int, n_2: int):
    Q_x = np.zeros((m_1, n_1))
    v = np.zeros(m_1)
    for idx in range(v.size):
        i = idx + 1 if idx < v.size - 1 else idx
        #v[idx] = 2. ** (-i / 2.)
        #v[idx] = np.random.uniform()
        v[idx] = 1 / math.sqrt(m_1)

    #norm_v = np.linalg.norm(v)
    #v /= norm_v

    #logging.info(v)
    for col_idx in range(0, n_1):
        for row_idx in range(m_1):
            if col_idx == 0:
                val = v[row_idx]
            elif row_idx == 0:
                val = v[col_idx]
            elif (row_idx == col_idx):
                val = (v[col_idx] ** 2 - v[0] - 1) / (v[0]+1)
            else:
                val = (v[col_idx] * v[row_idx]) / (v[0] + 1)
            Q_x[row_idx, col_idx] = val

    R_x = arrange_up_triang(n_1)
    #logging.info(R_x)
    #logging.info(Q_x[:, 0])

    X = Q_x @ R_x
    #Y = np.arange(m_1 * n_1, m_1 * n_1 + m_2 * n_2).reshape((m_2, n_2))
    Y = np.random.randint(10000, size =(m_2, n_2))
    X_ones = np.ones((m_1, 1))
    Y_ones = np.ones((m_2, 1))
    X = np.hstack((X_ones, X))
    Y = np.hstack((Y_ones, Y))
    X_df = pd.DataFrame(X).astype({0: "int32"})
    Y_df = pd.DataFrame(Y).astype({0: "int32"})

    return X_df, Y_df


def dump_cart_prod_accur(X_df: pd.DataFrame, Y_df: pd.DataFrame,
    output_path: str, db_idx: int):
    tmp_output_path = output_path.replace("{db_idx}", str(db_idx))
    tmp_output_path_x = tmp_output_path.replace("{rel_name}", "X")
    tmp_output_path_y = tmp_output_path.replace("{rel_name}", "Y")
    X_df.to_csv(tmp_output_path_x, index=False, header=False)
    Y_df.to_csv(tmp_output_path_y, index=False, header=False)


def create_dirs(path: str, num_dirs: int):
    for i in range(1, num_dirs + 1):
        tmp_path = os.path.join(path, str(i))
        if not os.path.exists(tmp_path):
            os.makedirs(tmp_path)


def generate_relation_schema_json(relation_name: str,
    num_non_join_attrs: int, db_output_path: str):
    relation_json = {"name": relation_name}
    attrs_a = [{"name": "A", "type": "int"}]
    for non_join_attr_idx in range(num_non_join_attrs):
        attr_name = relation_name + str(non_join_attr_idx + 1)
        attrs_a.append({"name": attr_name, "type": "float"})

    relation_json["attributes"] = attrs_a
    relation_json["primary_key"] = []
    relation_json["data_path"]  = db_output_path.replace(
            "{rel_name}", relation_name)

    return relation_json

def generate_db_schema(system_tests_path: str, num_non_join_attrs: int, db_output_path: str, db_idx: int):
    db_output_path_t = db_output_path.replace("{db_idx}", str(db_idx))
    x_rel_schema_json = generate_relation_schema_json("X", num_non_join_attrs, db_output_path_t)
    y_rel_schema_json = generate_relation_schema_json("Y", num_non_join_attrs, db_output_path_t)

    relations_json = [x_rel_schema_json, y_rel_schema_json]
    db_name = "DBCartesianProductAccuracy{db_idx}".replace("{db_idx}",
        str(db_idx))
    db_json = {"database": {"name": db_name,
        "relations": relations_json} }

    db_conf_path = os.path.join(system_tests_path, "test_syn_accur/databases/database_specs{db_idx}.conf").replace("{db_idx}", str(db_idx))

    with open (db_conf_path, 'w') as db_conf_file:
        json.dump(db_json, db_conf_file, indent=4)


def vary_row_and_col_num(system_tests_path: str, data_path: str):
    db_idx = 1
    db_dirs_path = os.path.join(data_path, "generated_databases/accuracy")
    db_output_path = os.path.join(db_dirs_path, "{db_idx}/{rel_name}.csv")


    num_rows_a = [512, 1024, 2048, 4096, 8192]
    num_cols_a = [16, 64, 256, 1024, 4096]
    create_dirs(db_dirs_path, len(num_rows_a) * len(num_cols_a))

    for num_rows in num_rows_a:
        for num_cols in num_cols_a:
            if num_rows <= num_cols:
                continue
            # Reproducible generation of datasets.
            np.random.seed(1)
            X_df, Y_df = generate_cart_prod_accur(num_rows, num_cols, num_rows, num_cols)
            generate_db_schema(system_tests_path, num_cols,
                db_output_path, db_idx)
            dump_cart_prod_accur(X_df, Y_df, db_output_path, db_idx)
            logging.info("Finished generation X: of database {} {} x {}".format(db_idx, num_rows, num_cols))
            logging.info("Generated: {} x {}".format(num_rows, num_cols))
            db_idx += 1


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--system_tests_path", action="store",
                    dest="system_tests_path", required=True)
    parser.add_argument("-d", "--data_path", action="store",
                        dest="data_path", required=True)
    args = parser.parse_args(args)

    data_path = args.data_path
    system_tests_path = args.system_tests_path

    vary_row_and_col_num(system_tests_path, data_path)


if __name__ == "__main__":
    init_logging()
    main(sys.argv[1:])

