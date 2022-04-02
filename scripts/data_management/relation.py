import json
import logging
from typing import List

class Attribute:
    def __init__(self, name, type, primary_key):
        self.name = name
        self.type = type
        self.primary_key = primary_key

    def get_flat_type(self):
        if self.type == "category":
            return "integer"
        elif self.type == "double":
            return "float"
        else:
            return self.type


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



    def get_attributes(self) -> List[Attribute]:
        return self.attributes

    def get_attribute_names(self, skip_attrs: List[str] = [], is_cat: bool = False) -> List[str]:
        attr_names = []
        for attribute in self.attributes:
            if (attribute.name not in skip_attrs) and \
                (not is_cat or (is_cat and attribute.type == "category")):
                attr_names.append(attribute.name)

        return attr_names


    def get_all_attribute_names(self):
        attr_names = []
        for attribute in self.attributes:
            attr_names.append(attribute.name)

        return attr_names

    def get_pk_attribute_names(self)-> List[str]:
        return [attribute.name for attribute in self.attributes if attribute.primary_key]


    def get_non_pk_attribute_names(self)-> List[str]:
         return [attribute.name for attribute in self.attributes if not attribute.primary_key]

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
                "data_path": "/home/username/Figaro/data/test1/T1.csv"
            })