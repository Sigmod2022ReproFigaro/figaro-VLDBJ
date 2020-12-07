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

    void Database::sortRelation(const std::string& relationName, const std::vector<std::string> vSortAttributeNames)
    {
        Relation& relation = m_relations.at(relationName);
        relation.sortData(vSortAttributeNames);
        // TODO: Test this. 
    }

    MatrixT* Database::computeHead(const std::string& relName)
    {
        return nullptr;
    }

    MatrixT* Database::computeHead(const std::string& relName, const std::string& attrName)
    {
        Relation& relation = m_relations.at(relName);//[relName];
        relation.computeHead(attrName);
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

    void  Database::joinRelations(std::vector<std::string> vRelationNames,
        const std::vector<std::tuple<std::string, std::string> >& vJoinAttributeNames)
    {
        Relation& relation1= m_relations.at(vRelationNames[0]);
        Relation& relation2 = m_relations.at(vRelationNames[1]);
        relation1.joinRelation(relation2, vJoinAttributeNames);
    }


    
    void Database::computeScaledCartesianProduct(std::array<std::string, 2> aRelationNames, const std::string& attrIterName)
    {
        constexpr uint32_t NUM_RELATIONS = 2;
        std::array<Relation*, NUM_RELATIONS> aRelations
        {&m_relations.at(aRelationNames.at(0)), 
         &m_relations.at(aRelationNames.at(1))};
        std::array<std::unordered_map<double, uint32_t>, NUM_RELATIONS> 
         aHashTabAttrCnt;
        
        for (uint32_t idxRel = 0; idxRel < aRelations.size(); idxRel++)
        {
            Relation* pRelation = aRelations[idxRel];
            pRelation->getAttributeValuesCountAggregates(attrIterName, 
                        aHashTabAttrCnt[idxRel]);
            FIGARO_LOG_DBG(aHashTabAttrCnt[idxRel]);
        }

        aRelations[0]->computeAndScaleGeneralizedHeadAndTail(
            attrIterName, aHashTabAttrCnt[1]);
        aRelations[1]->computeAndScaleGeneralizedHeadAndTail(
            attrIterName, aHashTabAttrCnt[0]);
        aRelations[0]->extend(*aRelations[1], attrIterName);
    }

    void Database::computeQRDecompositionHouseholder(const std::string& relationName,
    MatrixT* pR)
    {
        m_relations.at(relationName).applyEigenQR(pR);
    }

    void Database::swapAttributes(const std::string& relationName, const std::array<std::string, 2>& atributesSwap)
    {
        m_relations.at(relationName).swapAttributes(atributesSwap);
    }
        


}

