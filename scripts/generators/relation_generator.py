import itertools as it
import random 
import numpy as np
import pandas as pd
import argparse

class RelationGenerator:
    def __init__(self, schema):
        self.schema = schema 

    # [ {"A": {"name": "A", "type": "int", "PK": : True}}]

    def getSetPKs(self):
        setPKs = set()
        for attribute_info in self.schema:
            attribute_name = attribute_info["name"]
            if attribute_info["PK"]:
                setPKs.add(attribute_name)
        
        return setPKs

    def getAttributeNamesOrdered(self):
        return [attribute_info["name"] for attribute_info in self.schema]

    # attribute_domains [{"name": "A", "start": 1, "end": 10}]
    # PKs are unique
    # We assume the same order as defined by schema
    def generate(self, attribute_domains, num_tuples: int, pandas_format=True):
        setPKs = self.getSetPKs()
        all_tuples_in_domain = None
        generated_relation = []
        first_non_PK_idx = None
        domains = []
        for attr_idx, attr_domain in enumerate(attribute_domains):
            dom_start = attr_domain["start"]
            dom_end = attr_domain["end"]
            domain_expanded = [i for i in range(dom_start, dom_end+1)]
            if (attr_domain["name"] in setPKs):
                domains.append(domain_expanded)
        all_tuples_in_domain = [list(tup) for tup in it.product(*domains)]
        random.shuffle(all_tuples_in_domain)
        generated_tuples = all_tuples_in_domain[:num_tuples+1]   

        for attr_idx, attr_domain in enumerate(attribute_domains):
            dom_start = attr_domain["start"]
            dom_end = attr_domain["end"]
            if (attr_domain["name"] not in setPKs):
                generated_tuples = [[*tuples, np.random.uniform(dom_start, dom_end)] \
                                    for tuples in generated_tuples]
        if pandas_format:
            generated_tuples = pd.DataFrame(generated_tuples, 
                                        columns=self.getAttributeNamesOrdered())
        
        return generated_tuples


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    #parser.add_argument("--relation_schema", action="store",  
                        dest="relation_schema", required=True)
    #args = parser.parse_args()
    print(args.relation_schema)
    relation_generator = RelationGenerator(                     \
        [{"name": "A", "type": "int", "PK": True} ,      \
          {"name": "B", "type": "int", "PK": True}, 
          {"name": "C", "type": "int", "PK": True},
          {"name": "A1", "type": "double", "PK": False} 
        ])    
    generated_relation = relation_generator.generate(        \
        [{"name": "A", "start": 1, "end": 10},
        {"name": "B", "start": 1, "end": 5},
        {"name": "C", "start": 3, "end": 9},
        {"name": "A1", "start": -3, "end": 3}
        ], num_tuples=100)
    #print(generated_relation)
    generated_relation.to_csv('test.csv', index=False)

