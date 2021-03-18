Running experiments 
==================

1. Create virtual environment using virtualenv.
2. source bin/activate 
3. Position to scripts path 
4. Generate correct configuration files:  python -m data_management.data_formating -r /home/popina/Figaro/figaro-code -d /home/popina/Figaro/data
5. Run experiments: python -m evaluation.evaluator -p password -r /home/popina/Figaro/figaro-code
