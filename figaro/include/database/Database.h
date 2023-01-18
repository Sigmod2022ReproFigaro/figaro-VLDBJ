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

        std::tuple<std::string, std::string> saveQRResult(
            std::tuple<Relation*, Relation*> qrResult);

        std::tuple<std::string, std::string, std::string> saveSVDResult(
            std::tuple<Relation*, Relation*, Relation*> svdResult);

    public:
        Database(const std::string& schemaConfigPath);

        Database(std::vector<Relation>&& vRels);

        ErrorCode getInitializationErrorCode(void) const
         { return initializationErrorCode; }

        /**
         * Reads the data for each relation specified by the corresponding paths
         * and stores in the corresponding members of relations.
         */
        ErrorCode loadData(void);

        void resetComputations(void);

        /**
         * Sorts the data of each relation from the database.
         */
        void sortData(void);

        void sortRelation(const std::string& relationName, const std::vector<std::string>& vSortAttributeNames);

        std::vector<std::string> getRelationAttributeNames(const std::string& relationName);

        void dropAttributesFromRelations(
            const std::vector<std::string>& vDropAttrNames);

        void updateSchemaOfRelation(
            const std::string& relationName,
            const std::vector<std::string>& vAttrNames);

        void oneHotEncodeRelations(void);

        void renameRelation(
            const std::string& oldRelationName,
            const std::string& newRelationName);

        std::string copyRelation(
            const std::string& relName);

        void persistRelation(const std::string& relationName);

        void destroyAuxRelations(void);

        /**
         * @brief Join relation @p relationName with the children relations
         * specified in @p vChildRelNames on join attributes specified in @p vJoinAttrnames and @p vvJoinAttributeNames .
         * @p vParJoinAttrNames specifies which join attributes should remain after the join has finished.
         *
         * @return std::string a name of a tmp rotation that contains the join result.
         */
        std::string joinRelations(const std::string& relationName,
            const std::vector<std::string>& vChildRelNames,
            const std::vector<std::string>& vJoinAttrNames,
            const std::vector<std::string>& vParJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            bool trackProvenance);


        std::string joinRelations(
            const std::vector<std::string>& vRelNames,
            const std::vector<std::string>& vParRelNames,
            const std::vector<std::vector<std::string> >& vvJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvParJoinAttrNames,
            uint32_t joinSize);

        std::string joinRelationsAndAddColumns(const std::string& relationName,
            const std::vector<std::string>& vChildRelNames,
            const std::vector<std::string>& vJoinAttrNames,
            const std::vector<std::string>& vParJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            bool trackProvenance);

        std::string joinRelationsAndAddColumns(
            const std::vector<std::string>& vRelNames,
            const std::vector<std::string>& vParRelNames,
            const std::vector<std::vector<std::string> >& vvJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvParJoinAttrNames,
            const std::vector<uint32_t>& vDownCountsSizes,
            const std::vector<uint32_t>& vBlockSizes);


        std::string multiply(const std::string& relationName1,
            const std::string& relationName2,
            const std::vector<std::string>& vJoinAttrNames1,
            const std::vector<std::string>& vJoinAttrNames2,
            uint32_t startRowIdx2);

        std::string selfMatrixMultiply(
            const std::string& relationName,
            const std::vector<std::string>& vJoinAttrNames);

        std::string linearRegression(
            const std::string& relationRName,
            const std::string& labelName
        );

        std::string computeSVDSigmaVTranInverse(
            const std::string& relationSigmaName,
            const std::string& relationVTName
        );

        double checkOrthogonality(const std::string& relationName,
            const std::vector<std::string>& vJoinAttrNames) const;

        double getNorm(const std::string& relationName,
            const std::vector<std::string>& vJoinAttrNames) const;

        double estimateConditionNumber(const std::string& relationName,
            const std::vector<std::string>& vJoinAttrNames) const;

        double checkResidualErrorOfQR(const std::string& relationName,
            const std::string& qName, const std::string& rName);

        std::string inverse(const std::string& relationName1,
            const std::vector<std::string>& vJoinAttrNames);

        void buildIndices(
            const std::string& relationName,
            const std::vector<std::string>& vJoinAttrNames,
            const std::vector<std::string>& vParJoinAttrNames,
            bool isRootNode);

        void computeDownCounts(
            const std::string& relationName,
            const std::vector<std::string>& vChildRelNames,
            const std::vector<std::string>& vJoinAttrNames,
            const std::vector<std::string>& vParJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            bool isRootNode);

        void getDownCountSum(const std::string& relationName,
            std::vector<uint32_t>& vDownCountSum,
            std::vector<uint32_t>& vBlockSize) const;

        void computeUpAndCircleCounts(
            const std::string& relationName,
            const std::vector<std::string>& vChildRelNames,
            const std::vector<std::string>& vParJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            bool isRootNode);

        std::tuple<std::string, std::string> computeQRHeadsAndTails(
            const std::string& relationName,
            const std::vector<std::string>& vJoinAttrNames,
            bool isLeafNode);

        std::string aggregateAwayQRChildrenRelations(
            const std::string& relationName,
            const std::string& relHeadName,
            const std::vector<std::string>& vChildRelNames,
            const std::vector<std::string>& vChildHeadRelNames,
            const std::vector<std::string>& vJoinAttributeNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            const std::vector<std::string>& vSubTreeRelNames,
            const std::vector<std::vector<std::string> >& vvSubTreeRelnames);

        std::tuple<std::string, std::string>
        computeAndScaleQRGeneralizedHeadAndTail(
            const std::string& relationName,
            const std::string& aggrAwayRelName,
            const std::vector<std::string>& vJoinAttributeNames,
            const std::vector<std::string>& vParJoinAttributeNames,
            bool isRootNode,
            uint32_t numRelsSubTree);

        std::tuple<std::string, std::string>
        computeQRPostprocessing(
            const std::vector<std::string>& vRelationOrder,
            const std::string& genHeadRoot,
            const std::vector<std::string>& vTailRels,
            const std::vector<std::string>& vGenTailRels,
            Figaro::QRHintType qrHintType,
            bool saveResult,
            const std::string& joinRelName);


        std::tuple<std::string, std::string> computeLUHeadsAndTails(
            const std::string& relationName,
            const std::vector<std::string>& vJoinAttrNames,
            bool isLeafNode);

        std::string aggregateAwayLUChildrenRelations(
            const std::string& relationName,
            const std::string& relHeadName,
            const std::vector<std::string>& vChildRelNames,
            const std::vector<std::string>& vChildHeadRelNames,
            const std::vector<std::string>& vJoinAttributeNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            const std::vector<std::string>& vSubTreeRelNames,
            const std::vector<std::vector<std::string> >& vvSubTreeRelnames);

        std::tuple<std::string, std::string>
        computeAndScaleLUGeneralizedHeadAndTail(
            const std::string& relationName,
            const std::string& aggrAwayRelName,
            const std::vector<std::string>& vJoinAttributeNames,
            const std::vector<std::string>& vParJoinAttributeNames,
            bool isRootNode,
            uint32_t numRelsSubTree);

        std::tuple<std::string, std::string>
        computeLUPostprocessing(
            const std::vector<std::string>& vRelationOrder,
            const std::string& genHeadRoot,
            const std::vector<std::string>& vTailRels,
            const std::vector<std::string>& vGenTailRels,
            Figaro::LUHintType luHintType,
            bool saveResult,
            const std::string& joinRelName);

        std::string extractLUPermutationMatrix(const std::string& relName);

        void changeMemoryLayout(void);

        std::tuple<std::string, std::string> evalQRDecAlg(
            const std::string& relName,
            Figaro::QRHintType qrHintType,
            Figaro::MemoryLayout memoryLayout,
            bool computeQ,
            bool saveResult);

        std::tuple<std::string, std::string> evalLUDecAlg(
            const std::string& relName,
            Figaro::MemoryLayout memoryLayout,
            bool saveResult);

        std::tuple<std::string, std::string, std::string> evalSVDDecAlg(
            const std::string& relName,
            Figaro::SVDHintType svdHintType,
            Figaro::MemoryLayout memoryLayout,
            bool computeUAndV,
            bool saveResult);

        std::tuple<std::string, std::string, std::string> evalPCADecAlg(
            const std::string& relName,
            Figaro::PCAHintType pcaHintType,
            Figaro::MemoryLayout memoryLayout,
            uint32_t numDims,
            bool computeRed,
            bool saveResult);

        /**
         * @brief Destroys temporary relation @p relName with its data.
         *
         * @param relName
         * @return true In a case when relation alongside data is removed from a database.
         * @return false In a case when relation is not temporary and thus, not removed.
         */
        bool destroyTemporaryRelation(const std::string& relationName);

        std::string createDummyGenTailRelation(const std::string& relName);

        void outputRelationToFile(std::ostream& out, const std::string& relationName,
            char delimiter, uint32_t precision, bool header = false);

        /********** Getters  used for testing **********/

        std::map<std::vector<uint32_t>, uint32_t> getDownCounts(const std::string& relationName);

        std::map<std::vector<uint32_t>, uint32_t> getParDownCnts(
            const std::string& relationName,
            const std::vector<std::string>& vParJoinAttrNames);

        std::map<std::vector<uint32_t>, uint32_t> getParUpCnts(
            const std::string& relationName,
            const std::vector<std::string>& vParJoinAttrNames);

        std::map<std::vector<uint32_t>, uint32_t> getCircCounts(const std::string& relationName);

        void outputRelation(const std::string& relName) const;

        const Relation::MatrixDRowT& getHead(const std::string& relationName) const;

        const Relation::MatrixDRowT& getTail(const std::string& relationName) const;

        const Relation::MatrixDRowT& getGeneralizedHead(const std::string& relationName) const;

        const Relation::MatrixDRowT& getGeneralizedTail(const std::string& relationName) const;

        const Relation::MatrixDRowT& getScales(const std::string& relationName) const;

        const Relation::MatrixDRowT& getDataScales(const std::string& relationName) const;
    };
}
#endif