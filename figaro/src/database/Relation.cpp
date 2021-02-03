#include "database/Relation.h"
#include <string>
#include <sstream>
#include <fstream>
#include <execution>
#include <algorithm>
#include <limits>
#include <unordered_map>
#include "utils/Performance.h"
#include <omp.h>
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

    uint32_t Relation::getNumberOfPKAttributes(void) const
    {
        uint32_t pKAttributes = 0;
        /*
        std::reduce(std::execution::par, m_attributes.begin(), 
                    m_attributes.end(), 0.0,
                     [const& auto sum, const& auto T2]()
                    {
                        return ; 
                    })
        */
       pKAttributes = std::accumulate(m_attributes.begin(), m_attributes.end(), 0.0,
                     [](int curCnt, const Attribute& nextAtr) 
                     {
                         return nextAtr.m_isPrimaryKey ? (curCnt+1) : curCnt;
                     });
        return pKAttributes;
    }

    uint32_t Relation::getNumberOfNonPKAttributes(void) const
    {
         uint32_t nokPKAttributes = 0;
        /*
        std::reduce(std::execution::par, m_attributes.begin(), 
                    m_attributes.end(), 0.0,
                     [const& auto sum, const& auto T2]()
                    {
                        return ; 
                    })
        */
       nokPKAttributes = std::accumulate(m_attributes.begin(), m_attributes.end(), 0.0,
                     [](int curCnt, const Attribute& nextAtr) 
                     {
                         return nextAtr.m_isPrimaryKey ? curCnt : (curCnt + 1);
                     });
        return nokPKAttributes;
    }

    void Relation::getPKAttributeNames(std::vector<std::string>& vAttributeNamesPKs) const
    {
        for (const auto& attribute: m_attributes)
        {
            if (attribute.m_isPrimaryKey)
            {
                vAttributeNamesPKs.push_back(attribute.m_name);
            }
        }
    }

    void Relation::getNonPKAttributeNames(std::vector<std::string>& vAttributeNamesNonPKs) const
    {
        for (const auto& attribute: m_attributes)
        {
            if (!attribute.m_isPrimaryKey)
            {
                vAttributeNamesNonPKs.push_back(attribute.m_name);
            }
        }
    }

    void Relation::getPKAttributeIndices(std::vector<uint32_t>& vPkAttrIdxs) const
    {
        for (uint32_t idx = 0; idx < m_attributes.size(); idx++)
        {
            if (m_attributes[idx].m_isPrimaryKey)
            {
                vPkAttrIdxs.push_back(idx);
            }
        }
    }   

    void Relation::getNonPKAttributeIdxs(std::vector<uint32_t>& vNonPkAttrIdxs) const
    {
        for (uint32_t idx = 0; idx < m_attributes.size(); idx++)
        {
            if (!m_attributes[idx].m_isPrimaryKey)
            {
                vNonPkAttrIdxs.push_back(idx);
            }
        }
    }

    void Relation::schemaJoin(const Relation& relation, bool swapAttributes)
    {
        std::vector<uint32_t> vNonPKAttributeIdxs;
        relation.getNonPKAttributeIdxs(vNonPKAttributeIdxs);
        std::vector<Attribute> tmpV;
        
        if (swapAttributes)
        {
            for (const auto& attribute: m_attributes)
            {
                if (!attribute.m_isPrimaryKey)
                {
                    tmpV.push_back(attribute);
                }
            }
            while (!m_attributes.back().m_isPrimaryKey)
            {
                m_attributes.pop_back();
            }
        }

        for (const auto& m: tmpV)
        {
            FIGARO_LOG_DBG("tmpV", m.m_name)
        }

        for (const auto& m: m_attributes)
        {
            FIGARO_LOG_DBG("m_attributes", m.m_name)
        }

        for (const auto nonPKAttributeIdx: vNonPKAttributeIdxs)
        {
            FIGARO_LOG_DBG("Schema extended, Added attribute", nonPKAttributeIdx);
            m_attributes.push_back(relation.m_attributes[nonPKAttributeIdx]);
        }
        if (swapAttributes)
        {
            m_attributes.insert(std::end(m_attributes), std::begin(tmpV), std::end(tmpV));
        }

        //FIGARO_LOG_DBG("After schema change", *this);
    }

    Relation::Relation(json jsonRelationSchema): 
        m_data(0, 0), m_dataTails1(0, 0), m_dataTails2(0, 0), m_dataHead(0, 0)
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
        
        m_data = std::move(Matrix<double>(cntLines, numAttributes));
        

        // TODO: If there is time, write regex that will parse files based on the attributes type. 
        // Attributes in schema configuration correspond to the 
        // order of attributes in data representation.
        // TODO: Add header parsing in the case if we decide to add 
        // headers for attributes.   
        //
        for (uint32_t row = 0; row < cntLines; row ++)
        {
            std::getline(fileTable, str);
            strStream.clear();
            strStream.str(str);
            
            
            for (uint32_t col = 0; col < numAttributes; col ++)
            {
                std::getline(strStream, strVal, DELIMITER);
                double val = std::stod(strVal);
                m_data[row][col] = val;
            }
        }
        //FIGARO_LOG_DBG("m_data", m_data);
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
        uint32_t numRows = m_data.getNumRows();
        uint32_t numCols = m_data.getNumCols();
        MatrixDT tmpMatrix(numRows, numCols);
        getAttributesIdxs(vAttributeNames, vAttributesIdxs);

        std::vector<double*> vRowPts(numRows);
        for (uint32_t rowIdx = 0; rowIdx < numRows; rowIdx++)
        {
            vRowPts[rowIdx] = m_data[rowIdx];
        }
        std::sort(std::execution::par_unseq, 
                  vRowPts.begin(),
                  vRowPts.end(), 
                  [&vAttributesIdxs]
                  (const double* row1, const double* row2)
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

        for (uint32_t rowIdx = 0; rowIdx < numRows; rowIdx++)
        {
            for (uint32_t colIdx = 0; colIdx < numCols; colIdx++)
            {
                tmpMatrix[rowIdx][colIdx] = vRowPts[rowIdx][colIdx];
            }
        }
        m_data = std::move(tmpMatrix);
        //FIGARO_LOG_DBG("Relation: ", m_name);
        //FIGARO_LOG_DBG(m_data);
    }

    void Relation::sortData(void)
    {
        std::vector<std::string> vAttrNamesPKs;
        getPKAttributeNames(vAttrNamesPKs);
        FIGARO_LOG_DBG("Sorted Attributes", vAttrNamesPKs);
        sortData(vAttrNamesPKs);
    }

    uint32_t Relation::getDistinctValuesCount(const std::string& attributeName) const
    {
        double prevAttrVal;
        uint32_t distCnt; 
        uint32_t attrIdx;
        
        attrIdx = getAttributeIdx(attributeName);
        prevAttrVal = std::numeric_limits<double>::max();
        distCnt = 0;

        for (uint32_t rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
        {
            double curAttrVal = m_data[rowIdx][attrIdx];
            if (prevAttrVal != curAttrVal)
            {
                distCnt++;
                prevAttrVal = curAttrVal;
            }
        } 
        return distCnt;   
    }

    // Asumes tuples (t1, t2) are ordered in such way that t1 < t2 
    // iff t1[attrName] < t2[attrName]
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
        for (uint32_t rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
        {
            double curAttrVal = m_data[rowIdx][attrIdx];
            if (prevAttrVal != curAttrVal)
            {
                vDistinctVals[distCnt] = curAttrVal;
                prevAttrVal = curAttrVal;
                distCnt++;
            }
        } 
    }

    void Relation::getAttributeValuesCounts(
        const std::string& attrName, 
        std::unordered_map<double, uint32_t>& htCnts) const
    {
        uint32_t attrIdx; 
        attrIdx = getAttributeIdx(attrName);
        htCnts.reserve(m_data.getNumRows());
        for (uint32_t rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
        {
            double curAttrVal = m_data[rowIdx][attrIdx];
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

    void Relation::getRowPtrs(
        const std::string& attrName,
        std::unordered_map<double, const double*>& htRowPts) const
    {
        uint32_t attrIdx; 
        attrIdx = getAttributeIdx(attrName);
        htRowPts.reserve(m_data.getNumRows());
        for (uint32_t rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
        {
            double curAttrVal = m_data[rowIdx][attrIdx];
            htRowPts[curAttrVal] = m_data[rowIdx];
        }
    }

    
    void Relation::getDistinctValuesRowPositions(
        const std::string& attributeName,
        std::vector<uint32_t>& vDistinctValuesRowPositions,
        bool  preallocated) const
    {
        uint32_t attrIdx; 
        double prevAttrVal;
        uint32_t distCnt;
        double pushVal; 

        attrIdx = getAttributeIdx(attributeName);
        prevAttrVal = std::numeric_limits<double>::max();
        distCnt = 0; 

        // The first entry is saved for the end of imaginary predecessor of ranges. 
        for (uint32_t rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
        {
            double curAttrVal = m_data[rowIdx][attrIdx];
            if (prevAttrVal != curAttrVal)
            {
                pushVal = rowIdx - 1;
                if (preallocated)
                {
                    vDistinctValuesRowPositions[distCnt] = pushVal;
                }
                else 
                {
                    vDistinctValuesRowPositions.push_back(pushVal);
                }
                prevAttrVal = curAttrVal;
                distCnt++;
            }
        } 
        pushVal = m_data.getNumRows() - 1;
        if (preallocated)
        {
            vDistinctValuesRowPositions[distCnt] = pushVal;
        }
        else
        {
            vDistinctValuesRowPositions.push_back(pushVal);
        }
    }

    static void storeUniqueAndLimits(MatrixEigenT mat, uint32_t colIdx, 
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
                vDistinctVals[distCnt] = curAttrVal;
                vLimitEnds[distCnt] = rowIdx - 1;
                prevAttrVal = curAttrVal;
                distCnt++;
            }
        } 
        vLimitEnds[distCnt] = mat.rows() - 1;
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
        getNonPKAttributeIdxs(vnonPKAttrIdxs); 
        // TODO: Clean hardcoding. 
        nonPKAttrIdx = vnonPKAttrIdxs[0];
        prevAttrVal = std::numeric_limits<double>::max();
        numDistinctValues = getDistinctValuesCount(attributeName);
        
        aggregateByAttribute.resize(numDistinctValues);
        vDistinctVals.resize(numDistinctValues);
        vLimitEnds.resize(numDistinctValues + 1);
        //storeUniqueAndLimits(m_data, attrIdx, vDistinctVals, vLimitEnds);
        //FIGARO_LOG_DBG("RelatioN", m_name, "Attribute", attributeName)
        //FIGARO_LOG_DBG("vDistinctVals", vDistinctVals);
        //FIGARO_LOG_DBG("vLimitEnds", vLimitEnds);
        for (uint32_t limIdx = 0; limIdx < vDistinctVals.size(); limIdx++)
        {
           for (uint32_t rowIdx = vLimitEnds[limIdx] + 1; rowIdx <= vLimitEnds[limIdx+1]; 
                rowIdx++)
           {
               //FIGARO_LOG_DBG("FOR", "limIdx", limIdx, "rowIdx", rowIdx);
               // For now we assume only one column is added.
               // add non primary keys
               //aggregateByAttribute[limIdx] += m_data(rowIdx, nonPKAttrIdx);   
           } 
        }
        //FIGARO_LOG_DBG("Successful head computation");
    }

    // This code joins 1-N relations, where the cardinality of this is 1 and 
    // the cardinality of relation is N.
    void Relation::joinRelation(
        const Relation& relation, 
        const std::vector<std::tuple<std::string, std::string> >& vJoinAttributeNames,
        bool bSwapAttributes)
    {
        std::unordered_map<double, const double*> hashTabRowPt2;
        uint32_t attrIdx;
        std::vector<uint32_t> vDistValRowPos1;
        std::vector<uint32_t> vPkAttrIdxs1;
        std::vector<uint32_t> vNonPkAttrIdxs1;
        std::vector<uint32_t> vNonPkAttrIdxs2;
        uint32_t numAttrs1;
        uint32_t numPKAttrs1;
        uint32_t numPKAttrs2;
        uint32_t numNonPkAttrs2;
        std::vector<std::string> vAttributeNamesNonPKs1;
        std::vector<std::string> vAttributeNamesNonPKs2;
        uint32_t rowIdx2;
        const auto& joinAttributeNameTup = vJoinAttributeNames.at(0);
        std::string joinAttrName =  std::get<0>(joinAttributeNameTup);

        numAttrs1 = m_attributes.size();
        numNonPkAttrs2 = relation.getNumberOfNonPKAttributes();
        numPKAttrs1 = getNumberOfPKAttributes();
        numPKAttrs2 = relation.getNumberOfPKAttributes();
        getNonPKAttributeNames(vAttributeNamesNonPKs1);
        relation.getNonPKAttributeNames(vAttributeNamesNonPKs2);

        FIGARO_LOG_DBG("First nonPkNames", vAttributeNamesNonPKs1);
        FIGARO_LOG_DBG("Second NonPK names", vAttributeNamesNonPKs2);


        getNonPKAttributeIdxs(vNonPkAttrIdxs1);
        getPKAttributeIndices(vPkAttrIdxs1);
        relation.getNonPKAttributeIdxs(vNonPkAttrIdxs2);
        getDistinctValuesRowPositions(joinAttrName, vDistValRowPos1, false);
        uint32_t pkOffset2 = relation.getNumberOfPKAttributes();
        attrIdx = getAttributeIdx(joinAttrName);
        relation.getRowPtrs(joinAttrName, hashTabRowPt2);

        schemaJoin(relation, bSwapAttributes);
        Matrix<double> dataOutput {m_data.getNumRows(), (uint32_t)m_attributes.size()};

       
       #pragma omp parallel for schedule(static)
       for (uint32_t rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
       {
            const double joinAttrVal = m_data[rowIdx][attrIdx];
            const double* rowPtr2 = hashTabRowPt2[joinAttrVal];

            // Copy Pks
            for (auto pkIdx: vPkAttrIdxs1)
            {
                dataOutput[rowIdx][pkIdx] = m_data[rowIdx][pkIdx];
            }
            // Copy non-pk attributes
            if (bSwapAttributes)
            {
                for (const auto nonPKAttrIdx1: vNonPkAttrIdxs1)
                {
                    dataOutput[rowIdx][nonPKAttrIdx1 + numNonPkAttrs2] = m_data[rowIdx][nonPKAttrIdx1];
                }
                for (const auto nonPKAttrIdx2: vNonPkAttrIdxs2)
                {
                    dataOutput[rowIdx][numPKAttrs1 - numPKAttrs2 + nonPKAttrIdx2] = (rowPtr2)[nonPKAttrIdx2];
                }
            }
            else 
            {
                for (const auto nonPKAttrIdx1: vNonPkAttrIdxs1)
                {
                    dataOutput[rowIdx][nonPKAttrIdx1] = m_data[rowIdx][nonPKAttrIdx1];
                }
                for (const auto nonPKAttrIdx2: vNonPkAttrIdxs2)
                {
                    dataOutput[rowIdx][numAttrs1 - numPKAttrs2 + nonPKAttrIdx2 ] = (rowPtr2)[nonPKAttrIdx2];
                }
            }
        }
        m_data = std::move(dataOutput);
        
        FIGARO_LOG_DBG("End of Join", *this)
    }

    // TODO: Pass vectors. Now we assume the vector is all ones. 
    void Relation::computeAndScaleGeneralizedHeadAndTail(
        const std::string& attributeName,
        const std::unordered_map<double, uint32_t>& hashTabAttributeCounts
        )
    {
        uint32_t distinctValuesCounter;
        std::vector<double> vDistinctValues;
        std::vector<uint32_t> vDistinctValuesRowPositions; 
        std::vector<uint32_t> vnonPKAttrIdxs;
        uint32_t pkOffset;

        distinctValuesCounter = getDistinctValuesCount(attributeName);
        vDistinctValues.resize(distinctValuesCounter);
        vDistinctValuesRowPositions.resize(distinctValuesCounter + 1);
        getNonPKAttributeIdxs(vnonPKAttrIdxs);
        pkOffset = m_attributes.size() - vnonPKAttrIdxs.size();

        getAttributeDistinctValues(attributeName, vDistinctValues);
        getDistinctValuesRowPositions(attributeName, vDistinctValuesRowPositions);
        
        
        
        #pragma omp parallel for schedule(static)
        for (uint32_t distCnt = 0; distCnt < distinctValuesCounter; distCnt++)
        {
            uint32_t headRowIdx = vDistinctValuesRowPositions[distCnt] + 1;
            uint32_t aggregateCnt = vDistinctValuesRowPositions[distCnt + 1] 
                                  - vDistinctValuesRowPositions[distCnt]; 
            std::vector<double> vCurRowSum(vnonPKAttrIdxs.size()); 
            double attrVal = vDistinctValues[distCnt];
            double scalarCnt = hashTabAttributeCounts.at(attrVal);

            for (const uint32_t nonPKAttrIdx: vnonPKAttrIdxs)
            {
                vCurRowSum[nonPKAttrIdx - pkOffset] = m_data[headRowIdx][nonPKAttrIdx];
            }

            for (uint32_t rowIdx = headRowIdx + 1;
                rowIdx <= vDistinctValuesRowPositions[distCnt + 1]; rowIdx ++)
            {
                double i = rowIdx - headRowIdx + 1;
                for (const uint32_t nonPKAttrIdx: vnonPKAttrIdxs)
                {
                    double prevRowSum = vCurRowSum[nonPKAttrIdx - pkOffset];
                    vCurRowSum[nonPKAttrIdx - pkOffset] += m_data[rowIdx][nonPKAttrIdx];
                    m_data[rowIdx][nonPKAttrIdx] = 
                        (m_data[rowIdx][nonPKAttrIdx] * (i - 1) - 
                        prevRowSum) * std::sqrt(scalarCnt / (i * (i - 1)) );
                }
            }

            for (const uint32_t nonPKAttrIdx: vnonPKAttrIdxs)
            {
                m_data[headRowIdx][nonPKAttrIdx] = 
                    vCurRowSum[nonPKAttrIdx - pkOffset] * std::sqrt(scalarCnt / aggregateCnt);
            }
        }
        //FIGARO_LOG_INFO(*this);
    }
    
    // N-N join where zero rows are omitted.
    void Relation::extend(const Relation& relation, const std::string& attrIterName)
    {
        constexpr uint32_t NUM_RELS = 2; 
        std::array<const Relation*, NUM_RELS> apRelations
        { this, &relation};
        std::array<std::vector<uint32_t>, NUM_RELS> avDistinctValuesRowPositions;
        std::vector<uint32_t> vNonPkAttrIdxs;
        std::vector<uint32_t> vNonPkAttrIdxsRel;
        uint32_t distValCnt;
        uint32_t addRowsCnt;
        uint32_t colOffset; 
        uint32_t numPKAttrs;
        uint32_t numPKAttrsRel;
        uint32_t numNonPkAttrs;
        uint32_t numHeads;
        uint32_t numTails1;
        uint32_t numTails2;

        
        colOffset = m_attributes.size();
        numNonPkAttrs = getNumberOfNonPKAttributes();
        numPKAttrsRel  = relation.getNumberOfPKAttributes();
        numPKAttrs = getNumberOfPKAttributes();
        getNonPKAttributeIdxs(vNonPkAttrIdxs);
        relation.getNonPKAttributeIdxs(vNonPkAttrIdxsRel);
        
        for (uint32_t idxRel = 0; idxRel < apRelations.size(); idxRel ++)
        {
            apRelations[idxRel]->getDistinctValuesRowPositions(
                attrIterName, avDistinctValuesRowPositions[idxRel], false);
        }
        distValCnt = avDistinctValuesRowPositions[1].size() - 1; 
        addRowsCnt = relation.m_data.getNumRows() - distValCnt; 

        numHeads = distValCnt;
        // By definition there are no dangling tuples. 
        numTails1 = m_data.getNumRows() - distValCnt;             
        numTails2 = relation.m_data.getNumRows() - distValCnt;
        
        MatrixDT headOutput(numHeads, vNonPkAttrIdxs.size() + vNonPkAttrIdxsRel.size());
        MatrixDT tailOutput1(numTails1, getNumberOfNonPKAttributes());
        MatrixDT tailOutput2(numTails2, relation.getNumberOfNonPKAttributes());
        schemaJoin(relation);
        

        #pragma omp parallel for schedule(static)
        for (uint32_t distCnt = 0; distCnt < distValCnt; distCnt ++)
        {
            std::array<uint32_t, NUM_RELS> aHeadRowIdx;
            std::array<uint32_t, NUM_RELS> aTailIdxs;
            uint32_t precNextRowIdx;

            for (uint32_t idx = 0; idx < NUM_RELS; idx++)
            {
                aHeadRowIdx[idx] = avDistinctValuesRowPositions[idx][distCnt] + 1;
                aTailIdxs[idx] = aHeadRowIdx[idx] - distCnt;
                //FIGARO_LOG_DBG("aHeadRowIdx", aHeadRowIdx)
                //FIGARO_LOG_DBG("distCnt", distCnt)
                //FIGARO_LOG_DBG("aTailIdxs", aTailIdxs[idx])
            }
            
            // Horizontally concatenates heads. 
            // 
            for (const auto  nonPKAttrIdx: vNonPkAttrIdxs)
            {
                uint32_t colOffsetRel = nonPKAttrIdx - numPKAttrs;
                headOutput[distCnt][colOffsetRel] = m_data[aHeadRowIdx[0]][ nonPKAttrIdx];
            }
            for (const auto  nonPKAttrIdx: vNonPkAttrIdxsRel)
            {
                uint32_t colOffsetRel = nonPKAttrIdx - numPKAttrsRel;
                headOutput[distCnt][numNonPkAttrs + colOffsetRel]
                = relation.m_data[aHeadRowIdx[1]][nonPKAttrIdx];
            }

            // Copies tails to the end. 
            precNextRowIdx = avDistinctValuesRowPositions[0][distCnt+1];
            for (uint32_t rowIdx = aHeadRowIdx[0] + 1; rowIdx <= precNextRowIdx; rowIdx++ )
            {
                for (const auto  nonPKAttrIdx: vNonPkAttrIdxs)
                {
                    uint32_t colOffset1 = nonPKAttrIdx - numPKAttrs;
                    tailOutput1[aTailIdxs[0]][colOffset1]
                    = m_data[rowIdx][nonPKAttrIdx];
                }
                aTailIdxs[0]++;
            }

            precNextRowIdx = avDistinctValuesRowPositions[1][distCnt+1];
            for (uint32_t rowIdx = aHeadRowIdx[1] + 1; rowIdx <= precNextRowIdx; rowIdx++ )
            {
                for (const auto  nonPKAttrIdx: vNonPkAttrIdxsRel)
                {
                    uint32_t colOffset2 = nonPKAttrIdx - numPKAttrsRel;
                    tailOutput2[aTailIdxs[1]][colOffset2]
                    = relation.m_data[rowIdx][nonPKAttrIdx];
                }
                aTailIdxs[1]++;
            }
        }
        FIGARO_LOG_INFO("After extend", *this);
        m_dataHead = std::move(headOutput);
        m_dataTails1 = std::move(tailOutput1);
        m_dataTails2 = std::move(tailOutput2);
    }

    static void makeDiagonalElementsPositiveInR(MatrixEigenT& matR)
    {
        ArrayT&& aDiag = matR.diagonal().array().sign();
        for (uint32_t rowIdx = 0; rowIdx < matR.cols(); rowIdx ++)
        {
            matR.row(rowIdx) *= aDiag(rowIdx);
        }
    }

    static void copyMatrixToEigenMatrix(const Figaro::Relation::MatrixDT& mOur, MatrixEigenT& mEig)
    {
        mEig.resize(mOur.getNumRows(), mOur.getNumCols());
        for (uint32_t rowIdx = 0; rowIdx < mOur.getNumRows(); rowIdx++)
        {
            for (uint32_t colIdx = 0; colIdx < mOur.getNumCols(); colIdx++)
            {
                mEig(rowIdx, colIdx) = mOur[rowIdx][colIdx];
            }
        }
    }

    void Relation::applyEigenQR(MatrixEigenT* pR)
    {
        MICRO_BENCH_INIT(timer);
        uint32_t numNonPKAttributes = getNumberOfNonPKAttributes();
        Eigen::HouseholderQR<MatrixEigenT> qr{};
        MatrixEigenT matEigen;

        FIGARO_LOG_DBG("m_dataHead", m_dataHead)
        FIGARO_LOG_DBG("m_dataTails1", m_dataTails1)
        FIGARO_LOG_DBG("m_dataTails2", m_dataTails2)
        MICRO_BENCH_START(timer);
        
        // Tries to resize where the number of columns is much bigger than the number of rows.
        // This increases size and causes all sorts of problems. 
        if (m_dataHead.getNumCols() < m_dataHead.getNumRows())
        {
            m_dataHead.computeQRGivens();
            m_dataHead.resize(m_dataHead.getNumCols());
        }
        if (m_dataTails1.getNumCols() < m_dataTails1.getNumRows())
        {
            m_dataTails1.computeQRGivens();
            m_dataTails1.resize(m_dataTails1.getNumCols()); 

        }
        if (m_dataTails2.getNumCols() < m_dataTails2.getNumRows())
        {
            m_dataTails2.computeQRGivens();
            m_dataTails2.resize(m_dataTails2.getNumCols());

        }
        MICRO_BENCH_STOP(timer);
        FIGARO_LOG_BENCH("Figaro", "main", "computeQRDecompositionHouseholder", "computeQRGivens", MICRO_BENCH_GET_TIMER_LAP(timer));

        MICRO_BENCH_START(timer);
        m_dataTails1 = m_dataTails1.concatenateHorizontallyScalar(0, m_dataHead.getNumCols() - m_dataTails1.getNumCols());
        m_dataTails2 = Matrix<double>::zeros(m_dataTails2.getNumRows(), m_dataHead.getNumCols() - m_dataTails2.getNumCols()).concatenateHorizontally(m_dataTails2);
        auto&& m_dataFull = m_dataHead.concatenateVertically(m_dataTails1).concatenateVertically(m_dataTails2);
        MICRO_BENCH_STOP(timer);
        FIGARO_LOG_BENCH("Figaro", "main", "computeQRDecompositionHouseholder", "concatenate", MICRO_BENCH_GET_TIMER_LAP(timer));

        MICRO_BENCH_START(timer);
        copyMatrixToEigenMatrix(m_dataFull, matEigen);
        MICRO_BENCH_STOP(timer);
        FIGARO_LOG_BENCH("Figaro", "main", "computeQRDecompositionHouseholder", "copying", MICRO_BENCH_GET_TIMER_LAP(timer));

        // TODO: think how to avoid copy constructor. 
        MICRO_BENCH_START(timer);
        qr.compute(matEigen);
        MICRO_BENCH_STOP(timer);
        FIGARO_LOG_BENCH("Figaro", "main", "computeQRDecompositionHouseholder", "Householder reduced", MICRO_BENCH_GET_TIMER_LAP(timer));
        MICRO_BENCH_START(timer);
        if (nullptr != pR)
        {
            *pR = qr.matrixQR().topLeftCorner(numNonPKAttributes, numNonPKAttributes).triangularView<Eigen::Upper>();
            makeDiagonalElementsPositiveInR(*pR);
        }
        MICRO_BENCH_STOP(timer);
        FIGARO_LOG_BENCH("Figaro", "main", "computeQRDecompositionHouseholder", "extracting data", MICRO_BENCH_GET_TIMER_LAP(timer));
        FIGARO_LOG_BENCH("Figaro", "main", "computeQRDecompositionHouseholder", "total", MICRO_BENCH_GET_TIMER(timer));
    }    

    std::ostream& operator<<(std::ostream& out, const Relation& relation)
    {
        out << std::endl;
        out << "Relation " << relation.m_name << std::endl;
        out << "[ ";
        for (const auto& attribute: relation.m_attributes)
        {
            std::string strPk = attribute.m_isPrimaryKey ? "PK, " : "";
            std::string strType = attribute.mapTypeToStr.at(attribute.m_type);
            out << attribute.m_name  << " (" << strPk  
                << strType << ")" << "; ";
        }
        out << "]" << std::endl;
        out << relation.m_data;
        return out;
    }
}