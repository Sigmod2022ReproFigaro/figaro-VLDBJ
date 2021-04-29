import psycopg2
from psycopg2 import sql
from typing import List
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT
import argparse
import logging
from data_management.database import Database
from data_management.relation import Relation
from timeit import default_timer as timer

JOIN_TABLE_NAME = "join_table"

class DatabasePsql:
    def __init__(self, host_name, user_name, password, database_name = None):
        self.host_name = host_name
        self.user_name = user_name
        self.password = password
        self.database_name = database_name
        if database_name is not None:
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
        if check_db and not self.check_if_db_exists(self.database_name):
            self.connection = None
        else:
            self.connection = self.open_connection_static(self.host_name, self.user_name,
                     self.password, self.database_name)
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
            sql_attribute = sql_attribute.format(attribute.name, attribute.type)
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


    @staticmethod
    def get_non_join_order_of_attributes(relations: List[Relation]):
        non_join_attr_names = []
        for relation in relations:
            non_join_attr_names += relation.get_non_join_attribute_names()
        logging.debug("HOHO", non_join_attr_names)
        return non_join_attr_names


    @staticmethod
    def get_order_of_attributes(relations: List[Relation]):
        join_attr_names = []
        non_join_attribute_names = DatabasePsql.get_non_join_order_of_attributes(relations)

        for relation in relations:
            join_attr_names += relation.get_join_attribute_names()

        join_attr_names_unique = list(dict.fromkeys(join_attr_names))
        return join_attr_names_unique + non_join_attribute_names


    def evaluate_join(self, relations, num_repetitions: int,
    drop_attributes: List[str]):
        sql_join = "DROP TABLE IF EXISTS " + JOIN_TABLE_NAME + ";CREATE TABLE " + JOIN_TABLE_NAME + " AS (SELECT {} FROM {});"
        sql_from_natural_join = ""

        sql_select = ""
        ord_attr_names = self.get_order_of_attributes(relations)

        for attribute_name in ord_attr_names:
            if attribute_name not in drop_attributes:
                sql_select += attribute_name + ","

        sql_select = sql_select[:-1]
        for idx, relation in enumerate(relations):
            sql_from_table_name = relation.name if idx == 0 \
                else  " NATURAL JOIN " + relation.name
            sql_from_natural_join += sql_from_table_name
        sql_join = sql_join.format(sql_select, sql_from_natural_join)

        logging.debug(sql_join)
        for i in range(num_repetitions):
            cursor = self.connection.cursor()
            start = timer()
            cursor.execute(sql_join)
            end = timer()
            logging.info("##Figaro####Join##{}".format(end - start))
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


    def get_join_size(self) -> int:
        return self.get_relation_size(JOIN_TABLE_NAME)


    def log_relation_sizes(self, relation_names):
        for relation_name in relation_names:
            logging.info("Number of rows in relation {} is {}".
                        format(relation_name,
                        self.get_relation_size(relation_name)))


    def dump_join(self, relations, skip_attributes, output_file_path):
        non_join_attribute_names = DatabasePsql.get_non_join_order_of_attributes(relations)
        for skip_attribute in skip_attributes:
            non_join_attribute_names.remove(skip_attribute)
        cursor = self.connection.cursor()
        with open(output_file_path, 'w') as file_csv:
            cursor.copy_to(file_csv, JOIN_TABLE_NAME, sep=',',
            columns=non_join_attribute_names)


    # Passes database spec to be created,
    def create_database(self, database: Database):
        self.close_connection()
        self.database_name = database.name

        connection_admin = self.open_connection_admin()
        cursor = connection_admin.cursor()
        cursor.execute(sql.SQL("CREATE DATABASE {}").format(
                            sql.Identifier(self.database_name)))
        cursor.close()
        connection_admin.close()
        self.open_connection()

        relations = database.get_relations()
        for relation in relations:
            self.create_table(relation)



    def drop_database(self):
        self.close_connection()
        connection_admin = self.open_connection_admin()
        cursor = connection_admin.cursor()
        cursor.execute(sql.SQL("DROP DATABASE IF EXISTS {} ").format(
                            sql.Identifier(self.database_name)))
        cursor.close()
        connection_admin.close()
        self.database_name = None



if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--password", action="store",
                        dest="password", required=True)
    parser.add_argument("-d", "--dump_path", action="store", dest="dump_path")
    args = parser.parse_args()
    db_specs_path = "/home/popina/Figaro/figaro-code/system_tests/test1/database_specs.conf"
    database = Database(db_specs_path)
    database_name = "DB1"
    database_psql = DatabasePsql(host_name="",user_name="popina", password=args.
                                password, database_name=database_name)
    database_psql.drop_database()
    database_psql.create_database(database)
    database_psql.evaluate_join(database.get_relations())
    database_psql.dump_join(database.get_relations(), args.dump_path)

