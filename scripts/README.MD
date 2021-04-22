Running experiments
==================

1. Create virtual environment using virtualenv.
2. source bin/activate
3. Position to scripts path
4. Generate correct configuration files:  python -m data_management.data_formating -r /home/popina/Figaro/figaro-code -d /home/popina/Figaro/data -s /home/popina/Figaro/figaro-code/system_tests --backup
5. Run experiments: python -m evaluation.evaluator -u username -p password -r /home/popina/Figaro/figaro-code -s /home/popina/Figaro/figaro-code/system_tests --test 3
