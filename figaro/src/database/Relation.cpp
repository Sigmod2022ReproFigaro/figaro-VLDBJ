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
        FIGARO_LOG_ERROR("Index out of bounds for name", attributeName);
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
        FIGARO_LOG_DBG("m_data", m_data);
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
        // TODO: Think about optimizations. 
        for (uint32_t colIdx = 0; colIdx < m_data.cols(); colIdx++)
        {
            for (uint32_t rowIdx = 0; rowIdx < m_data.rows(); rowIdx++)
            {
                m_data(rowIdx, colIdx) = m_dataVectorOfVectors[rowIdx][colIdx];
            }
        }
        FIGARO_LOG_DBG("Relation: ", m_name);
        FIGARO_LOG_DBG(m_dataVectorOfVectors);
    }

    void Relation::sortData(void)
    {
        std::vector<std::string> vAttrNamesPKs;
        getAttributeNamesOfAttributePKs(vAttrNamesPKs);
        FIGARO_LOG_DBG(vAttrNamesPKs);
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

    uint32_t Relation::getDistinctValuesCount(const std::string& attributeName) const
    {
        double prevAttrVal;
        uint32_t distCnt; 
        uint32_t attrIdx;
        
        attrIdx = getAttributeIdx(attributeName);
        prevAttrVal = std::numeric_limits<double>::max();
        distCnt = 0;

        for (uint32_t rowIdx = 0; rowIdx < m_data.rows(); rowIdx++)
        {
            double curAttrVal = m_data(rowIdx, attrIdx);
            if (prevAttrVal != curAttrVal)
            {
                distCnt++;
                prevAttrVal = curAttrVal;
            }
        } 
        return distCnt;   
    }

    // TODO: Asumes order. Remove this assumpiton.
    void Relation::getAttributeDistinctValues(
        const std::string& attrName, 
        std::vector<double>& vDistinctVals) const
    {
        double prevAttrVal;
        uint32_t distCnt;
        uint32_t attrIdx;

        distCnt = 0;
        attrIdx = getAttributeIdx(attrName);
        prevAttrVal = std::numeric_limits<double>::max();
        // The first entry is saved for the end of imaginary predecessor of ranges. 
        for (uint32_t rowIdx = 0; rowIdx < m_data.rows(); rowIdx++)
        {
            double curAttrVal = m_data(rowIdx, attrIdx);
            if (prevAttrVal != curAttrVal)
            {
                vDistinctVals[distCnt] = curAttrVal;
                prevAttrVal = curAttrVal;
                distCnt++;
            }
        } 
    }

    void Relation::getAttributeValuesCountAggregates(
        const std::string& attrName, 
        std::unordered_map<double, uint32_t>& htCnts) const
    {
        uint32_t attrIdx; 
        FIGARO_LOG_DBG("Count Aggregates for attribute", attrName)
        attrIdx = getAttributeIdx(attrName);
        for (uint32_t rowIdx = 0; rowIdx < m_data.rows(); rowIdx++)
        {
            double curAttrVal = m_data(rowIdx, attrIdx);
            if (htCnts.find(curAttrVal) != htCnts.end())
            {
                htCnts[curAttrVal] ++; 
            }
            else
            {
                htCnts[curAttrVal] = 1;
            }
        }
    }
    void Relation::getDistinctValuesRowPositions(
        const std::string& attributeName,
        std::vector<uint32_t>& vDistinctValuesRowPositions) const
    {
        uint32_t attrIdx; 
        double prevAttrVal;
        uint32_t distCnt; 

        attrIdx = getAttributeIdx(attributeName);
        prevAttrVal = std::numeric_limits<double>::max();
        distCnt = 0; 

        // The first entry is saved for the end of imaginary predecessor of ranges. 
        for (uint32_t rowIdx = 0; rowIdx < m_data.rows(); rowIdx++)
        {
            double curAttrVal = m_data(rowIdx, attrIdx);
            if (prevAttrVal != curAttrVal)
            {
                vDistinctValuesRowPositions[distCnt] = rowIdx - 1;
                prevAttrVal = curAttrVal;
                distCnt++;
            }
        } 
        FIGARO_LOG_DBG("distCnt", distCnt);
        vDistinctValuesRowPositions[distCnt] = m_data.rows() - 1;
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

    void Relation::getNonPKAttributeIdx(std::vector<uint32_t>& nonPkAttrIdxs) const
    {
        for (uint32_t idx = 0; idx < m_attributes.size(); idx++)
        {
            if (!m_attributes[idx].m_isPrimaryKey)
            {
                nonPkAttrIdxs.push_back(idx);
            }
        }
    }

    void Relation::computeHead(const std::string& attributeName)
    {
        uint32_t attrIdx;
        std::uint32_t nonPKAttrIdx;
        double prevAttrVal;
        uint32_t numDistinctValues;

        std::vector<uint32_t> vnonPKAttrIdxs;
        std::vector<double> aggregateByAttribute;
        std::vector<double> vDistinctVals; 
        std::vector<uint32_t> vLimitEnds; 

        attrIdx = getAttributeIdx(attributeName);
        getNonPKAttributeIdx(vnonPKAttrIdxs); 
        // TODO: Clean hardcoding. 
        nonPKAttrIdx = vnonPKAttrIdxs[0];
        prevAttrVal = std::numeric_limits<double>::max();
        numDistinctValues = countUnique(m_data, attrIdx);
        
        aggregateByAttribute.resize(numDistinctValues);
        vDistinctVals.resize(numDistinctValues);
        vLimitEnds.resize(numDistinctValues + 1);
        storeUniqueAndLimits(m_data, attrIdx, vDistinctVals, vLimitEnds);
        FIGARO_LOG_DBG("RelatioN", m_name, "Attribute", attributeName)
        FIGARO_LOG_DBG("vDistinctVals", vDistinctVals);
        FIGARO_LOG_DBG("vLimitEnds", vLimitEnds);
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
        FIGARO_LOG_DBG("Successful head computation");
    }

    void Relation::joinRelation(const Relation& relation, 
        const std::vector<std::tuple<std::string, std::string> >& vJoinAttributeNames)
    {
        // TODO: Update a number of cols depending on number of nonPK cols in relation.  
        std::unordered_map<double, double> hashTableVals;
        uint32_t joinAttrIdx1;
        uint32_t joinAttrIdx2;
        uint32_t numAttributes1;
        uint32_t numAttributes2;
        const auto& joinAttributeName = vJoinAttributeNames.at(0);

        joinAttrIdx1 = getAttributeIdx(std::get<0>(joinAttributeName));
        joinAttrIdx2 = relation.getAttributeIdx(std::get<1>(joinAttributeName));
        
        numAttributes1 = m_attributes.size();
        numAttributes2 = relation.m_attributes.size();
        FIGARO_LOG_DBG("Attribute Idxs", joinAttrIdx1, joinAttrIdx2);

        for (uint32_t rowIdx = 0; rowIdx < relation.m_data.rows(); rowIdx ++)
        {
            // TODO: Remove hardcoding. 
            FIGARO_LOG_DBG("Second relation data", relation.m_data(rowIdx, joinAttrIdx1), relation.m_data(rowIdx, numAttributes2-1));
            hashTableVals[relation.m_data(rowIdx, joinAttrIdx2)] = relation.m_data(rowIdx, numAttributes2-1);
        }
        m_data.conservativeResize(Eigen::NoChange_t::NoChange, m_data.cols() + 1);
        
        for (uint32_t rowIdx = 0; rowIdx < m_data.rows(); rowIdx++)
        {
            // TODO: Clean this.
            m_dataVectorOfVectors[rowIdx].resize(m_data.cols());
            double joinAttrVal = m_data(rowIdx, joinAttrIdx1);
            double nonjoinAttrVal = hashTableVals[joinAttrVal]; 
            m_data(rowIdx, m_data.cols() - 1) = nonjoinAttrVal;
            m_dataVectorOfVectors[rowIdx][m_data.cols()-1] = nonjoinAttrVal;
        }
        FIGARO_LOG_DBG("Relation join", m_name, relation.m_name);
        FIGARO_LOG_DBG("m_data", m_data);
        m_attributes.push_back(relation.m_attributes.back());
    }

    void Relation::computeTail(const std::string& attrName)
    {

    }
    // TODO: Pass vectors. Now we assume the vector is all ones. 
    void Relation::computeAndScaleGeneralizedHeadAndTail(
        const std::string& attributeName,
        const std::unordered_map<double, uint32_t>& hashTabAttributeCounts
        )
    {
        uint32_t distinctValuesCounter;
        sortData({attributeName});
        std::vector<double> vDistinctValues;
        std::vector<uint32_t> vDistinctValuesRowPositions; 
        std::unordered_map<double, uint32_t> htCnts;
        std::vector<uint32_t> vnonPKAttrIdxs;

        distinctValuesCounter = getDistinctValuesCount(attributeName);
        vDistinctValues.resize(distinctValuesCounter);
        vDistinctValuesRowPositions.resize(distinctValuesCounter + 1);
        getNonPKAttributeIdx(vnonPKAttrIdxs);

        getAttributeDistinctValues(attributeName, vDistinctValues);
        getDistinctValuesRowPositions(attributeName, vDistinctValuesRowPositions);
        //getAttributeValuesCountAggregates(attributeName, htCnts);
        
        for (uint32_t distCnt = 0; distCnt < distinctValuesCounter; distCnt++)
        {
            uint32_t headRowIdx = vDistinctValuesRowPositions[distCnt] + 1;
            uint32_t aggregateCnt = vDistinctValuesRowPositions[distCnt + 1] 
                                  - vDistinctValuesRowPositions[distCnt]; 
            std::vector<double> vCurRowSum(vnonPKAttrIdxs.size()); 
            double attrVal = vDistinctValues[distCnt];
            double scalarCnt = hashTabAttributeCounts.at(attrVal);

            for (uint32_t rowIdx = headRowIdx;
                rowIdx <= vDistinctValuesRowPositions[distCnt + 1]; rowIdx ++)
            {
                double i = rowIdx - headRowIdx + 1;
                if (i > 1)
                {
                    for (const uint32_t nonPKAttrIdx: vnonPKAttrIdxs)
                    {
                        m_dataVectorOfVectors[rowIdx][nonPKAttrIdx] = 
                            (m_dataVectorOfVectors[rowIdx][nonPKAttrIdx] * (i - 1) - 
                            vCurRowSum[nonPKAttrIdx]) * scalarCnt / std::sqrt(i * (i - 1) );
                    }
                }
                for (const uint32_t nonPKAttrIdx: vnonPKAttrIdxs)
                {
                    vCurRowSum[nonPKAttrIdx] += m_dataVectorOfVectors[rowIdx][nonPKAttrIdx];
                }
                FIGARO_LOG_DBG(m_dataVectorOfVectors);
            }
            for (const uint32_t nonPKAttrIdx: vnonPKAttrIdxs)
            {
                m_dataVectorOfVectors[headRowIdx][nonPKAttrIdx] = 
                    vCurRowSum[nonPKAttrIdx] * scalarCnt/ std::sqrt(aggregateCnt);
            }
        }
    }

}