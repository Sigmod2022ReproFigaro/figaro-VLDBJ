
import shutil
import os
import argparse
import fileinput


def replace_symbols_with_text(file_name: str, symbol_to_texts: dict):
    out_file_name = file_name + ".out"
    with open(file_name, "r") as file_read:
        with open(out_file_name, "w") as file_write:
            for line in file_read:
                str_rep = line
                for symbol, text in symbol_to_texts.items():
                    str_rep = str_rep.replace(symbol, text)
                file_write.write(str_rep)

    os.rename(out_file_name, file_name)




if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-r", "--root", action="store",
                        dest="root_path", required=False)
    parser.add_argument("-d", "--data", action="store",
                        dest="data_path", required=False)
    parser.add_argument("-s", "--system_tests_path", action="store",
                        dest="system_tests_path", required=True)
    parser.add_argument("-b", "--backup", action="store_true",
                        dest="backup", required=False)
    args = parser.parse_args()

    root_path = args.root_path if args.root_path is not None \
        else "/home/username/Figaro/figaro-code"
    data_path = args.data_path if args.data_path is not None \
            else "/home/username/Figaro/data"
    system_tests_path = args.system_tests_path

    if args.backup:
        system_tests_path_backup = system_tests_path + "_backup"
        if os.path.exists(system_tests_path_backup):
            shutil.rmtree(system_tests_path_backup)
        shutil.copytree(system_tests_path, system_tests_path_backup)

    data_path_symbol ="$HOME_DATA"
    root_path_symbol = "$HOME_SRC"
    translation_table = {data_path_symbol: data_path, root_path_symbol: root_path}
    for root, _, files in os.walk(system_tests_path):
        for file in files:
            file_path = os.path.join(root, file)
            replace_symbols_with_text(file_path, translation_table)


