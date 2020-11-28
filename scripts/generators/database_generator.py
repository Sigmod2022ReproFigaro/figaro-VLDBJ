import json
class DatabaseGenerator:
    def __init__(self, database_specs_path):
        self.database_specs_path = database_specs_path
        with open(database_specs_path) as json_file:
            json_db_schema = json.load(json_file)
            self.json_db_schema = json_db_schema["stats"]
            # Extract stats to generate


    def generate(self, output_path):
        pass


if __name__ == "__main__":
    database_generator = DatabaseGenerator("/home/popina/Figaro/figaro-code/system_tests/test1/database_specs.conf")



        