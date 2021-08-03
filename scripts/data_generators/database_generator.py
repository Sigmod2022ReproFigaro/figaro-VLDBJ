import json
from logging import log
import logging
import os
import sys
import argparse
from data_management.database import Database
from data_management.database_psql import DatabasePsql
from data_management.query import Query
from data_management.relation import Relation
from data_generators.relation_generator import RelationGeneratedSpecs
from data_generators.relation_generator import RelationGenerator
from  typing import List
from  typing import Tuple
import pandas as pd
from timeit import default_timer as timer

from evaluation.custom_logging import init_logging

class DatabaseGenerator:
    def __init__(self, generate_specs_path: str, database: Database, query: Query,
    username: str, password: str):
        with open(generate_specs_path) as json_file:
            json_db_schema = json.load(json_file)["database"]

        self.json_rel_gen_specs = json_db_schema["stats"]["distribution_specs"]["relations"]

        relation_generators = []
        relation_gen_specs = []
        self.database = database
        for json_rel_gen_spec in self.json_rel_gen_specs:
            relation = database.get_relation(json_rel_gen_spec["name"])
            relation_generator = RelationGenerator(relation,
            query.get_join_attrs(relation.name))
            relation_generators.append(relation_generator)

            relation_gen_spec = RelationGeneratedSpecs(json_rel_gen_spec)
            relation_gen_specs.append(relation_gen_spec)

        self.relation_generators = relation_generators
        self.relation_gen_specs = relation_gen_specs
        self.query = query
        self.username = username
        self.password = password


    def remove_dangling_tuples(self):
        database_psql = DatabasePsql(host_name="", user_name=self.username,
            password=self.password, database=self.database)
        database_psql.drop_database()
        database_psql.create_database(self.database)
        database_psql.full_reducer_join(self.query)
        database_psql.drop_database()


    def generate(self, full_reducer: bool):
        rel_data_tuples = []

        for rel_idx, relation_generator in enumerate(self.relation_generators):
            relation_gen_spec = self.relation_gen_specs[rel_idx]
            gen_rel = relation_generator.generate(relation_gen_spec, dump=False)
            rel_data_tuples.append(gen_rel)

        for rel_idx, relation_generator in enumerate(self.relation_generators):
            relation_generator.dump_to_csv(rel_data_tuples[rel_idx])
        logging.info("Removing dangling tuples")
        if full_reducer:
            self.remove_dangling_tuples()


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
    parser.add_argument("-r", "--reducer", default=False, action="store_true")
    args = parser.parse_args(args)

    db_config_path = args.db_config_path
    query_config_path = args.query_config_path
    username = args.username
    password = args.password
    full_reducer = args.reducer

    database = Database(db_config_path, "")
    query = Query(query_config_path=query_config_path, database=database)
    database_generator = DatabaseGenerator(db_config_path, database, query, username, password)
    database_generator.generate(full_reducer)



if __name__ == "__main__":
    init_logging()
    main(sys.argv[1:])


