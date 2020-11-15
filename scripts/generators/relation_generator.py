import itertools as it
import random 

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
    def generate(self, attribute_domains, num_tuples: int):
        setPKs = self.getSetPKs()
        all_tuples_in_domain = None
        generated_relation = []
        first_non_PK_idx = None
        for attr_name, attr_domain in attribute_domains.items():
            dom_start = attr_domain["start"]
            dom_end = attr_domain["end"]
            domain_expanded = [i for i in range(dom_start, dom_end+1)]
            if (attr_domain["name"] in setPKs):
                if all_tuples_in_domain is None:
                    all_tuples_in_domain = domain_expanded
                else:
                    all_tuples_in_domain = it.product(all_tuples_in_domain, \
                                                domain_expanded))
                    print(all_tuples_in_domain)
            else:
                first_non_PK_idx = attr_idx
        

        random.shuffle(all_tuples_in_domain)
        generated_tuples = all_tuples_in_domain[:num_tuples+1]        
        return generated_tuples


        # TODO: Iterate over PKs 
        # Create all PK combinations in the selected domains 
        # then by random choose some value to be selected. 

if __name__ == "__main__":
    relation_generator = RelationGenerator(                     \
        { "A": {"name": "A", "type": "int", "PK": True} ,      \
          "B": {"name": "B", "type": "int", "PK": True}, 
          "C": {"name": "B", "type": "int", "PK": True} } )    
    generated_relation = relation_generator.generate(        \
        {"A": {"name": "A", "start": 1, "end": 10},
        "B": {"name": "B", "start": 1, "end": 5},
        "C": {"name": "C", "start": 3, "end": 9},
        }, num_tuples=5)
    print(generated_relation)
