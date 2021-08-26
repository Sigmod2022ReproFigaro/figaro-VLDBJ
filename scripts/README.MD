Running experiments
==================

**Instalation**
1. Create virtual environment using virtualenv
    1. python3 -m venv run-env-openblas
    2. python3 -m venv run-env-mkl
2. Install libraries: for both
    1. source run-env-xxx/bin/activate
    2. pip install -r requirements.txt
3. Install numpy + mkl for run-env-openblas:
    1. pip uninstall numpy
    2. install numpy from source with mkl: https://medium.com/@black_swan/using-mkl-to-boost-numpy-performance-on-ubuntu-f62781e63c38

**Running tests**
1. Change director to scripts path.
2. Generate correct configuration files:  python -m data_management.data_formating -r /home/popina/Figaro/figaro-code -d /home/popina/Figaro/data -s /home/popina/Figaro/figaro-code/system_tests --backup
3. Run experiments: python -m evaluation.evaluator -u username -p password -r /home/popina/Figaro/figaro-code -s /home/popina/Figaro/figaro-code/system_tests --test 3
