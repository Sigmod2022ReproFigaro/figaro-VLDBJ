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

    void Database::resetComputations(void)
    {
        for (auto& [relName, relation]: m_relations)
        {
            relation.resetComputations();
        }
    }

    void Database::sortData(void)
    {
        for (auto& [relName, relation]: m_relations)
        {
            relation.sortData();
        }
    }

    void Database::sortRelation(const std::string& relationName, const std::vector<std::string>&  vSortAttributeNames)
    {
        Relation& relation = m_relations.at(relationName);
        relation.sortData(vSortAttributeNames);
    }

    std::vector<std::string> Database::getRelationAttributeNames(const std::string& relationName)
    {
        auto& relation = m_relations.at(relationName);
        return relation.getAttributeNames();
    }

    void Database::dropAttributesFromRelations(
            const std::vector<std::string>& vDropAttrNames)
    {
        for (auto& [relName, relation]: m_relations)
        {
            relation.dropAttributes(vDropAttrNames);
        }
    }

    void Database::updateSchemaOfRelation(
        const std::string& relationName,
        const std::vector<std::string>& vAttrNames)
    {
        auto& relation = m_relations.at(relationName);
        return relation.updateSchema(vAttrNames);
    }

    void Database::oneHotEncodeRelations(void)
    {
        for (auto& [relName, relation]: m_relations)
        {
            FIGARO_LOG_INFO("One hot encoding relation", relName)
            relation.oneHotEncode();
        }
    }



    void  Database::joinRelations(std::vector<std::string> vRelationNames,
        const std::vector<std::tuple<std::string, std::string> >& vJoinAttributeNames, bool swapAttributes)
    {
        Relation& relation1= m_relations.at(vRelationNames[0]);
        Relation& relation2 = m_relations.at(vRelationNames[1]);

        relation1.joinRelation(relation2, vJoinAttributeNames, swapAttributes);
    }

    void Database::computeDownCounts(
        const std::string& relationName,
        const std::vector<std::string>& vChildRelNames,
        const std::vector<std::string>& vJoinAttrNames,
        const std::vector<std::string>& vParJoinAttrNames,
        const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
        bool isRootNode)
    {
        std::vector<Relation*> vpChildRels;
        Relation& rel = m_relations.at(relationName);

        for (const auto childRelName: vChildRelNames)
        {
            Relation* pRel = &m_relations.at(childRelName);
            vpChildRels.push_back(pRel);
        }
        rel.computeDownCounts(vpChildRels, vJoinAttrNames, vParJoinAttrNames,
            vvJoinAttributeNames, isRootNode);
    }


    void Database::computeUpAndCircleCounts(
        const std::string& relationName,
        const std::vector<std::string>& vChildRelNames,
        const std::vector<std::string>& vParJoinAttrNames,
        const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
        bool isRootNode)
    {
        std::vector<Relation*> vpChildRels;
        Relation& rel = m_relations.at(relationName);

        for (const auto childRelName: vChildRelNames)
        {
            Relation* pRel = &m_relations.at(childRelName);
            vpChildRels.push_back(pRel);
        }
        rel.computeUpAndCircleCounts(vpChildRels, vParJoinAttrNames,
            vvJoinAttributeNames, isRootNode);
    }

    std::map<std::vector<uint32_t>, uint32_t> Database::getDownCounts(
        const std::string& relationName)
    {
        Relation& rel = m_relations.at(relationName);
        return rel.getDownCounts();
    }


    std::map<std::vector<uint32_t>, uint32_t> Database::getParDownCnts(
            const std::string& relationName,
            const std::vector<std::string>& vParJoinAttrNames)
    {
        Relation& rel = m_relations.at(relationName);
        return rel.getParDownCntsFromHashTable(vParJoinAttrNames);
    }

    std::map<std::vector<uint32_t>, uint32_t> Database::getParUpCnts(
            const std::string& relationName,
            const std::vector<std::string>& vParJoinAttrNames)
    {
        Relation& rel = m_relations.at(relationName);
        return rel.getParUpCntsFromHashTable(vParJoinAttrNames);
    }

    std::map<std::vector<uint32_t>, uint32_t> Database::getCircCounts(
        const std::string& relationName)
    {
        Relation& rel = m_relations.at(relationName);
        return rel.getCircCounts();
    }

    void Database::computeHeadsAndTails(
        const std::string& relationName,
        const std::vector<std::string>& vJoinAttrNames,
        bool isLeafNode)
    {
        Relation& rel = m_relations.at(relationName);
        rel.computeHeadsAndTails(vJoinAttrNames, isLeafNode);
    }

    void Database::aggregateAwayChildrenRelations(
            const std::string& relationName,
            const std::vector<std::string>& vChildRelNames,
            const std::vector<std::string>& vJoinAttributeNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames)
    {
        Relation& rel = m_relations.at(relationName);
        std::vector<Relation*> vpChildRels;

        for (const auto childRelName: vChildRelNames)
        {
            Relation* pRel = &m_relations.at(childRelName);
            vpChildRels.push_back(pRel);
        }
        rel.aggregateAwayChildrenRelations(vpChildRels,
             vJoinAttributeNames, vvJoinAttributeNames);
    }

    void Database::computeAndScaleGeneralizedHeadAndTail(
        const std::string& relationName,
        const std::vector<std::string>& vJoinAttributeNames,
        const std::vector<std::string>& vParJoinAttributeNames,
        bool isRootNode)
    {
        Relation& rel = m_relations.at(relationName);
        rel.computeAndScaleGeneralizedHeadAndTail(vJoinAttributeNames, vParJoinAttributeNames,
            isRootNode);
    }

    void Database::computePostprocessing(
        const std::vector<std::string>& vRelationOrder,
        MatrixD::QRGivensHintType qrHintType,
        MatrixEigenT* pR

    )
    {
        std::vector<Relation*> vpRels;
        Relation* pRootRel;
        for (const auto relName: vRelationOrder)
        {
            Relation* pRel = &m_relations.at(relName);
            FIGARO_LOG_ASSERT(pRel != nullptr)
            vpRels.push_back(pRel);
            pRel->computeQROfTail(qrHintType);
            pRel->computeQROfGeneralizedTail(qrHintType);
        }
        pRootRel = vpRels[0];
        pRootRel->computeQROfGeneralizedHead(vpRels, qrHintType);
        pRootRel->computeQROfConcatenatedGeneralizedHeadAndTails(vpRels, qrHintType, pR);
    }

    const Relation::MatrixDT& Database::getHead(const std::string& relationName) const
    {
        const Relation& rel = m_relations.at(relationName);
        return rel.getHead();
    }

    const Relation::MatrixDT& Database::getTail(const std::string& relationName) const
    {
        const Relation& rel = m_relations.at(relationName);
        return rel.getTail();
    }

    const Relation::MatrixDT& Database::getGeneralizedTail(const std::string& relationName) const
    {
        const Relation& rel = m_relations.at(relationName);
        return rel.getGeneralizedTail();
    }

    const Relation::MatrixDT& Database::getScales(const std::string& relationName) const
    {
        const Relation& rel = m_relations.at(relationName);
        return rel.getScales();
    }

    const Relation::MatrixDT& Database::getDataScales(const std::string& relationName) const
    {
        const Relation& rel = m_relations.at(relationName);
        return rel.getDataScales();
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
        }
        MICRO_BENCH_STOP(hash)
        FIGARO_LOG_BENCH("Figaro", "main", "computeScaledCartesianProduct", "hash", MICRO_BENCH_GET_TIMER(hash));

        MICRO_BENCH_START(compute)
        aRelations[0]->computeAndScaleGeneralizedHeadAndTail(
            attrIterName, aHashTabAttrCnt[1]);
        MICRO_BENCH_STOP(compute)
        FIGARO_LOG_BENCH("Figaro", "main", "computeScaledCartesianProduct", "scale", MICRO_BENCH_GET_TIMER_LAP(compute));
        MICRO_BENCH_START(compute)
        aRelations[1]->computeAndScaleGeneralizedHeadAndTail(
            attrIterName, aHashTabAttrCnt[0]);
        MICRO_BENCH_STOP(compute)
        FIGARO_LOG_BENCH("Figaro", "main", "computeScaledCartesianProduct", "scale", MICRO_BENCH_GET_TIMER_LAP(compute));

        MICRO_BENCH_START(extend)
        aRelations[0]->extend(*aRelations[1], attrIterName);
        MICRO_BENCH_STOP(extend)
        FIGARO_LOG_BENCH("Figaro", "main", "computeScaledCartesianProduct", "extend", MICRO_BENCH_GET_TIMER(extend));

    }

    void Database::computeQRDecompositionHouseholder(const std::string& relationName,
    MatrixEigenT* pR)
    {
        m_relations.at(relationName).applyEigenQR(pR);
    }

}

