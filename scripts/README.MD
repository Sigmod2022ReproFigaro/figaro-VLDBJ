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
where exp is an experiment number (from 1 to 4)


