Data Generators
==================

This directory contains the following scripts:

- synthetic accuracy data generator: [accuracy_cart_prod_generatory.py](accuracy_cart_prod_generator.py): It generates the Cartesian product of relations S and T where parts of the factor R_S in the QR decomposition of S is fixed in advance. Details on the construction can be found in [appendix.pdf](appendix.pdf). It only accepts two arguments:
    - --system_tests_path : path to the system_tests directory;
    - --data_path : path to the data directory.
- synthetic performance data generator: [cartesian_product_generator.py](cartesian_product_generator): It generates the Cartesian product of relations S and T where values are drawn from a uniform distribution. It accepts the following arguments:
    - --system_tests_path : path to the system_tests directory;
    - --data_path : path to the data directory;
    - --username : username for the psql instance;
    - --password: password for the psql instance;
- database_generator: [database_generator.py](database_generator.py) : it generates the database according to the specification in the corresponding database configuration file.
- relation_generator: [relation_generator](relation_generator.py) it generates the relation according to the relation specification in the corresponding relation configuration.
- relation_schema_generator: [relation_schema_generator](relation_schema_generator.py) it generates the database schema according to the configuration for generation of synthetic performance datasets.