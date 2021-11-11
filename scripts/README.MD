## Code organization

Python scripts are divided in the following packages:

- data_generators: contains files related to synthetic dataset generation.
    More in [data_generators](data_generators/README.MD)
- data_management: contains files regarding
    - database engine wrappers:
        - pandas: [database_pandas](data_management/database_pandas.py);
        - psql: [database_psql](data_management/database_psql.py);
    - relation information based on configuration files from systems: [relation](data_management/relation.py);
    - database information based on configuration files from systems: [database](data_management/database.py);
    - generic query information based on configuration files from system_tests: [query](data_management/query.py);
    - reduced datasets generations:
        - percent where percent goes from 0.01 up to 0.1 by increment 0.01 and from 0.1 to 1.0 by increment 0.1:  [real_dataset_reduced](data_management/real_dataset_reduced.py);
        - one hot encoded data where join attributes are duplicated at the end of the relations and the newly created attributes are considered to be non-join attributes. Further, one of the newly created attributes is one hot encoded. Then its domain is reduced by projecting to the percent of the original domain. This percent is varied from 0.1 to 1.0.
        [real_dataset_reduced_ohe](data_management/real_dataset_reduced_ohe.py);
    - full reducer that removes dangling tuples from the relations. [database_full_reducer](data_management/database_full_reducer.py)
    - renaming $HOME_DATA and $HOME_SRC paths to the ones specified by the user: [data_formating](data_management/data_formating.py)
- evaluation: contains files regarding evaluation of data generation and system tests. More in [evaluation](evaluation/README.MD)
- plots_and_results contains the following folders that contain code for collecting data results, exporting data results to the appropriate format and drawing plots. In particular:
    - real_data contains code for both real data where one of join attributes is one hot encoded and not.
    - synthetic_data contains code for both synthetic accuracy and performance data.