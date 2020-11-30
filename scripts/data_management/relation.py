import json


class Attribute:
    def __init__(self, name, type, primary_key):
        self.name = name
        self.type = type
        self.primary_key = primary_key


class Relation: 
    def __init__(self, json_schema):
        self.extract_schema_from_json(json_schema)


    def extract_schema_from_json(self, json_schema):
        self.name = json_schema["name"]
        json_attributes = json_schema["attributes"]
        set_pks = set(json_schema["primary_key"])
        
        attributes = []
        for json_attribute in json_attributes:
            is_attribute_pk = True if json_attribute["name"] in set_pks else False
            attribute = Attribute(name=json_attribute["name"], 
                    type=json_attribute["type"],
                    primary_key=is_attribute_pk)
            attributes.append(attribute)
        
        self.attributes = attributes
        self.data_path = json_schema["data_path"] if "data_path" in json_schema else None

    def get_attributes(self):
        return self.attributes

    def get_attribute_names(self):
        return [attribute.name for attribute in self.attributes]


    def get_set_pk_attribute_names(self):
        setPks = set()
        for attribute in self.attributes:
            if attribute.primary_key:
                setPks.add(attribute.name)
        
        return setPks

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