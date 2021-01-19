#include "database/Database.h"
#include <string>
#include <iostream>
#include <fstream>
#include "utils/Performance.h"

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
        const std::vector<std::tuple<std::string, std::string> >& vJoinAttributeNames, bool swapAttributes)
    {
        Relation& relation1= m_relations.at(vRelationNames[0]);
        Relation& relation2 = m_relations.at(vRelationNames[1]);
        
        relation1.joinRelation(relation2, vJoinAttributeNames, swapAttributes);
    }

    
    void Database::computeScaledCartesianProduct(std::array<std::string, 2> aRelationNames, const std::string& attrIterName)
    {
        constexpr uint32_t NUM_RELATIONS = 2;
        std::array<Relation*, NUM_RELATIONS> aRelations
        {&m_relations.at(aRelationNames.at(0)), 
         &m_relations.at(aRelationNames.at(1))};
        std::array<std::unordered_map<double, uint32_t>, NUM_RELATIONS> 
         aHashTabAttrCnt;
        
        MICRO_BENCH_INIT(hash)
        MICRO_BENCH_INIT(compute)
        MICRO_BENCH_START(hash)
        MICRO_BENCH_INIT(extend)

        #pragma omp parallel for schedule(static)
        for (uint32_t idxRel = 0; idxRel < aRelations.size(); idxRel++)
        {
            Relation* pRelation = aRelations[idxRel];
            pRelation->getAttributeValuesCounts(attrIterName, 
                        aHashTabAttrCnt[idxRel]);
            FIGARO_LOG_DBG(aHashTabAttrCnt[idxRel]);
        }
        MICRO_BENCH_STOP(hash)
        FIGARO_LOG_BENCH("Figaro", "main", "hash", MICRO_BENCH_GET_TIMER(hash));

        MICRO_BENCH_START(compute)
        aRelations[0]->computeAndScaleGeneralizedHeadAndTail(
            attrIterName, aHashTabAttrCnt[1]);
        MICRO_BENCH_STOP(compute)
        FIGARO_LOG_BENCH("Figaro", "main", "scale", MICRO_BENCH_GET_TIMER_LAP(compute));
        MICRO_BENCH_START(compute)
        aRelations[1]->computeAndScaleGeneralizedHeadAndTail(
            attrIterName, aHashTabAttrCnt[0]);
        MICRO_BENCH_STOP(compute)
        FIGARO_LOG_BENCH("Figaro", "main", "scale", MICRO_BENCH_GET_TIMER_LAP(compute));
        FIGARO_LOG_BENCH("Figaro", "main", "scaleTotal", MICRO_BENCH_GET_TIMER(compute));

        MICRO_BENCH_START(extend)
        aRelations[0]->extend(*aRelations[1], attrIterName);
        MICRO_BENCH_STOP(extend)
        FIGARO_LOG_BENCH("Figaro", "main", "extend", MICRO_BENCH_GET_TIMER(extend));

    }

    void Database::computeQRDecompositionHouseholder(const std::string& relationName,
    MatrixT* pR)
    {
        m_relations.at(relationName).applyEigenQR(pR);
    }

}

