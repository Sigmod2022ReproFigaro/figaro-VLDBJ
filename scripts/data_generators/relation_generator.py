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

    
    def __str__(self):
        ret = self.distribution + str(self.attribute_domains) + str(self.num_tuples)
        return ret


    def get_attribute_domains(self):
        return self.attribute_domains


    def add_full_domain(self, attr_name: str, full_domain: list):
        attr_idx = None
        for idx, attribute_domain in enumerate(self.attribute_domains):
            if attr_name == attribute_domain["name"]:
                attr_idx = idx
                break
        if attr_idx is not None:
            self.attribute_domains[attr_idx] 
            self.attribute_domains[attr_idx]["full"] = full_domain

class RelationGenerator:
    def __init__(self, relation: Relation):
        self.name = relation.name
        self.setPks = set(relation.get_pk_attribute_names())
        self.attribute_names = relation.get_attribute_names() 
        self.data_path = relation.data_path


    def dump_to_csv(self, generated_relation, output_path: str = None):
        if output_path is None:
            output_path = self.data_path
        print("Relation generated on:", output_path)
        generated_relation.to_csv(output_path, index=False, header=False)


    # attribute_domains [{"name": "A", "start": 1, "end": 10}]
    # PKs are unique
    # We assume the same attribute order as defined by schema
    #def generate(self, attribute_domains, num_tuples: int, pandas_format=True):
    def generate(self, rel_specs: RelationGeneratedSpecs, dump: bool = False, 
                output_path: str =None)-> pd.DataFrame:
        all_tuples_in_domain = None
        generated_relation = []
        first_non_PK_idx = None
        domains = []

        for attr_idx, attr_domain in enumerate(rel_specs.attribute_domains):
            domain_start = attr_domain["start"]
            domain_end = attr_domain["end"]
            domain_range = [i for i in range(domain_start, domain_end + 1)]
            domain_full = attr_domain["full"] if "full" in attr_domain \
                            else domain_range
            domain = list(set(domain_full) & set(domain_range))
            if (attr_domain["name"] in self.setPks):
                domains.append(domain)

        all_tuples_in_domain = [list(tup) for tup in it.product(*domains)]
        random.shuffle(all_tuples_in_domain)
        generated_tuples = all_tuples_in_domain[:rel_specs.num_tuples]   

        for attr_idx, attr_domain in enumerate(rel_specs.attribute_domains):
            dom_start = attr_domain["start"]
            dom_end = attr_domain["end"]
            if (attr_domain["name"] not in self.setPks):
                generated_tuples = [[*tuples, np.random.uniform(dom_start, dom_end)] \
                                    for tuples in generated_tuples]

        generated_tuples = pd.DataFrame(generated_tuples, 
                                    columns=self.attribute_names)
        if dump:
            self.dump_to_csv(generated_tuples, output_path)

        return generated_tuples
        


if __name__ == "__main__":
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
    rel_specs = RelationGeneratedSpecs( {
        "distribution": "uniform",
        "attribute_domains": [
            {"name": "A", "start": 1, "end": 10, "full": [1, 2, 3]},
            {"name": "B", "start": 1, "end": 5},
            {"name": "C", "start": 3, "end": 9},
            {"name": "A1", "start": -3, "end": 3}],
        "num_tuples": 100} )
    relation_generator = RelationGenerator(relation)    
    generated_relation = relation_generator.generate(rel_specs)
    pd.set_option('display.max_rows', 100)
    print(generated_relation)

