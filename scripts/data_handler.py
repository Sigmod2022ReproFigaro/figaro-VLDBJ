import json


class Table: 
    def __init__(self, json_schema):
        self.json_schema = json_schema


    def extract_schema_from_json(self, json_schema):
        self.name = json_schema["name"]
        self.attributes = json_schema["attributes"]
        set_pks = set(json_schema["primary_key"])
        for attribute in self.attributes:
            attribute["is_pk"] = True if attribute["name"] in set_pks else False
        self.data_path = json_schema["data_path"]

    def load_data(self):
        pass


class Database:
    def __init__(self, database_specs_path):
        with open(database_specs_path) as json_file:
            self.json_db_schema = json.load(json_file)
            self.load_db_schema()

    def load_db_schema(self):
        pass
                

# Class Database Schema 
# - Join relations 
# - Additional statistics 
# - Think what to use. 