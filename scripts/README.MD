Running experiments
==================
## Global variables (as an example)
```bash
FIGARO_CODE_PATH = /local/scratch/Figaro/figaro-code
FIGARO_DATA_PATH = /local/scratch/Figaro/data
FIGARO_SYSTEMS_TESTS_PATH = /local/scratch/Figaro/figaro-code/system_tests
FIGARO_SCRIPTS_PATH = /local/scratch/Figaro/figaro-code/scripts
FIGARO_PSQL_USER = username
FIGARO_PSQL_PASSWORD = 123456789
```

## Instalation
1. Change directory
```bash
cd $FIGARO_SCRIPTS_PATH
```
2. Create virtual environment using virtualenv
```bash
    1. python3 -m venv run-env-openblas
    2. python3 -m venv run-env-mkl
```
3. Install libraries: for both
```bash
    source run-env-xxx/bin/activate
    pip install -r requirements.txt
```
4. Install numpy + mkl for run-env-mkl:

```bash
    pip uninstall numpy;
```

then: install numpy from source with mkl: https://medium.com/@black_swan/using-mkl-to-boost-numpy-performance-on-ubuntu-f62781e63c38

6. Install nummpy + mkl for run-env-openblas:
```bash
    pip uninstall numpy
```
install numpy from source with openblas


## Initial path updates

1. Change the working directory to $FIGARO_SCRIPTS_PATH:
```bash
cd $FIGARO_SCRIPTS_PATH:
```
2. Generate correct configuration files:
```bash
python -m data_management.data_formating -r $FIGARO_CODE_PATH -d $FIGARO_DATA_PATH -s $FIGARO_SYSTEMS_TESTS_PATH --backup
```

## Datasets generation:
**Do not share any of the datasets**

```bash
python -m evaluation.data_generation -u $FIGARO_PSQL_USERNAME -p $FIGARO_PSQL_PASSWOD  -s $FIGARO_SYSTEMS_TESTS_PATH -d $FIGARO_DATA_PATH --data_type download_real_data
python -m evaluation.data_generation -u $FIGARO_PSQL_USERNAME -p $FIGARO_PSQL_PASSWOD  -s $FIGARO_SYSTEMS_TESTS_PATH -d $FIGARO_DATA_PATH --data_type all
```

## Running experiments
Run experiments:
```bash
python -m evaluation.experiment -u $FIGARO_PSQL_USER -p $FIGARO_PSQL_PASSWORD -r $FIGARO_ROOT_PATH -s $FIGARO_SYSTEMS_TESTS_PATH -e exp
```
where exp is an experiment number (from 1 to 4). They should be run in the order of enumeration: 1, 2, 3, 4.


## Code organization

Python scripts are divided in the following packages:

- data_generators: contains files related to synthetic dataset generation.
    More in the [data_generators](data_generators/README.MD)
- data_management: contains files regarding :
    - database engine wrappers:
        - pandas: [database_pandas](data_management/database_pandas.py);
        - psql: [database_psql](data_management/database_psql.py);
    - relation information based on configuration files from systems: [relation](data_management/relation.py);
    - database information based on configuration files from systems: [database](data_management/database.py);
    - generic query information based on configuration files from system_tests: [query](data_management/query.py);
    - reduced datasets generations:
        - percent where percent goes from 0.01 up to 0.1 by increment 0.01 and from 0.1 to 1.0 by increment 0.1:  [real_dataset_reduced](data_management/real_dataset_reduced.py);
        - ohe data where join attributes are duplicated at the end of the relations and the newly created attributes are considered to be non-join attributes. Further, one of the newly created attributes is one hot encoded. Then its domain is reduced by projecting to the percent of the original domain. This percent is varied from 0.1 to 1.0.
        [real_dataset_reduced_ohe](data_management/real_dataset_reduced_ohe.py);
    - full reducer that removes dangling tuples from the relations. [database_full_reducer](data_management/database_full_reducer.py)
    - renaming $HOME_DATA and $HOME_SRC paths to the ones specified by the user: [data_formating](data_management/data_formating.py)
- evaluation: contains files regarding evaluation of data generation and system tests. More in the [evaluation](evaluation/README.MD)
- plots_and_results contains the following folders that contain code for collecting data results, exporting data results to the appropriate format and drawing plots. In particular:
    - real_data contains code for both real data where one of join attributes is one hot encoded and not.
    - synthetic_data contains code for both synthetic accuracy and performance data.