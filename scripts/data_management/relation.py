import json


class Relation: 
    def __init__(self, json_schema):
        self.extract_schema_from_json(json_schema)


    def extract_schema_from_json(self, json_schema):
        self.name = json_schema["name"]
        self.attributes = json_schema["attributes"]
        set_pks = set(json_schema["primary_key"])
        for attribute in self.attributes:
            attribute["PK"] = True if attribute["name"] in set_pks else False
        self.data_path = json_schema["data_path"] if "data_path" in json_schema else None

    def get_attributes(self):
        return self.attributes

    def load_data(self):
        pass


if __name__ == "__main__":
    relation = Relation({
                "name": "T1",
                "attributes": 
                [ {"name": "R","type": "int"}, 
                {"name": "S","type": "int"},
                {"name": "A1","type": "int"}
                ],
                "primary_key": ["R", "S"],
                "data_path": "/home/popina/Figaro/data/test1/T1.csv"
            })