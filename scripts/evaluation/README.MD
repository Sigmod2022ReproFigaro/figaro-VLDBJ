Evaluation system
========================

This folder contains the implementation for the evaluation of experiments.
It is divided into the following packages and files:

- accuracy contains files that load Rs computed by different systems, compares them entrywise and computes the relative and absolute errors.
- peformance contains files that parse logs and extract microbenchmarks and benchmarks and outputs to the corresponding locations.
- [custom_logging.py](custom_logging.py) implements setting up logging configurations for the evaluator.
- [data_generation.py](data_generation.py) implements the data generation entry point. This is used to generate the corresponding data by specifying the following arguments:
    - --system_tests_path : path to the system_tests directory;
    - --data_path : path to the data directory;
    - --username : username for the psql instance;
    - --password: password for the psql instance;
    - --data_type: type of data to generate. In particular if we pass
        - download_real_data it downloads the original data;
        - real_data it generates real data ;
        - real_data_ohe it generates one hot encoded data;
        - syn_perf  it generates synthetic performance data ;
        - syn_accur it generates synthetic accuracy data;
- [evaluator.py](evaluator.py) implement the evaluator of system tests. Arguments are
    - --root_path : path to the root of the code directory;
    - --system_tests_path : path to the system_tests directory;
    - --data_path : path to the data directory;
    - --username : username for the psql instance;
    - --password: password for the psql instance;
    - --test test_name: specifies which set of tests specified under the directory test_name/tests_specs.conf to run ;
- [experiment.py](experiment.py) implements experiment evaluator. It accepts the following command line arguments:
    - --root_path : path to the root of the code directory;
    - --system_tests_path : path to the system_tests directory;
    - --data_path : path to the data directory;
    - --username : username for the psql instance;
    - --password: password for the psql instance;
    - --experiment: specifies which experiment from the paper to run. It can be 1, 2, 3 or 4. They should be evaluated in this order.
- system_tests contains the wrapper around the systems that will be called. In particular:
    - abstract system test specification: system_test.py
        - figaro: system_test_figaro.py
        - database engines:
            - abstract specification: system_test_dbms.py
            - pandas: system_test_dbms_pandas.py
            - psql: system_test_dbms_psql.py
        - figaro competitors:
            - python: system_test_python.py:
            - Figaro postprocessing system: system_test_postprocessing.py
