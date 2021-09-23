Data Generators
==================

This directory contains the following scripts:

- synthetic accuracy data generator: [accuracy_cart_prod_generatory.py](accuracy_cart_prod_generator.py): It generates Cartesian product of relations S and T where R_S is fixed in advance and Q_S is determined according to formula and fixed in advance. In particular, if order non-zero  values of R_s in row-major order they are 1, 2,... Further, the entries in the first column of Q_S are 1 / sqrt(nrows(Q_S)) .  Values of T are drawn from a uniform distribution. It only accepts two arguments:
    - --system_tests_path : path to system_tests directory;
    - --data_path : path to data directory.
- synthetic performance data generator: [cartesian_product_generator.py](cartesian_product_generator): It generates Cartesian product of relations S and T where values are drawn from a uniform distribution. It accepts the following arguments:
    - --system_tests_path : path to system_tests directory;
    - --data_path : path to data directory;
    - --username : username for psql instance;
    - --password: password for psql instance;
- database_generator: [database_generator.py](database_generator.py) : it generates the database according to the specification in the corresponding database configuration file.
- relation_generator: [relation_generator](relation_generator.py) it generates the relation according to the relation specification in the corresponding relation configuration.
- relation_schema_generator: [relation_schema_generator](relation_schema_generator.py) it generates database schema configuration for generation of synthetic performance datasets.