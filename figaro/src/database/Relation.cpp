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
    void Relation::copyVectorOfVectorsToEigenData(void)
    {
        MICRO_BENCH_INIT(copy);
        MICRO_BENCH_START(copy);
        for (uint32_t row = 0; row < m_dataVectorOfVectors.getNumRows(); row ++)
        {
            for (uint32_t col = 0; col < m_attributes.size(); col ++)
            {
                m_data(row, col) =  m_dataVectorOfVectors[row][col]; 
            }
        }
        MICRO_BENCH_STOP(copy);
        FIGARO_LOG_BENCH("Figaro", "main", "copyVectorOfVectorsToEigenData", MICRO_BENCH_GET_TIMER(copy));

    }

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

        FIGARO_LOG_DBG("After schema change", *this);
    }

    Relation::Relation(json jsonRelationSchema): m_dataVectorOfVectors(0, MAX_NUM_COLS), m_dataTails1(0, MAX_NUM_COLS), m_dataTails2(0, MAX_NUM_COLS),
    m_dataHead(0, MAX_NUM_COLS)
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
        //for (auto& row: m_dataVectorOfVectors)
        //{
            //row.resize(numAttributes);
        //}
        

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
        uint32_t numRows = m_dataVectorOfVectors.getNumRows();
        uint32_t numCols = m_dataVectorOfVectors.getNumCols();
        VectorOfVectorsT tmpMatrix(numRows, numCols);
        getAttributesIdxs(vAttributeNames, vAttributesIdxs);

        std::vector<double*> vRowPts(numRows);
        for (uint32_t rowIdx = 0; rowIdx < numRows; rowIdx++)
        {
            vRowPts[rowIdx] = m_dataVectorOfVectors[rowIdx];
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
        FIGARO_LOG_DBG("tmpMatrix", tmpMatrix);
        FIGARO_LOG_DBG("m_dataVectorOfVectors", m_dataVectorOfVectors);
        m_dataVectorOfVectors = std::move(tmpMatrix);
        FIGARO_LOG_DBG("tmpMatrix", tmpMatrix);
        FIGARO_LOG_DBG("Relation: ", m_name);
        FIGARO_LOG_DBG(m_dataVectorOfVectors);
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

        for (uint32_t rowIdx = 0; rowIdx < m_dataVectorOfVectors.getNumRows(); rowIdx++)
        {
            double curAttrVal = m_dataVectorOfVectors[rowIdx][attrIdx];
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
        for (uint32_t rowIdx = 0; rowIdx < m_dataVectorOfVectors.getNumRows(); rowIdx++)
        {
            double curAttrVal = m_dataVectorOfVectors[rowIdx][attrIdx];
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
        htCnts.reserve(m_dataVectorOfVectors.getNumRows());
        for (uint32_t rowIdx = 0; rowIdx < m_dataVectorOfVectors.getNumRows(); rowIdx++)
        {
            double curAttrVal = m_dataVectorOfVectors[rowIdx][attrIdx];
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
        htRowPts.reserve(m_dataVectorOfVectors.getNumRows());
        for (uint32_t rowIdx = 0; rowIdx < m_dataVectorOfVectors.getNumRows(); rowIdx++)
        {
            double curAttrVal = m_dataVectorOfVectors[rowIdx][attrIdx];
            htRowPts[curAttrVal] = m_dataVectorOfVectors[rowIdx];
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
        for (uint32_t rowIdx = 0; rowIdx < m_dataVectorOfVectors.getNumRows(); rowIdx++)
        {
            double curAttrVal = m_dataVectorOfVectors[rowIdx][attrIdx];
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
        pushVal = m_dataVectorOfVectors.getNumRows() - 1;
        if (preallocated)
        {
            vDistinctValuesRowPositions[distCnt] = pushVal;
        }
        else
        {
            vDistinctValuesRowPositions.push_back(pushVal);
        }
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
        uint32_t numAttrs2;
        uint32_t numPKAttrs1;
        uint32_t numPKAttrs2;
        uint32_t numNonPkAttrs2;
        std::vector<std::string> vAttributeNamesNonPKs1;
        std::vector<std::string> vAttributeNamesNonPKs2;
        uint32_t rowIdx2;
        const auto& joinAttributeNameTup = vJoinAttributeNames.at(0);
        std::string joinAttrName =  std::get<0>(joinAttributeNameTup);

        numAttrs1 = m_attributes.size();
        numAttrs2 = relation.m_attributes.size();
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
        Matrix<double> dataOutput {m_dataVectorOfVectors.getNumRows(), m_attributes.size()};
        m_data.conservativeResize(Eigen::NoChange_t::NoChange, m_attributes.size());

       
       #pragma omp parallel for schedule(static)
       for (uint32_t rowIdx = 0; rowIdx < m_dataVectorOfVectors.getNumRows(); rowIdx++)
       {
            const double joinAttrVal = m_dataVectorOfVectors[rowIdx][attrIdx];
            const double* rowPtr2 = hashTabRowPt2[joinAttrVal];

            // Copy Pks
            for (auto pkIdx: vPkAttrIdxs1)
            {
                dataOutput[rowIdx][pkIdx] = m_dataVectorOfVectors[rowIdx][pkIdx];
            }
            // Copy non-pk attributes
            if (bSwapAttributes)
            {
                for (const auto nonPKAttrIdx1: vNonPkAttrIdxs1)
                {
                    dataOutput[rowIdx][nonPKAttrIdx1 + numNonPkAttrs2] = m_dataVectorOfVectors[rowIdx][nonPKAttrIdx1];
                    FIGARO_LOG_DBG("attrIdx1 + numNonPkAttrs2", nonPKAttrIdx1 + numNonPkAttrs2, "attrIdx1", nonPKAttrIdx1)
                }
                for (const auto nonPKAttrIdx2: vNonPkAttrIdxs2)
                {
                    dataOutput[rowIdx][numPKAttrs1 - numPKAttrs2 + nonPKAttrIdx2] = (rowPtr2)[nonPKAttrIdx2];
                    FIGARO_LOG_DBG("numPKAttrs1", numPKAttrs1, "numPKAttrs2", numPKAttrs2, "numPKAttrs1 - numAttrs2 + nonPKAttrIdx2", numPKAttrs1 - numPKAttrs2 + nonPKAttrIdx2, "nonPKAttrIdx2", nonPKAttrIdx2)
                }
            }
            else 
            {
                for (const auto nonPKAttrIdx1: vNonPkAttrIdxs1)
                {
                    dataOutput[rowIdx][nonPKAttrIdx1] = m_dataVectorOfVectors[rowIdx][nonPKAttrIdx1];
                }
                for (const auto nonPKAttrIdx2: vNonPkAttrIdxs2)
                {
                    dataOutput[rowIdx][numAttrs1 + nonPKAttrIdx2 - numPKAttrs2] = (rowPtr2)[nonPKAttrIdx2];
                }
            }
        }
        m_dataVectorOfVectors = std::move(dataOutput);
        
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

        MICRO_BENCH_INIT(aggregates)


        MICRO_BENCH_START(aggregates)
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
                vCurRowSum[nonPKAttrIdx - pkOffset] = m_dataVectorOfVectors[headRowIdx][nonPKAttrIdx];
            }

            for (uint32_t rowIdx = headRowIdx + 1;
                rowIdx <= vDistinctValuesRowPositions[distCnt + 1]; rowIdx ++)
            {
                double i = rowIdx - headRowIdx + 1;
                for (const uint32_t nonPKAttrIdx: vnonPKAttrIdxs)
                {
                    double prevRowSum = vCurRowSum[nonPKAttrIdx - pkOffset];
                    vCurRowSum[nonPKAttrIdx - pkOffset] += m_dataVectorOfVectors[rowIdx][nonPKAttrIdx];
                    m_dataVectorOfVectors[rowIdx][nonPKAttrIdx] = 
                        (m_dataVectorOfVectors[rowIdx][nonPKAttrIdx] * (i - 1) - 
                        prevRowSum) * std::sqrt(scalarCnt / (i * (i - 1)) );
                }
            }

            for (const uint32_t nonPKAttrIdx: vnonPKAttrIdxs)
            {
                m_dataVectorOfVectors[headRowIdx][nonPKAttrIdx] = 
                    vCurRowSum[nonPKAttrIdx - pkOffset] * std::sqrt(scalarCnt / aggregateCnt);
            }
        }
        MICRO_BENCH_STOP(aggregates)
         FIGARO_LOG_BENCH("Figaro", "main", "aggregates", MICRO_BENCH_GET_TIMER(aggregates));
        copyVectorOfVectorsToEigenData();
        FIGARO_LOG_INFO(*this);
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
        uint32_t numPKAttributesRel;
        uint32_t numNonPkAttrs;
        uint32_t appIdx;
        uint32_t tailIdx1 = 0;
        uint32_t tailIdx2 = 0;
        uint32_t numHeads;
        uint32_t numTails1;
        uint32_t numTails2;
        uint32_t rightNonPKs;

        
        colOffset = m_attributes.size();
        numNonPkAttrs = getNumberOfNonPKAttributes();
        numPKAttributesRel  = relation.getNumberOfPKAttributes();
        numPKAttrs = getNumberOfPKAttributes();
        getNonPKAttributeIdxs(vNonPkAttrIdxs);
        relation.getNonPKAttributeIdxs(vNonPkAttrIdxsRel);
        appIdx = m_dataVectorOfVectors.getNumRows();
        rightNonPKs = relation.getNumberOfNonPKAttributes();
        
        for (uint32_t idxRel = 0; idxRel < apRelations.size(); idxRel ++)
        {
            apRelations[idxRel]->getDistinctValuesRowPositions(
                attrIterName, avDistinctValuesRowPositions[idxRel], false);
        }
        distValCnt = avDistinctValuesRowPositions[1].size() - 1; 
        addRowsCnt = relation.m_data.rows() - distValCnt; 

        m_data.conservativeResize(m_data.rows() + addRowsCnt, 
            m_attributes.size() + vNonPkAttrIdxsRel.size());
        numHeads = distValCnt;
        // By definition there are no dangling tuples. 
        numTails1 = m_dataVectorOfVectors.getNumRows() - distValCnt;             
        numTails2 = relation.m_dataVectorOfVectors.getNumRows() - distValCnt;
        
        VectorOfVectorsT headOutput(numHeads, m_attributes.size() + vNonPkAttrIdxsRel.size());
        VectorOfVectorsT tailOutput1(numTails1, getNumberOfNonPKAttributes());
        VectorOfVectorsT tailOutput2(numTails2, getNumberOfPKAttributes());
        schemaJoin(relation);
        
        for (const auto  nonPKAttrIdx: vNonPkAttrIdxsRel)
        {
            uint32_t colOffsetRel = nonPKAttrIdx - numPKAttributesRel;
            m_data.col(colOffset + colOffsetRel).setZero();
        }

        // TODO: reorder column first order. 
        //#pragma omp parallel for schedule(static)
        for (uint32_t distCnt = 0; distCnt < distValCnt; distCnt ++)
        {
            std::array<uint32_t, NUM_RELS> aHeadRowIdx;
            uint32_t precNextRowIdx;
            for (uint32_t idx = 0; idx < NUM_RELS; idx++)
            {
                aHeadRowIdx[idx] = avDistinctValuesRowPositions[idx][distCnt] + 1;
            }
            


            for (const auto  nonPKAttrIdx: vNonPkAttrIdxsRel)
            {
                uint32_t colOffsetRel = nonPKAttrIdx - numPKAttrs;
                //FIGARO_LOG_DBG("HeadRowIdx", headRowIdx, "colIdx", colOffset + colOffsetRel,
                //"colOffsetRel", colOffsetRel);
                headOutput[distCnt][colOffsetRel] = m_data(aHeadRowIdx[0], nonPKAttrIdx);
            }
            for (const auto  nonPKAttrIdx: vNonPkAttrIdxsRel)
            {
                uint32_t colOffsetRel = nonPKAttrIdx - numPKAttributesRel;
                //FIGARO_LOG_DBG("HeadRowIdx", headRowIdx, "colIdx", colOffset + colOffsetRel,
                //"colOffsetRel", colOffsetRel);
                headOutput[distCnt][numNonPkAttrs + colOffsetRel]
                = relation.m_data(aHeadRowIdx[1], nonPKAttrIdx);
            }
            
            // Appends computed heads from the second relation to the end of rows. 
            // 
            for (const auto  nonPKAttrIdx: vNonPkAttrIdxsRel)
            {
                uint32_t colOffsetRel = nonPKAttrIdx - numPKAttributesRel;
                //FIGARO_LOG_DBG("HeadRowIdx", headRowIdx, "colIdx", colOffset + colOffsetRel,
                //"colOffsetRel", colOffsetRel);
                m_data(aHeadRowIdx[0], colOffset + colOffsetRel)
                = relation.m_data(aHeadRowIdx[1], nonPKAttrIdx);
            }
            

            // Copies tails to the end. 
            precNextRowIdx = avDistinctValuesRowPositions[0][distCnt+1];
            for (uint32_t rowIdx = aHeadRowIdx[0] + 1; rowIdx <= precNextRowIdx; rowIdx++ )
            {
                for (const auto  nonPKAttrIdx: vNonPkAttrIdxs)
                {
                    uint32_t colOffset1 = nonPKAttrIdx - numPKAttrs;
                    tailOutput1[tailIdx1][colOffset1]
                    = m_data(rowIdx, nonPKAttrIdx);
                }
                tailIdx1++;
            }

            precNextRowIdx = avDistinctValuesRowPositions[1][distCnt+1];
            for (uint32_t rowIdx = aHeadRowIdx[1] + 1; rowIdx <= precNextRowIdx; rowIdx++ )
            {
                for (const auto  nonPKAttrIdx: vNonPkAttrIdxsRel)
                {
                    uint32_t colOffset2 = nonPKAttrIdx - numPKAttributesRel;
                    tailOutput2[tailIdx2][colOffset2]
                    = relation.m_data(rowIdx, nonPKAttrIdx);
                }
                tailIdx2++;
            }

            
            // Appends computed tails from the second relation to the end of 
            // the first relation. 
            // 
            precNextRowIdx = avDistinctValuesRowPositions[1][distCnt+1];
            for (uint32_t rowIdx = aHeadRowIdx[1] + 1; rowIdx <= precNextRowIdx; rowIdx++ )
            {
                m_data.row(appIdx).setZero();
                for (const auto  nonPKAttrIdx: vNonPkAttrIdxsRel)
                {
                    uint32_t colOffsetRel = nonPKAttrIdx - numPKAttributesRel;
                    m_data(appIdx, colOffset  + colOffsetRel)
                    = relation.m_data(rowIdx, nonPKAttrIdx);
                }
                appIdx++;
            }
            
        }
        FIGARO_LOG_INFO("After extend", *this);
    }

    static void makeDiagonalElementsPositiveInR(MatrixT& matR)
    {
        ArrayT&& aDiag = matR.diagonal().array().sign();
        for (uint32_t rowIdx = 0; rowIdx < matR.cols(); rowIdx ++)
        {
            matR.row(rowIdx) *= aDiag(rowIdx);
        }
    }

    void Relation::applyEigenQR(MatrixT* pR)
    {
        MICRO_BENCH_INIT(timer);
        uint32_t numNonPKAttributes = getNumberOfNonPKAttributes();
        MICRO_BENCH_START(timer);
        const auto& rVals { m_data.rightCols(numNonPKAttributes)};
        const auto& rHeads {m_dataHead};
        const auto& rTails1 {m_dataTails1};
        const auto& rTails2 {m_dataTails2};
        FIGARO_LOG_BENCH("numNonPKAttributes", numNonPKAttributes);
        MICRO_BENCH_STOP(timer);
        FIGARO_LOG_BENCH("Figaro", "main", "applyEigenQR", "copying", MICRO_BENCH_GET_TIMER_LAP(timer));
        Eigen::HouseholderQR<MatrixT> qr{};
        // TODO: think how to avoid copy constructor. 
        MICRO_BENCH_START(timer);
        qr.compute(rVals);
        MICRO_BENCH_STOP(timer);
        FIGARO_LOG_BENCH("Figaro", "main", "applyEigenQR", "computeHeads", MICRO_BENCH_GET_TIMER_LAP(timer));
        if (nullptr != pR)
        {
            *pR = qr.matrixQR().topLeftCorner(numNonPKAttributes, numNonPKAttributes).triangularView<Eigen::Upper>();
            makeDiagonalElementsPositiveInR(*pR);
        }
        MICRO_BENCH_STOP(timer);
        FIGARO_LOG_BENCH("Figaro", "main", "applyEigenQR", "extracting data", MICRO_BENCH_GET_TIMER_LAP(timer));
        FIGARO_LOG_BENCH("Figaro", "main", "applyEigenQR",MICRO_BENCH_GET_TIMER(timer));
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