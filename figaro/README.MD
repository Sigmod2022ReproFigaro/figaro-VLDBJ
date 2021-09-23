Figaro system
==================

This directory contains implementation of Figaro system.
It is divided in the following directories:
- [CMakeLists.txt](CMakeLists.txt): specifying how the system is built; **Note**: LD_LIBRARY_PATH and LIBRARY_PATH in .bashrc were altered to include intel/mkl/2021.2.0/lib/intel64/ path.
- [setup.sh](setup.sh) : running figaro system with the specific configuration. Run setup.sh --help for more details.
- [clean.sh](clean.sh) : cleaning built files;
- build: storing binary files;
- dump, log: the same purpose as in root directory;
- src: storing source files; It contains the following files and directories:
    - [main.cpp](src/main.cpp) It contains entry point to Figaro system. It works by first loading database configuration alognside the specified data. Further, it loads query. After that it evaluates query and if needed dumps the computed R.
    - [Utils.cpp](src/utils/Utils.cpp) It contains functions for setting the number of threads, outputting relation to csv and setIntersection.
    - [Database.cpp](src/database/Database.cpp)
     TODO: add serious explanations
    - [Relation.cpp](src/database/Relation.cpp)
    TODO: add serious explanations
    - database/query: join order specification where we specify: join tree data structure alongside join tree node types, join tree visitors and query evaluators.
        - visitors:
            - sdfsdf
            - sddf
        - join tree node type specification:
        - query evaluator:
        sdfsdf
- include: storing header files for the corresponding source files. The only additional files are inside utils and database/storage. In particular:
    - in utils ErrorCode.h: error code, Logger.h: logging functions, Performance.h: microbenchmark functions.
    in database/storage : ArrayStorage.h: specifies inlined data structure that is used for storing data and Matrix.h: specifies a matrix data structure in row-major order. It implements the following functions:
    TODO: add functions and explanations

## Code example running
5. Position to Figaro algorithm implementation.
6. Run experiments: ./setup.sh --test_mode=UNIT_TEST --root_path=/home/user/zivanovic/Figaro/figaro-code/figaro
