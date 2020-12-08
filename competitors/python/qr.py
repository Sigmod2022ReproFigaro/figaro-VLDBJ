import sys
import numpy as np
import pandas as pd
from  argparse import ArgumentParser


def make_diagonal_positive(r):
    diag_sgn_r = np.sign(r.diagonal())
    print(diag_sgn_r)
    matr_sgn_r = np.diag(diag_sgn_r)
    print(matr_sgn_r)
    r_pos =  np.dot(matr_sgn_r, r)
    return r_pos


def dump_qr(dump_file_path, r):
    np.savetxt(dump_file_path, np.asarray(r), delimiter=',')


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("-d", "--data_path", dest="data_path", required=True)
    parser.add_argument("-D", "--dump_file", dest="dump_file", required=False)
    parser.add_argument("-p", "--precision", dest="precision", required=False)
    print (sys.argv[1:])

    args = parser.parse_args()
    data_path = args.data_path
    dump_file = args.dump_file
    
    precision = 15 if args.precision is None else int(args.precision)
    print(precision)
    np.set_printoptions(threshold=sys.maxsize, precision=precision)
    pd.set_option('display.max_columns', 500)

    data = pd.read_csv(data_path, delimiter=",", header=None)
    r = np.linalg.qr(data, mode='r')
    print(r)
    r = make_diagonal_positive(r)
    print(r)
    if dump_file is not None:
        dump_qr(dump_file, r)