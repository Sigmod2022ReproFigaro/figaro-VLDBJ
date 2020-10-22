#include "database/Database.h"

int main() 
{
    Figaro::Database database("/home/popina/Figaro/figaro-code/system_tests/test1/database_specs.conf");
    database.loadData();
    return 0;
}