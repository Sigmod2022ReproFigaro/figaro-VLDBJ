from evaluation.system_test import SystemTest
from evaluation.system_test_figaro import SystemTestFigaro
from evaluation.system_test_psql import SystemTestPsql
from evaluation.system_test_python import SystemTestPython
import argparse

def normal_test(password):
    SYSTEM_PY_TEST_PATH = "/home/popina/Figaro/figaro-code/system_tests/test1/systems/system_python.conf"
    SYSTEM_PSQL_TEST_PATH = "/home/popina/Figaro/figaro-code/system_tests/test1/systems/system_psql.conf"
    
    system_psql = SystemTestPsql.from_specs_path(SYSTEM_PSQL_TEST_PATH,
                        password=password)
    system_py = SystemTestPython.from_specs_path(SYSTEM_PY_TEST_PATH)
    
    system_psql.setTestDataType(SystemTest.TestDataType.DEBUG)
    system_py.setTestDataType(SystemTest.TestDataType.DEBUG)
    system_psql.run()
    system_py.set_join_path(system_psql.join_path)
    system_py.run()


def figaro_test():
     SYSTEM_FIGARO_TEST_PATH = "/home/popina/Figaro/figaro-code/system_tests/test1/systems/system_figaro.conf"
     system_figaro = SystemTestFigaro.from_specs_path(SYSTEM_FIGARO_TEST_PATH)
     system_figaro.run()
     

if __name__ == "__main__":
    ROOT_PATH = "/home/popina/Figaro/figaro-code"
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--password", action="store",  
                        dest="password", required=True)
    args = parser.parse_args()
    #figaro_test()
    normal_test(args.password)