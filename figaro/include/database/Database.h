#ifndef _FIGARO_DATABASE_H_
#define _FIGARO_DATABASE_H_

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "utils/Logger.h"
#include "database/Relation.h"

namespace Figaro
{
    class Database
    {
        std::map<std::string, Relation> m_relations; 

        void loadDatabaseRelationsSchema(const json& jsonRelInfos);

        void loadDatabaseSchema(const std::string& schemaConfigPath); 

       
    public:
        Database(const std::string& schemaConfigPath); 

        void loadData(void);
    };
}
#endif 