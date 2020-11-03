#ifndef _FIGARO_RELATION_H_
#define _FIGARO_RELATION_H_

#include "utils/Utils.h"
#include <vector>

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
    public:
        // By default we will map strings to int
        enum class AttributeType 
        {
            FLOAT, INTEGER, CATEGORY
        };
        // key: PK values -> value: corresponding aggregate
        typedef std::map<std::vector<double>, double> GroupByT;
        typedef std::vector<std::vector<double>> VectorOfVectorsT;
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
        };
    private:
        std::string m_name;
        ErrorCode initalizationErrorCode = ErrorCode::NO_ERROR;
        std::vector<Attribute> m_attributes;
        std::string m_dataPath;
        MatrixT m_data; 
        VectorOfVectorsT m_dataVectorOfVectors;
        Relation::GroupByT m_countAggregates;
        
        uint32_t getAttributeIdx(const std::string& attributeName) const;

        /**
         * For each attribute denoted by name stored in vector @p vAttributeNames, return
         * the corresponding column index stored in @p vAttributeIdx. 
         * The computed  order of indices is the same as the order of attribute names
         * provided by @p vAttributeNames. 
         */
        void getAttributesIdxs(const std::vector<std::string>& vAttributeNames, 
        std::vector<uint32_t>& vAttributeIdxs) const;

        uint32_t getNumberOfAttributePKs(void) const;
        
        /**
         * For each part of a composite PK compute the corresponding column index. The
         * order of returned indices is the same as specified initially by the 
         * relational schema. 
         */
        void getAttributeNamesOfAttributePKs(std::vector<std::string>& vAttributeNamesPKs);

        
        // TODO: Update this to return vector.
        uint32_t getNonPKAttributeIdx(void) const;

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
             const std::vector<std::tuple<std::string, std::string> >& vJoinAttributeNames);

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
        void computeHead(const std::vector<std::string>& attrNames);

        void computeHead(const std::string& attrName);

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

    };
        
}
#endif