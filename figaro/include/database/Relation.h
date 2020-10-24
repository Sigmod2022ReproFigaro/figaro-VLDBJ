#ifndef _FIGARO_RELATION_H_
#define _FIGARO_RELATION_H_

#include "utils/Utils.h"
#include <vector>

namespace Figaro
{
    /** 
     * @class Relation
     * 
     * @brief We prevent attributes with the same name. The order of attributes
     * reprsenst the order in which they are stored in the corresponding 
     * csf file. In the constructor, the relation schema is initalized from
     * json object. The data is not loaded until requested 
     * with function @see loadData.   
     */
    class Relation
    {
        static constexpr char DELIMITER = ',';
    public:
        // By default we will map strings to int
        enum class Type 
        {
            FLOAT, INTEGER, CATEGORY
        };

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
            Type m_type = Type::FLOAT; 
            bool m_isPrimaryKey = false;
            Attribute(json jsonAttributeInfo)
            {
                std::string strType;
                const static std::map<std::string, Type> mapStrTypeToType =
                {
                    std::make_pair("int", Type::INTEGER),
                    std::make_pair("float", Type::FLOAT),
                    std::make_pair("category", Type::CATEGORY)
                };
                m_name = jsonAttributeInfo["name"];
                strType = jsonAttributeInfo["type"];
                m_type = mapStrTypeToType.at(strType);
            }
        };
    private:
        std::string m_name;
        std::vector<Attribute> m_attributes;
        std::string m_dataPath;
        MatrixT m_data; 
        uint32_t getAttributeIdx(const std::string& attributeName);
    public:
        Relation(const Relation&) = delete;
        Relation(Relation&& ) = default;
        Relation(json jsonRelationSchema);

        const std::vector<Attribute>& getAttribute() const 
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
        void loadData(void);
           
    };
        
}
#endif