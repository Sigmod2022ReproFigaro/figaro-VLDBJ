import json
import os
import argparse
from data_management.database import Database
from data_management.relation import Relation
from data_generators.relation_generator import RelationGeneratedSpecs
from data_generators.relation_generator import RelationGenerator


class DatabaseGenerator:
    def __init__(self, generate_specs_path, database):
        with open(generate_specs_path) as json_file:
            json_db_schema = json.load(json_file)["database"]
        
        self.json_rel_gen_specs = json_db_schema["stats"]["distribution_specs"]["relations"]
        
        relation_generators = []
        relation_gen_specs = []
        for json_rel_gen_spec in self.json_rel_gen_specs:
            relation = database.get_relation(json_rel_gen_spec["name"])
            relation_generator = RelationGenerator(relation)
            relation_generators.append(relation_generator)

            relation_gen_spec = RelationGeneratedSpecs(json_rel_gen_spec)
            relation_gen_specs.append(relation_gen_spec)
        
        self.relation_generators = relation_generators
        self.relation_gen_specs = relation_gen_specs
            


    def generate(self, output_path):
        prev_gen_rel = None
        prev_rel = None
        for rel_idx, relation_generator in enumerate(self.relation_generators):
            relation_gen_spec = self.relation_gen_specs[rel_idx]
            relation = database.get_relation(relation_generator.name) 
            output_file_path = os.path.join(output_path, relation_generator.name)
            output_file_path += ".csv"
            if prev_gen_rel is not None:
                for attr_name in prev_rel.get_pk_attribute_names():
                    full_domain = prev_gen_rel[attr_name].unique()
                    relation_gen_spec.add_full_domain(attr_name, full_domain)

            prev_gen_rel = relation_generator.generate(relation_gen_spec, dump=True,
                            output_path=output_file_path)
            prev_rel = relation


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--test", action="store", 
                        dest="test", required=False)
    args = parser.parse_args()

    db_conf_path = "/home/popina/Figaro/figaro-code/system_tests/test{}/database_specs.conf"
    test_num = 2 if args.test is None else args.test
    db_conf_path = db_conf_path.format(test_num)
    
    database = Database(db_conf_path)
    database_generator = DatabaseGenerator(db_conf_path, database)
    database_generator.generate("/home/popina/Figaro/figaro-code/data/db_generated/1")



        