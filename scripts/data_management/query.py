from cmath import log
import json
import logging

from typing import List
from data_management.database import Database

class Query:
    def __init__(self, query_config_path, database: Database):
        self.query_config_path = query_config_path
        self.database = database

        #TODO: Change order to allow other order than preorder.

        self.load_query()


    @staticmethod
    def get_json_rel(json_rel):
        node_type = json_rel["operator"]
        if node_type == "natural_join":
            rel = json_rel["central_relation"]
        else:
            rel = json_rel
        return rel


    def compute_join_and_non_join_attrs(self, json_eval_hint):
        self.rel_attr_names = {rel: self.database.get_attribute_names(rel, False) for rel in self.relation_order}
        self.rel_join_attrs = {rel: [] for rel in self.relation_order}
        self.rel_non_join_attrs = {rel: [] for rel in self.relation_order}
        self.eval_hint = json_eval_hint
        self.visit_node_join_tree(json_eval_hint["operands"][0], None)


    def compute_common_attrs(self, rel_name1: str, rel_name2: str):
        join_attrs_set = set(self.rel_attr_names[rel_name1]).intersection(self.rel_attr_names[rel_name2])
        return join_attrs_set


    def visit_node_join_tree(self, json_cur_rel, par_rel_name: str)-> None:
        node_type = json_cur_rel["operator"]
        if node_type == "natural_join":
            cur_rel = json_cur_rel["central_relation"]
            json_child_rels = json_cur_rel["children"]
        else:
            cur_rel = json_cur_rel
            json_child_rels = []

        cur_rel_name = cur_rel["relation"]
        cur_rel_attr_names = cur_rel.get("attributes_order", self.rel_attr_names[cur_rel_name])
        join_attrs_set = set()
        if par_rel_name is not None:
            join_attrs_small_set = self.compute_common_attrs(cur_rel_name, par_rel_name)
            join_attrs_set = join_attrs_set.union(join_attrs_small_set)

        for json_child_rel in json_child_rels:
            json_pure_rel = Query.get_json_rel(json_child_rel)
            child_rel_name = json_pure_rel["relation"]
            self.visit_node_join_tree(json_cur_rel=json_child_rel, par_rel_name=cur_rel_name)

            join_attrs_small_set = self.compute_common_attrs(cur_rel_name, child_rel_name)
            join_attrs_set = join_attrs_set.union(join_attrs_small_set)


        #TODO: TESTTTTT!!!
        #logging.info("cur_rel_attr_names {}".format(cur_rel_attr_names))
        #logging.info("join_attrs_set {}".format(join_attrs_set))
        #logging.info("skip_attrs {}".format(self.skip_attrs))
        non_join_attrs_set = set(cur_rel_attr_names).difference(join_attrs_set).difference(self.skip_attrs)
        join_attrs_list = list(join_attrs_set)
        non_join_attrs_list = list(non_join_attrs_set)
        def key_fun(attr_name: str):
            return cur_rel_attr_names.index(attr_name)

        join_attrs_list.sort(key=key_fun)
        non_join_attrs_list.sort(key=key_fun)
        self.rel_join_attrs[cur_rel_name] = join_attrs_list
        self.rel_non_join_attrs[cur_rel_name] = non_join_attrs_list


    def load_query(self):
        json_query = None
        with open(self.query_config_path) as json_file:
            json_query = json.load(json_file)["query"]
            self.name = json_query["name"]
            json_eval_hint = json_query["evaluation_hint"]
            # TODO: Compute Change order of schema
            self.relation_order = json_eval_hint["relation_order"]
            self.num_threads = json_eval_hint.get("num_threads", None)
            self.skip_attrs = json_eval_hint.get(
                "skip_attributes", [])
            self.label_name = json_eval_hint.get("label_name", "")

            self.compute_join_and_non_join_attrs(json_eval_hint)

        #logging.info("Join Attrs {}".format(self.rel_join_attrs))
        #logging.info("Non join atttrs {}".format(self.rel_non_join_attrs))


    def get_eval_hint(self) -> dict:
        return self.eval_hint


    def get_conf_path(self) -> str:
        return self.query_config_path


    def get_name(self) -> str:
        return self.name


    def get_num_threads(self) -> int:
        return self.num_threads


    def get_relation_order(self)-> List[str]:
        return self.relation_order


    def get_skip_attrs(self)-> List[str]:
        return self.skip_attrs


    def get_label_name(self)->str:
        return self.label_name


    def get_join_attrs(self, relation_name: str)-> List[str]:
        return self.rel_join_attrs[relation_name]


    def get_non_join_attrs(self, relation_name: str)-> List[str]:
        return self.rel_non_join_attrs[relation_name]


    def get_non_join_attr_names_ordered(self)-> List[str]:
        non_join_attrs_ordered = []
        for rel in self.relation_order:
            non_join_attrs_ordered += self.rel_non_join_attrs[rel]
        return non_join_attrs_ordered


    def get_non_join_cat_attr_names_ordered(self):
        cat_attr_names = []
        non_join_attr_names = self.get_non_join_attr_names_ordered()
        for rel in self.relation_order:
            cat_attr_names += self.database.get_attribute_names(rel_name=rel, is_cat=True)
        non_join_cat_attr_names = [non_join_attr_name for non_join_attr_name in
            non_join_attr_names if non_join_attr_name in cat_attr_names]
        return non_join_cat_attr_names


    def get_join_attr_names_ordered(self)-> List[str]:
        join_attr_names = []
        for rel in self.relation_order:
            join_attr_names += self.rel_join_attrs[rel]
        join_attr_names_unique = list(dict.fromkeys(join_attr_names))
        return join_attr_names_unique


    def get_attr_names_ordered(self):
        join_attr_names = self.get_join_attr_names_ordered()
        non_join_attr_names = self.get_non_join_attr_names_ordered()
        return join_attr_names + non_join_attr_names





