#ifndef _FIGARO_RELATION_H_
#define _FIGARO_RELATION_H_

#include "utils/Utils.h"
#include "database/storage/Matrix.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <set>
#include "tbb/atomic.h"

// TODO: Optimize tuple instanciated Relation based on number of attributes.
namespace Figaro
{
    /**
     * @class Relation
     *
     * @brief We prevent attributes with the same name. The order of attributes
     * represent the order in which they are stored in the corresponding
     * csv file. In the constructor, the relation schema is initalized from
     * json object. The data is not loaded until requested
     * with function @see loadData.
     */
    class Relation
    {
        static constexpr char DELIMITER = ',';

        static bool compareTuples(const double pTuple1[], const double pTuple2[],
                const std::vector<uint32_t>& vAttrIdxs)
        {
            for (const auto& attrIdx: vAttrIdxs)
            {
                if (pTuple1[attrIdx] != pTuple2[attrIdx])
                {
                    return true;
                }
            }
            return false;
        }
    public:
        // By default we will map strings to int
        enum class AttributeType
        {
            FLOAT, INTEGER, CATEGORY
        };
        // key: PK values -> value: corresponding aggregate
        typedef std::map<std::vector<double>, double> GroupByT;
        typedef Figaro::Matrix<double> MatrixDT;
        typedef Figaro::Matrix<uint32_t> MatrixUI32T;
        typedef std::tuple<uint32_t, tbb::atomic<uint32_t> > DownUpCntT;

        /**
         * 1) Allocates matrix of type MatrixEigen @p matEig in memory.
         * 2) Copies the content of @p matDT to @p matEig.
         */
        static void copyMatrixDTToMatrixEigen(const MatrixDT& matDT, MatrixEigenT& matEig);

        /**
         * @struct Attribute
         *
         * This structure containts metadata about the attribute.
         * Metadata includes name, type, if the attribute is part of
         * primary key.
         */
        struct Attribute
        {
            std::string m_name = "";
            AttributeType m_type = AttributeType::FLOAT;
            bool m_isPrimaryKey = false;
            const std::map<AttributeType, std::string> mapTypeToStr =
            {
                std::make_pair(AttributeType::INTEGER, "int"),
                std::make_pair(AttributeType::FLOAT, "float"),
                std::make_pair(AttributeType::FLOAT, "double"),
                std::make_pair(AttributeType::CATEGORY, "category")
            };

            Attribute(){}

            Attribute(const json& jsonAttributeInfo)
            {
                std::string strType;
                const static std::map<std::string, AttributeType> mapStrTypeToType =
                {
                    std::make_pair("int", AttributeType::INTEGER),
                    std::make_pair("float", AttributeType::FLOAT),
                    std::make_pair("double", AttributeType::FLOAT),
                    std::make_pair("category", AttributeType::CATEGORY)
                };
                m_name = jsonAttributeInfo["name"];
                strType = jsonAttributeInfo["type"];
                m_type = mapStrTypeToType.at(strType);
            }

            Attribute& operator=(const Attribute& other)
            {
                if (this != &other)
                {
                    m_name = other.m_name;
                    m_type = other.m_type;
                    m_isPrimaryKey = m_isPrimaryKey;
                }
                return *this;
            }
            friend void swap(Attribute& attr1, Attribute& attr2)
            {
                std::swap(attr1.m_name, attr2.m_name);
                std::swap(attr1.m_type, attr2.m_type);
                std::swap(attr1.m_isPrimaryKey, attr2.m_isPrimaryKey);
            }
        };
    private:
        std::string m_name;
        ErrorCode initializationErrorCode = ErrorCode::NO_ERROR;
        std::vector<Attribute> m_oldAttributes;
        std::vector<Attribute> m_attributes;
        std::string m_dataPath;
        MatrixDT m_data;

        MatrixDT m_dataHead;
        MatrixDT m_dataTails;
        MatrixDT m_dataTailsGen;

        MatrixDT m_scales;
        MatrixDT m_dataScales;
        std::vector<double> m_allScales;
        /**
         * Contains the names of all relations in the subtree rooted
         * at this relation including this relation. The order of relations
         * is preorder.
         */
        std::vector<std::string> m_vSubTreeRelNames;
        /**
         * Contains the data ofsets of all relations in the subtree rooted
         * at this relation including this relation. The order of relations
         * is preorder.
         */
        std::vector<uint32_t> m_vSubTreeDataOffsets;

        MatrixUI32T m_countsJoinAttrs;
        std::vector<uint32_t> m_vParBlockStartIdxs;
        std::vector<uint32_t> m_vParBlockStartIdxsAfterFirstPass;

        /**
         * @brief This a pointer to the corresponding associative container whose
         * keys are the corresponding distinct parent attribute values and
         * the values are the corresponding tuples of up and down counts.
         */
        std::shared_ptr<void> m_pHTParCounts;

        /**
         * @brief Column index in the @p m_CountsJoinAttrs. that stores the corresponding
         * down count.
         */
        uint32_t m_cntsJoinIdxD;
        /**
         * @brief Column index in the @p m_CountsJoinAttrs. that stores the corresponding
         * number of tuples corresponding to the key in this relation.
         */
        uint32_t m_cntsJoinIdxV;
        /**
         * @brief Column index in the @p m_CountsJoinAttrs. that stores the corresponding
         * circle count.
         */
        uint32_t m_cntsJoinIdxC;
        uint32_t m_cntsJoinIdxE;

        MatrixDT m_dataTails1;
        MatrixDT m_dataTails2;

        uint32_t getAttributeIdx(const std::string& attributeName) const;

        /**
         * For each attribute denoted by name stored in vector @p vAttributeNames, return
         * the corresponding column index stored in @p vAttributeIdx.
         * The computed  order of indices is the same as the order of attribute names
         * provided by @p vAttributeNames.
         */
        void getAttributesIdxs(
            const std::vector<std::string>& vAttributeNames,
            std::vector<uint32_t>& vAttributeIdxs) const;

        /**
         * Computes complement set of attribute indices corresponding to
         * the @p vAttributeIdxs from the attributes for the
         * current relation. The order of indices in @p vAttributeIdxs
         * should be always ascending, otherwise the function does not work.
         */
        void getAttributesIdxsComplement(const std::vector<uint32_t>& vAttributeIdxs,
            std::vector<uint32_t>& vAttributesCompIdxs) const;

        uint32_t getNumberOfPKAttributes(void) const;

        uint32_t getNumberOfNonPKAttributes(void) const;

        uint32_t getNumberOfAttributes(void) const
        {
            return m_attributes.size();
        }

        /**
         * For each part of a composite PK compute the corresponding column index. The
         * order of returned indices is the same as specified initially by the
         * relational schema.
         */
        void getPKAttributeNames(
            std::vector<std::string>& vAttributeNamesPKs) const;

        void getNonPKAttributeNames(
            std::vector<std::string>& vAttributeNamesNonPKs) const;

        void getPKAttributeIndices(std::vector<uint32_t>& vPkAttrIdxs) const;

        /**
         * Returns a vector of indices of the attributes that are not part
         * of composite primary key for this relation. Indexing of attributes
         * starts from 0.
         */
        void getNonPKAttributeIdxs(std::vector<uint32_t>& vNonPkAttrIdxs) const;

        void getCategoricalAttributeIdxs(std::vector<uint32_t>& vCatAttrIdxs) const;


        void schemaJoin(const Relation& relation, bool swapAttributes = false);

        /**
         *  Updates schema of the current relation such that
         * non-join attributes of @p vpChildRels are appended to the end.
         * @p m_vSubTreeRelNames and @p m_vSubTreeDataOffsets are also
         * updated accordingly.
         */
        void schemaJoins(
            const std::vector<Relation*>& vpChildRels,
            const std::vector<std::vector<uint32_t> >& vvJoinAttrIdxs,
            const std::vector<std::vector<uint32_t> >& vvNonJoinAttrIdxs);


        void schemaRemoveNonParJoinAttrs(
            const std::vector<uint32_t>& vJoinAttrIdxs,
            const std::vector<uint32_t>& vParJoinAttrIdxs);

        void schemaDropAttrs(std::vector<uint32_t> vDropAttrIdxs);

        void schemaOneHotEncode(
            const std::map<uint32_t, std::set<uint32_t> >& mCatAttrsDist);

        void getDistinctValsAndBuildIndices(
            const std::vector<uint32_t>& vAttrIdxs,
            const std::vector<uint32_t>& vParAttrIdxs,
            MatrixUI32T& cntDiffVals,
            std::vector<uint32_t>& vRowIdxParDiffVals,
            std::vector<uint32_t>& vParBlockStartIdxsAfterFirstPass,
            bool isRootNode);

        /**
         *  Builds hash index where key is @p vJoinAttrIdx over the @p data
         * returns it as a pointer @p pHashTablePt.
         * @note Destructor needs to be called after the usage.
         */
        void initHashTableRowIdxs(
            const std::vector<uint32_t>& vParJoinAttrIdxs,
            void*& pHashTablePt,
            const MatrixDT& data);

        /**
         * Looks up in the hash table @p hashTabRowPt for join attributes from parent
         * @p vParJoinAttrIdxs and the value specified in @p dataParent [ @p rowIdx ]
         */
        uint32_t getChildRowIdx(uint32_t rowIdx,
            const std::vector<uint32_t>& vParJoinAttrIdxs,
            void*  hashTabRowPt,
            const MatrixDT& dataParent);

        void destroyHashTableRowIdxs(
            const std::vector<uint32_t>& vParJoinAttrIdxs,
            void*& pHashTablePt);

        void initHashTable(const std::vector<uint32_t>& vParAttrIdx,
            uint32_t hashTableSize);

        void insertParDownCntFromHashTable(
            const std::vector<uint32_t>& vParAttrIdx,
            const uint32_t* pRow,
            uint32_t downCnt);

        Figaro::Relation::DownUpCntT& getParCntFromHashTable(
            const std::vector<uint32_t>& vParJoinAttrIdxs,
            void*  htChildRowIdx,
            const uint32_t* pRow);

        Figaro::Relation::DownUpCntT& getParCntFromHashTable(
            const std::vector<uint32_t>& vParJoinAttrIdxs,
            void*  htChildRowIdx,
            const double* pRow);

        static void makeDiagonalElementsPositiveInR(MatrixEigenT& matR);

        /***************** OLD IMPLEMENTATTION ***********/
        uint32_t getDistinctValuesCount(const std::string& attributeName) const;

        void getAttributeDistinctValues(const std::string& attributeName,
                std::vector<double>& vDistinctValues) const;

        void getAttributeDistinctValues(const std::vector<uint32_t>& vAttrIdxs,
                std::vector<double>& vDistinctValues) const;

        void getRowPtrs(
            const std::string& attrName,
            std::unordered_map<double, const double*>& htRowPts) const;

        void getDistinctValuesRowPositions(const std::string& attributeName,
            std::vector<uint32_t>& vDistinctValuesRowPositions,
            bool preallocated = true) const;
    public:
        Relation(const Relation&) = delete;
        Relation(Relation&& ) = default;
        Relation(json jsonRelationSchema);
        void resetComputations(void);

        const std::vector<Attribute>& getAttributes(void) const
        {
            return m_attributes;
        }

        std::vector<std::string> getAttributeNames(void) const;

        const std::string& getName(void) const { return m_name; }


        uint32_t numberOfAttributes() const
        {
            return m_attributes.size();
        }

        const std::string& getAttributeName(uint32_t attributedIdx) const
        {
            return m_attributes.at(attributedIdx).m_name;
        }

        /**
         * Fills the table data from the file path specified by @p filePath .
         * The data should be formated in CSV format where the separator is |
         * and where the number of attribues is the same as in schema. The
         * attribute types of lodaded data need to agree with the relational
         * schema in this class.
         */
        ErrorCode loadData(void);

        /**
         * Sorts the data stored in @p m_data in ascending
         * order of PKs. For now, we assume PKs are leading attributes in
         * the relation schema.
         */
        void sortData(void);

        void sortData(const std::vector<std::string>& vAttributeNames);

        void dropAttributes(const std::vector<std::string>& vAttributeNames);

        void updateSchema(const std::vector<std::string>& vAttributeNames);

        void oneHotEncode(void);

        void computeDownCounts(
            const std::vector<Relation*>& vpChildRels,
            const std::vector<std::string>& vJoinAttrNames,
            const std::vector<std::string>& vParJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            bool isRootNode);


        void computeUpAndCircleCounts(
            const std::vector<Relation*>& vpChildRels,
            const std::vector<std::string>& vParJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            bool isRoot = false);

        /*********************** Testing getters for counts ***************/
        std::map<std::vector<uint32_t>, uint32_t> getDownCounts(void);

        std::map<std::vector<uint32_t>, uint32_t> getParDownCntsFromHashTable(
        const std::vector<std::string>& vParJoinAttrNames);

        std::map<std::vector<uint32_t>, uint32_t> getParUpCntsFromHashTable(
        const std::vector<std::string>& vParJoinAttrNames);

        std::map<std::vector<uint32_t>, uint32_t> getCircCounts(void);

        /**
         * It will copy the underlying data and apply head transformation onto it.
         * The Head and Tail transformation will be applied as if we had:
         * SELECT attrNames[0], attrNames[1], ... attrNames[n]
         *  HEAD(remaining attributes),
         *  TAIL(remaining attributes)
         * GROUP BY attrNames.
         */
        void computeHeadsAndTails(
            const std::vector<std::string>& vJoinAttrNames, bool isLeafNode);

        /**
         *  It will join relations by copying data from the children relations @p vpChildRels
         *  in the join tree to the current head data. The join attributes that are
         *  not in the relation ( @p vParJoinAttributeNames ) that is in the parent node
         *  will be omitted. We assume @p vParJoinAttributeNames is a subset of
         *  @p vJoinAttributeNames and each @p vvJoinAttributeNames[i] is a subset of
         *  @p vJoinAttributeNames .
         */
        void aggregateAwayChildrenRelations(
            const std::vector<Relation*>& vpChildRels,
            const std::vector<std::string>& vJoinAttributeNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames);

        void computeAndScaleGeneralizedHeadAndTail(
            const std::vector<std::string>& vJoinAttributeNames,
            const std::vector<std::string>& vParJoinAttributeNames,
            bool isRootNode);

        void computeQROfGeneralizedHead(uint32_t numNonJoinAttrs);

        void computeQROfTail(void);

        void computeQROfGeneralizedTail(void);


        // Should be called for a root relation.
        void computeQROfConcatenatedGeneralizedHeadAndTails(
            const std::vector<Relation*>& pRelationOrder,
            MatrixEigenT* pR);

        /**
         *  Returns computed head for the corresponding relation, without
         *  dividing by square roots.
         */
        const MatrixDT& getHead(void) const;

        const MatrixDT& getTail(void) const;

        const MatrixDT& getGeneralizedTail(void) const;

        const MatrixDT& getScales(void) const;

        const MatrixDT& getDataScales(void) const;

        /****************** OLD IMPLEMENTATION **************************/
        void getAttributeValuesCounts(const std::string& attributeName,
            std::unordered_map<double, uint32_t>& htCnts) const;

        void joinRelation(const Relation& relation,
             const std::vector<std::tuple<std::string, std::string> >& vJoinAttributeNames,
             bool bSwapAttributes);

        void computeAndScaleGeneralizedHeadAndTail(
            const std::string& attributeName,
            const std::unordered_map<double, uint32_t>& hashTabAttributeCounts);

        void extend(const Relation& rel, const std::string& attrIterName);

        void applyEigenQR(MatrixEigenT* pR = nullptr);

        friend std::ostream& operator<<(
            std::ostream& out,
            const Relation& relation);
    };
}
#endif