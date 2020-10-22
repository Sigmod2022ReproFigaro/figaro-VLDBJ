#ifndef _FIGARO_RELATION_H_
#define _FIGARO_RELATION_H_

#include "utils/Utils.h"

#include <string>
#include <sstream>
#include <vector>
#include <set>

namespace Figaro
{
    class Relation
    {
        static constexpr char DELIMITER = '|';
    public:
        // By default we will map strings to int
        enum class Type 
        {
            FLOAT, INTEGER, CATEGORY
        };

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
        uint32_t getAttributeIdx(std::string attributeName)
        {
            for (uint32_t idx = 0; idx < m_attributes.size(); idx ++)
            {
                const auto& attribute = m_attributes[idx];
                if (attribute.m_name == attributeName)
                {
                    return idx;
                }
            }
            FIGARO_LOG_ERROR("Index out of bounds");
            return UINT32_MAX;
        }
    public:
        Relation(const Relation&) = delete;
        Relation(Relation&& ) = default;
        Relation(json jsonRelationSchema)
        {
            m_name = jsonRelationSchema["name"];
            json jsonRelationAttribute = jsonRelationSchema["attributes"];
            FIGARO_LOG_INFO("Relation", m_name)
            for (const auto& jsonRelationAttribute: jsonRelationAttribute)
            {
                Attribute attribute(jsonRelationAttribute);
                m_attributes.push_back(attribute);
            }

            json jsonRelationPKs = jsonRelationSchema["primary_key"];
            for (const auto& jsonRelationPK: jsonRelationPKs)
            {
                uint32_t attributeIdx = getAttributeIdx(jsonRelationPK);
                m_attributes[attributeIdx].m_isPrimaryKey = true;
                FIGARO_LOG_INFO("Primary key", jsonRelationPK);
            }
            m_dataPath = jsonRelationSchema["data_path"];
        }
        Relation(const std::vector<Attribute>& _attributes): m_attributes(_attributes){}
        const std::vector<Attribute>& getAttribute() const 
        {
            return m_attributes;
        }

        const std::string& getName(void) const { return m_name; }

        uint32_t numberOfAttributes() const 
        {
            return m_attributes.size();
        }

        const std::string& getAttributeName(uint32_t attributedIdx)
        {
            return m_attributes.at(attributedIdx).m_name;
        }   

        
        /*******************************************
         * Fills the table data from the file path specified by @p filePath . 
         * The data should be formated in CSV format where the separator is |  * and where the number of attribues is the same as in schema. The 
         * attribute types of lodaded data need to agree with the relational 
         * schema in this class. 
         ******************************************/
        void loadData(void) 
        {
            std::ifstream fileTable(m_dataPath);
            std::istringstream strStream;
            std::string str;
            std::string strVal;
            uint32_t cntLines;
            uint32_t numAttributes;

            if (fileTable.fail())
            {
                FIGARO_LOG_ERROR("Table path is incorrect", m_dataPath);
                return;
            }
            FIGARO_LOG_INFO("Loading data for relation", m_name, "from path", m_dataPath)

            cntLines = getNumberOfLines(m_dataPath);
            numAttributes = numberOfAttributes();
            m_data = Eigen::MatrixXd::Zero(cntLines, numAttributes);

            // TODO: If there is time, write regex that will parse files based on 
            // attributs.  
            //
            for (uint32_t row = 0; row < cntLines; row ++)
            {
                std::getline(fileTable, str);
                FIGARO_LOG_DBG("STRING", str)
                strStream.clear();
                strStream.str(str);
                for (uint32_t col = 0; col < numAttributes; col ++)
                {
                    std::getline(strStream, strVal, DELIMITER);
                    double val = std::stod(str);
                    m_data(row, col) =  val; 
                    FIGARO_LOG_DBG("Value", val);
                }
            }
            printMatrix(std::cout, m_data);
        }
        
    };
        
}
#endif