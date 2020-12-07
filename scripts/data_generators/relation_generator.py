import itertools as it
import random 
import numpy as np
import pandas as pd
import argparse

from data_management.relation import Relation

class RelationGeneratedSpecs:
    def __init__(self, json_relation_generated_specs):
        self.distribution = json_relation_generated_specs["distribution"]
        self.attribute_domains = json_relation_generated_specs["attribute_domains"]
        self.num_tuples = json_relation_generated_specs["num_tuples"]

    def get_attribute_domains(self):
        return self.attribute_domains

class RelationGenerator:
    def __init__(self, relation: Relation):
        self.name = relation.name
        self.setPks = set(relation.get_pk_attribute_names())
        self.attribute_names = relation.get_attribute_names() 
        self.data_path = relation.data_path


    def dump_to_csv(self, generated_relation, output_path: str = None):
        if output_path is None:
            output_path = self.data_path
        generated_relation.to_csv(output_path, index=False, header=False)


    # attribute_domains [{"name": "A", "start": 1, "end": 10}]
    # PKs are unique
    # We assume the same order as defined by schema
    #def generate(self, attribute_domains, num_tuples: int, pandas_format=True):
    def generate(self, rel_specs: RelationGeneratedSpecs, pandas_format=True, output_path: str =None):
        all_tuples_in_domain = None
        generated_relation = []
        first_non_PK_idx = None
        domains = []
        for attr_idx, attr_domain in enumerate(rel_specs.attribute_domains):
            dom_start = attr_domain["start"]
            dom_end = attr_domain["end"]
            domain_expanded = [i for i in range(dom_start, dom_end+1)]
            if (attr_domain["name"] in self.setPks):
                domains.append(domain_expanded)
        all_tuples_in_domain = [list(tup) for tup in it.product(*domains)]
        random.shuffle(all_tuples_in_domain)
        generated_tuples = all_tuples_in_domain[:rel_specs.num_tuples+1]   

        for attr_idx, attr_domain in enumerate(rel_specs.attribute_domains):
            dom_start = attr_domain["start"]
            dom_end = attr_domain["end"]
            if (attr_domain["name"] not in self.setPks):
                generated_tuples = [[*tuples, np.random.uniform(dom_start, dom_end)] \
                                    for tuples in generated_tuples]
        if pandas_format:
            generated_tuples = pd.DataFrame(generated_tuples, 
                                        columns=self.attribute_names)
        
        print("Relation generated on:", output_path)
        
        self.dump_to_csv(generated_tuples, output_path)
        


if __name__ == "__main__":
    #parser = argparse.ArgumentParser()
    #parser.add_argument("--relation_schema", action="store",  
    #                    dest="relation_schema", required=True)
    #args = parser.parse_args()
    #print(args.relation_schema)
    relation = Relation({
                "name": "T1",
                "attributes": 
                [ {"name": "A","type": "int"}, 
                {"name": "B","type": "int"},
                {"name": "C","type": "int"},
                {"name": "A1","type": "double"}
                ],
                "primary_key": ["A", "B", "C"]
            })
    rel_specs = RelationGeneratedSpecs({
        "distribution": "uniform",
        "attribute_domains": [{"name": "A", "start": 1, "end": 10},
        {"name": "B", "start": 1, "end": 5},
        {"name": "C", "start": 3, "end": 9},
        {"name": "A1", "start": -3, "end": 3}
        ], "num_tuples": 100})
    relation_generator = RelationGenerator(relation)    
    generated_relation = relation_generator.generate(rel_specs)
    print(generated_relation)
    #generated_relation.to_csv('test.csv', index=False)

