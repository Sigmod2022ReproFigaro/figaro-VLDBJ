# FIGARO-code

This repository accompanies the submission "Givens QR Decomposition over Relational Databases" to SIGMOD 2022.

The material and the data is made available for the review process only. Please do not store or share the data elsewhere.


## Using the code

Detailed instructions on how to set up the system and running the experiments can be found in this file: [Setup and running experiments](./USAGE.md)


## Code organization

We give an overview of the file structure of this repository.

### 1. figaro
This folder contains the implementation of the Figaro system and the postprocessing methods.
The code is explained more in [Figaro](figaro/README.MD)

### 2. scripts
This folder contains Python scripts used for generating data as well as running and evaluating experiments.
More explanations can be found in [scripts](scripts/README.MD)

### 3. competitors

This folder contains code for running numpy + (mkl or openblas).


### 4. system_tests
The directory system_tests contains the configuration files used in running each of the experiments. It has the following subfolders.

- systems: contains configuration files for the systems that are used in the experiments, including psql, numpy+mkl and figaro.
- test_accuracy_car_prod: contains the configuration files for testing the accuracy for synthetic data.
- test_cartesian_product: contains the configuration files for testing performance for synthetic data.
- test_real_data: contains the configuration files for experiments over real datasets.
- test_real_data_ohe: contains the configuration files for experiments over real datasets where one of join attributes is one hot encoded.

Each of test_* folders has the following files and folders with json configurations:

- folder databases: contains the schema specification of the databases used in the experiments, including the types of the attributes of the relations (categorical, double and int),  primary keys, and paths to the data.
- folder queries: contains the query specifications of a join on which the QR decomposition is applied. In particular, it specifies a join tree. Further it  includes which attributes are discarded (skip_attributes), what is the number of threads used in Figaro system: (num_threads), and a order of relations in the join result. Such a relational order is always the preorder of the corresponding join order. It also can specify the order of attributes in each of the relations. This is used to swap places of certain join attributes.

- folder dataset_conf: specifies which query is used for which database.

- folder tests: specifies which systems are used on which datasets. For each of the systems we can specify whether we want to use one of the following modes: dump, performance, accuracy, performance_analysis.
    - dump evaluates the corresponding systems only once and then dumps the data. In the case of PSQL and pandas this is a join result according to the configuration. For numpy and figaro this is R after the decomposition.
    - performance evaluates the corresponding system for a number of times that is specified in the configuration files (standard: 5) and measures the execution times. Every time the system is evaluated as a separate process.
    - accuracy compares R computed by that system to other systems. In particular, this is useful to check that Figaro works correctly on real datasets.
    - performance_analysis parses the logs with evaluation times and creates xlsx with times. This was used in microbenchmarks to optimize the implementation of the algorithm.
- file tess_specs.conf: specifies which groups of tests are evaluated.

The "disable" property in each configuration file specifies whether the certain type is skipped.

### 5. comparisons

After an experiment is run, this folder contains the results of performance and accuracy analyses for each of the systems inside the corresponding folder. For example: for performance analysis for figaro with the postprocessing method "thin", the database DBFavorita10 and the join order StoresRoot48, the results are stored in path ./comparisons/performance/figaro/thin_diag/DbFavorita10/StoresRoot48/, in particular in the file time.xlsx

### 6. logs (only after running experiments)

After an experiment is run, this folder contains logs for each evaluation.

### 7. dumps (only after running experiments)

When an experiment is run, different systems dump data in this folder. This can be the computed matrix R, the join result, or other intermediate data.
