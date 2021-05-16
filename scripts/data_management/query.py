import json
import logging

from typing import List

from data_management.database import Database

class Query:
    def __init__(self, query_config_path, database: Database):
        self.query_config_path = query_config_path
        self.load_query()
        self.database = database
        #TODO: Move database methods to query class
        self.database.order_relations(self.relation_order)
        self.database.set_join_attrs()


    def load_query(self):
        json_query = None
        with open(self.query_config_path) as json_file:
            json_query = json.load(json_file)["query"]
            self.name = json_query["name"]
            json_eval_hint = json_query["evaluation_hint"]
            self.relation_order = json_eval_hint["relation_order"]
            self.skip_attrs = []
            if "skip_attributes" in json_eval_hint:
                self.skip_attrs = json_eval_hint["skip_attributes"]


    def get_conf_path(self):
        return self.query_config_path

    def get_name(self) -> str:
        return self.name


    def get_relation_order(self)-> List[str]:
        return self.relation_order


    def get_skip_attrs(self)-> List[str]:
        return self.skip_attrs

    def get_attr_names_ordered(self):
        return self.database.get_attr_names_ordered(self.relation_order)

    def get_non_join_attr_names_ordered(self)-> List[str]:
        return self.database.get_non_join_attr_names_ordered(
            self.relation_order, self.skip_attrs)

    def get_non_join_cat_attr_names_ordered(self):
        return self.database.get_non_join_cat_attr_names_ordered(
                self.relation_order, self.skip_attrs)