import json

from data_management.relation import Relation

class Database:
    def __init__(self, database_specs_path):
        with open(database_specs_path) as json_file:
            self.json_db_schema = json.load(json_file)["database"]
        self.load_db_schema()

    def load_db_schema(self):
        self.name = self.json_db_schema["name"]
        json_relations = self.json_db_schema["relations"]
        relations = []
        for json_relation in json_relations:
            relation = Relation(json_relation)
            relations.append(relation)
        self.relations = relations


    def get_relations(self):
        return self.relations


    def get_relation_names(self):
        relation_names = [relation.name for relation in self.relations]
        return relation_names

    
    def get_relation(self, name):
        for relation in self.relations:
            if relation.name == name:
                return relation
        return None


    
                