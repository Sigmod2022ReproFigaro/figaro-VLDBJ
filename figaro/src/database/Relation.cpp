#include "database/Relation.h"
#include <string>
#include <sstream>
#include <fstream>
#include <execution>
#include <algorithm>
#include <limits>
#include <unordered_map>

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

    void Relation::getAttributeNamesOfAttributePKs(std::vector<std::string>& vAttributeNamesPKs)
    {
        for (const auto& attribute: m_attributes)
        {
            if (attribute.m_isPrimaryKey)
            {
                vAttributeNamesPKs.push_back(attribute.m_name);
            }
        }
    }

    Relation::Relation(json jsonRelationSchema)
    {
        m_name = jsonRelationSchema["name"];
        json jsonRelationAttribute = jsonRelationSchema["attributes"];
        FIGARO_LOG_INFO("Relation", m_name)
        for (const auto& jsonRelationAttribute: jsonRelationAttribute)
        {
            FIGARO_LOG_DBG("WTF BEGIN")
            Attribute attribute(jsonRelationAttribute);
            m_attributes.push_back(attribute);
            FIGARO_LOG_DBG("WTF END")
            
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

    void Relation::getAttributesIdxs(const std::vector<std::string>& vAttributeNames, 
        std::vector<uint32_t>& vAttributeIdxs) const
    {
        for (const auto& attributeName: vAttributeNames)
        {
            uint32_t attributeIdx = getAttributeIdx(attributeName);
            vAttributeIdxs.push_back(attributeIdx);
        }
    }

     void Relation::sortData(const std::vector<std::string>& vAttributeNames)
    {
        std::vector<uint32_t> vAttributesIdxs;
        getAttributesIdxs(vAttributeNames, vAttributesIdxs);
        std::sort(std::execution::par_unseq, m_dataVectorOfVectors.begin(),
                  m_dataVectorOfVectors.end(), 
                  [&vAttributesIdxs]
                  (const std::vector<double>& row1, const std::vector<double>& row2)
                  {
                      for (const auto& vAttributesIdx: vAttributesIdxs)
                      {
                        if (row1[vAttributesIdx] != row2[vAttributesIdx])
                        {
                            return row1[vAttributesIdx] < row2[vAttributesIdx];
                        }
                      }
                      return false; 
                  });

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

    void Relation::sortData(void)
    {
        std::vector<std::string> vAttrNamesPKs;
        getAttributeNamesOfAttributePKs(vAttrNamesPKs);
        sortData(vAttrNamesPKs);
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

    void computeHead(const std::vector<std::string>& attrNames);

    void computeHead(const VectorT& v);

    static uint32_t countUnique(MatrixT mat, uint32_t colIdx)
    {
        double prevAttrVal = std::numeric_limits<double>::max();
        uint32_t distCnt = 0; 
        for (uint32_t rowIdx = 0; rowIdx < mat.rows(); rowIdx++)
        {
            double curAttrVal = mat(rowIdx, colIdx);
            if (prevAttrVal != curAttrVal)
            {
                distCnt++;
                prevAttrVal = curAttrVal;
            }
        } 
        return distCnt;   
    }

    static void storeUniqueAndLimits(MatrixT mat, uint32_t colIdx, 
            std::vector<double>& vDistinctVals, std::vector<uint32_t>& vLimitEnds)
    {
        double prevAttrVal = std::numeric_limits<double>::max();
        uint32_t distCnt = 0; 

        // The first entry is saved for the end of imaginary predecessor of ranges. 
        for (uint32_t rowIdx = 0; rowIdx < mat.rows(); rowIdx++)
        {
            double curAttrVal = mat(rowIdx, colIdx);
            if (prevAttrVal != curAttrVal)
            {
                FIGARO_LOG_DBG("PrevAttrVal", prevAttrVal, "curAttrVal", curAttrVal, "distCnt", distCnt);
                FIGARO_LOG_DBG("rowIdx", rowIdx);
                vDistinctVals[distCnt] = curAttrVal;
                vLimitEnds[distCnt] = rowIdx - 1;
                prevAttrVal = curAttrVal;
                distCnt++;
            }
        } 
        FIGARO_LOG_DBG("distCnt", distCnt);
        vLimitEnds[distCnt] = mat.rows() - 1;
    }

    uint32_t Relation::getNonPKAttributeIdx(void) const
    {
        for (uint32_t idx = 0; idx < m_attributes.size(); idx++)
        {
            if (!m_attributes[idx].m_isPrimaryKey)
            {
                return idx;
            }
        }
    }

    void Relation::computeHead(const std::string& attribute)
    {
        uint32_t attrIdx;
        std::uint32_t nonPKAttrIdx;
        double prevAttrVal;
        uint32_t numDistinctValues;
        std::vector<double> aggregateByAttribute;
        std::vector<double> vDistinctVals; 
        std::vector<uint32_t> vLimitEnds; 

        attrIdx = getAttributeIdx(attribute);
        nonPKAttrIdx = getNonPKAttributeIdx(); 
        prevAttrVal = std::numeric_limits<double>::max();
        numDistinctValues = countUnique(m_data, attrIdx);
        
        aggregateByAttribute.resize(numDistinctValues);
        vDistinctVals.resize(numDistinctValues);
        vLimitEnds.resize(numDistinctValues + 1);
        storeUniqueAndLimits(m_data, attrIdx, vDistinctVals, vLimitEnds);
        FIGARO_LOG_DBG("RelatioN", m_name, "Attribute", attribute)
        printVector(std::cout, vDistinctVals);
        printVector(std::cout, vLimitEnds);
        for (uint32_t limIdx = 0; limIdx < vDistinctVals.size(); limIdx++)
        {
           for (uint32_t rowIdx = vLimitEnds[limIdx] + 1; rowIdx <= vLimitEnds[limIdx+1]; rowIdx++)
           {
               FIGARO_LOG_DBG("FOR", "limIdx", limIdx, "rowIdx", rowIdx);
               // For now we assume only one column is added.
               // add non primary keys
               aggregateByAttribute[limIdx] += m_data(rowIdx, nonPKAttrIdx);   
           } 
        }
        FIGARO_LOG_DBG("Successfull head computation");
    }

    void Relation::joinRelation(const Relation& relation, 
        const std::vector<std::tuple<std::string, std::string> >& vJoinAttributeNames)
    {
        // TODO: Update a number of cols depending on number of nonPK cols in relation.  
        std::unordered_map<double, double> hashTableVals;
        uint32_t attrIdx1;
        uint32_t attrIdx2;
        uint32_t numAttributes1;
        uint32_t numAttributes2;
        const auto& joinAttributeName = vJoinAttributeNames.at(0);

        attrIdx1 = getAttributeIdx(std::get<0>(joinAttributeName));
        attrIdx2 = relation.getAttributeIdx(std::get<1>(joinAttributeName));
        
        numAttributes1 = m_attributes.size();
        numAttributes2 = relation.m_attributes.size();
        for (uint32_t rowIdx = 0; rowIdx < relation.m_data.rows(); rowIdx ++)
        {
            // TODO: Remove hardcoding. 
            hashTableVals[relation.m_data(rowIdx, attrIdx1)] = relation.m_data(rowIdx, numAttributes2-1);
        }
        m_data.conservativeResize(Eigen::NoChange_t::NoChange, m_data.cols() + 1);
        for (uint32_t rowIdx = 0; rowIdx < m_data.rows(); rowIdx++)
        {
            // TODO: Clean this.
            double PKAttrVal = m_data(rowIdx, attrIdx1);
            m_data(rowIdx, m_data.cols() - 1) = hashTableVals[PKAttrVal];
        }
        FIGARO_LOG_DBG("Relation join", m_name, relation.m_name);
        printMatrix(std::cout, m_data);
    }

    void Relation::computeTail(const std::string& attrName)
    {

    }

}