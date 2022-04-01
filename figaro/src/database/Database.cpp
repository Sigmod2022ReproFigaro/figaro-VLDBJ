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

    std::tuple<std::string, std::string>
    Database::saveQRResult(
            std::tuple<Relation*, Relation*> qrResult)
    {
        auto [pR, pQ] = qrResult;
        std::string rName = "";
        std::string qName = "";
        Relation& matR = *pR;
        Relation& matQ = *pQ;
        if (pR != nullptr)
        {
            rName = pR->getName();
            m_relations.emplace(rName, std::move(matR));

        }
        if (pQ != nullptr)
        {
            qName = pQ->getName();
            m_relations.emplace(qName, std::move(matQ));
        }
        return std::make_tuple(rName, qName);
    }

    Database::Database(const std::string& schemaConfigPath)
    {
        initializationErrorCode = loadDatabaseSchema(schemaConfigPath);
    }

    Database::Database(std::vector<Relation>&& vRels)
    {
        for (auto& rel: vRels)
        {
            std::string name = rel.getName();
            m_relations.emplace(name, std::move(rel));
        }
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

    void Database::renameRelation(const std::string& oldRelationName,
            const std::string& newRelationName)
    {
        Relation& rel = m_relations.at(oldRelationName);
        rel.renameRelation(newRelationName);
        auto nodeHandler = m_relations.extract(oldRelationName);
        nodeHandler.key() = newRelationName;
        m_relations.insert(std::move(nodeHandler));
    }

    std::string Database::copyRelation(const std::string& relName)
    {
        Relation& rel = m_relations.at(relName);
        auto copiedRel = rel.copyRelation();
        std::string newName = copiedRel.getName();
        m_relations.emplace(newName, std::move(copiedRel));
        return newName;
    }


    void Database::persistRelation(const std::string& relationName)
    {
        Relation& rel = m_relations.at(relationName);
        rel.persist();
    }

    void Database::destroyAuxRelations(void)
    {
        std::vector<std::string> auxRelNames;
        for (const auto& [relName, rel]: m_relations)
        {
            if (rel.isTmp())
            {
                auxRelNames.push_back(relName);
            }
        }
        for (const auto& relName: auxRelNames)
        {
            m_relations.erase(relName);
        }
    }

    std::string Database::joinRelations(const std::string& relationName,
            const std::vector<std::string>& vChildRelNames,
            const std::vector<std::string>& vJoinAttrNames,
            const std::vector<std::string>& vParJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            bool trackProvenance)
    {
        std::vector<Relation*> vpChildRels;
        Relation& rel = m_relations.at(relationName);
        FIGARO_LOG_INFO("Relation names", vChildRelNames)
        for (const auto childRelName: vChildRelNames)
        {
            Relation* pRel = &m_relations.at(childRelName);
            vpChildRels.push_back(pRel);
        }

        Relation relJoin = rel.joinRelations(
            vpChildRels, vJoinAttrNames,
            vParJoinAttrNames,  vvJoinAttributeNames, trackProvenance);
        std::string relJoinName = relJoin.getName();
        m_relations.emplace(relJoin.getName(), std::move(relJoin));;
        FIGARO_LOG_INFO("New rel name", relJoin.getName())
        return relJoinName;
    }

    std::string Database::joinRelations(
            const std::vector<std::string>& vRelNames,
            const std::vector<std::string>& vParRelNames,
            const std::vector<std::vector<std::string> >& vvJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvParJoinAttrNames,
            uint32_t joinSize)
    {
        std::vector<Relation*> vpRels;
        std::vector<Relation*> vpParRels;
        for (const auto relName: vRelNames)
        {
            Relation* pRel = &m_relations.at(relName);
            vpRels.push_back(pRel);
        }

        for (const auto parRelName: vParRelNames)
        {
            if (parRelName == "")
            {
                vpParRels.push_back(nullptr);
            }
            else
            {
                Relation* pRel = &m_relations.at(parRelName);
                vpParRels.push_back(pRel);
            }
        }

        Relation relJoin = Relation::joinRelations(
            vpRels, vpParRels,
            vvJoinAttrNames, vvParJoinAttrNames, joinSize);
        std::string relJoinName = relJoin.getName();
        m_relations.emplace(relJoin.getName(), std::move(relJoin));;
        FIGARO_LOG_INFO("New rel name", relJoin.getName())
        return relJoinName;
    }

    std::string Database::joinRelationsAndAddColumns(const std::string& relationName,
            const std::vector<std::string>& vChildRelNames,
            const std::vector<std::string>& vJoinAttrNames,
            const std::vector<std::string>& vParJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            bool trackProvenance)
    {
        std::vector<Relation*> vpChildRels;
        Relation& rel = m_relations.at(relationName);
        FIGARO_LOG_INFO("Relation names", vChildRelNames)
        for (const auto childRelName: vChildRelNames)
        {
            Relation* pRel = &m_relations.at(childRelName);
            vpChildRels.push_back(pRel);
        }

        Relation relJoin = rel.joinRelationsAndAddColumns(
            vpChildRels, vJoinAttrNames,
            vParJoinAttrNames,  vvJoinAttributeNames, trackProvenance);
        std::string relJoinName = relJoin.getName();
        m_relations.emplace(relJoin.getName(), std::move(relJoin));;
        FIGARO_LOG_INFO("New rel name", relJoin.getName())
        return relJoinName;
    }


    std::string Database::joinRelationsAndAddColumns(
            const std::vector<std::string>& vRelNames,
            const std::vector<std::string>& vParRelNames,
            const std::vector<std::vector<std::string> >& vvJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvParJoinAttrNames,
            uint32_t joinSize)
    {
        std::vector<Relation*> vpRels;
        std::vector<Relation*> vpParRels;
        for (const auto relName: vRelNames)
        {
            Relation* pRel = &m_relations.at(relName);
            vpRels.push_back(pRel);
        }

        for (const auto parRelName: vParRelNames)
        {
            if (parRelName == "")
            {
                vpParRels.push_back(nullptr);
            }
            else
            {
                Relation* pRel = &m_relations.at(parRelName);
                vpParRels.push_back(pRel);
            }
        }

        Relation relJoin = Relation::joinRelationsAndAddColumns(
            vpRels, vpParRels,
            vvJoinAttrNames, vvParJoinAttrNames, joinSize);
        std::string relJoinName = relJoin.getName();
        m_relations.emplace(relJoin.getName(), std::move(relJoin));;
        FIGARO_LOG_INFO("New rel name", relJoin.getName())
        return relJoinName;
    }


    std::string Database::multiply(const std::string& relationName1,
            const std::string& relationName2,
            const std::vector<std::string>& vJoinAttrNames1,
            const std::vector<std::string>& vJoinAttrNames2,
            uint32_t startRowIdx2)
    {
        Relation& rel1 = m_relations.at(relationName1);
        Relation& rel2 = m_relations.at(relationName2);

        Relation mulRel = rel1.multiply(rel2, vJoinAttrNames1, vJoinAttrNames2,
            startRowIdx2);

        std::string mulRelName = mulRel.getName();
        m_relations.emplace(mulRel.getName(), std::move(mulRel));
        return mulRelName;
    }

    std::string Database::selfMatrixMultiply(
            const std::string& relationName,
            const std::vector<std::string>& vJoinAttrNames)
    {
        Relation& rel = m_relations.at(relationName);

        FIGARO_LOG_DBG(rel)

        Relation smmRel = rel.selfMatrixMultiply(vJoinAttrNames);

        std::string smmRelName = smmRel.getName();
        m_relations.emplace(smmRelName, std::move(smmRel));
        return smmRelName;
    }

    std::string Database::linearRegression(
            const std::string& relationName,
            const std::string& labelName)
    {
        Relation& rel = m_relations.at(relationName);

        FIGARO_LOG_DBG(rel)

        Relation linRegRel = rel.linearRegression(labelName);

        std::string linRegRelName = linRegRel.getName();
        m_relations.emplace(linRegRelName, std::move(linRegRel));
        return linRegRelName;
    }

    std::string Database::inverse(const std::string& relationName,
            const std::vector<std::string>& vJoinAttrNames)
    {
        Relation& rel = m_relations.at(relationName);

        FIGARO_LOG_DBG(rel)

        Relation invRel = rel.inverse(vJoinAttrNames);

        std::string invRelName = invRel.getName();
        m_relations.emplace(invRelName, std::move(invRel));
        return invRelName;
    }

    double Database::checkOrthogonality(const std::string& relationName,
            const std::vector<std::string>& vJoinAttrNames) const
    {
        const Relation& rel = m_relations.at(relationName);

        double ortMeasure = rel.checkOrthogonality(vJoinAttrNames);

        return ortMeasure;
    }

    void Database::buildIndices(
            const std::string& relationName,
            const std::vector<std::string>& vJoinAttrNames,
            const std::vector<std::string>& vParJoinAttrNames,
            bool isRootNode)
    {
        Relation& rel = m_relations.at(relationName);

        rel.buildIndices(vJoinAttrNames, vParJoinAttrNames, isRootNode);
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

    uint32_t Database::getDownCountSum(const std::string& relationName) const
    {
        const Relation& rel = m_relations.at(relationName);
        return rel.getDownCountSum();
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

    std::tuple<std::string, std::string> Database::computeHeadsAndTails(
        const std::string& relationName,
        const std::vector<std::string>& vJoinAttrNames,
        bool isLeafNode)
    {
        Relation& rel = m_relations.at(relationName);
        auto [relHeads, relTails] = rel.computeHeadsAndTails(vJoinAttrNames, isLeafNode);
        auto headsName = relHeads.getName();
        auto tailName = relTails.getName();
        m_relations.emplace(relHeads.getName(), std::move(relHeads));
        m_relations.emplace(relTails.getName(), std::move(relTails));
        return {headsName, tailName};
    }

    std::string Database::aggregateAwayChildrenRelations(
            const std::string& relationName,
            const std::string& relHeadName,
            const std::vector<std::string>& vChildRelNames,
            const std::vector<std::string>& vChildHeadRelNames,
            const std::vector<std::string>& vJoinAttributeNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            const std::vector<std::string>& vSubTreeRelNames,
            const std::vector<std::vector<std::string> >& vvSubTreeRelnames)
    {
        Relation& rel = m_relations.at(relationName);
        Relation& relHead = m_relations.at(relHeadName);
        std::vector<Relation*> vpChildRels;
        std::vector<Relation*> vpChildHeadRels;

        for (const auto childRelName: vChildRelNames)
        {
            Relation* pRel = &m_relations.at(childRelName);
            vpChildRels.push_back(pRel);
        }
        FIGARO_LOG_INFO(vChildHeadRelNames);
        for (const auto childHeadRelName: vChildHeadRelNames)
        {
            Relation* pRel = &m_relations.at(childHeadRelName);
            vpChildHeadRels.push_back(pRel);
        }
        FIGARO_LOG_INFO(vChildRelNames);

        Relation relAggAway = rel.aggregateAwayChildrenRelations(&relHead, vpChildRels,
            vpChildHeadRels, vJoinAttributeNames, vvJoinAttributeNames,
            vSubTreeRelNames, vvSubTreeRelnames);
        std::string aggregatedAwayName = relAggAway.getName();
        m_relations.emplace(aggregatedAwayName, std::move(relAggAway));
        return aggregatedAwayName;
    }

    std::tuple<std::string, std::string>
    Database::computeAndScaleGeneralizedHeadAndTail(
        const std::string& relationName,
        const std::string& aggrAwayRelName,
        const std::vector<std::string>& vJoinAttributeNames,
        const std::vector<std::string>& vParJoinAttributeNames,
        bool isRootNode,
        uint32_t numRelsSubTree)
    {
        Relation& rel = m_relations.at(relationName);
        Relation& aggAwayRel = m_relations.at(aggrAwayRelName);
        auto [genHeadRel, genTailRel] =
        rel.computeAndScaleGeneralizedHeadAndTail(
            &aggAwayRel,
            vJoinAttributeNames, vParJoinAttributeNames,
            isRootNode, numRelsSubTree);
        std::string genHeadRelname = genHeadRel.getName();
        std::string genTailRelname = genTailRel.getName();

        m_relations.emplace(genHeadRelname, std::move(genHeadRel));
        m_relations.emplace(genTailRelname, std::move(genTailRel));

        return {genHeadRelname, genTailRelname};
    }

    std::tuple<std::string, std::string>
    Database::computePostprocessing(
            const std::vector<std::string>& vRelationOrder,
            const std::string& genHeadRoot,
            const std::vector<std::string>& vTailRels,
            const std::vector<std::string>& vGenTailRels,
            Figaro::QRHintType qrHintType,
            bool saveResult,
            const std::string& joinRelName)
    {
        std::vector<Relation*> vpRels;
        std::vector<Relation*> vpRelTails;
        std::vector<Relation*> vpRelGenTails;
        Relation* pRootRel;
        Relation* pJoinRel = nullptr;
        for (const auto relName: vRelationOrder)
        {
            Relation* pRel = &m_relations.at(relName);
            vpRels.push_back(pRel);
        }
        for (const auto relName: vTailRels)
        {
            Relation* pRel = &m_relations.at(relName);
            FIGARO_LOG_ASSERT(pRel != nullptr)
            vpRelTails.push_back(pRel);
            pRel->computeQRInPlace(qrHintType);
            FIGARO_LOG_INFO("Tail name", relName)
        }

        for (const auto relName: vGenTailRels)
        {
            //MICRO_BENCH_INIT(measureGenTail)
            //MICRO_BENCH_START(measureGenTail)
            Relation* pRel = &m_relations.at(relName);
            FIGARO_LOG_ASSERT(pRel != nullptr)
            vpRelGenTails.push_back(pRel);
            pRel->computeQRInPlace(qrHintType);
            //MICRO_BENCH_STOP(measureGenTail)
            //FIGARO_LOG_BENCH("Gen tail time" + relName, MICRO_BENCH_GET_TIMER_LAP(measureGenTail))
            FIGARO_LOG_INFO("Gen Tail name", relName)
        }

        pRootRel = &m_relations.at(genHeadRoot);
        FIGARO_LOG_INFO("Root Rel name", pRootRel->getName())
        pRootRel->computeQROfGeneralizedHead(vpRelTails, qrHintType);

        if (joinRelName != "")
        {
            pJoinRel = &m_relations.at(joinRelName);
        }
        auto qrResult = pRootRel->computeQROfConcatenatedGeneralizedHeadAndTails(
            vpRels,
            pRootRel,
            vpRelTails, vpRelGenTails,
            qrHintType, saveResult, pJoinRel);
        return saveQRResult(qrResult);
    }

    void Database::changeMemoryLayout(void)
    {
        for (auto& [relName, relation]: m_relations)
        {
            FIGARO_LOG_INFO("One hot encoding relation", relName)
            relation.changeMemoryLayout(MemoryLayout::COL_MAJOR);
        }
    }

    std::tuple<std::string, std::string>
    Database::evalPostprocessing(
            const std::string& relName,
            Figaro::QRHintType qrHintType,
            Figaro::MemoryLayout memoryLayout,
            bool computeQ,
            bool saveResult)
    {
        Relation& rel = m_relations.at(relName);
        auto qrResult = rel.computeQR(qrHintType, memoryLayout, computeQ, saveResult);
        return saveQRResult(qrResult);
    }

    bool Database::destroyTemporaryRelation(const std::string& relationName)
    {
        Relation& rel = m_relations.at(relationName);
        if (rel.isTmp())
        {
            m_relations.erase(relationName);
            return true;
        }
        return false;
    }

    std::string Database::createDummyGenTailRelation(const std::string& relationName)
    {
        Relation& rel = m_relations.at(relationName);
        Relation dummyGenTail = rel.createDummyGenTailRelation();
        std::string dummRelName = dummyGenTail.getName();
        m_relations.emplace(dummRelName, std::move(dummyGenTail));
        return dummRelName;
    }

    void Database::outputRelationToFile(std::ostream& out, const std::string& relationName,
        char sep, uint32_t precision, bool header)
    {
        const Relation& rel = m_relations.at(relationName);
        rel.outputToFile(out, sep, precision, header);
    }

    void Database::outputRelation(const std::string& relName) const
    {
        FIGARO_LOG_DBG("relation", m_relations.at(relName))
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
}

