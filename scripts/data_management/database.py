import json
import logging

from typing import List
from data_management import relation
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


    def get_name(self)-> str:
        return self.name

    def get_full_name(self)-> str:
        return self.full_name


    def get_relations(self)-> List[Relation]:
        return self.relations


    def get_attribute_names(self, rel_name: str, is_cat: bool = False) -> List[str]:
        for rel in self.relations:
            if rel.name == rel_name:
                return rel.get_attribute_names([], is_cat)


    def get_all_attribute_names(self, rel_name: str, is_cat: bool = False) -> List[str]:
        for rel in self.relations:
            if rel.name == rel_name:
                return rel.get_all_attribute_names()


    def get_relation(self, name) -> Relation:
        for relation in self.relations:
            if relation.name == name:
                return relation
        return None