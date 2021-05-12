import json
import logging

from typing import List
from data_management.relation import Relation

class Database:
    def __init__(self, database_specs_path, name_ext):
        self.db_config_path = database_specs_path
        with open(database_specs_path) as json_file:
            self.json_db_schema = json.load(json_file)["database"]
        self.load_db_schema(name_ext)


    def load_db_schema(self, name_ext):
        self.name = self.json_db_schema["name"]
        self.full_name = self.name + name_ext
        json_relations = self.json_db_schema["relations"]
        relations = []
        for json_relation in json_relations:
            relation = Relation(json_relation)
            relations.append(relation)
        self.relations = relations


    def get_name(self):
        return self.name

    def get_full_name(self):
        return self.full_name


    def get_relations(self):
        return self.relations


    def order_relations(self, relation_order: List[Relation]):
        def key_fun(r: Relation):
            return relation_order.index(r.name)
        self.relations.sort(key=key_fun)


    #TODO: Create join tree from query config.
    def set_join_attrs(self):
        for cur_rel in self.relations:
            join_attrs = set()
            for check_rel in self.relations:
                if cur_rel != check_rel:
                    tmp_join_attrs = set(cur_rel.get_attribute_names()).\
                        intersection(set(check_rel.get_attribute_names()))
                    join_attrs = join_attrs.union(tmp_join_attrs)
            cur_rel.set_join_attribute_names(list(join_attrs))

    def get_relation_names(self) -> List[str]:
        relation_names = [relation.name for relation in self.relations]
        return relation_names


    def get_relation(self, name) -> Relation:
        for relation in self.relations:
            if relation.name == name:
                return relation
        return None


    def __get_non_join_attr_names_ordered(self, relations: List[Relation],
        skip_attrs: List[str]=[]) -> List[str]:

        non_join_attr_names = []
        for relation in relations:
            non_join_attr_names += relation.get_non_join_attribute_names(skip_attrs)
        return non_join_attr_names


    def get_non_join_attr_names_ordered(self, relation_names: List[str],
        skip_attrs: List[str]=[]) -> List[str]:

        relations = [self.get_relation(rel_name) for rel_name in relation_names]
        return self.__get_non_join_attr_names_ordered(relations, skip_attrs)


    def get_non_join_cat_attr_names_ordered(self, relation_names: List[str],
        skip_attrs: List[str]=[]) -> List[str]:
        relations = [self.get_relation(rel_name) for rel_name in relation_names]
        non_join_cat_attr_names = []
        for relation in relations:
            non_join_cat_attr_names += relation.get_non_join_cat_attr_names(skip_attrs)

        return non_join_cat_attr_names

    def get_attr_names_ordered(self, relation_names: List[str]) -> List[str]:
        relations = [self.get_relation(rel_name) for rel_name in relation_names]
        join_attr_names = []
        non_join_attribute_names = self.__get_non_join_attr_names_ordered(relations)

        for relation in relations:
            join_attr_names += relation.get_join_attribute_names()

        join_attr_names_unique = list(dict.fromkeys(join_attr_names))
        return join_attr_names_unique + non_join_attribute_names




