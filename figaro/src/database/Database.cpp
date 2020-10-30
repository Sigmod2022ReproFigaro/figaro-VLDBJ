#include "database/Database.h"
#include <string>
#include <iostream>
#include <fstream>


namespace Figaro
{
    ErrorCode Database::loadDatabaseRelationsSchema(const json& jsonRelInfos)
    {
        for (const auto& jsonRelInfo: jsonRelInfos)
        {
            Relation relation(jsonRelInfo);
            std::string relationName = relation.getName();
            m_relations.emplace(relationName, std::move(relation));
        }
        return ErrorCode::NO_ERROR;
    }

    ErrorCode Database::loadDatabaseSchema(const std::string& schemaConfigPath) 
    {
        std::ifstream inputFileStream(schemaConfigPath);
        json jsonDbConfig;

        if (inputFileStream.fail())
        {
            FIGARO_LOG_ERROR("Database configuration path incorrect", schemaConfigPath);
            return ErrorCode::WRONG_PATH;  
        }
        inputFileStream >> jsonDbConfig;
        FIGARO_LOG_INFO("Database Configuration", jsonDbConfig);
        
        loadDatabaseRelationsSchema(jsonDbConfig["database"]["relations"]);
        return ErrorCode::NO_ERROR;
    }

    Database::Database(const std::string& schemaConfigPath) 
    {
        initializationErrorCode = loadDatabaseSchema(schemaConfigPath); 
    }

    ErrorCode Database::loadData(void)
    {
        for (auto& [relName, relation]: m_relations)
        {
            ErrorCode errorCode = relation.loadData();
            if (errorCode != ErrorCode::NO_ERROR)
            {
                return errorCode;
            }
        }
        return ErrorCode::NO_ERROR;
    }

    void Database::sortData(void)
    {
        for (auto& [relName, relation]: m_relations)
        {
            relation.sortData();
        }
    }

    MatrixT* Database::computeHead(const std::string& relName)
    {
        //m_relations[relName].compu;
        return nullptr;
    }

    MatrixT* Database::computeTail(const std::string& relName)
    {
        return nullptr;
    }

    void Database::computeScaledCartesianProduct(std::array<std::string, 2> relationNames,
            std::array<Eigen::VectorXd, 2> vectors)
    {
        
    }

        


}

