#ifndef _FIGARO_DATABASE_H_
#define _FIGARO_DATABASE_H_

#include "utils/Utils.h"
#include "Relation.h"

namespace Figaro
{
    class Database
    {
        std::map<std::string, Relation> m_relations;
        ErrorCode initializationErrorCode = ErrorCode::NO_ERROR;

        ErrorCode loadDatabaseRelationsSchema(const json& jsonRelInfos);

        ErrorCode loadDatabaseSchema(const std::string& schemaConfigPath);


    public:
        Database(const std::string& schemaConfigPath);

        ErrorCode getInitializationErrorCode(void) const
         { return initializationErrorCode; }

        /**
         * Reads the data for each relation specified by the corresponding paths
         * and stores in the corresponding members of relations.
         */
        ErrorCode loadData(void);

        /**
         * Sorts the data of each relation from the database.
         */
        void sortData(void);

        void sortRelation(const std::string& relationName, const std::vector<std::string>& vSortAttributeNames);

        std::vector<std::string> getRelationAttributeNames(const std::string& relationName);

        void computeDownCounts(
            const std::string& relationName,
            const std::vector<std::string>& vChildRelNames,
            const std::vector<std::string>& vJoinAttrNames,
            const std::vector<std::string>& vParJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            bool isRootNode);

        void computeUpAndCircleCounts(
            const std::string& relationName,
            const std::vector<std::string>& vChildRelNames,
            const std::vector<std::string>& vParJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            bool isRootNode);

        std::map<std::vector<double>, uint32_t> getDownCounts(const std::string& relationName);

        std::map<std::vector<double>, uint32_t> getParDownCnts(
            const std::string& relationName,
            const std::vector<std::string>& vParJoinAttrNames);

        std::map<std::vector<double>, uint32_t> getParUpCnts(
            const std::string& relationName,
            const std::vector<std::string>& vParJoinAttrNames);

        std::map<std::vector<double>, uint32_t> getCircCounts(const std::string& relationName);

        void computeHeadsAndTails(
            const std::string& relationName,
            const std::vector<std::string>& vJoinAttrNames,
            bool isLeafNode);

        void aggregateAwayChildrenRelations(
            const std::string& relationName,
            const std::vector<std::string>& vChildRelNames,
            const std::vector<std::string>& vJoinAttributeNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames);

        void computeAndScaleGeneralizedHeadAndTail(
            const std::string& relationName,
            const std::vector<std::string>& vJoinAttributeNames,
            const std::vector<std::string>& vParJoinAttributeNames,
            bool isRootNode);


        void computeQROfConcatenatedGeneralizedHeadAndTails(
            const std::vector<std::string>& vRelationOrder);


        const Relation::MatrixDT& getHead(const std::string& relationName) const;

        const Relation::MatrixDT& getTail(const std::string& relationName) const;

        const Relation::MatrixDT& getGeneralizedTail(const std::string& relationName) const;

        void joinRelations(std::vector<std::string> vRelationNames,
        const std::vector<std::tuple<std::string, std::string> >& vJoinAttributeNames, bool swapAttributes = false);

        void computeScaledCartesianProduct(std::array<std::string, 2> relationNames,
            std::array<Eigen::VectorXd, 2> vectors);

        void computeScaledCartesianProduct(std::array<std::string, 2> aRelationNames,
                        const std::string& attrIterName);

        void computeQRDecompositionHouseholder(const std::string& relationName, MatrixEigenT* pR = nullptr);
    };
}
#endif