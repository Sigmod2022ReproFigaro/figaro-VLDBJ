import logging
import sys

FORMATER_STR = '-- %(levelname)5s -- [%(filename)20s:%(lineno)3s - %(funcName)20s()] %(message)s'


def add_logging_file_handler(file_path: str, debug_level: int = logging.DEBUG):
    formatter = logging.Formatter(FORMATER_STR)

    file_handler = logging.FileHandler(file_path, mode='w')
    file_handler.setFormatter(formatter)
    file_handler.setLevel(debug_level)

    root = logging.getLogger()
    root.addHandler(file_handler)

    return file_handler


def remove_logging_file_handler(file_handler):
    root = logging.getLogger()
    root.removeHandler(file_handler)


def init_logging():
    formatter = logging.Formatter(FORMATER_STR)

    stdout_handler = logging.StreamHandler(sys.stdout)
    stdout_handler.setFormatter(formatter)
    stdout_handler.setLevel(logging.DEBUG)

    add_logging_file_handler('log.txt')

    root = logging.getLogger()
    root.setLevel(logging.DEBUG)
    root.addHandler(stdout_handler)


def set_logging_level(level: int):
    root = logging.getLogger()
    root.setLevel(level)



