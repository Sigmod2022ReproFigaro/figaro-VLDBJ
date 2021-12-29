import sys
import numpy as np
import pandas as pd
from ctypes import CDLL
from  argparse import ArgumentParser
from timeit import default_timer as timer
from sklearn.base import BaseEstimator, TransformerMixin
from sklearn.preprocessing import OneHotEncoder
from sklearn.compose import ColumnTransformer
import threadpoolctl
from threadpoolctl import threadpool_limits

class DummyEncoder(BaseEstimator, TransformerMixin):
    def __init__(self, sparse):
        self.sparse = sparse

    def transform(self, X):
        ohe = OneHotEncoder(handle_unknown='ignore', sparse=self.sparse)
        return ohe.fit_transform(X)[:,1:]

    def fit(self, X, y=None, **fit_params):
        return self


class IdentityTransformer(BaseEstimator, TransformerMixin):
    def transform(self, input_array):
        return input_array

    def fit(self, X, y=None, **fit_params):
        return self


def make_diagonal_positive(r):
    diag_sgn_r = np.sign(r.diagonal())
    matr_sgn_r = np.diag(diag_sgn_r)
    r_pos =  np.dot(matr_sgn_r, r)
    return r_pos


def dump_qr(dump_file_path, r):
    np.savetxt(dump_file_path, np.asarray(r), delimiter=',')


def transform_data(data, columns, cat_columns, sparse):
    transformer_a = []
    for column in columns:
        if column in cat_columns:
            transf_name = column + "_onehot"
            transformer_a.append((transf_name, DummyEncoder(sparse), [column]))
        else:
            transf_name = column + "_identity"
            transformer_a.append((transf_name, IdentityTransformer(), [column]))

    preprocessor = ColumnTransformer(transformers=transformer_a)
    one_hot_a = preprocessor.fit_transform(data)
    return one_hot_a


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("-d", "--data_path", dest="data_path", required=True)
    parser.add_argument("-D", "--dump_file", dest="dump_file", required=False)
    parser.add_argument("-p", "--precision", dest="precision", required=False)
    parser.add_argument("-t", "--num_threads", dest="num_threads", required=False)
    parser.add_argument("-c", "--columns", dest="columns", nargs='*', required=False)
    parser.add_argument("--column_major", default=False, action='store_true')
    parser.add_argument("-C", "--cat_columns", dest="cat_columns", nargs='*', required=False)
    parser.add_argument('--compute_all', default=False, action='store_true')
    print (sys.argv[1:])
    #TODO: Add sparse argument

    print(sys.executable)
    np.show_config()

    args = parser.parse_args()
    data_path = args.data_path
    dump_file = args.dump_file
    columns = args.columns
    cat_columns = args.cat_columns
    column_major = bool(args.column_major)
    num_threads = int(args.num_threads)
    compute_all = bool(args.compute_all)

    precision = 15 if args.precision is None else int(args.precision)
    print(precision)
    print(columns)
    print(cat_columns)
    np.set_printoptions(threshold=sys.maxsize, precision=precision)
    pd.set_option('display.max_columns', 500)

    start = timer()
    data = pd.read_csv(data_path, names=columns, delimiter=",", header=None)
    #print(data)
    end = timer()
    print("##Figaro####loading##{}".format(end - start))

    start = timer()
    data = transform_data(data, columns, cat_columns, False)
    end = timer()
    print("##Figaro####ohe##{}".format(end - start))

    if column_major:
        data = np.asfortranarray(data)


    #print(data)
    #mkl = CDLL('/local/scratch/local/intel/mkl/2021.2.0/lib/intel64/libmkl_rt.so')
    #print(mkl.MKL_Get_Max_Threads())
    #mkl.MKL_Set_Dynamic(1)
    #mkl.MKL_Set_Num_Threads(48)
    #print(mkl.MKL_Get_Max_Threads())

    #start = timer()
    #r = np.linalg.qr(data, mode='r')
    #end = timer()
    #print("##Figaro####computation##{}".format(end - start))


    #print(mkl.MKL_Get_Max_Threads())
    #mkl.MKL_Set_Dynamic(0)
    #mkl.MKL_Set_Num_Threads(48)
    #print(mkl.MKL_Get_Max_Threads())


    with threadpool_limits(limits=num_threads, user_api='blas'):
        start = timer()
        if compute_all:
            q, r = np.linalg.qr(data, mode='reduced')
        else:
            r = np.linalg.qr(data, mode='r')
        end = timer()
        print("##Figaro####computation##{}".format(end - start))

    if dump_file is not None:
        r = make_diagonal_positive(r)
        dump_qr(dump_file, r)