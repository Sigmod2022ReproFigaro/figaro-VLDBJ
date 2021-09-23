Figaro system
==================

This directory contains implementation of Figaro system.
It is divided in the following directories:
- [CMakeLists.txt](CMakeLists.txt): specifying how the system is built; **Note**: LD_LIBRARY_PATH and LIBRARY_PATH in .bashrc were altered to include intel/mkl/2021.2.0/lib/intel64/ path.
- [setup.sh](setup.sh) : running figaro system with the specific configuration;
- [clean.sh](clean.sh) : cleaning built files;
- build: storing binary files;
- dump, log: the same purpose as in root directory;
- src: storing source files;
- include: storing header files.

## Code example running
5. Position to Figaro algorithm implementation.
6. Run experiments: ./setup.sh --test_mode=UNIT_TEST --root_path=/home/user/zivanovic/Figaro/figaro-code/figaro
## Code example running