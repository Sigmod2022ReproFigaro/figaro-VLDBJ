# FIGARO-code


## Required applications
The following applications must be installed to run experiments:
1. [gcc-10.1 (built from source)](https://solarianprogrammer.com/2016/10/07/building-gcc-ubuntu-linux/);
2. cmake 3.13.4;
3. python 3.7.3;
4. psql 11.12 where a new user with all admin priviliges is created (create database, roles ...)

## Required C++ libraries
The following C++ libraries must be installed to run experiments:
1. [Intel MKL 2021.2.0](https://software.intel.com/content/www/us/en/develop/tools/oneapi/base-toolkit/download.html?operatingsystem=linux&distributions=webdownload&options=offline);
2. Boost 1.67.0;
3. Eigen 3.3.7;
4. [Openblas 0.3.13 (built from source)](https://github.com/xianyi/OpenBLAS/releases);
5. Thread Building Blocks 2018.0;
6. [nlohoman json library](https://github.com/nlohmann/json);
7. Gtest 1.8.1.

Update Intel MKL paths in figaro/CMakeLists.txt

---

## Organization of code
I will explain what each of the folder in root path contains.
### 1. system_tests
system_tests directory contains configuration files used in running each of the experiments. I will explain what each of the configuration folders contains.

- systems: configuration files for systems used in the experiments: including psql, numpy+mkl, figaro.
- test_accuracy_car_prod: configuration files for synthetic accuracy experiments.
- test_cartesian_product: configuration files for synthetic accuracy experiments.
- test_real_data: configuration files for real datasets.
- test_real_data_ohe: configuration files for real datasets where one of join attributes is one hot encoded.


Each of test_* folders has the following files and folders with json configurations:

- databases folder: it contains schema specification of databases used in experiments alongside types of attributes in relations (categorical, double and int),  primary keys, and paths to data used in relations.
- queries folder: it contains query specification of a join on which qr decomposition is applied. In particular, it specifies a join tree. Further it  includes which attributes are discarded (skip_attributes), what is the number of threads used in Figaro system: (num_threads), and a order of relations in the join result. Such a relational order is always the preorder of the corresponding join order. It also can specify the order of attributes in each of the relations. This is used to swap places of certain join attributes.

- dataset_conf folder: it contains a specification of which query is used for which database.

- tests folder: it contains a specification which systems are used on which datasets. For each of the systems we can specify whether we want to use one of the following modes: dump, performance, accuracy, performance_analysis.
    - dump evaluates the corresponding systems only once, and dumps the data. In the case of PSQL and pandas this is a join result according to the configuration. For numpy and figaro this is R after the decomposition.
    - performance evaluates the corresponding system number of times specified in configuration files (5) and measures times. Every time the system is evaluated as a separate process.
    - accuracy compares R computed by that system to other systems. In particular, this is useful to compare whether Figaro works correctly on real datasets.
    - performance_analysis parses the logs with evaluation times and creates xlsx with times. This was used in microbencmarks to determine the bottlenecks of the algorithm.
- tess_specs.conf file: it contains which groups of tests are evaluated.

"disable" property in each configuration file specifies whether the certain type is skipped.

### 2. scripts
The code is explained more in the [scripts](scripts/README.MD)
### 3. logs

It contains logs for each evaluation.

### 4. dumps

It contains dumps used in different systems: psql, R for each of the systems, and other intermediate data.

### 5. competitors

It contains code of numpy + (mkl or openblas).

### 6. comparisons

It contains results of performance and accuracy analysis for each of the systems inside the corresponding folder. For example: for performance analysis for thin version of figaro and database DBFavorita10, and the join order StoresRoot48, the results are stored on path ./comparisons/performance/figaro/thin_diag/DbFavorita10/StoresRoot48/, in particular in file time.xlsx

### 7. figaro
It contains implementation of Figaro system and the postprocessing methods.
The code is explained more in [Figaro](figaro/README.MD)

[How to run experiments](scripts/README.MD)