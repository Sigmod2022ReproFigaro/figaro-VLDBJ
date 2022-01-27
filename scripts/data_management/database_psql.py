import psycopg2
from psycopg2 import sql
from typing import List
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT
import argparse
import os
import logging
from data_management.database import Database
from data_management.query import Query
from data_management.relation import Relation
from timeit import default_timer as timer

from evaluation.system_test.system_test import DumpConf


class DatabasePsql:
    JOIN_TABLE_NAME = "join_table"
    def __init__(self, host_name: str, user_name: str,
    password: str, database: Database = None):
        self.host_name = host_name
        self.user_name = user_name
        self.password = password
        self.database = database
        if database is not None:
            self.open_connection()
        else:
            self.connection = None

    @staticmethod
    def open_connection_static(host_name, user_name, password, database_name):
        connection = psycopg2.connect(host=host_name, user=user_name,
                        password=password, dbname=database_name)
        connection.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        return connection


    def open_connection_admin(self):
        connection = DatabasePsql.open_connection_static(host_name='', user_name=self.user_name,
                            password=self.password, database_name='postgres')
        return connection



    def open_connection(self, check_db: bool = True):
        if check_db and not self.check_if_db_exists(self.database.name):
            self.connection = None
        else:
            self.connection = self.open_connection_static(self.host_name, self.user_name,
                     self.password, self.database.name)
        return self.connection


    def close_connection(self):
        if self.connection is not None:
            self.connection.close()
        self.connection = None


    def check_if_db_exists(self, db_name: str):
        connection_admin = self.open_connection_admin()
        cursor = connection_admin.cursor()
        sql_query = "SELECT * FROM pg_database WHERE datname='{}';"
        sql_query = sql_query.format(db_name)
        cursor.execute(sql_query)
        db_exists = True if cursor.fetchall().__len__() > 0 else False
        connection_admin.close()

        return db_exists


    def create_table(self, relation: Relation):
        attributes = relation.attributes

        sql_attributes = ""
        for attribute in attributes:
            sql_attribute = "{} {}"
            sql_attribute = sql_attribute.format(attribute.name, attribute.get_flat_type())
            sql_attributes += sql_attribute + ","

        # Check for PK existence
        if (relation.get_pk_attribute_names().__len__() > 0):
            sql_pks = "PRIMARY KEY({})"
            sql_pk_attributes = ""
            for attribute_name in relation.get_pk_attribute_names():
                sql_pk_attributes += attribute_name + ","
            sql_pks = sql_pks.format(sql_pk_attributes[:-1])
            sql_attributes += sql_pks
        else:
            sql_attributes = sql_attributes[:-1]

        sql_query = "CREATE TABLE {} ({});"
        sql_query = sql_query.format(relation.name, sql_attributes)

        logging.debug(sql_query)
        cursor = self.connection.cursor()
        cursor.execute(sql_query)
        cursor.close()

        self.import_data_to_table(relation.name, relation.data_path)


    def import_data_to_table(self, table_name: str, data_path: str):
        cursor = self.connection.cursor()
        with open(data_path) as data_csv:
            cursor.copy_from(data_csv, table_name, sep=',')
        cursor.close()


    def drop_table(self, relation):
        pass


    def remove_dangling_tuples(self, query: Query):
        join_table_name = DatabasePsql.get_join_table_name(query)
        sql_delete_tuples = "DELETE FROM {} WHERE ({}) NOT IN (SELECT {} FROM {})"
        for relation in self.database.get_relations():
            # Join table and the table with dangling tuples have
            # have the same named join attributes
            rel_name = relation.name
            join_attr_names = query.get_join_attrs(rel_name)
            join_attr_names_str = ','.join(join_attr_names)
            sql_delete_tuples_rel = sql_delete_tuples.format(rel_name, join_attr_names_str,
                join_attr_names_str, join_table_name)
            logging.info(sql_delete_tuples_rel)
            cursor = self.connection.cursor()
            cursor.execute(sql_delete_tuples_rel)
            cursor.close()



    @staticmethod
    def get_join_table_name(query: Query):
        join_table_name = DatabasePsql.JOIN_TABLE_NAME + query.get_name()
        return join_table_name

    def evaluate_join(self, query: Query, num_repetitions: int, order_by: DumpConf.OrderRelation):
        join_table_name = DatabasePsql.get_join_table_name(query)
        sql_join = "DROP TABLE IF EXISTS " + join_table_name + ";CREATE TABLE " + join_table_name + " AS (SELECT {} FROM {} ORDER BY {});"

        sql_from_natural_join = ""

        sql_select = ""
        ord_attr_names = query.get_attr_names_ordered()

        if order_by == DumpConf.OrderRelation.JOIN_ATTRIBUTE:
            join_attr_names = query.get_join_attr_names_ordered()
            sql_order_by = ""
            for attribute_name in join_attr_names:
                if attribute_name not in query.get_skip_attrs():
                    sql_order_by += attribute_name + ","

            sql_order_by = sql_order_by[:-1]
        else:
            sql_order_by = "RANDOM()"
            sql_join = "SELECT setseed(.42);" + sql_join


        for attribute_name in ord_attr_names:
            if attribute_name not in query.get_skip_attrs():
                sql_select += attribute_name + ","

        sql_select = sql_select[:-1]
        for idx, relation_name in enumerate(query.get_relation_order()):
            sql_from_table_name = relation_name if idx == 0 \
                else  " NATURAL JOIN " + relation_name
            sql_from_natural_join += sql_from_table_name
        sql_join = sql_join.format(sql_select, sql_from_natural_join, sql_order_by)

        logging.debug(sql_join)
        for i in range(num_repetitions):
            cursor = self.connection.cursor()
            start = timer()
            cursor.execute(sql_join)
            end = timer()
            logging.info("##Figaro####Join##{}".format(end - start))
            cursor.close()



    def limit_join(self, query: Query, percent: float):
        join_table_name = DatabasePsql.get_join_table_name(query)
        join_attr_names = query.get_join_attr_names_ordered()
        sql_order_by = ""
        for attribute_name in join_attr_names:
            if attribute_name not in query.get_skip_attrs():
                sql_order_by += attribute_name + ","

        sql_order_by = sql_order_by[:-1]

        join_table_name_per = join_table_name + "_percent"
        sql_join_per = """SELECT * INTO {}
            FROM {}
            ORDER BY {} DESC
            LIMIT (SELECT (FLOOR(count(*) * {})) AS selnum FROM {})"""
        sql_join_drop = "DROP TABLE {}"
        sql_rename_per_join = "ALTER TABLE {} RENAME TO {}"


        sql_join_per = sql_join_per.format(join_table_name_per, join_table_name,
                             sql_order_by, percent, join_table_name)
        sql_join_drop = sql_join_drop.format(join_table_name)
        sql_rename_per_join = sql_rename_per_join.format(join_table_name_per, join_table_name)

        logging.debug(sql_join_per)
        logging.debug(sql_join_drop)
        logging.debug(sql_rename_per_join)

        cursor = self.connection.cursor()
        cursor.execute(sql_join_per)
        cursor.close()

        cursor = self.connection.cursor()
        cursor.execute(sql_join_drop)
        cursor.close()

        cursor = self.connection.cursor()
        cursor.execute(sql_rename_per_join)
        cursor.close()


    def get_relation_size(self, relation_name) -> int:
        sql_relation_count = "SELECT COUNT(*) FROM {};"
        sql_relation_count = sql_relation_count.format(relation_name)
        logging.debug(sql_relation_count)
        cursor = self.connection.cursor()
        cursor.execute(sql_relation_count)
        relation_size = cursor.fetchone()
        cursor.close()
        return relation_size


    def get_join_size(self, query: Query) -> int:
        join_table_name = DatabasePsql.get_join_table_name(query)
        return self.get_relation_size(join_table_name)


    def log_relation_sizes(self, query: Query):
        for relation_name in query.get_relation_order():
            logging.info("Number of rows in relation {} is {}".
                        format(relation_name,
                        self.get_relation_size(relation_name)))



    def dump_relations(self, percent: float = None):
        for relation in self.database.get_relations():
            cursor = self.connection.cursor()
            if percent is not None:
                head_tail = os.path.split(relation.data_path)
                db_per_dir = os.path.join(head_tail[0], str(percent))
                if not os.path.exists(db_per_dir):
                    os.makedirs(db_per_dir)
                dump_path = os.path.join(db_per_dir, head_tail[1])
            else:
                dump_path = relation.data_path

            logging.info("Dump path {} for relation {}".
                    format(dump_path, relation.name))
            with open(dump_path, 'w') as file_csv:
                cursor.copy_to(file_csv, relation.name, sep=',',
                columns=relation.get_attribute_names())
            cursor.close()


    def dump_join(self, query: Query, output_file_path: str):
        non_join_attribute_names = query.get_non_join_attr_names_ordered()
        join_table_name = DatabasePsql.get_join_table_name(query)
        cursor = self.connection.cursor()
        with open(output_file_path, 'w') as file_csv:
            cursor.copy_to(file_csv, join_table_name, sep=',',
            columns=non_join_attribute_names)
        cursor.close()


    def full_reducer_join(self, query: Query, percent: float=None):
        self.evaluate_join(query, num_repetitions=1, order_by=DumpConf.OrderRelation.JOIN_ATTRIBUTE)
        if percent is not None:
            self.limit_join(query, percent)
        self.remove_dangling_tuples(query)
        self.dump_relations(percent=percent)


    # Passes database spec to be created,
    def create_database(self, database: Database):
        self.close_connection()

        self.database = database
        database_exists = False
        try:
            connection_admin = self.open_connection_admin()
            cursor = connection_admin.cursor()
            cursor.execute(sql.SQL("CREATE DATABASE {}").format(
                                sql.Identifier(self.database.name)))
        except:
            logging.info("Database {} exists".format(database.name))
            database_exists = True
        finally:
            cursor.close()
            connection_admin.close()
            self.open_connection()
        if not database_exists:
            relations = self.database.get_relations()
            for relation in relations:
                self.create_table(relation)


    def drop_database(self):
        self.close_connection()
        connection_admin = self.open_connection_admin()
        cursor = connection_admin.cursor()
        cursor.execute(sql.SQL("DROP DATABASE IF EXISTS {} ").format(
                            sql.Identifier(self.database.name)))
        cursor.close()
        connection_admin.close()
        self.database = None



if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--password", action="store",
                        dest="password", required=True)
    parser.add_argument("-d", "--dump_path", action="store", dest="dump_path")
    args = parser.parse_args()
    db_specs_path = "/home/username/Figaro/figaro-code/system_tests/test1/database_specs.conf"
    database = Database(db_specs_path)
    database_name = "DB1"
    database_psql = DatabasePsql(host_name="",user_name="username", password=args.
                                password, database_name=database_name)
    database_psql.drop_database()
    database_psql.create_database(database)
    database_psql.evaluate_join(database.get_relations())
    database_psql.dump_join(database.get_relations(), args.dump_path)

