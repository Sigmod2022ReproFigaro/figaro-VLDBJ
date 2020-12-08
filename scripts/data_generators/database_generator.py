import json
import os
import argparse
from data_management.database import Database
from data_management.relation import Relation
from data_generators.relation_generator import RelationGeneratedSpecs
from data_generators.relation_generator import RelationGenerator
from  typing import List
from  typing import Tuple
import pandas as pd

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
    

    def remove_dangling_tuples(self, generated_relations: List[Tuple[Relation, pd.DataFrame]]):
        prev_tup_rel_gen: Tuple[Relation, pd.DataFrame] = None
        for (relation, gen_rel) in generated_relations: 
            print("***************************")
            print(relation.name)
            if prev_tup_rel_gen is not None:
                for attr_name in prev_tup_rel_gen[0].get_pk_attribute_names():
                    print(attr_name)
                    if attr_name in gen_rel.columns:
                        full_domain = prev_tup_rel_gen[1][attr_name].unique()
                        is_fk_in_ref_relation = ~gen_rel[attr_name].isin(full_domain)
                        idx_dangling = gen_rel[is_fk_in_ref_relation].index
                        print(full_domain)
                        print(idx_dangling)
                        print(len(gen_rel))
                        gen_rel.drop(idx_dangling, inplace=True)
                        print(len(gen_rel))
            
            prev_tup_rel_gen = (relation, gen_rel)


    def generate(self):
        prev_gen_rel = None
        prev_rel = None
        rel_data_tuples = []

        for rel_idx, relation_generator in enumerate(self.relation_generators):
            relation_gen_spec = self.relation_gen_specs[rel_idx]
            relation = database.get_relation(relation_generator.name) 
            if prev_gen_rel is not None:
                for attr_name in prev_rel.get_pk_attribute_names():
                    full_domain = prev_gen_rel[attr_name].unique()
                    print(attr_name, full_domain)
                    relation_gen_spec.add_full_domain(attr_name, full_domain)
                print()
            print(rel_idx, relation_gen_spec)
            prev_gen_rel = relation_generator.generate(relation_gen_spec, dump=False)
            rel_data_tuples.append( (relation, prev_gen_rel) )
            prev_rel = relation
        
        rel_data_tuples.reverse()
        self.remove_dangling_tuples(rel_data_tuples)
        rel_data_tuples.reverse()
        for rel_idx, relation_generator in enumerate(self.relation_generators):
            relation_generator.dump_to_csv(rel_data_tuples[rel_idx][1])


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
    output_path = "/home/popina/Figaro/figaro-code/data/db_generated/1"
    database_generator.generate()



        