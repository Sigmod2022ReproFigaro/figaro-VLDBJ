#include "database/Database.h"
#include <string>
#include <iostream>
#include <fstream>


namespace Figaro
{
    void Database::loadDatabaseRelationsSchema(const json& jsonRelInfos)
    {
        for (const auto& jsonRelInfo: jsonRelInfos)
        {
            Relation relation(jsonRelInfo);
            std::string relationName = relation.getName();
            m_relations.emplace(relationName, std::move(relation));
        }
    }

    void Database::loadDatabaseSchema(const std::string& schemaConfigPath) 
    {
        std::ifstream inputFileStream(schemaConfigPath);
        json jsonDbConfig;

        if (inputFileStream.fail())
        {
            FIGARO_LOG_ERROR("Database configuration path incorrect", schemaConfigPath);
            return; 
        }
        inputFileStream >> jsonDbConfig;
        FIGARO_LOG_INFO("Database Configuration", jsonDbConfig);
        
        loadDatabaseRelationsSchema(jsonDbConfig["database"]["relations"]);
    }

    Database::Database(const std::string& schemaConfigPath) 
    {
        loadDatabaseSchema(schemaConfigPath); 
    }

    void Database::loadData(void)
    {
        for (auto& [relName, relation]: m_relations)
        {
            relation.loadData();
        }
    }
}

