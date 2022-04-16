import logging
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from data_management.query import Query
import argparse
import sys
import numpy as np
from evaluation.custom_logging import init_logging


def remove_dangling_tuples(username: str, password: str, database: Database, query: Query,
    percent: float = None):
    database_psql = DatabasePsql(host_name="", user_name=username,
        password=password, database=database)
    database_psql.drop_database()
    database_psql.create_database(database)
    database_psql.full_reducer_join(query=query, percent=percent)
    database_psql.drop_database()


def vary_percentage(username: str, password: str, database: Database, query: Query):
    perc_range = np.append(np.linspace(0.01, 0.1, 10), np.linspace(0.1, 1, 10))
    for percent in perc_range:
        per_float = round(percent, 2)
        print(per_float)
        remove_dangling_tuples(username=username, password=password,
            database=database, query=query, percent=per_float)


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--db_config_path", action="store",
                        dest="db_config_path", required=True)
    parser.add_argument("-q", "--query_config_path", action="store",
                        dest="query_config_path", required=True)
    parser.add_argument("-p", "--password", action="store",
                        dest="password", required=True)
    parser.add_argument("-u", "--username", action="store",
                        dest="username", required=True)
    parser.add_argument("--percent", action="store_true",
        dest="percent", required=False)
    args = parser.parse_args(args)

    db_config_path = args.db_config_path
    query_config_path = args.query_config_path
    username = args.username
    password = args.password
    percent = args.percent

    database = Database(db_config_path, "")
    query = Query(query_config_path=query_config_path, database=database)
    if percent:
        vary_percentage(username, password, database, query)
    else:
        remove_dangling_tuples(username, password, database, query)




if __name__ == "__main__":
    init_logging()
    main(sys.argv[1:])
