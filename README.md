# FIGARO-code

[How to run experiments](scripts/README.MD)

The following applications must be installed to run experiments:
1. [gcc-10.1 (built from source)](https://solarianprogrammer.com/2016/10/07/building-gcc-ubuntu-linux/);
2. cmake 3.13.4;
3. python 3.7.3;
4. psql 11.12 where a new user with all admin priviliges is created (create database, roles ...)


The following C++ libraries must be installed to run experiments:
1. [Intel MKL 2021.2.0](https://software.intel.com/content/www/us/en/develop/tools/oneapi/base-toolkit/download.html?operatingsystem=linux&distributions=webdownload&options=offline);
2. Boost 1.67.0;
3. Eigen 3.3.7;
4. [Openblas 0.3.13 (built from source)](https://github.com/xianyi/OpenBLAS/releases);
5. Thread Building Blocks 2018.0;
6. [nlohoman json library](https://github.com/nlohmann/json);
7. Gtest 1.8.1.

Update Intel MKL paths in figaro/CMakeLists.txt

