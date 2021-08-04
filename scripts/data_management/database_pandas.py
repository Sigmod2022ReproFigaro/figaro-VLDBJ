import logging
import pandas as pd

from data_management.database import Database
from data_management.query import Query
from data_management.relation import Relation

class DatabasePandas:
    def __init__(self, database: Database) -> None:
        self.tables = {}
        for relation in database.get_relations():
            self.create_table(relation)

    def create_table(self, rel: Relation):
        self.tables[rel.name] = pd.read_csv(rel.data_path, delimiter=",",
                                            names=rel.get_attribute_names(), header=None)
        logging.debug(self.tables[rel.name])


    def evaluate_join(self, query: Query, num_repetitions: int):
        eval_hint = query.get_eval_hint()
        left_rel_name = eval_hint["operands"][0]["central_relation"]["relation"]
        right_rel_name = eval_hint["operands"][0]["children"][0]["relation"]
        logging.debug(left_rel_name)
        logging.debug(right_rel_name)

        left_table = self.tables[left_rel_name]
        right_table = self.tables[right_rel_name]
        left_join_attr_names = query.get_join_attrs(left_rel_name)
        right_join_attr_names = query.get_join_attrs(right_rel_name)

        self.join = pd.merge(left_table, right_table, how="inner", right_on=right_join_attr_names, left_on=left_join_attr_names)
        logging.debug(self.join)


    def dump_join(self, query: Query, output_file_path: str):
        non_join_attribute_names = query.get_non_join_attr_names_ordered()
        logging.debug(non_join_attribute_names)
        logging.debug(output_file_path)
        self.join.to_csv(output_file_path,
            columns=non_join_attribute_names, header=None, sep=",",
             index=False)






