Evalulation system
========================

This folder contains implementation of evaluation of experiments.
It is divided in the following packages and files:

- accuracy contains files that load Rs computed by different systems and compares them entry wise and their relative and absolute errors.
- peformance contains files that parse logs and extract microbencmarks and bencmarks and output to the corresponding locations.
- [custom_logging.py](custom_logging.py) implements setting up logging configuration for the evaluator.
- [data_generation.py](data_generation.py) implements data generation entry point. This is used to generate the corresponding data by specifying the following arguments:
    - --system_tests_path : path to system_tests directory;
    - --data_path : path to data directory;
    - --username : username for psql instance;
    - --password: password for psql instance;
    - --data_type: type of data to generate. In particular if we pas
        - download_real_data it downloads the original data;
        - real_data it generates real data ;
        - real_data_ohe it generates one hot encoded data;
        - syn_perf  it generates synthetic performance data ;
        - syn_accur it generates synthetic accuracy data;
- [evaluator.py](evaluator.py) implement evaluator of system tests. In
    - --root_path : path to the root of the code directory;
    - --system_tests_path : path to system_tests directory;
    - --data_path : path to data directory;
    - --username : username for psql instance;
    - --password: password for psql instance;
    - --test test_name: specifies which set of tests specified under the directory test_name/tests_specs.conf to run ;
- [experiment.py](experiment.py) implements experiment evaluator. It accepts the following command line arguments:
    - --root_path : path to the root of the code directory;
    - --system_tests_path : path to system_tests directory;
    - --data_path : path to data directory;
    - --username : username for psql instance;
    - --password: password for psql instance;
    - --experiment: specifies which experiment from the paper to run. It can be 1, 2, 3 or 4. They should be evaluated in sequential order.
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
