import json
import logging

class Query:
    def __init__(self, query_config_path):
        self.query_config_path = query_config_path
        self.load_query()


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

    def get_name(self):
        return self.name


    def get_relation_order(self):
        return self.relation_order


    def get_skip_attrs(self):
        return self.skip_attrs