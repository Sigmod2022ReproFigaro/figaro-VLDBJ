import itertools as it
import logging
import numpy as np
from numpy.core import numeric
from numpy.lib.ufunclike import _deprecate_out_named_y
import pandas as pd
import argparse
import os
from  typing import List

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
    def __init__(self, relation: Relation, join_attr_names: List[str]):
        self.name = relation.name
        self.setPks = set(relation.get_pk_attribute_names())
        self.attribute_names = relation.get_attribute_names()
        self.data_path = relation.data_path
        self.join_attr_names = join_attr_names


    def dump_to_csv(self, generated_relation, output_path: str = None):
        if output_path is None:
            output_path = self.data_path
        logging.info("Relation generated on:{}".format(output_path))
        head_tail = os.path.split(output_path)
        os.makedirs(head_tail[0], exist_ok=True)
        generated_relation.to_csv(output_path, index=False, header=False, columns=self.attribute_names)


    def cartesian_product(self, *arrays):
        ndim = len(arrays)
        return np.stack(np.meshgrid(*arrays), axis=-1).reshape(-1, ndim)


    # attribute_domains [{"name": "A", "start": 1, "end": 10}]
    # PKs are unique
    # We assume the same attribute order as defined by schema
    def generate(self, rel_specs: RelationGeneratedSpecs, dump: bool = False,
                output_path: str =None)-> pd.DataFrame:
        domains = {}
        domain_pks = []
        pk_attr_names = []
        generated_pk = False
        pk_exists = False

        num_tuples = rel_specs.num_tuples

        for _, attr_domain in enumerate(rel_specs.attribute_domains):
            domain_start = attr_domain["start"]
            domain_end = attr_domain["end"]
            attr_name = attr_domain["name"]
            domain = None
            if (attr_name in self.setPks):
                pk_exists = True
                domain_range = [i for i in range(domain_start, domain_end + 1)]
                domain_full = attr_domain["full"] if "full" in attr_domain \
                                else domain_range

                domain = np.array(list(set(domain_full) & set(domain_range)))

                domain_pks.append(domain)
                pk_attr_names.append(attr_name)
            else:
                if pk_exists and not generated_pk:
                    domain_pks = self.cartesian_product(*domain_pks)
                    np.random.shuffle(domain_pks)
                    domain_pks = domain_pks[:num_tuples]
                    num_tuples = domain_pks.shape[0]
                    num_pk_attrs = domain_pks.shape[1]
                    for idx in range(0, num_pk_attrs):
                        domains[pk_attr_names[idx]] = domain_pks[:, idx]

                    generated_pk = True

                if (attr_name in self.join_attr_names):
                    domain = np.random.randint(low=domain_start, high=domain_end+1, size=(num_tuples))
                else:
                    domain = np.random.uniform(domain_start, domain_end, size=(num_tuples))

                domains[attr_name] = domain

        generated_tuples = pd.DataFrame(domains)
        for column in generated_tuples:
            if (column in self.join_attr_names or column in self.setPks):
                generated_tuples.astype({column: 'int32'}, copy=False)

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

