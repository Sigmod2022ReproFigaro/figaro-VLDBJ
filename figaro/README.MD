Running unit tests
==================

1. Create virtual environment using virtualenv.
2. source bin/activate
3. Position to scripts path
4. Generate correct configuration files:  python -m data_management.data_formating -r /home/popina/Figaro/figaro-code -d /home/popina/Figaro/data
5. Position to Figaro algorithm implementation.
6. Run experiments: ./setup.sh --test_mode=UNIT_TEST --root_path=/home/user/zivanovic/Figaro/figaro-code/figaro
