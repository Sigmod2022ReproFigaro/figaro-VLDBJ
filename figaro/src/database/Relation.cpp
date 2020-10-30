#include "database/Relation.h"
#include <string>
#include <sstream>
#include <fstream>
#include <execution>
#include <algorithm>


namespace Figaro
{
    uint32_t Relation::getAttributeIdx(const std::string& attributeName) const
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

    uint32_t Relation::getNumberOfAttributePKs(void) const
    {
        uint32_t partsPK = 0;
        /*
        std::reduce(std::execution::par, m_attributes.begin(), 
                    m_attributes.end(), 0.0,
                     [const& auto sum, const& auto T2]()
                    {
                        return ; 
                    })
        */
       partsPK = std::accumulate(m_attributes.begin(), m_attributes.end(), 0.0,
                     [](int curCnt, const Attribute& nextAtr) 
                     {
                         return nextAtr.m_isPrimaryKey ? (curCnt+1) : curCnt;
                     });
        return partsPK;
    }

    Relation::Relation(json jsonRelationSchema)
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

    ErrorCode Relation::loadData(void)
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
            return ErrorCode::WRONG_PATH;
        }
        FIGARO_LOG_INFO("Loading data for relation", m_name, "from path", m_dataPath)

        cntLines = getNumberOfLines(m_dataPath);
        numAttributes = numberOfAttributes();
        m_data = Eigen::MatrixXd::Zero(cntLines, numAttributes);
        m_dataVectorOfVectors.resize(cntLines);
        for (auto& row: m_dataVectorOfVectors)
        {
            row.resize(numAttributes);
        }
        

        // TODO: If there is time, write regex that will parse files based on the attributes type. 
        // Attributes in schema configuration correspond to the 
        // order of attributes in data representation.
        // TODO: Add header parsing in the case if we decide to add 
        // headers for attributes.   
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
                double val = std::stod(strVal);
                m_data(row, col) =  val; 
                m_dataVectorOfVectors[row][col] = val;
            }
        }
        printMatrix(std::cout, m_data);
        return ErrorCode::NO_ERROR;
    }

    void Relation::sortData(void)
    {
        std::sort(std::execution::par_unseq, m_dataVectorOfVectors.begin(), m_dataVectorOfVectors.end());

        std::cout << "Relation: " << m_name << std::endl;
        for (int row = 0; row < m_data.rows(); row ++)
        {
            for (int col = 0; col < m_data.cols(); col++)
            {
                std::cout << m_dataVectorOfVectors[row][col] << " ";
            }
            std::cout << std::endl;
        }
    }

    void Relation::computeCountAggregates(void)
    {
        uint32_t partsPK = getNumberOfAttributePKs();
        m_countAggregates.clear();
    }

    const Relation::GroupByT& Relation::getCountAggregates(void) const
    {
        return m_countAggregates;
    }
}