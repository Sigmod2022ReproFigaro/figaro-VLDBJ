import itertools as it
import random 
import numpy as np
import pandas as pd
class RelationGenerator:
    def __init__(self, schema):
        self.schema = schema 

    # [ {"A": {"name": "A", "type": "int", "PK": : True}}]

    def getSetPKs(self):
        setPKs = set()
        for attribute_name, attribute_info in self.schema.items():
            if attribute_info["PK"]:
                setPKs.add(attribute_name)
        
        return setPKs

    # attribute_domains {"A":{"name": "A", "start": 1, "end": 10}}
    # PKs are unique
    # We assume the same order as defined by schema
    def generate(self, attribute_domains, num_tuples: int, pandas_format=True):
        setPKs = self.getSetPKs()
        all_tuples_in_domain = None
        generated_relation = []
        first_non_PK_idx = None
        domains = []
        for attr_idx, (attr_name, attr_domain) in enumerate(attribute_domains.items()):
            dom_start = attr_domain["start"]
            dom_end = attr_domain["end"]
            domain_expanded = [i for i in range(dom_start, dom_end+1)]
            if (attr_domain["name"] in setPKs):
                domains.append(domain_expanded)
        all_tuples_in_domain = [list(tup) for tup in it.product(*domains)]
        random.shuffle(all_tuples_in_domain)
        generated_tuples = all_tuples_in_domain[:num_tuples+1]   

        for attr_idx, (attr_name, attr_domain) in enumerate(attribute_domains.items()):
            dom_start = attr_domain["start"]
            dom_end = attr_domain["end"]
            if (attr_domain["name"] not in setPKs):
                generated_tuples = [[*tuples, np.random.uniform(dom_start, dom_end)] \
                                    for tuples in generated_tuples]
        if pandas_format:
            generated_tuples = pd.DataFrame(generated_tuples, 
                                        columns=self.schema.keys())
        
        return generated_tuples


if __name__ == "__main__":
    relation_generator = RelationGenerator(                     \
        { "A": {"name": "A", "type": "int", "PK": True} ,      \
          "B": {"name": "B", "type": "int", "PK": True}, 
          "C": {"name": "B", "type": "int", "PK": True},
          "A1": {"name": "A1", "type": "double", "PK": False} 
        })    
    generated_relation = relation_generator.generate(        \
        {"A": {"name": "A", "start": 1, "end": 10},
        "B": {"name": "B", "start": 1, "end": 5},
        "C": {"name": "C", "start": 3, "end": 9},
        "A1": {"name": "A1", "start": -3, "end": 3}
        }, num_tuples=100)
    print(generated_relation)
    generated_relation.to_csv('test.csv', index=False)

