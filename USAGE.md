## Setting up the system

We explain how the system is set up before any experiment can be run.

### Required applications
The following applications must be installed to run experiments:
1. [gcc-10.1 (built from source)](https://solarianprogrammer.com/2016/10/07/building-gcc-ubuntu-linux/);
2. cmake 3.13.4;
3. python 3.7.3;
4. psql 11.12 where a new user with all admin priviliges is created (create database, roles ...)

### Required C++ libraries
The following C++ libraries must be installed to run experiments:
1. [Intel MKL 2021.2.0](https://software.intel.com/content/www/us/en/develop/tools/oneapi/base-toolkit/download.html?operatingsystem=linux&distributions=webdownload&options=offline);
2. Boost 1.67.0;
3. Eigen 3.3.7;
4. [Openblas 0.3.13 (built from source)](https://github.com/xianyi/OpenBLAS/releases);
5. Thread Building Blocks 2018.0;
6. [nlohoman json library](https://github.com/nlohmann/json);
7. Gtest 1.8.1.

Please update the Intel MKL paths in the file figaro/CMakeLists.txt.

### Reference: Global variables
The following explanations use global variables to improve readability. They could for example have the following values:
```bash
FIGARO_CODE_PATH = /local/scratch/Figaro/figaro-code
FIGARO_DATA_PATH = /local/scratch/Figaro/data
FIGARO_SYSTEMS_TESTS_PATH = /local/scratch/Figaro/figaro-code/system_tests
FIGARO_SCRIPTS_PATH = /local/scratch/Figaro/figaro-code/scripts
FIGARO_PSQL_USER = username
FIGARO_PSQL_PASSWORD = 123456789
```

### Installation
The experiments use numpy with the openblas as well as with the mkl implementation of the LAPACK API. We employ virtual environments to allow using these systems side by side. 
Whenever one of the virtual environments is active, it will suppress the usage of the other one. Thus if run-env-mkl is active, all the measured runtimes with respect to numpy would be numpy + mkl.
If we want to get the results of numpy + openblas in experiment 1, the python mkl system configuration should be disabled (tests/percent/performance->system_test_python_mkl), and python openblas (tests/percent/performance->system_test_python_openblas) should be enabled. Otherwise, by default numpy+mkl is used.

1. Change the directory
```bash
cd $FIGARO_SCRIPTS_PATH
```
2. Create virtual environments using virtualenv
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


### Initial path updates

1. Change the working directory to $FIGARO_SCRIPTS_PATH:
```bash
cd $FIGARO_SCRIPTS_PATH:
```
2. Generate the correct configuration files:
```bash
python -m data_management.data_formating -r $FIGARO_CODE_PATH -d $FIGARO_DATA_PATH -s $FIGARO_SYSTEMS_TESTS_PATH --backup
```

### Datasets generation:
The following script will download the three real-world datasets and generate the synthetic ones. From the three real-world datasets, two are public (Yelp and Favorita), while the third has been provided to us under a confidentiality agreement and is not to be shared.

```bash
python -m evaluation.data_generation -u $FIGARO_PSQL_USERNAME -p $FIGARO_PSQL_PASSWOD  -s $FIGARO_SYSTEMS_TESTS_PATH -d $FIGARO_DATA_PATH --data_type download_real_data
python -m evaluation.data_generation -u $FIGARO_PSQL_USERNAME -p $FIGARO_PSQL_PASSWOD  -s $FIGARO_SYSTEMS_TESTS_PATH -d $FIGARO_DATA_PATH --data_type all
```

## Running experiments
The experiments described in the submission are run by executing the following command:
```bash
python -m evaluation.experiment -u $FIGARO_PSQL_USER -p $FIGARO_PSQL_PASSWORD -r $FIGARO_ROOT_PATH -s $FIGARO_SYSTEMS_TESTS_PATH -e exp
```
where exp is an experiment number (from 1 to 4). The experiments should be run in the order 1, 2, 3, 4.







