#ifndef _FIGARO_RELATION_H_
#define _FIGARO_RELATION_H_

#include "utils/Utils.h"
#include "database/storage/Matrix.h"
#include <vector>
#include <unordered_map>

// TODO: Optimize tuple instanciated Relation based on number of attributes. 
namespace Figaro
{
    /** 
     * @class Relation
     * 
     * @brief We prevent attributes with the same name. The order of attributes
     * represent the order in which they are stored in the corresponding 
     * csf file. In the constructor, the relation schema is initalized from
     * json object. The data is not loaded until requested 
     * with function @see loadData.   
     */
    class Relation
    {
        static constexpr char DELIMITER = ',';
        static constexpr uint32_t NUM_COLS_REL = 4;
    public:
        static constexpr uint32_t MAX_NUM_COLS = 4 * NUM_COLS_REL + 2;
        // By default we will map strings to int
        enum class AttributeType 
        {
            FLOAT, INTEGER, CATEGORY
        };
        // key: PK values -> value: corresponding aggregate
        typedef std::map<std::vector<double>, double> GroupByT;
        //typedef std::array<double, MAX_NUM_COLS> RowT;
        //typedef std::vector<RowT> VectorOfVectorsT;
        typedef Figaro::Matrix<double> VectorOfVectorsT;

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
        ErrorCode initalizationErrorCode = ErrorCode::NO_ERROR;
        std::vector<Attribute> m_attributes;
        std::string m_dataPath;
        
        MatrixT m_data; 
        VectorOfVectorsT m_dataVectorOfVectors;
        
        //MatrixT m_dataTails1;
        //MatrixT m_dataTails2;
        //MatrixT m_dataHead;

        VectorOfVectorsT m_dataHead;
        VectorOfVectorsT m_dataTails1;
        VectorOfVectorsT m_dataTails2;

        GroupByT m_countAggregates;

        void copyVectorOfVectorsToEigenData(void);

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

        uint32_t getNumberOfPKAttributes(void) const;

        uint32_t getNumberOfNonPKAttributes(void) const;

        
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

        void schemaJoin(const Relation& relation, bool swapAttributes = false);

    public:
        Relation(const Relation&) = delete;
        Relation(Relation&& ) = default;
        Relation(json jsonRelationSchema);

        const std::vector<Attribute>& getAttributes(void) const 
        {
            return m_attributes;
        }

        const std::string& getName(void) const { return m_name; }


        uint32_t numberOfAttributes() const 
        {
            return m_attributes.size();
        }

        const std::string& getAttributeName(uint32_t attributedIdx) const
        {
            return m_attributes.at(attributedIdx).m_name;
        }   

        uint32_t getDistinctValuesCount(const std::string& attributeName) const;

        void getAttributeDistinctValues(const std::string& attributeName, 
                std::vector<double>& vDistinctValues) const;

        void getAttributeValuesCounts(const std::string& attributeName, 
            std::unordered_map<double, uint32_t>& htCnts) const;

        void getRowPtrs(
            const std::string& attrName,
            std::unordered_map<double, const double*>& htRowPts) const;

        void getDistinctValuesRowPositions(const std::string& attributeName,
             std::vector<uint32_t>& vDistinctValuesRowPositions,
             bool preallocated = true) const;

        /**
         * Fills the table data from the file path specified by @p filePath . 
         * The data should be formated in CSV format where the separator is |  
         * and where the number of attribues is the same as in schema. The 
         * attribute types of lodaded data need to agree with the relational 
         * schema in this class. 
         */
        ErrorCode loadData(void);

        /**
         * Sorts the data stored in @p m_dataVectorOfVectors in ascending 
         * order of PKs. For now, we assume PKs are leading attributes in
         * the relation schema. 
         */
        void sortData(void);

        void sortData(const std::vector<std::string>& vAttributeNames);
        

        void computeCountAggregates(void);

        /**
         * Returns associative data structure whose keys are values group
         * byed on and values are the corresponding counts. 
         */ 
        const Relation::GroupByT& getCountAggregates(void) const;

        void joinRelation(const Relation& relation,
             const std::vector<std::tuple<std::string, std::string> >& vJoinAttributeNames, bool bSwapAttributes);

        /**
         * It will copy the underlying data and apply head transformation onto it.  
         * The Head transformation will be applied as if we had:
         * SELECT PK.part1, PK.part2, ... PK.partn HEAD(remaining attributes)
         * GROUP BY PK.  
         */
        void computeHead(void);

        /**
         * It will copy the underlying data and apply head transformation onto it.  
         * The Head transformation will be applied as if we had:
         * SELECT attrNames[0], attrtNames[1], ... attrNames[n] HEAD(remaining attributes)
         * GROUP BY attrNames.  
         */
        void computeHead(const std::vector<std::string>& attributeNames);

        void computeHead(const std::string& attributeName);


        void computeAndScaleGeneralizedHeadAndTail(
            const std::string& attributeName,
            const std::unordered_map<double, uint32_t>& hashTabAttributeCounts);


        static void extend(std::array<Relation*, 2>& aRelations, const std::string& attrIterName);

        void extend(const Relation& rel, const std::string& attrIterName);

         /**
         * It will copy the underlying data and apply head transformation onto it.  
         * The Head transformation will be applied as if we had:
         * SELECT PK.part1, PK.part2, ... PK.partn HEAD(remaining attributes, v)
         * GROUP BY PK.  
         */
        void computeHead(const VectorT& v);

        /**
         * It will copy the underlying data and apply head transformation onto it.  
         * The Head transformation will be applied as if we had:
         * SELECT attrNames[0], attrtNames[1], ... attrNames[n] HEAD(remaining attributes, v)
         * GROUP BY attrNames.  
         */
        void computeHead(const std::vector<std::string> attrNames, const VectorT& v);

        /**
         * It will copy the underlying data and apply head transformation onto it.  
         * The Head transformation will be applied as if we had:
         * SELECT PK.part1, PK.part2, ... PK.partn HEAD(remaining attributes)
         * GROUP BY PK.  
         */
        void computeTail(void);

        void computeTail(const std::string& attrName);

        void applyEigenQR(MatrixT* pR = nullptr);

        friend std::ostream& operator<<(
            std::ostream& out, 
            const Relation& relation);

    };

        
}
#endif