import json
import os
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
        for rel_idx, relation_generator in enumerate(self.relation_generators):
            relation_gen_spec = self.relation_gen_specs[rel_idx]
            name, generated_relation = relation_generator.generate(relation_gen_spec)

            output_file_path = os.path.join(output_path, name)
            output_file_path += ".csv"
            print(output_file_path)
            generated_relation.to_csv(output_file_path, index=False, header=False)


if __name__ == "__main__":
    db_specs_path = "/home/popina/Figaro/figaro-code/system_tests/test1/database_specs.conf"
    database = Database(db_specs_path)
    database_generator = DatabaseGenerator(db_specs_path, database)
    database_generator.generate("/home/popina/Figaro/figaro-code/scripts/data_generators/databases_generated")



        