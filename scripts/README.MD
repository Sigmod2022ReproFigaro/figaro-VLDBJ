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
## Datasets:

[Datasets](https://drive.google.com/file/d/1vPtXFzMfENMa5W46U0lVQgaDtJm6Hgxv/view?usp=sharing)
**Do not share any of the datasets**

## Reduced real dataset generation

1. Generation of datasets where non of join attributes is one hot encoded.
```bash
python -m data_management.real_data_set_reduced -d $FIGARO_DATA_PATH -s $FIGARO_SYSTEMS_TESTS_PATH -p $FIGARO_PSQL_PASSWORD -u $FIGARO_PSQL_USER
```
2. Generation of datasets where one of join attributes is one hot encoded.
```bash
python -m data_management.real_dataset_reduced_ohe --data_path $FIGARO_DATA_PATH
```


## Synthetic data generation

1. Performance:
```bash
python -m data_generators.cartesian_product_generator -s $FIGARO_SYSTEMS_TESTS_PATH -p $FIGARO_PSQL_PASSWORD -u $FIGARO_PSQL_USER -d $FIGARO_DATA_PATH
```
2. Accuracy:
```bash
python -m data_generators.accuracy_cart_prod_generator -d $FIGARO_DATA_PATH -s $FIGARO_SYSTEMS_TESTS_PATH
```

## Running experiments

Run experiments:
```bash
python -m evaluation.evaluator -u $FIGARO_PSQL_USER -p $FIGARO_PSQL_PASSWORD -r $FIGARO_CODE_PATH -s $FIGARO_SYSTEMS_TESTS_PATH --test test_name
```
test_name is: _accuracy_cart_prod, _cartesian_product, _real_data, _real_data_ohe
_accuracy_cart_prod is runs experiments regarding synthetic accuracy experiments.
_cartesian_product runs experiments regarding synthetic performance experiments
_real_data runs experiments regarding real data.
_real_data_ohe runs experiments on real data where one of join attributes is one hot encoded.
The instructions assume every subset of tasks is disabled at start. How to run experiments:

1. Real datasets:
* Update $FIGARO_SYSTEM_TESTS_PATH/test_real_data/tests_specs.conf by enabling percent_psql_dump, percent_performance and percent_perf_analysis.
* Update $FIGARO_SYSTEM_TESTS_PATH/test_real_data_ohe/tests_specs.conf by enabling percent_psql_dump, percent_performance and percent_perf_analysis.
Run the following commands
```bash
python -m evaluation.evaluator -u $FIGARO_PSQL_USER -p $FIGARO_PSQL_PASSWORD -r $FIGARO_CODE_PATH -s $FIGARO_SYSTEMS_TESTS_PATH --test _real_data

python -m evaluation.evaluator -u $FIGARO_PSQL_USER -p $FIGARO_PSQL_PASSWORD -r $FIGARO_CODE_PATH -s $FIGARO_SYSTEMS_TESTS_PATH --test _real_data_ohe
```
Synthetic datasets:
* Update $FIGARO_SYSTEM_TESTS_PATH/test_real_data/tests_specs.conf by enabling synthetic_performance_figaro, synthetic_performance_mkl. Run the following commands:
```bash
python -m evaluation.evaluator -u $FIGARO_PSQL_USER -p $FIGARO_PSQL_PASSWORD -r $FIGARO_CODE_PATH -s $FIGARO_SYSTEMS_TESTS_PATH --test _cartesian_product
```
2. Threading experiments:

* Update $FIGARO_SYSTEM_TESTS_PATH/test_real_data/tests_specs.conf by enabling thread_perf, thread_perf_analysis.
Run the following commands:
```bash
python -m evaluation.evaluator -u $FIGARO_PSQL_USER -p $FIGARO_PSQL_PASSWORD -r $FIGARO_CODE_PATH -s $FIGARO_SYSTEMS_TESTS_PATH --test _real_data
```
3. Join order experiments:

* Update $FIGARO_SYSTEM_TESTS_PATH/test_real_data/tests_specs.conf by enabling join_order_perf and join_order_perf_analysis.
* Update $FIGARO_SYSTEM_TESTS_PATH/test_real_data_ohe/tests_specs.conf by enabling join_order_perf and join_order_perf_analysis.
Run the following commands
```bash
python -m evaluation.evaluator -u $FIGARO_PSQL_USER -p $FIGARO_PSQL_PASSWORD -r $FIGARO_CODE_PATH -s $FIGARO_SYSTEMS_TESTS_PATH --test _real_data

python -m evaluation.evaluator -u $FIGARO_PSQL_USER -p $FIGARO_PSQL_PASSWORD -r $FIGARO_CODE_PATH -s $FIGARO_SYSTEMS_TESTS_PATH --test _real_data_ohe
```
4. Accuracy experiments:

Synthetic datasets:
* Update $FIGARO_SYSTEM_TESTS_PATH/test_accuracy_cart_prod_data/tests_specs.conf by enabling synthetic_accuracy_figaro, synthetic_accuracy_mkl. Run the following commands:

```bash
python -m evaluation.evaluator -u $FIGARO_PSQL_USER -p $FIGARO_PSQL_PASSWORD -r $FIGARO_CODE_PATH -s $FIGARO_SYSTEMS_TESTS_PATH --test _accuracy_cart_prod
```

**Explanation:**

Each of the experiments can be run more fine grained by disabling certain subset of tasks in the corresponding .conf files. For example if we want to disable percent_performance group of tests in test_real_data/tests_specs.conf we set int the corresponding percent_performance entry disable to true.
Similarly, by setting the appropriate disable of data_sets in dataset_conf or one of the systems in tests folder. For example in test_real_data/tests/percent/performance.conf by default openblas system is disabled.



## Scripts collecting
1. Real dataset percent:
```bash
python -m plots_and_results.real_data.performance_percent --root_path $FIGARO_CODE_PATH --exp_names figaro_thin post_proc_mkl post_proc_thin --dump_results
```

2. Real dataset join_order and threading:
```bash
python -m plots_and_results.real_data.performance_threads_join_orders --root_path $FIGARO_CODE_PATH --exp_name figaro_thin --dump_results
```

3. Real dataset ohe percent:
```bash
python -m plots_and_results.real_data.performance_percent --root_path $FIGARO_CODE_PATH --exp_names figaro_thin post_proc_mkl post_proc_thin --dump_results --ohe
 ```

4. Real dataset join order and threading:
```bash
python -m plots_and_results.real_data.performance_threads_join_orders --root_path $FIGARO_CODE_PATH --exp_name figaro_thin --dump_results --ohe
```

5. Synthetic accuracy:
6. Synthetic performance: