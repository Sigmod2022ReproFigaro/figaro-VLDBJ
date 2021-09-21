Running experiments
==================

## Instalation

1. Create virtual environment using virtualenv
    1. python3 -m venv run-env-openblas
    2. python3 -m venv run-env-mkl
2. Install libraries: for both
    1. source run-env-xxx/bin/activate
    2. pip install -r requirements.txt
3. Install numpy + mkl for run-env-mkl:
    1. pip uninstall numpy;
    2. install numpy from source with mkl: https://medium.com/@black_swan/using-mkl-to-boost-numpy-performance-on-ubuntu-f62781e63c38
4. Install nummpy + mkl for run-env-openblas:
    1. pip uninstall numpy
    2. install numpy from source with openblas

## Initial path updates

1. Change the working directory to the scripts path.
2. Generate correct configuration files:  python -m data_management.data_formating -r figaro_code_path_full -d data_path_full -s systems_path_full --backup

## Reduced real dataset generation

1. python -m data_management.real_data_set_reduced -d data_path_full -s system_tests_path p psql_password -u psql_username
2. python -m data_management.real_dataset_reduced_ohe --data_path data_path_full

## Synthetic data generation

1. Performance: python -m data_generators.cartesian_product_generator -s systems_path_full -p psql_password -u psql_username -d data_path_full
2. Accuracy: python -m data_generators.accuracy_cart_prod_generator -d data_path_full -s systems_path_full

## Running experiments

1. Run experiments: python -m evaluation.evaluator -u psql_username -p psql_password -r figaro_code_path_full -s systems_path_full --test test_name
test_name is: _accuracy_cart_prod, _cartesian_product, _real_data, _real_data_ohe
_accuracy_cart_prod is runs experiments regarding synthetic accuracy experiments.
_cartesian_product runs experiments regarding synthetic performance experiments
_real_data runs experiments regarding real data.
_real_data_ohe runs experiments on real data where one of join attributes is one hot encoded.

Each of the experiments can be run more fine grained by disabling certain subset of tasks in the corresponding .conf files. For example if we want to disable percent_performance group of tests in test_real_data/tests_specs.conf we set int the corresponding percent_performance entry disable to true.
Similarly, by setting the appropriate disable of data_sets in dataset_conf or one of the systems in tests folder. For example in test_real_data/tests/percent/performance.conf by default openblas system is disabled.


## Scripts collecting
1. Real dataset percent: python -m plots_and_results.real_data.performance_percent --root_path /local/scratch/Figaro/figaro-code --exp_names figaro_thin post_proc_mkl post_proc_thin --dump_results
2. Real dataset join_order and threading:
python -m plots_and_results.real_data.performance_threads_join_orders --root_path /local/scratch/Figaro/figaro-code --exp_names figaro
3. Real dataset ohe percent:
4. Real dataset join order and threading:
5. Synthetic accuracy:
6. Synthetic performance: