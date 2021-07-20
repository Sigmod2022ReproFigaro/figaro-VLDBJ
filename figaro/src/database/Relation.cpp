#include "database/Relation.h"
#include <string>
#include <sstream>
#include <fstream>
#include <execution>
#include <algorithm>
#include <limits>
#include <set>
#include <type_traits>
#include <tuple>
#include <unordered_map>
#include "utils/Performance.h"
#include <omp.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_hash_map.h>


// Structure that defines hashing and comparison operations for user's type.
namespace tbb
{
    template<typename... TupleArgs>
    struct tbb_hash<std::tuple<TupleArgs...> >
    {
        tbb_hash() {}
        size_t operator()(const std::tuple<TupleArgs...>& key) const {
            return std::hash<std::tuple<TupleArgs...>>{}(key);
        }
    };
}

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
        return UINT32_MAX;
    }

    uint32_t Relation::getNumberOfPKAttributes(void) const
    {
        uint32_t pKAttributes = 0;
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
         nokPKAttributes = std::accumulate(m_attributes.begin(), m_attributes.end(), 0.0,
                     [](int curCnt, const Attribute& nextAtr)
                     {
                         return nextAtr.m_isPrimaryKey ? curCnt : (curCnt + 1);
                     });
        return nokPKAttributes;
    }

    std::vector<std::string> Relation::getAttributeNames(void) const
    {
        std::vector<std::string> vAttrNames;
        for (const auto& attribute: m_attributes)
        {
            vAttrNames.push_back(attribute.m_name);
        }
        return vAttrNames;
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

    void Relation::getCategoricalAttributeIdxs(std::vector<uint32_t>& vCatAttrIdxs) const
    {
          for (uint32_t idx = 0; idx < m_attributes.size(); idx++)
        {
            if (m_attributes[idx].m_type == AttributeType::CATEGORY)
            {
                vCatAttrIdxs.push_back(idx);
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

        for (const auto nonPKAttributeIdx: vNonPKAttributeIdxs)
        {
            FIGARO_LOG_DBG("Schema extended, Added attribute", nonPKAttributeIdx);
            m_attributes.push_back(relation.m_attributes[nonPKAttributeIdx]);
        }
        if (swapAttributes)
        {
            m_attributes.insert(std::end(m_attributes), std::begin(tmpV), std::end(tmpV));
        }
    }

    Relation::Relation(json jsonRelationSchema):
        m_data(0, 0), m_dataTails1(0, 0), m_dataTails2(0, 0), m_dataHead(0, 0),
        m_dataTails(0, 0), m_dataScales(0, 0), m_scales(0, 0), m_dataTailsGen(0, 0),
        m_countsJoinAttrs(0, 0)
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

    void Relation::resetComputations(void)
    {
        m_attributes = m_oldAttributes;
        m_dataHead = std::move(MatrixDT{0, 0});
        m_dataTails = std::move(MatrixDT{0, 0});
        m_dataTailsGen = std::move(MatrixDT{0, 0});
        m_scales = std::move(MatrixDT{0, 0});
        m_dataScales = std::move(MatrixDT{0, 0});
        m_allScales.clear();
        m_vSubTreeRelNames.clear();
        m_vSubTreeDataOffsets.clear();

        m_countsJoinAttrs = std::move(MatrixUI32T{0, 0});
        m_vParBlockStartIdxs.clear();
        m_vParBlockStartIdxsAfterFirstPass.clear();
        m_pHTParCounts.reset();
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

        m_data = std::move(MatrixDT(cntLines, numAttributes));


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
        return ErrorCode::NO_ERROR;
    }

    void Relation::getAttributesIdxs(const std::vector<std::string>& vAttributeNames,
        std::vector<uint32_t>& vAttributeIdxs) const
    {
        for (const auto& attributeName: vAttributeNames)
        {
            uint32_t attributeIdx = getAttributeIdx(attributeName);
            if (UINT32_MAX != attributeIdx)
            {
                vAttributeIdxs.push_back(attributeIdx);
            }
        }
    }

    void Relation::getAttributesIdxsComplement(const std::vector<uint32_t>& vAttributeIdxs,
        std::vector<uint32_t>& vAttributesCompIdxs) const
    {
        // Index that is incremented if the current idxAttr is equal to it.
        // In this case we do not add it to vAttributesCompIdxs.
        uint32_t idxAttrV = 0;
        for (uint32_t idxAttr = 0; idxAttr < getNumberOfAttributes(); idxAttr++)
        {
            if ((idxAttrV >= vAttributeIdxs.size()) ||
                (idxAttr < vAttributeIdxs[idxAttrV]))
            {
                vAttributesCompIdxs.push_back(idxAttr);
            }
            else
            {
                idxAttrV++;
            }
        }
    }

     void Relation::sortData(const std::vector<std::string>& vAttributeNames)
    {
        std::vector<uint32_t> vAttributesIdxs;
        uint32_t numRows = m_data.getNumRows();
        uint32_t numCols = m_data.getNumCols();
        MICRO_BENCH_INIT(sortData)
        MICRO_BENCH_START(sortData)
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
        MICRO_BENCH_STOP(sortData)
        FIGARO_LOG_BENCH("Sorting", m_name, MICRO_BENCH_GET_TIMER_LAP(sortData))
    }

    void Relation::sortData(void)
    {
        std::vector<std::string> vAttrNamesPKs;
        getPKAttributeNames(vAttrNamesPKs);
        FIGARO_LOG_DBG("Sorted Attributes", vAttrNamesPKs);
        sortData(vAttrNamesPKs);
    }

    void Relation::schemaDropAttrs(std::vector<uint32_t> vDropAttrIdxs)
    {
        std::set<uint32_t> sDropIdxs{vDropAttrIdxs.begin(), vDropAttrIdxs.end()};
        uint32_t curAttrIdx = 0;

        for (uint32_t attrIdx = 0; attrIdx < m_attributes.size(); attrIdx++)
        {
            if (sDropIdxs.find(attrIdx) == sDropIdxs.end())
            {
                m_attributes[curAttrIdx] = m_attributes[attrIdx];
                curAttrIdx++;
            }
            else
            {
                FIGARO_LOG_DBG("Found attrIdx", attrIdx)
            }
        }
        m_attributes.resize(curAttrIdx);
    }

    void Relation::schemaOneHotEncode(
        const std::map<uint32_t, std::set<uint32_t> >& mCatAttrsDist)
    {
        std::vector<uint32_t> attrIdxs;
        getAttributesIdxs(getAttributeNames(), attrIdxs);
        std::vector<Attribute> vOneHotEncAttrs;
        for (const auto& attrIdx: attrIdxs)
        {
            const auto& attribute = m_attributes[attrIdx];
            if (attribute.m_type == AttributeType::CATEGORY)
            {
                for (const auto& catAttrDist: mCatAttrsDist.at(attrIdx))
                {
                    uint32_t catPos = std::distance(mCatAttrsDist.at(attrIdx).begin(),
                                                mCatAttrsDist.at(attrIdx).find(catAttrDist));
                    if (catPos != 0)
                    {
                        Attribute oheAttr = attribute;
                        oheAttr.m_name += std::to_string(catAttrDist);
                        oheAttr.m_type = AttributeType::INTEGER;
                        vOneHotEncAttrs.push_back(oheAttr);
                    }
                }
            }
            else
            {
                vOneHotEncAttrs.push_back(attribute);
            }
        }
        m_attributes = vOneHotEncAttrs;
        FIGARO_LOG_INFO("One hot encoded", m_attributes)
    }

    void Relation::dropAttributes(const std::vector<std::string>& vDropAttrNames)
    {
        std::vector<uint32_t> vBeforeDropAttrIdxs;
        std::vector<uint32_t> vDropAttrIdxs;
        std::vector<uint32_t> vNonDropAttrIdxs;
        std::vector<uint32_t> vAfterDropAttrIdxs;

        getAttributesIdxs(vDropAttrNames, vDropAttrIdxs);
        getAttributesIdxsComplement(vDropAttrIdxs, vNonDropAttrIdxs);
        getAttributesIdxs(getAttributeNames(), vBeforeDropAttrIdxs);

        FIGARO_LOG_DBG("vDropAttrNames", vDropAttrNames)
        FIGARO_LOG_DBG("vDropAttrNames", vDropAttrIdxs)
        FIGARO_LOG_DBG("m_attributes", m_attributes)
        schemaDropAttrs(vDropAttrIdxs);
        FIGARO_LOG_DBG("m_attributes", m_attributes)
        getAttributesIdxs(getAttributeNames(), vAfterDropAttrIdxs);

        MatrixDT tmpData {m_data.getNumRows(), vAfterDropAttrIdxs.size()};

        FIGARO_LOG_DBG("vDropAttrIdxs", vDropAttrIdxs)
        FIGARO_LOG_DBG("vNonDropAttrIdxs", vNonDropAttrIdxs)
        FIGARO_LOG_DBG("vAfterDropAttrIdxs", vAfterDropAttrIdxs)
        for (uint32_t rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
        {
            for (uint32_t idx = 0; idx < vNonDropAttrIdxs.size(); idx++)
            {
                uint32_t attrBeforeIdx = vNonDropAttrIdxs[idx];
                uint32_t attrAfterIdx = vAfterDropAttrIdxs[idx];
                tmpData[rowIdx][attrAfterIdx] = m_data[rowIdx][attrBeforeIdx];
            }
        }
        m_data = std::move(tmpData);
        FIGARO_LOG_DBG("m_attributes", m_attributes)
        FIGARO_LOG_ASSERT(m_attributes.size() == m_data.getNumCols())
    }


    void Relation::updateSchema(const std::vector<std::string>& vAttributeNames)
    {
        std::vector<Attribute> vNewAttributes;
        std::vector<uint32_t> vOldAttrIdxs;
        std::vector<uint32_t> vBeforeNewAttrIdxs;
        std::vector<uint32_t> vAfterNewAttrIdxs;

        getAttributesIdxs(getAttributeNames(), vOldAttrIdxs);
        getAttributesIdxs(vAttributeNames,  vBeforeNewAttrIdxs);
        for (const auto& attrIdx: vBeforeNewAttrIdxs)
        {
            vNewAttributes.push_back(m_attributes[attrIdx]);
        }
        m_attributes = vNewAttributes;
        getAttributesIdxs(getAttributeNames(), vAfterNewAttrIdxs);
        FIGARO_LOG_DBG("vBeforeNewAttrIdxs", vBeforeNewAttrIdxs)
        FIGARO_LOG_DBG("vAfterNewAttrIdxs", vAfterNewAttrIdxs)

        MatrixDT tmpData {m_data.getNumRows(), vAfterNewAttrIdxs.size()};

        for (uint32_t rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
        {
            for (uint32_t idx = 0; idx < vBeforeNewAttrIdxs.size(); idx++)
            {
                uint32_t attrBeforeIdx = vBeforeNewAttrIdxs[idx];
                uint32_t attrAfterIdx = vAfterNewAttrIdxs[idx];
                tmpData[rowIdx][attrAfterIdx] = m_data[rowIdx][attrBeforeIdx];
            }
        }
        m_data = std::move(tmpData);
        FIGARO_LOG_DBG("m_attributes", m_attributes)
        FIGARO_LOG_ASSERT(m_attributes.size() == m_data.getNumCols())
    }

    // We assume the first variable is not categorical.
    void Relation::oneHotEncode(void)
    {
        std::map<uint32_t, std::set<uint32_t> > mCatAttrsDist;
        std::vector<uint32_t> vAttrIdxs;
        std::vector<uint32_t> vCatAttrIdxs;
        std::vector<uint32_t> vShiftAttrIdxs;
        std::vector<uint32_t> vShiftIdxs;
        uint32_t curShift;

        getCategoricalAttributeIdxs(vCatAttrIdxs);
        getAttributesIdxs(getAttributeNames(), vAttrIdxs);

        FIGARO_LOG_DBG("vCatAttrIdxs", vCatAttrIdxs)
        FIGARO_LOG_DBG("vAttrIdxs", vAttrIdxs)

        for (const auto& attrIdx: vCatAttrIdxs)
        {
            mCatAttrsDist[attrIdx] = std::set<uint32_t>{};
        }

        for (uint32_t rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
        {
            for (const auto& attrIdx: vCatAttrIdxs)
            {
               mCatAttrsDist[attrIdx].insert(static_cast<uint32_t>(m_data[rowIdx][attrIdx]));
            }
        }

        // Set up shifts.
        curShift = 0;
        vShiftIdxs.push_back(0);
        for (const auto& attrIdx: vAttrIdxs)
        {
            const auto& curAttr = m_attributes[attrIdx];
            if (curAttr.m_type == AttributeType::CATEGORY)
            {
                // Number of different categorical attributes  - 1 the first
                curShift += mCatAttrsDist[attrIdx].size() - 2;
            }
            if (attrIdx + 1 < vAttrIdxs.size())
            {
                vShiftIdxs.push_back(curShift);
            }
        }
        FIGARO_LOG_DBG("vShiftIdxs", vShiftIdxs)

        MatrixDT oneHotEncData{0, 1};
        oneHotEncData = std::move(MatrixDT::zeros(m_data.getNumRows(), m_data.getNumCols() + curShift));

        for (uint32_t rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
        {
            for (const auto& attrIdx: vAttrIdxs)
            {
                const auto& curAttr = m_attributes[attrIdx];
                if (curAttr.m_type == AttributeType::CATEGORY)
                {
                    uint32_t val = static_cast<uint32_t>(m_data[rowIdx][attrIdx]);
                    uint32_t catPos = std::distance(mCatAttrsDist[attrIdx].begin(),
                                                    mCatAttrsDist[attrIdx].find(val));
                    // We skip the first category in categorical variable.
                    if (catPos != 0)
                    {
                        uint32_t colIdx = attrIdx + vShiftIdxs[attrIdx] + catPos - 1;
                        oneHotEncData[rowIdx][colIdx] = 1;
                    }
                }
                else
                {
                    uint32_t colIdx = attrIdx + vShiftIdxs[attrIdx];
                    oneHotEncData[rowIdx][colIdx] = m_data[rowIdx][attrIdx];
                }
            }
        }

        schemaOneHotEncode(mCatAttrsDist);

        m_data =  std::move(oneHotEncData);
        m_oldAttributes = m_attributes;
    }


    void Relation::getDistinctValsAndBuildIndices(
        const std::vector<uint32_t>& vJoinAttrIdxs,
        const std::vector<uint32_t>& vParAttrIdxs,
        MatrixUI32T& cntJoinVals,
        std::vector<uint32_t>& vParBlockStartIdxs,
        std::vector<uint32_t>& vParBlockStartIdxsAfterFirstPass,
        bool isRootNode)
    {
            uint32_t distCnt;
        uint32_t prevRowIdx;
        uint32_t cumParBlockSizes;
        uint32_t rowIdx;
        uint32_t blockSize;
        std::vector<double> vPrevAttrVals(vJoinAttrIdxs.size(),
            std::numeric_limits<double>::max());
        std::vector<double> vParPrevAttrVals(vParAttrIdxs.size(),
            std::numeric_limits<double>::max());
        const double* pPrevAttrVals = &vPrevAttrVals[0];
        const double* pParPrevAttrVals = &vParPrevAttrVals[0];

        std::vector<uint32_t> vPKIndices;

        cumParBlockSizes = 0;
        distCnt = 0;
        prevRowIdx = -1;

        getPKAttributeIndices(vPKIndices);
        bool areJoinAttrsPK = false;
        if (vPKIndices.size() == vJoinAttrIdxs.size())
        {
            bool theSame = true;
            for (uint32_t idx = 0; idx < vJoinAttrIdxs.size(); idx ++)
            {
                if (vJoinAttrIdxs[idx] != vPKIndices[idx])
                {
                    theSame = false;
                    break;
                }
            }
            areJoinAttrsPK = theSame;
        }
        FIGARO_LOG_DBG("vPKIndices", m_name, vPKIndices, vJoinAttrIdxs)
        FIGARO_LOG_DBG("areJoinAttrsPK", m_name, areJoinAttrsPK)
        //areJoinAttrsPK = false;
        // TODO: Add parallel detection of
        // For counter and down count.

        if (isRootNode)
        {
            for (rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
            {
                const double* pCurAttrVals = m_data[rowIdx];
                bool joinAttrsDiff;

                if (areJoinAttrsPK)
                {
                    joinAttrsDiff = true;
                }
                else
                {
                    joinAttrsDiff = compareTuples(pCurAttrVals, pPrevAttrVals, vJoinAttrIdxs);
                }
                if (joinAttrsDiff)
                {
                    // We have a new block of join attributes. Stores the block size.
                    if (distCnt > 0)
                    {
                        blockSize =  rowIdx - prevRowIdx;
                        cumParBlockSizes += blockSize - 1;
                        cntJoinVals[distCnt-1][m_cntsJoinIdxV] = blockSize;
                        cntJoinVals[distCnt-1][m_cntsJoinIdxE] = rowIdx;
                    }
                    prevRowIdx = rowIdx;
                    pPrevAttrVals = pCurAttrVals;
                    for (const auto& joinAttrIdx: vJoinAttrIdxs)
                    {
                        cntJoinVals[distCnt][joinAttrIdx] = (uint32_t)m_data[rowIdx][joinAttrIdx];
                    }
                    distCnt++;
                    vParBlockStartIdxsAfterFirstPass.push_back(rowIdx - cumParBlockSizes);
                }
            }
            blockSize = rowIdx - prevRowIdx;
            cumParBlockSizes += blockSize - 1;
            cntJoinVals[distCnt-1][m_cntsJoinIdxV] = blockSize;
            cntJoinVals[distCnt-1][m_cntsJoinIdxE] = rowIdx;
            cntJoinVals.resize(distCnt);
            // Dummy indices
            vParBlockStartIdxs.push_back(distCnt);
            vParBlockStartIdxsAfterFirstPass.push_back(rowIdx - cumParBlockSizes);
        }
        else
        {

            if (areJoinAttrsPK)
            {
                MICRO_BENCH_INIT(testStupidLoop)
                MICRO_BENCH_START(testStupidLoop)
                for (rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
                {
                    const double* pCurAttrVals = m_data[rowIdx];
                    bool parAttrsDiff = compareTuples(pCurAttrVals, pParPrevAttrVals, vParAttrIdxs);
                    if (parAttrsDiff)
                    {
                        pParPrevAttrVals = pCurAttrVals;
                        vParBlockStartIdxs.push_back(rowIdx);
                    }
                }
                vParBlockStartIdxs.push_back(rowIdx);
                MICRO_BENCH_STOP(testStupidLoop)
                FIGARO_LOG_BENCH("Stupid loop", MICRO_BENCH_GET_TIMER_LAP(testStupidLoop))

                distCnt = 0;
                pParPrevAttrVals = &vParPrevAttrVals[0];

                vParBlockStartIdxsAfterFirstPass.resize(vParBlockStartIdxs.size());
                #pragma omp parallel for schedule(static)
                for (uint32_t distParCnt = 0; distParCnt < vParBlockStartIdxs.size() - 1; distParCnt++)
                {
                    uint32_t parStartIdx;
                    uint32_t parNextStartIdx;
                    parStartIdx = vParBlockStartIdxs[distParCnt];
                    parNextStartIdx = vParBlockStartIdxs[distParCnt+1];
                    vParBlockStartIdxsAfterFirstPass[distParCnt] = parStartIdx;
                    for (uint32_t rowIdx = parStartIdx; rowIdx < parNextStartIdx; rowIdx++)
                    {
                        // We have a new block of join attributes. Stores the block size.
                        cntJoinVals[rowIdx][m_cntsJoinIdxV] = 1;
                        cntJoinVals[rowIdx ][m_cntsJoinIdxE] = rowIdx + 1;
                        for (const auto& joinAttrIdx: vJoinAttrIdxs)
                        {
                            cntJoinVals[rowIdx][joinAttrIdx] = (uint32_t)m_data[rowIdx][joinAttrIdx];
                        }
                    }
                    // Dummy indices
                }
                distCnt = m_data.getNumRows();
                vParBlockStartIdxsAfterFirstPass.back() = distCnt;
                cntJoinVals.resize(distCnt);
                FIGARO_LOG_DBG("cntJoinVals", m_name, cntJoinVals)
            }
            else
            {
                for (rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
                {
                    const double* pCurAttrVals = m_data[rowIdx];
                    bool parAttrsDiff = compareTuples(pCurAttrVals, pParPrevAttrVals, vParAttrIdxs);
                    //bool joinAttrsDiff;// = areJoinAttrsPK;
                    bool joinAttrsDiff = compareTuples(pCurAttrVals, pPrevAttrVals, vJoinAttrIdxs);

                    //if (!areJoinAttrsPK)
                    //{
                    //}
                    if (parAttrsDiff || joinAttrsDiff)
                    {
                        // We have a new block of join attributes. Stores the block size.
                        if (distCnt > 0)
                        {
                            blockSize =  rowIdx - prevRowIdx;
                            cumParBlockSizes += blockSize - 1;
                            cntJoinVals[distCnt-1][m_cntsJoinIdxV] = blockSize;
                            cntJoinVals[distCnt-1][m_cntsJoinIdxE] = rowIdx;
                        }
                        prevRowIdx = rowIdx;
                        pPrevAttrVals = pCurAttrVals;
                        for (const auto& joinAttrIdx: vJoinAttrIdxs)
                        {
                            cntJoinVals[distCnt][joinAttrIdx] = (uint32_t)m_data[rowIdx][joinAttrIdx];
                        }
                        distCnt++;
                    }
                    if (parAttrsDiff)
                    {
                        pParPrevAttrVals = pCurAttrVals;
                        vParBlockStartIdxs.push_back(distCnt - 1);
                        vParBlockStartIdxsAfterFirstPass.push_back(rowIdx - cumParBlockSizes);
                    }
                }
                blockSize = rowIdx - prevRowIdx;
                cumParBlockSizes += blockSize - 1;
                cntJoinVals[distCnt-1][m_cntsJoinIdxV] = blockSize;
                cntJoinVals[distCnt-1][m_cntsJoinIdxE] = rowIdx;
                cntJoinVals.resize(distCnt);
                // Dummy indices
                vParBlockStartIdxs.push_back(distCnt);
                vParBlockStartIdxsAfterFirstPass.push_back(rowIdx - cumParBlockSizes);
            }
        }
    }


    std::map<std::vector<uint32_t>, uint32_t> Relation::getDownCounts(void)
    {
        std::map<std::vector<uint32_t>, uint32_t> downCounts;
        for (uint32_t idxRow = 0; idxRow < m_countsJoinAttrs.getNumRows(); idxRow++)
        {
            std::vector<uint32_t> curJoinAttr;
            for (uint32_t attrIdx = 0; attrIdx < m_countsJoinAttrs.getNumCols() - 4;
                attrIdx ++)
            {
                curJoinAttr.push_back(m_countsJoinAttrs[idxRow][attrIdx]);
            }
            downCounts[curJoinAttr] = m_countsJoinAttrs[idxRow][m_cntsJoinIdxD];
        }
        return downCounts;
    }


    std::map<std::vector<uint32_t>, uint32_t> Relation::getCircCounts(void)
    {
        std::map<std::vector<uint32_t>, uint32_t> circCounts;
        for (uint32_t idxRow = 0; idxRow < m_countsJoinAttrs.getNumRows(); idxRow++)
        {
            std::vector<uint32_t> curJoinAttr;
            for (uint32_t attrIdx = 0; attrIdx < m_countsJoinAttrs.getNumCols() - 4;
                attrIdx ++)
            {
                curJoinAttr.push_back(m_countsJoinAttrs[idxRow][attrIdx]);
            }
            circCounts[curJoinAttr] = m_countsJoinAttrs[idxRow][m_cntsJoinIdxC];
        }
        return circCounts;
    }


       std::map<std::vector<uint32_t>, uint32_t> Relation::getParDownCntsFromHashTable(
        const std::vector<std::string>& vParJoinAttrNames)
    {
        std::vector<uint32_t> vParJoinAttrIdxs;
        getAttributesIdxs(vParJoinAttrNames, vParJoinAttrIdxs);
        std::map<std::vector<uint32_t>, uint32_t> parDownCounts;
        if (vParJoinAttrIdxs.size() == 1)
        {

            // TODO: Extract join for relation specific from join attribute value.
            tbb::concurrent_unordered_map<uint32_t, DownUpCntT> htChildRowIdxOne = *(tbb::concurrent_unordered_map<uint32_t, DownUpCntT>*)(m_pHTParCounts.get());
            for (const auto& val: htChildRowIdxOne)
            {
                std::vector<uint32_t> curAttrs;
                curAttrs.push_back(val.first);
                uint32_t downCount = std::get<0>(val.second);
                parDownCounts[curAttrs] = downCount;
            }
        }
        else if (vParJoinAttrIdxs.size() == 2)
        {
            tbb::concurrent_unordered_map<std::tuple<uint32_t, uint32_t>, DownUpCntT > htChildRowIdxOne = *(tbb::concurrent_unordered_map<std::tuple<uint32_t, uint32_t>, DownUpCntT>*)(m_pHTParCounts.get());
            for (const auto& val: htChildRowIdxOne)
            {
                std::vector<uint32_t> curAttrs;
                curAttrs.push_back(std::get<0>(val.first));
                curAttrs.push_back(std::get<1>(val.first));
                uint32_t downCount = std::get<0>(val.second);
                parDownCounts[curAttrs] = downCount;
            }
            return parDownCounts;
        }
        else
        {
            // TODO: Consider how to handle this case.
            //const std::vector<double> t =
        }
        return parDownCounts;
    }


    std::map<std::vector<uint32_t>, uint32_t> Relation::getParUpCntsFromHashTable(
        const std::vector<std::string>& vParJoinAttrNames)
    {
        std::vector<uint32_t> vParJoinAttrIdxs;
        getAttributesIdxs(vParJoinAttrNames, vParJoinAttrIdxs);
        std::map<std::vector<uint32_t>, uint32_t> parUpCounts;
        if (vParJoinAttrIdxs.size() == 1)
        {

            // TODO: Extract join for relation specific from join attribute value.
            tbb::concurrent_unordered_map<uint32_t, DownUpCntT > htChildRowIdxOne = *(tbb::concurrent_unordered_map<uint32_t, DownUpCntT>*)(m_pHTParCounts.get());
            for (const auto& val: htChildRowIdxOne)
            {
                std::vector<uint32_t> curAttrs;
                curAttrs.push_back(val.first);
                uint32_t upCount = std::get<1>(val.second);
                parUpCounts[curAttrs] = upCount;
            }
        }
        else if (vParJoinAttrIdxs.size() == 2)
        {
            tbb::concurrent_unordered_map<std::tuple<uint32_t, uint32_t>, DownUpCntT> htChildRowIdxOne = *(tbb::concurrent_unordered_map<std::tuple<uint32_t, uint32_t>, DownUpCntT>*)(m_pHTParCounts.get());
            for (const auto& val: htChildRowIdxOne)
            {
                std::vector<uint32_t> curAttrs;
                curAttrs.push_back(std::get<0>(val.first));
                curAttrs.push_back(std::get<1>(val.first));
                uint32_t upCount = std::get<1>(val.second);
                parUpCounts[curAttrs] = upCount;
            }
            return parUpCounts;
        }
        else
        {
            // TODO: Consider how to handle this case.
            //const std::vector<double> t =
        }
        return parUpCounts;
    }


    void Relation::initHashTable(const std::vector<uint32_t>& vParAttrIdx,
        uint32_t hashTableSize)
    {
        if (vParAttrIdx.size() == 0)
        {
            // Do not do anything. This is a root node.
        }
        if (vParAttrIdx.size() == 1)
        {
            m_pHTParCounts = std::make_unique< tbb::concurrent_unordered_map<uint32_t, DownUpCntT> >();
        }
        else if (vParAttrIdx.size() == 2)
        {

            m_pHTParCounts = std::make_unique
                <tbb::concurrent_unordered_map<std::tuple<uint32_t, uint32_t>, DownUpCntT > > ();
        }
        else
        {

        }
    }


    void Relation::insertParDownCntFromHashTable(
        const std::vector<uint32_t>& vParAttrIdx,
        const uint32_t* pRow,
        uint32_t downCnt)
    {
        if (vParAttrIdx.size() == 0)
        {
            // Do not do anything. This is a root node.
        }
        if (vParAttrIdx.size() == 1)
        {
            tbb::concurrent_unordered_map<uint32_t, DownUpCntT>* tpHashTablePt =  (tbb::concurrent_unordered_map<uint32_t, DownUpCntT >*)(m_pHTParCounts.get());
            (*tpHashTablePt)[(uint32_t)pRow[vParAttrIdx[0]]] = std::make_tuple(downCnt, 0);
            //FIGARO_LOG_DBG("Inserted in", pRow[vParAttrIdx[0]], "value", downCnt)
        }
        else if (vParAttrIdx.size() == 2)
        {
            tbb::concurrent_unordered_map<std::tuple<uint32_t, uint32_t>, DownUpCntT >* tpHashTablePt =
             (tbb::concurrent_unordered_map<std::tuple<uint32_t, uint32_t>, DownUpCntT >*) (m_pHTParCounts.get());
             (*tpHashTablePt)[std::make_tuple(
                    (uint32_t)pRow[vParAttrIdx[0]], (uint32_t)pRow[vParAttrIdx[1]])] =
             std::make_tuple(downCnt, 0);
        }
        else
        {
            FIGARO_LOG_DBG("Damn")

        }
    }

    Figaro::Relation::DownUpCntT& Relation::getParCntFromHashTable(
        const std::vector<uint32_t>& vParJoinAttrIdxs,
        void*  htChildParAttrs,
        const uint32_t* pRow)
    {
        //static DownUpCntT t = std::make_tuple<uint32_t, uint32_t>(1 ,2);
        try {


        if (vParJoinAttrIdxs.size() == 1)
        {
            const uint32_t joinAttrVal = pRow[vParJoinAttrIdxs[0]];
            tbb::concurrent_unordered_map<uint32_t, DownUpCntT >* phtChildOneParAttrs = (tbb::concurrent_unordered_map<uint32_t, DownUpCntT>*)(htChildParAttrs);
            return phtChildOneParAttrs->at(joinAttrVal);
            //return (*phtChildOneParAttrs)[joinAttrVal];
        }
        else if (vParJoinAttrIdxs.size() == 2)
        {
            const std::tuple<uint32_t, uint32_t> joinAttrVal =
            std::make_tuple(pRow[vParJoinAttrIdxs[0]],
                            pRow[vParJoinAttrIdxs[1]]);
            tbb::concurrent_unordered_map<std::tuple<uint32_t, uint32_t>, DownUpCntT > *phtChildTwoParAttrs = (tbb::concurrent_unordered_map<std::tuple<uint32_t, uint32_t>, DownUpCntT >*)(htChildParAttrs);
            //return (*phtChildTwoParAttrs)[joinAttrVal];
            return phtChildTwoParAttrs->at(joinAttrVal);
        }
        else
        {
            // TODO: Consider how to handle this case.
            //const std::vector<double> t =
            FIGARO_LOG_DBG("Damn")
        }
        }
        catch (...)
        {
            if (vParJoinAttrIdxs.size() == 1)
            {
                const uint32_t joinAttrVal = pRow[vParJoinAttrIdxs[0]];
                FIGARO_LOG_DBG("DAMN ", m_name, vParJoinAttrIdxs[0], joinAttrVal)
                //return t;
            }
            else if (vParJoinAttrIdxs.size() == 2)
            {
                const uint32_t joinAttrVal1 = pRow[vParJoinAttrIdxs[0]];
                const uint32_t joinAttrVal2 = pRow[vParJoinAttrIdxs[1]];
                FIGARO_LOG_DBG("DAMN ", m_name, joinAttrVal1, joinAttrVal2)
            }

        }

    }


    Figaro::Relation::DownUpCntT& Relation::getParCntFromHashTable(
        const std::vector<uint32_t>& vParJoinAttrIdxs,
        void*  htChildParAttrs,
        const double* pRow)
    {
        if (vParJoinAttrIdxs.size() == 1)
        {
            // TODO: Extract join for relation specific from join attribute value.
            const uint32_t joinAttrVal = (uint32_t)pRow[vParJoinAttrIdxs[0]];
            tbb::concurrent_unordered_map<uint32_t, DownUpCntT >* phtChildOneParAttrs = (tbb::concurrent_unordered_map<uint32_t, DownUpCntT>*)(htChildParAttrs);
            return phtChildOneParAttrs->at(joinAttrVal);
            //return (*phtChildOneParAttrs)[joinAttrVal];
        }
        else if (vParJoinAttrIdxs.size() == 2)
        {
            //FIGARO_LOG_DBG("It shouldn't be")
            const std::tuple<uint32_t, uint32_t> joinAttrVal =
            std::make_tuple((uint32_t)pRow[vParJoinAttrIdxs[0]],
                            (uint32_t)pRow[vParJoinAttrIdxs[1]]);
            tbb::concurrent_unordered_map<std::tuple<uint32_t, uint32_t>, DownUpCntT > *phtChildTwoParAttrs = (tbb::concurrent_unordered_map<std::tuple<uint32_t, uint32_t>, DownUpCntT >*)(htChildParAttrs);
            //return (*phtChildTwoParAttrs)[joinAttrVal];
            return phtChildTwoParAttrs->at(joinAttrVal);
        }
        else
        {
            // TODO: Consider how to handle this case.
            //const std::vector<double> t =
            FIGARO_LOG_DBG("Damn")
        }

    }


    // TODO: Convert this to a template.
    void Relation::initHashTableRowIdxs(
        const std::vector<uint32_t>& vParAttrIdx,
        void*& pHashTablePt,
        const MatrixDT& data)
    {
        if (vParAttrIdx.size() == 1)
        {
            std::unordered_map<double, uint32_t>* tpHashTablePt = new std::unordered_map<double, uint32_t> ();
            tpHashTablePt->reserve(data.getNumRows());
            for (uint32_t rowIdx = 0; rowIdx < data.getNumRows(); rowIdx++)
            {
                double curAttrVal = data[rowIdx][vParAttrIdx[0]];
                (*tpHashTablePt)[curAttrVal] = rowIdx;
            }
            pHashTablePt = tpHashTablePt;
        }
        else if (vParAttrIdx.size() == 2)
        {
            std::unordered_map<std::tuple<double, double>, uint32_t>* tpHashTablePt =
             new std::unordered_map<std::tuple<double, double>, uint32_t> ();
            tpHashTablePt->reserve(data.getNumRows());
            for (uint32_t rowIdx = 0; rowIdx < data.getNumRows(); rowIdx++)
            {
                std::tuple<double, double> curAttrVal =
                std::make_tuple(data[rowIdx][vParAttrIdx[0]],
                                data[rowIdx][vParAttrIdx[1]]);
                (*tpHashTablePt)[curAttrVal] = rowIdx;
            }
            pHashTablePt = tpHashTablePt;
        }
        else
        {

        }
    }

    uint32_t Relation::getChildRowIdx(
        uint32_t rowIdx,
        const std::vector<uint32_t>& vParJoinAttrIdxs,
        void*  htChildRowIdx,
        const MatrixDT& dataParent)
    {
        uint32_t rowChildIdx = UINT32_MAX;
        try{
        if (vParJoinAttrIdxs.size() == 1)
        {
            // TODO: Extract join for relation specific from join attribute value.
            const double joinAttrVal = dataParent[rowIdx][vParJoinAttrIdxs[0]];
            std::unordered_map<double, uint32_t>* pHtChildRowIdxOne = (std::unordered_map<double, uint32_t>*)(htChildRowIdx);
            //rowChildIdx = (*pHtChildRowIdxOne)[joinAttrVal];
            rowChildIdx = pHtChildRowIdxOne->at(joinAttrVal);
        }
        else if (vParJoinAttrIdxs.size() == 2)
        {
            const std::tuple<double, double> joinAttrVal =
            std::make_tuple(dataParent[rowIdx][vParJoinAttrIdxs[0]],
                            dataParent[rowIdx][vParJoinAttrIdxs[1]]);
            std::unordered_map<std::tuple<double, double>, uint32_t>* htChildRowIdxOne =(std::unordered_map<std::tuple<double, double>, uint32_t>*)(htChildRowIdx);
            ///rowChildIdx = (*htChildRowIdxOne)[joinAttrVal];
            rowChildIdx = htChildRowIdxOne->at(joinAttrVal);
        }
        else
        {
            // TODO: Consider how to handle this case.
            //const std::vector<double> t =
        }
        }
        catch(...)
        {
            FIGARO_LOG_DBG("Relation", m_name, "rowChildIdx", rowChildIdx, "rowIdx", rowIdx, vParJoinAttrIdxs, dataParent[rowIdx][vParJoinAttrIdxs[0]], dataParent[rowIdx][vParJoinAttrIdxs[1]])
            return 0;

        }


        return rowChildIdx;
    }

    void Relation::destroyHashTableRowIdxs(
        const std::vector<uint32_t>& vParJoinAttrIdxs,
        void*& pHashTablePt)
    {
        if (vParJoinAttrIdxs.size() == 1)
        {
            std::unordered_map<double, uint32_t>* pHtChildRowIdxOne =(std::unordered_map<double, uint32_t>*)(pHashTablePt);
            delete pHtChildRowIdxOne;
        }
        else if (vParJoinAttrIdxs.size() == 2)
        {
            std::unordered_map<std::tuple<double, double>, uint32_t>* pHtChildRowIdxOne = (std::unordered_map<std::tuple<double, double>, uint32_t>*)(pHashTablePt);
            delete pHtChildRowIdxOne;
        }
        else
        {
            // TODO: Consider how to handle this case.
            //const std::vector<double> t =
        }

    }

    void Relation::computeDownCounts(
        const std::vector<Relation*>& vpChildRels,
        const std::vector<std::string>& vJoinAttrNames,
        const std::vector<std::string>& vParJoinAttrNames,
        const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
        bool isRootNode)
    {
        std::vector<uint32_t> vJoinAttrIdxs;
        std::vector<uint32_t> vParJoinAttrIdxs;
        std::vector<uint32_t> vParBlockStartIdxs;
        std::vector<uint32_t> vParBlockStartIdxsAfterFirstPass;
        std::vector<std::vector<uint32_t> > vvJoinAttrIdxs;
        std::vector<std::vector<uint32_t> >  vvCurJoinAttrIdxs;
        uint32_t numDistParVals;

        vvCurJoinAttrIdxs.resize(vvJoinAttributeNames.size());
        vvJoinAttrIdxs.resize(vvJoinAttributeNames.size());

        getAttributesIdxs(vJoinAttrNames, vJoinAttrIdxs);
        getAttributesIdxs(vParJoinAttrNames, vParJoinAttrIdxs);

        for (uint32_t idxChild = 0; idxChild < vpChildRels.size(); idxChild++)
        {
            vpChildRels[idxChild]->getAttributesIdxs(vvJoinAttributeNames[idxChild], vvJoinAttrIdxs[idxChild]);
            getAttributesIdxs(vvJoinAttributeNames[idxChild], vvCurJoinAttrIdxs[idxChild]);
        }

        MatrixUI32T cntsJoin {m_data.getNumRows(), vJoinAttrNames.size() + 4};
        vParBlockStartIdxs.reserve(m_data.getNumRows());
        vParBlockStartIdxsAfterFirstPass.reserve(m_data.getNumRows());

        m_cntsJoinIdxE = cntsJoin.getNumCols() - 4;
        m_cntsJoinIdxC = cntsJoin.getNumCols() - 3;
        m_cntsJoinIdxD = cntsJoin.getNumCols() - 2;
        m_cntsJoinIdxV = cntsJoin.getNumCols() - 1;

        MICRO_BENCH_INIT(flagsCmp)
        //MICRO_BENCH_INIT(hashTable)
        MICRO_BENCH_INIT(pureDownCnt)
        MICRO_BENCH_START(flagsCmp)
        getDistinctValsAndBuildIndices(vJoinAttrIdxs, vParJoinAttrIdxs, cntsJoin,
            vParBlockStartIdxs, vParBlockStartIdxsAfterFirstPass, isRootNode);
        MICRO_BENCH_STOP(flagsCmp)
        FIGARO_LOG_BENCH("Figaro", "flag computation " + m_name, MICRO_BENCH_GET_TIMER_LAP(flagsCmp))
        numDistParVals = vParBlockStartIdxs.size() - 1;
        //MICRO_BENCH_START(hashTable)
        initHashTable(vParJoinAttrIdxs, numDistParVals);
        //MICRO_BENCH_STOP(hashTable)
        //FIGARO_LOG_BENCH("Figaro hashTable computation", m_name, MICRO_BENCH_GET_TIMER_LAP(hashTable))
        MICRO_BENCH_START(pureDownCnt)
        // TODO: Replace this with template function.
        if (isRootNode)
        {
            #pragma omp parallel for schedule(static)
            for (uint32_t distCnt = 0;
                distCnt < cntsJoin.getNumRows(); distCnt++)
            {
                cntsJoin[distCnt][m_cntsJoinIdxC] = cntsJoin[distCnt][m_cntsJoinIdxV];

                // SELECT COUNT(*) JOIN RELATIONS IN SUBTREE
                // WHERE join_attributes = current join attribute
                uint32_t curDownCnt = cntsJoin[distCnt][m_cntsJoinIdxV];
                for (uint32_t idxChild = 0; idxChild < vpChildRels.size(); idxChild++)
                {
                    FIGARO_LOG_DBG("CHILD", vpChildRels[idxChild]->m_name)
                    Figaro::Relation::DownUpCntT& cnts =
                        getParCntFromHashTable(
                            vvCurJoinAttrIdxs[idxChild],
                            vpChildRels[idxChild]->m_pHTParCounts.get(),
                            cntsJoin[distCnt]);
                    uint32_t downCnt = std::get<0>(cnts);
                    curDownCnt *= downCnt;
                }
                // J_i
                cntsJoin[distCnt][m_cntsJoinIdxD] = curDownCnt;
            }
        }
        else
        {
            #pragma omp parallel for schedule(static)
            for (uint32_t distCntPar = 0; distCntPar < numDistParVals; distCntPar++)
            {
                uint32_t sum = 0;
                uint32_t parCurBlockStartIdx = vParBlockStartIdxs[distCntPar];
                uint32_t parNextBlockStartIdx = vParBlockStartIdxs[distCntPar + 1];
                // Distinct parent attribute
                for (uint32_t distCnt = parCurBlockStartIdx;
                    distCnt < parNextBlockStartIdx; distCnt++)
                {
                    cntsJoin[distCnt][m_cntsJoinIdxC] = cntsJoin[distCnt][m_cntsJoinIdxV];


                    // SELECT COUNT(*) JOIN RELATIONS IN SUBTREE
                    // WHERE join_attributes = current join attribute
                    uint32_t curDownCnt = cntsJoin[distCnt][m_cntsJoinIdxV];
                    for (uint32_t idxChild = 0; idxChild < vpChildRels.size(); idxChild++)
                    {
                        Figaro::Relation::DownUpCntT& cnts =
                            getParCntFromHashTable(
                                vvCurJoinAttrIdxs[idxChild],
                                vpChildRels[idxChild]->m_pHTParCounts.get(),
                                cntsJoin[distCnt]);
                        uint32_t downCnt = std::get<0>(cnts);
                        curDownCnt *= downCnt;
                    }
                    cntsJoin[distCnt][m_cntsJoinIdxD] = curDownCnt;
                    sum += curDownCnt;

                }
                insertParDownCntFromHashTable(vParJoinAttrIdxs,
                    cntsJoin[parCurBlockStartIdx], sum);
            }
        }
        MICRO_BENCH_STOP(pureDownCnt)
        FIGARO_LOG_BENCH("Figaro pure down count", MICRO_BENCH_GET_TIMER_LAP(pureDownCnt))
        m_countsJoinAttrs = std::move(cntsJoin);
        m_vParBlockStartIdxs = std::move(vParBlockStartIdxs);
        m_vParBlockStartIdxsAfterFirstPass = std::move(vParBlockStartIdxsAfterFirstPass);
    }

   void Relation::computeUpAndCircleCounts(
        const std::vector<Relation*>& vpChildRels,
        const std::vector<std::string>& vParJoinAttrNames,
        const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
        bool isRootNode)
    {
        std::vector<std::vector<uint32_t> > vvJoinAttrIdxs;
        std::vector<std::vector<uint32_t> >  vvCurJoinAttrIdxs;
        std::vector<uint32_t> vParJoinAttrIdxs;
        uint32_t numDistParVals;

        vvCurJoinAttrIdxs.resize(vvJoinAttributeNames.size());
        vvJoinAttrIdxs.resize(vvJoinAttributeNames.size());
        getAttributesIdxs(vParJoinAttrNames, vParJoinAttrIdxs);

        for (uint32_t idxRel = 0; idxRel < vvJoinAttributeNames.size(); idxRel++)
        {
            vpChildRels[idxRel]->getAttributesIdxs(vvJoinAttributeNames[idxRel], vvJoinAttrIdxs[idxRel]);
            getAttributesIdxs(vvJoinAttributeNames[idxRel], vvCurJoinAttrIdxs[idxRel]);
        }
        numDistParVals = m_vParBlockStartIdxs.size() - 1;

        // TODO: Rethink how to refactor the code to omit these checks.
        if (isRootNode)
        {
            #pragma omp parallel for schedule(static)
            for (uint32_t distCnt = 0; distCnt < m_countsJoinAttrs.getNumRows(); distCnt++)
            {
                // SELECT COUNT(*) JOIN RELATIONS IN SUBTREE
                // WHERE join_attributes = current join attribute
                uint32_t curDownCnt = m_countsJoinAttrs[distCnt][m_cntsJoinIdxD];
                uint32_t fullCnt = curDownCnt;
                // TODO: Add copying of counts from cnt_u to cnt_c in leaves.
                m_countsJoinAttrs[distCnt][m_cntsJoinIdxC] = fullCnt / m_countsJoinAttrs[distCnt][m_cntsJoinIdxV];

                for (uint32_t idxChild = 0; idxChild < vpChildRels.size(); idxChild++)
                {
                    Figaro::Relation::DownUpCntT& cnts = getParCntFromHashTable(
                                vvCurJoinAttrIdxs[idxChild],
                                vpChildRels[idxChild]->m_pHTParCounts.get(),
                                m_countsJoinAttrs[distCnt]);
                    (std::get<1>(cnts)).fetch_and_add(fullCnt);
                }
            }
        }
        else
        {
            #pragma omp parallel for schedule(static)
            for (uint32_t distCntPar = 0; distCntPar < numDistParVals; distCntPar++)
            {
                uint32_t parCurBlockStartIdx = m_vParBlockStartIdxs[distCntPar];
                uint32_t parNextBlockStartIdx = m_vParBlockStartIdxs[distCntPar + 1];
                Figaro::Relation::DownUpCntT& cnts =
                        getParCntFromHashTable(
                            vParJoinAttrIdxs,
                            m_pHTParCounts.get(),
                            m_countsJoinAttrs[parCurBlockStartIdx]);
                // Division by downCnt
                // This does not need mutex since each tuple will be unique.
                //
                std::get<1>(cnts) = std::get<1>(cnts) / std::get<0>(cnts);
                uint32_t upCnt = std::get<1>(cnts);

                for (uint32_t distCnt = parCurBlockStartIdx;
                    distCnt < parNextBlockStartIdx; distCnt++)
                {
                    // SELECT COUNT(*) JOIN RELATIONS IN SUBTREE
                    // WHERE join_attributes = current join attribute
                    uint32_t curDownCnt = m_countsJoinAttrs[distCnt][m_cntsJoinIdxD];
                    uint32_t fullCnt = curDownCnt * upCnt;
                    // TODO: Add copying of counts from cnt_u to cnt_c in leaves.
                    m_countsJoinAttrs[distCnt][m_cntsJoinIdxC] = fullCnt / m_countsJoinAttrs[distCnt][m_cntsJoinIdxV];

                    for (uint32_t idxChild = 0; idxChild < vpChildRels.size(); idxChild++)
                    {
                        Figaro::Relation::DownUpCntT& cnts = getParCntFromHashTable(
                                    vvCurJoinAttrIdxs[idxChild],
                                    vpChildRels[idxChild]->m_pHTParCounts.get(),
                                    m_countsJoinAttrs[distCnt]);
                        (std::get<1>(cnts)).fetch_and_add(fullCnt);
                    }
                }
            }
        }
    }

   // We assume join attributes are before nonJoinAttributes.
    void Relation::computeHeadsAndTails(
        const std::vector<std::string>& vJoinAttrNames, bool isLeafNode)
    {
        std::vector<uint32_t> vJoinAttrIdxs;
        std::vector<uint32_t> vNonJoinAttrIdxs;
        uint32_t numDistinctValues;
        uint32_t numJoinAttrs;
        uint32_t numNonJoinAttrs;
        uint32_t numTailRows;

        getAttributesIdxs(vJoinAttrNames, vJoinAttrIdxs);
        getAttributesIdxsComplement(vJoinAttrIdxs, vNonJoinAttrIdxs);

        FIGARO_LOG_DBG("vJoinAttrNames", vJoinAttrNames, "Relation", m_name)
        FIGARO_LOG_DBG("vJoinAttrIdxs", vJoinAttrIdxs)
        FIGARO_LOG_DBG("vNonJoinAttrIdxs", vNonJoinAttrIdxs)

        numDistinctValues = m_countsJoinAttrs.getNumRows();
        numJoinAttrs = vJoinAttrIdxs.size();
        numNonJoinAttrs = vNonJoinAttrIdxs.size();
        numTailRows = m_data.getNumRows() - numDistinctValues;

        // 1) Preallocate memory for heads and tails, scales, dataScales, allScales.
        MatrixDT dataHeads{numDistinctValues, getNumberOfAttributes()};
        MatrixDT dataTails{numTailRows, numNonJoinAttrs};
        MatrixDT dataScale{numDistinctValues, 1};
        MatrixDT scale{numDistinctValues, 1};
        std::vector<double> allScales(numDistinctValues);

        // 2) Iterate over join attributes and compute Heads and Tails of relation that
        // project away these attributes.
        #pragma omp parallel for schedule(static)
        for (uint32_t distCnt = 0; distCnt < numDistinctValues; distCnt++)
        {
            uint32_t headRowIdx;
            uint32_t nextHeadRowIdx;
            uint32_t numDistVals;
            std::vector<double> vCurRowSum(numNonJoinAttrs);

            headRowIdx = (distCnt == 0) ? 0 : m_countsJoinAttrs[distCnt - 1][m_cntsJoinIdxE];
            nextHeadRowIdx = m_countsJoinAttrs[distCnt][m_cntsJoinIdxE];
            numDistVals =  m_countsJoinAttrs[distCnt][m_cntsJoinIdxV];

            for (const uint32_t nonJoinAttrIdx: vNonJoinAttrIdxs)
            {
                vCurRowSum[nonJoinAttrIdx - numJoinAttrs] = m_data[headRowIdx][nonJoinAttrIdx];
            }
            for (uint32_t rowIdx = headRowIdx + 1;
                rowIdx < nextHeadRowIdx;
                rowIdx++)
            {
                uint32_t tailRowIdx = rowIdx - distCnt - 1;
                // Double needed due to casts.
                double i = rowIdx - headRowIdx + 1;
                double sqrtCircCnt = std::sqrt(m_countsJoinAttrs[distCnt][m_cntsJoinIdxC]);
                for (const uint32_t nonJoinAttrIdx: vNonJoinAttrIdxs)
                {
                    double prevRowSum;
                    double tailVal;
                    prevRowSum = vCurRowSum[nonJoinAttrIdx - numJoinAttrs];
                    vCurRowSum[nonJoinAttrIdx - numJoinAttrs] += m_data[rowIdx][nonJoinAttrIdx];
                    tailVal = (m_data[rowIdx][nonJoinAttrIdx] * (i - 1) - prevRowSum)
                    / std::sqrt(i * (i - 1));
                    dataTails[tailRowIdx][nonJoinAttrIdx - numJoinAttrs] =
                        tailVal * sqrtCircCnt;
                }
            }

            // Copy join attributes to be used as indices.
            for (const uint32_t joinAttrIdx: vJoinAttrIdxs)
            {
                dataHeads[distCnt][joinAttrIdx] = m_data[headRowIdx][joinAttrIdx];
            }
            // Copy arithmetic sum.
            for (const uint32_t nonJoinAttrIdx: vNonJoinAttrIdxs)
            {
                dataHeads[distCnt][nonJoinAttrIdx] = vCurRowSum[nonJoinAttrIdx - numJoinAttrs];
            }

            // TODO: Push square root as late as possible.
            scale[distCnt][0] = std::sqrt(numDistVals);
            dataScale[distCnt][0] = 1 / std::sqrt(numDistVals);
            allScales[distCnt] = scale[distCnt][0];
        }

        m_dataHead = std::move(dataHeads);
        m_dataTails = std::move(dataTails);

        m_scales = std::move(scale);
        m_dataScales = std::move(dataScale);
        m_allScales = std::move(allScales);

        m_vSubTreeDataOffsets.push_back(vJoinAttrNames.size());
        m_vSubTreeRelNames.push_back(m_name);
    }

    void Relation::schemaJoins(
        const std::vector<Relation*>& vpChildRels,
        const std::vector<std::vector<uint32_t> >& vvJoinAttrIdxs,
        const std::vector<std::vector<uint32_t> >& vvNonJoinAttrIdxs)
    {
        for (uint32_t idxRel = 0; idxRel < vpChildRels.size(); idxRel++)
        {
            uint32_t prevSize = m_attributes.size();
            uint32_t prevvSTDOffsSize = m_vSubTreeDataOffsets.size();
            for (const auto nonJoinAttrIdx: vvNonJoinAttrIdxs[idxRel])
            {
                m_attributes.push_back(vpChildRels[idxRel]->m_attributes[nonJoinAttrIdx]);
            }

            // For each child, copy relation names from the subtree.
            m_vSubTreeRelNames.insert(m_vSubTreeRelNames.end(),
                vpChildRels[idxRel]->m_vSubTreeRelNames.begin(),
                vpChildRels[idxRel]->m_vSubTreeRelNames.end());
            FIGARO_LOG_DBG("m_vSubTreeRelNames", m_vSubTreeRelNames)
            // Copy the offsest to the data.
            m_vSubTreeDataOffsets.insert(m_vSubTreeDataOffsets.end(),
                vpChildRels[idxRel]->m_vSubTreeDataOffsets.begin(),
                vpChildRels[idxRel]->m_vSubTreeDataOffsets.end());

            // Updates offsets since they are added at the end.
            for (uint32_t idxRelSub = prevvSTDOffsSize;
                idxRelSub < m_vSubTreeDataOffsets.size(); idxRelSub ++)
            {
                m_vSubTreeDataOffsets[idxRelSub] += prevSize - vvJoinAttrIdxs[idxRel].size();
            }
        }
        FIGARO_LOG_DBG("Schema Joins", m_name, m_attributes, m_vSubTreeRelNames, m_vSubTreeDataOffsets)
    }

    void Relation::schemaRemoveNonParJoinAttrs(
            const std::vector<uint32_t>& vJoinAttrIdxs,
            const std::vector<uint32_t>& vParJoinAttrIdxs)
    {
        // We have assumption here that the parent join attribute will be always the first one.
        m_attributes.erase(m_attributes.begin() + vParJoinAttrIdxs.size(),
            m_attributes.begin() + vJoinAttrIdxs.size());
    }

    void Relation::aggregateAwayChildrenRelations(
        const std::vector<Relation*>& vpChildRels,
        const std::vector<std::string>& vJoinAttributeNames,
        const std::vector<std::vector<std::string> >& vvJoinAttributeNames)
    {
        uint32_t numJoinAttrsCurRel;
        std::vector<uint32_t> vJoinAttrIdxs;
        std::vector<std::vector<uint32_t> >  vvCurJoinAttrIdxs;
        std::vector<uint32_t> vNonJoinAttrIdxs;
        std::vector<std::vector<uint32_t> > vvJoinAttrIdxs;
        std::vector<std::vector<uint32_t> > vvNonJoinAttrIdxs;
        std::vector<uint32_t> vNumJoinAttrs;
        // Cumulative sum of non-join attributes of relations before.
        // We assume preorder layout of data columns.
        std::vector<uint32_t> vCumNumNonJoinAttrs;
        std::vector<uint32_t> vCumNumRelSubTree;
        std::vector<void*> vpHashTabRowPt;

        numJoinAttrsCurRel = vJoinAttributeNames.size();
        vvJoinAttrIdxs.resize(vvJoinAttributeNames.size());
        vvNonJoinAttrIdxs.resize(vvJoinAttributeNames.size());
        vvCurJoinAttrIdxs.resize(vvJoinAttributeNames.size());
        vNumJoinAttrs.resize(vvJoinAttributeNames.size());
        vCumNumNonJoinAttrs.resize(vpChildRels.size());
        vCumNumRelSubTree.resize(vpChildRels.size());
        vpHashTabRowPt.resize(vpChildRels.size());

        getAttributesIdxs(vJoinAttributeNames, vJoinAttrIdxs);
        getAttributesIdxsComplement(vJoinAttrIdxs, vNonJoinAttrIdxs);
        MICRO_BENCH_INIT(aggregateAway)
        MICRO_BENCH_START(aggregateAway)

        for (uint32_t idxRel = 0; idxRel < vvJoinAttributeNames.size(); idxRel++)
        {
            FIGARO_LOG_DBG("Building hash indices for child", idxRel)
            vpChildRels[idxRel]->getAttributesIdxs(vvJoinAttributeNames[idxRel],
                vvJoinAttrIdxs[idxRel]);
            vpChildRels[idxRel]->getAttributesIdxsComplement(vvJoinAttrIdxs[idxRel], vvNonJoinAttrIdxs[idxRel]);
            FIGARO_LOG_DBG("vvJoinAttrIdxs[idxRel]", vvJoinAttributeNames[idxRel], vvJoinAttrIdxs[idxRel], vpChildRels[idxRel]->m_attributes)
            getAttributesIdxs(vvJoinAttributeNames[idxRel], vvCurJoinAttrIdxs[idxRel]);
            vpChildRels[idxRel]->initHashTableRowIdxs(vvJoinAttrIdxs[idxRel],
                vpHashTabRowPt[idxRel], vpChildRels[idxRel]->m_dataHead);
            vNumJoinAttrs[idxRel] = vvJoinAttributeNames[idxRel].size();
            if (idxRel == 0)
            {
                vCumNumNonJoinAttrs[idxRel] = vNonJoinAttrIdxs.size();
                vCumNumRelSubTree[idxRel] = 1;
            }
            else
            {
                vCumNumNonJoinAttrs[idxRel] = vCumNumNonJoinAttrs[idxRel-1] +
                                            vvNonJoinAttrIdxs[idxRel - 1].size();
                vCumNumRelSubTree[idxRel] = vCumNumRelSubTree[idxRel-1] + vpChildRels[idxRel-1]->m_vSubTreeRelNames.size();
            }
        }
        FIGARO_LOG_DBG("vvJoinAttributeNames", vvJoinAttributeNames)
        schemaJoins(vpChildRels, vvJoinAttrIdxs, vvNonJoinAttrIdxs);

        MatrixDT dataOutput {m_dataHead.getNumRows(), (uint32_t)m_attributes.size()};
        MatrixDT scales{m_dataHead.getNumRows(), vpChildRels.size() + 1};
        MatrixDT dataScales{m_dataHead.getNumRows(), m_vSubTreeRelNames.size()};

       #pragma omp parallel for schedule(static)
       for (uint32_t rowIdx = 0; rowIdx < m_dataHead.getNumRows(); rowIdx++)
       {
            // TODO: Optimization change the way how the values are extracted to tree.
            for (const auto& joinAttrIdx: vJoinAttrIdxs)
            {
                dataOutput[rowIdx][joinAttrIdx] = m_dataHead[rowIdx][joinAttrIdx];
            }
            // TODO: Add reordering relations based on global order
            for (const auto& nonJoinAttrIdx: vNonJoinAttrIdxs)
            {
                dataOutput[rowIdx][nonJoinAttrIdx] = m_dataHead[rowIdx][nonJoinAttrIdx];
            }
            for (uint32_t idxRel = 0; idxRel < vpChildRels.size(); idxRel ++)
            {

                uint32_t childRowIdx = getChildRowIdx(rowIdx, vvCurJoinAttrIdxs[idxRel],
                                                      vpHashTabRowPt[idxRel], m_dataHead);
                // Copying data from children relations.
                const double* childRowPt = vpChildRels[idxRel]->m_dataHead[childRowIdx];
                for (const auto nonJoinAttrIdx: vvNonJoinAttrIdxs[idxRel])
                {
                    uint32_t idxOut = numJoinAttrsCurRel + vCumNumNonJoinAttrs[idxRel] +
                                    nonJoinAttrIdx - vNumJoinAttrs[idxRel];
                    dataOutput[rowIdx][idxOut] = childRowPt[nonJoinAttrIdx];
                }
                // Updating dataScales data from relations from subtrees.
                for (uint32_t idxSubTreeRel = 0;
                        idxSubTreeRel < vpChildRels[idxRel]->m_vSubTreeRelNames.size();
                        idxSubTreeRel++)
                {
                    uint32_t shiftIdxSTRel = idxSubTreeRel + vCumNumRelSubTree[idxRel];
                    dataScales[rowIdx][shiftIdxSTRel] =
                        vpChildRels[idxRel]->m_dataScales[childRowIdx][idxSubTreeRel];
                }
                // +1 because 0th is for the first relation
                scales[rowIdx][idxRel + 1] = vpChildRels[idxRel]->m_scales[childRowIdx][0];
                m_allScales[rowIdx] *= scales[rowIdx][idxRel + 1];
            }
            dataScales[rowIdx][0] = m_dataScales[rowIdx][0];
            scales[rowIdx][0] = m_scales[rowIdx][0];
            for (uint32_t idxRel = 0; idxRel < vpChildRels.size(); idxRel ++)
            {
                for (uint32_t idxSubTreeRel = 0;
                            idxSubTreeRel < vpChildRels[idxRel]->m_vSubTreeRelNames.size();
                            idxSubTreeRel++)
                {
                    uint32_t shiftIdxSTRel = idxSubTreeRel + vCumNumRelSubTree[idxRel];
                    dataScales[rowIdx][shiftIdxSTRel] *= m_allScales[rowIdx];
                    dataScales[rowIdx][shiftIdxSTRel] /= scales[rowIdx][idxRel + 1];
                }
            }
            // Updated datascales for the central relation.
            dataScales[rowIdx][0] *= m_allScales[rowIdx];
            dataScales[rowIdx][0] /= scales[rowIdx][0];
            scales[rowIdx][0] = m_allScales[rowIdx];
        }
        for (uint32_t idxChild = 0; idxChild < vpChildRels.size(); idxChild++)
        {
            destroyHashTableRowIdxs(vvCurJoinAttrIdxs[idxChild], vpHashTabRowPt[idxChild]);
        }

        m_dataHead = std::move(dataOutput);
        m_dataScales = std::move(dataScales);
        m_scales = std::move(scales);

        MICRO_BENCH_STOP(aggregateAway)
        FIGARO_LOG_BENCH("Figaro", "aggregate away" + m_name,  MICRO_BENCH_GET_TIMER_LAP(aggregateAway));

    }

    void Relation::computeAndScaleGeneralizedHeadAndTail(
        const std::vector<std::string>& vJoinAttributeNames,
        const std::vector<std::string>& vParJoinAttributeNames,
        bool isRootNode
        )
    {
        std::vector<uint32_t> vJoinAttrIdxs;
        std::vector<uint32_t> vParJoinAttrIdxs;
        uint32_t numParDistVals;
        uint32_t numRelsSubTree;
        uint32_t numJoinAttrs;
        uint32_t numParJoinAttrs;
        uint32_t numOmittedAttrs;
        uint32_t numNonJoinAttrs;

        MICRO_BENCH_INIT(genHT)
        MICRO_BENCH_START(genHT)
        getAttributesIdxs(vJoinAttributeNames, vJoinAttrIdxs);
        getAttributesIdxs(vParJoinAttributeNames, vParJoinAttrIdxs);
        // TODO: Replace this
        numParDistVals = m_vParBlockStartIdxsAfterFirstPass.size() - 1;
        numRelsSubTree = m_vSubTreeRelNames.size();
        numJoinAttrs = vJoinAttributeNames.size();
        numNonJoinAttrs = getNumberOfAttributes() - numJoinAttrs;
        numParJoinAttrs = vParJoinAttrIdxs.size();
        numOmittedAttrs = numJoinAttrs - numParJoinAttrs;

        MatrixDT dataHeadOut { numParDistVals, getNumberOfAttributes() - numOmittedAttrs};
        MatrixDT dataTailsOut{ m_dataHead.getNumRows() - numParDistVals, numNonJoinAttrs};
        MatrixDT scales{numParDistVals, 1};
        MatrixDT dataScales{numParDistVals, m_dataScales.getNumCols()};

        FIGARO_LOG_INFO("Compute Generalized Head and Tail for relation", m_name)

        FIGARO_LOG_DBG("vJoinAttributeNames", vJoinAttributeNames)
        // temporary adds an element to denote the end limit.
        m_vSubTreeDataOffsets.push_back(m_attributes.size());
        MICRO_BENCH_INIT(genHTMainLoop)
        //MICRO_BENCH_START(genHTMainLoop)
        #pragma omp parallel for schedule(static)
        for (uint32_t distParCnt = 0; distParCnt < numParDistVals; distParCnt++)
        {
            uint32_t startIdx;
            double sumSqrScalesPrev;
            double sumSqrScalesCur;
            double vi;
            double sqrtDownCnt;
            double sqrtUpCnt;
            std::vector<double> vCurScaleSum(numNonJoinAttrs);

            // TODO: Initialize up count from hash table.

            startIdx = m_vParBlockStartIdxsAfterFirstPass[distParCnt];
            //FIGARO_LOG_DBG("Getting counts")
            if (isRootNode)
            {
                sqrtDownCnt =  std::sqrt(m_countsJoinAttrs[distParCnt][m_cntsJoinIdxD]);
                sqrtUpCnt = 1;
            }
            else
            {
                Figaro::Relation::DownUpCntT& cnts =
                getParCntFromHashTable(vParJoinAttrIdxs, m_pHTParCounts.get(), m_dataHead[startIdx]);
                sqrtDownCnt = std::sqrt(std::get<0>(cnts));
                sqrtUpCnt = std::sqrt((uint32_t)std::get<1>(cnts));
            }
            sumSqrScalesPrev = 0;
            sumSqrScalesCur = m_scales[startIdx][0] * m_scales[startIdx][0];

            //FIGARO_LOG_INFO("Scaling data by data_scale")
            // Scaling data by data_scale
            for (uint32_t idxRel = 0; idxRel < numRelsSubTree; idxRel++)
            {
                for (uint32_t attrIdx = m_vSubTreeDataOffsets[idxRel];
                            attrIdx < m_vSubTreeDataOffsets[idxRel + 1];
                            attrIdx++)
                {
                    m_dataHead[startIdx][attrIdx] *= m_dataScales[startIdx][idxRel];
                    vCurScaleSum[attrIdx - numJoinAttrs] = m_dataHead[startIdx][attrIdx] * m_scales[startIdx][0];
                }
            }
            //FIGARO_LOG_INFO("Generalized tail computation")
            // Generalized head and tail computation.
            for (uint32_t rowIdx = startIdx + 1;
                rowIdx < m_vParBlockStartIdxsAfterFirstPass[distParCnt+1];
                rowIdx++)
            {
                uint32_t tailRowIdx = rowIdx - distParCnt - 1;
                vi = m_scales[rowIdx][0];
                // \pnorm{v_{[i-1]}}{2}^2
                sumSqrScalesPrev = sumSqrScalesCur;
                // \pnorm{v_{[i]}}{2}^2
                sumSqrScalesCur = sumSqrScalesPrev + vi * vi;
                //FIGARO_LOG_DBG("sumSqrScalesPrev", sumSqrScalesPrev, "sumSqrScalesCur", sumSqrScalesCur)
                for (uint32_t idxRel = 0; idxRel < numRelsSubTree; idxRel++)
                {
                    for (uint32_t attrIdx = m_vSubTreeDataOffsets[idxRel];
                        attrIdx < m_vSubTreeDataOffsets[idxRel + 1];
                        attrIdx++)
                    {
                        uint32_t shiftIdx = attrIdx - numJoinAttrs;
                        // A_i
                        m_dataHead[rowIdx][attrIdx] *= m_dataScales[rowIdx][idxRel];
                        dataTailsOut[tailRowIdx][shiftIdx] =
                            (m_dataHead[rowIdx][attrIdx] * sumSqrScalesPrev
                            - vi * vCurScaleSum[shiftIdx])
                            / std::sqrt(sumSqrScalesCur * sumSqrScalesPrev);
                        // Multiplies generalized tails with up count.
                        dataTailsOut[tailRowIdx][shiftIdx] *= sqrtUpCnt;
                        vCurScaleSum[shiftIdx] += m_dataHead[rowIdx][attrIdx] * vi;
                    }
                }
            }

            scales[distParCnt][0] = sqrtDownCnt;

            // Copies join parent attributes to generalized head.
            for (const auto& parJoinAttrIdx: vParJoinAttrIdxs)
            {
                dataHeadOut[distParCnt][parJoinAttrIdx] = m_dataHead[startIdx][parJoinAttrIdx];
            }

            //  Generalized Head computation.
            for (uint32_t idxRel = 0; idxRel < numRelsSubTree; idxRel++)
            {
                double curDataScale = 1 / scales[distParCnt][0];
                double multiplier;
                dataScales[distParCnt][idxRel] = curDataScale;
                if (isRootNode)
                {
                    multiplier = curDataScale;
                }
                else
                {
                    multiplier = 1;
                }
                for (uint32_t attrIdx = m_vSubTreeDataOffsets[idxRel];
                    attrIdx < m_vSubTreeDataOffsets[idxRel + 1];
                    attrIdx++)
                {
                    dataHeadOut[distParCnt][attrIdx - numOmittedAttrs] =
                        multiplier * vCurScaleSum[attrIdx - numJoinAttrs];
                }

            }
        }
        m_vSubTreeDataOffsets.pop_back();
        // Updates m_vSubTreeDataOffsets to drop omitted attrs.
        for (uint32_t idxRel = 0; idxRel < numRelsSubTree; idxRel++)
        {
            m_vSubTreeDataOffsets[idxRel] -= numOmittedAttrs;
        }

        schemaRemoveNonParJoinAttrs(vJoinAttrIdxs, vParJoinAttrIdxs);

        m_dataHead = std::move(dataHeadOut);
        m_dataTailsGen = std::move(dataTailsOut);
        m_dataScales = std::move(dataScales);
        m_scales = std::move(scales);
        FIGARO_LOG_DBG("Attributes_name", m_attributes)
        MICRO_BENCH_STOP(genHT)
        //MICRO_BENCH_STOP(genHTMainLoop)
        FIGARO_LOG_BENCH("Figaro", "computeAndScaleGeneralizedHeadAndTail " + m_name,  MICRO_BENCH_GET_TIMER_LAP(genHT));
        //FIGARO_LOG_BENCH("Figaro",  "Generalized head and tail main loop",  MICRO_BENCH_GET_TIMER_LAP(genHTMainLoop));

    }


    void Relation::computeQROfGeneralizedHead(uint32_t numNonJoinAttrs)
    {
        FIGARO_LOG_ASSERT(numNonJoinAttrs <= m_dataHead.getNumCols())
        MatrixDT tmp = m_dataHead.getRightCols(numNonJoinAttrs);
        m_dataHead = std::move(tmp);
        FIGARO_LOG_INFO("QR generalized head", m_name)
        FIGARO_LOG_INFO("m_dataHead", m_dataHead)
        if (m_dataHead.getNumCols() > m_dataHead.getNumRows())
        {
            FIGARO_LOG_ERROR("Head", m_name)
            return;
        }
        MICRO_BENCH_INIT(qrHead)
        MICRO_BENCH_START(qrHead)
        //#pragma omp parallel
        {
            m_dataHead.computeQRGivens(getNumberOfThreads());
        }
        m_dataHead.resize(m_dataHead.getNumCols());
        MICRO_BENCH_STOP(qrHead)
        FIGARO_LOG_BENCH("Figaro", "QR Head",  MICRO_BENCH_GET_TIMER_LAP(qrHead));
    }

    void Relation::computeQROfTail(void)
    {
        FIGARO_LOG_INFO("QR Tail", m_name)
        if (m_dataTails.getNumCols() > m_dataTails.getNumRows())
        {
            FIGARO_LOG_INFO("Tail", m_name)
            return;
        }
        FIGARO_LOG_DBG("m_dataTails", m_dataTails)
        MICRO_BENCH_INIT(qrTail)
        MICRO_BENCH_START(qrTail)
        //#pragma omp parallel
        {
            m_dataTails.computeQRGivens(getNumberOfThreads());
        }
        m_dataTails.resize(m_dataTails.getNumCols());
        MICRO_BENCH_STOP(qrTail)
        FIGARO_LOG_BENCH("Figaro", "Tail", m_name,  MICRO_BENCH_GET_TIMER_LAP(qrTail));
    }

    void Relation::computeQROfGeneralizedTail(void)
    {
        FIGARO_LOG_INFO("QR generalized Tail", m_name)
        if (m_dataTailsGen.getNumCols() > m_dataTailsGen.getNumRows())
        {
            FIGARO_LOG_INFO("Generalized tail", m_name)
            return;
        }
        MICRO_BENCH_INIT(qrGenTail)
        MICRO_BENCH_START(qrGenTail)
        {
            m_dataTailsGen.computeQRGivens(getNumberOfThreads());
        }
        m_dataTailsGen.resize(m_dataTailsGen.getNumCols());
        MICRO_BENCH_STOP(qrGenTail)
        FIGARO_LOG_BENCH("Figaro", "Generalized Tail", m_name,  MICRO_BENCH_GET_TIMER_LAP(qrGenTail));
    }


    void Relation::computeQROfConcatenatedGeneralizedHeadAndTails(
        const std::vector<Relation*>& vpRels,
        MatrixEigenT* pR)
    {
        MatrixEigenT matEigen;
        Eigen::HouseholderQR<MatrixEigenT> qr{};
        std::vector<uint32_t> vLeftCumNumNonJoinAttrs;
        std::vector<uint32_t> vRightCumNumNonJoinAttrs;
        std::vector<uint32_t> vCumNumRowsUp;
        uint32_t totalNumRows;
        uint32_t totalNumCols;
        uint32_t numRels;
        uint32_t minNumRows;

        numRels = vpRels.size();
        vCumNumRowsUp.resize(numRels + 1);
        vLeftCumNumNonJoinAttrs.resize(numRels + 1);

        vLeftCumNumNonJoinAttrs[0] = 0;
        for (uint32_t idx = 1; idx < vLeftCumNumNonJoinAttrs.size(); idx++)
        {
            vLeftCumNumNonJoinAttrs[idx] = vLeftCumNumNonJoinAttrs[idx - 1] +
                    vpRels[idx-1]->m_dataTails.getNumCols();
        }
        totalNumCols = vLeftCumNumNonJoinAttrs[numRels];

        // TODO: Move this to database class
        computeQROfGeneralizedHead(totalNumCols);
        vCumNumRowsUp[0] = m_dataHead.getNumRows();
        FIGARO_LOG_DBG("vCumNumRowsUp[0]", vCumNumRowsUp[0])
        for (uint32_t idx = 1; idx < vCumNumRowsUp.size(); idx++)
        {
            vCumNumRowsUp[idx] = vCumNumRowsUp[idx-1] +
            vpRels[idx-1]->m_dataTails.getNumRows() +
            vpRels[idx-1]->m_dataTailsGen.getNumRows();
            FIGARO_LOG_DBG("vpRels[idx-1]->m_dataTails.getNumRows()", vpRels[idx-1]->m_dataTails.getNumRows())
            FIGARO_LOG_DBG("vpRels[idx-1]->m_dataTailsGen.getNumRows()", vpRels[idx-1]->m_dataTailsGen.getNumRows())
        }
        totalNumRows = vCumNumRowsUp[numRels];
        MatrixDT catGenHeadAndTails{totalNumRows, totalNumCols};

        // Copying dataHead.
        for (uint32_t rowIdx = 0; rowIdx < m_dataHead.getNumRows(); rowIdx++)
        {
            for (uint32_t colIdx = 0; colIdx < m_dataHead.getNumCols(); colIdx++)
            {
                catGenHeadAndTails[rowIdx][colIdx] = m_dataHead[rowIdx][colIdx];
            }
        }

        FIGARO_LOG_DBG("m_dataHead",  m_dataHead)

        // Vertically concatenating tails and generalized tail to the head.
        for (uint32_t idxRel = 0; idxRel < vpRels.size(); idxRel ++)
        {
            uint32_t startTailIdx = vCumNumRowsUp[idxRel];
            uint32_t startGenTailIdx = vCumNumRowsUp[idxRel] + vpRels[idxRel]->m_dataTails.getNumRows();
            //FIGARO_LOG_DBG("idxRel", idxRel, startTailIdx, startGenTailIdx)
            for (uint32_t rowIdx = startTailIdx; rowIdx < startGenTailIdx; rowIdx ++)
            {
                //FIGARO_LOG_DBG("rowIdx", idxRel, rowIdx)
                // Set Left zeros
                for (uint32_t colIdx = 0; colIdx < vLeftCumNumNonJoinAttrs[idxRel];
                    colIdx ++)
                {
                    catGenHeadAndTails[rowIdx][colIdx] = 0;
                }
                uint32_t tailsRowIdx = rowIdx - startTailIdx;
                //FIGARO_LOG_DBG("rowIdx", rowIdx, tailsRowIdx)

                // Set central Relations
                for (uint32_t colIdx = vLeftCumNumNonJoinAttrs[idxRel];
                    colIdx < vLeftCumNumNonJoinAttrs[idxRel+1]; colIdx++ )
                {
                    uint32_t tailsColIdx = colIdx - vLeftCumNumNonJoinAttrs[idxRel];
                    catGenHeadAndTails[rowIdx][colIdx] =
                     vpRels[idxRel]->m_dataTails[tailsRowIdx][tailsColIdx];
                }
                // Set Right zeros
                for (uint32_t colIdx = vLeftCumNumNonJoinAttrs[idxRel+1];
                    colIdx < totalNumCols; colIdx ++)
                {
                    catGenHeadAndTails[rowIdx][colIdx] = 0;
                }
            }

            for (uint32_t rowIdx = startGenTailIdx;
                rowIdx < vCumNumRowsUp[idxRel+1]; rowIdx ++)
            {
                // Set Left zeros
                for (uint32_t colIdx = 0; colIdx < vLeftCumNumNonJoinAttrs[idxRel];
                    colIdx ++)
                {
                    catGenHeadAndTails[rowIdx][colIdx] = 0;
                }
                uint32_t tailsRowIdx = rowIdx - startGenTailIdx;
                //FIGARO_LOG_DBG("rowIdx", rowIdx, tailsRowIdx)

                // Set central Relations
                // Since the the order of relations is preordered,
                // generalized tails will be one after another.
                for (uint32_t colIdx = vLeftCumNumNonJoinAttrs[idxRel];
                    colIdx < vLeftCumNumNonJoinAttrs[idxRel] + vpRels[idxRel]->m_dataTailsGen.getNumCols();
                    colIdx++ )
                {
                    uint32_t tailsColIdx = colIdx - vLeftCumNumNonJoinAttrs[idxRel];
                    catGenHeadAndTails[rowIdx][colIdx] =
                     vpRels[idxRel]->m_dataTailsGen[tailsRowIdx][tailsColIdx];
                }
                // Set Right zeros
                for (uint32_t colIdx = vLeftCumNumNonJoinAttrs[idxRel] + vpRels[idxRel]->m_dataTailsGen.getNumCols();
                    colIdx < totalNumCols; colIdx ++)
                {
                    catGenHeadAndTails[rowIdx][colIdx] = 0;
                }
            }
        }
        FIGARO_LOG_DBG("catGenHeadAndTails", catGenHeadAndTails)
        FIGARO_LOG_INFO("Computing QR using eigen")
        MICRO_BENCH_INIT(eigen)
        MICRO_BENCH_START(eigen)
        copyMatrixDTToMatrixEigen(catGenHeadAndTails, matEigen);
        qr.compute(matEigen);
        MICRO_BENCH_STOP(eigen)
        FIGARO_LOG_INFO("Extracting R from eigen")
        FIGARO_LOG_INFO("Extracting R from eigen", matEigen.rows(), matEigen.cols())
        minNumRows = std::min(totalNumRows, totalNumCols);
        FIGARO_LOG_DBG("totalNumCols", totalNumCols, totalNumRows)
        *pR = qr.matrixQR().topLeftCorner(minNumRows, totalNumCols).triangularView<Eigen::Upper>();
        if (totalNumCols > minNumRows)
        {
            appendZeroRows(*pR, totalNumCols - minNumRows);
        }
        FIGARO_LOG_BENCH("Figaro", "Eigen QR",  MICRO_BENCH_GET_TIMER_LAP(eigen));
        FIGARO_LOG_DBG(*pR);
        makeDiagonalElementsPositiveInR(*pR);
    }

    const Relation::MatrixDT& Relation::getHead(void) const
    {
        return m_dataHead;
    }

    const Relation::MatrixDT& Relation::getTail(void) const
    {
        return m_dataTails;
    }

    const Relation::MatrixDT& Relation::getGeneralizedTail(void) const
    {
        return m_dataTailsGen;
    }


    const Relation::MatrixDT& Relation::getScales(void) const
    {
        return m_scales;
    }

    const Relation::MatrixDT& Relation::getDataScales(void) const
    {
        return m_dataScales;
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

    // This code joins 1-N relations, where the cardinality of @p this is 1 and
    // the cardinality of @p relation is N.
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
        attrIdx = getAttributeIdx(joinAttrName);
        relation.getRowPtrs(joinAttrName, hashTabRowPt2);

        schemaJoin(relation, bSwapAttributes);
        MatrixDT dataOutput {m_data.getNumRows(), (uint32_t)m_attributes.size()};


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
        uint32_t numPKAttrs;
        uint32_t numPKAttrsRel;
        uint32_t numNonPkAttrs;
        uint32_t numHeads;
        uint32_t numTails1;
        uint32_t numTails2;


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
        m_dataHead = std::move(headOutput);
        m_dataTails1 = std::move(tailOutput1);
        m_dataTails2 = std::move(tailOutput2);
    }

    void Relation::appendZeroRows(MatrixEigenT& matR, uint32_t numAppendedRows)
    {
        uint32_t oldNumRows = matR.rows();
        matR.conservativeResize(matR.rows() + numAppendedRows, Eigen::NoChange_t::NoChange);
        for (uint32_t rowIdx = oldNumRows; rowIdx < matR.rows(); rowIdx++)
        {
            for (uint32_t colIdx = 0; colIdx < matR.cols(); colIdx++)
            {
                matR(rowIdx, colIdx) = 0;
            }
        }
    }

    void Relation::makeDiagonalElementsPositiveInR(MatrixEigenT& matR)
    {
        ArrayT&& aDiag = matR.diagonal().array().sign();
        for (uint32_t rowIdx = 0; rowIdx < matR.cols(); rowIdx ++)
        {
            matR.row(rowIdx) *= aDiag(rowIdx);
        }
    }

    void Relation::copyMatrixDTToMatrixEigen(
        const MatrixDT& matDT, MatrixEigenT& matEig)
    {
        matEig.resize(matDT.getNumRows(), matDT.getNumCols());
        for (uint32_t rowIdx = 0; rowIdx < matDT.getNumRows(); rowIdx++)
        {
            for (uint32_t colIdx = 0; colIdx < matDT.getNumCols(); colIdx++)
            {
                matEig(rowIdx, colIdx) = matDT[rowIdx][colIdx];
            }
        }
    }

    void Relation::applyEigenQR(MatrixEigenT* pR)
    {
        MICRO_BENCH_INIT(timer);
        uint32_t numNonPKAttributes = getNumberOfNonPKAttributes();
        Eigen::HouseholderQR<MatrixEigenT> qr{};
        MatrixEigenT matEigen;
        std::array<MatrixDT*, 2> pDataTails = {&m_dataTails1, &m_dataTails2};

        FIGARO_LOG_DBG("m_dataHead", m_dataHead)
        FIGARO_LOG_DBG("m_dataTails1", m_dataTails1)
        FIGARO_LOG_DBG("m_dataTails2", m_dataTails2)
        MICRO_BENCH_START(timer);

        // Tries to resize where the number of columns is much bigger than the number of rows.
        // This increases size and causes all sorts of problems.
        if (m_dataHead.getNumCols() < m_dataHead.getNumRows())
        {
            m_dataHead.computeQRGivens(getNumberOfThreads());
            m_dataHead.resize(m_dataHead.getNumCols());
        }

        omp_set_num_threads(pDataTails.size());
        //omp_set_num_threads(1);
        #pragma omp parallel
        {
            #pragma omp for
            for (uint32_t idx = 0; idx < pDataTails.size(); idx ++)
            {
                if (pDataTails[idx]->getNumCols() < pDataTails[idx]->getNumRows())
                {
                    pDataTails[idx]->computeQRGivens(omp_get_num_procs() / pDataTails.size());
                    pDataTails[idx]->resize(pDataTails[idx]->getNumCols());

                }
            }
        }

        MICRO_BENCH_STOP(timer);
        FIGARO_LOG_BENCH("Figaro", "main", "computeQRDecompositionHouseholder", "computeQRGivens", MICRO_BENCH_GET_TIMER_LAP(timer));

        MICRO_BENCH_START(timer);
        m_dataTails1 = m_dataTails1.concatenateHorizontallyScalar(0, m_dataHead.getNumCols() - m_dataTails1.getNumCols());
        m_dataTails2 = MatrixDT::zeros(m_dataTails2.getNumRows(), m_dataHead.getNumCols() - m_dataTails2.getNumCols()).concatenateHorizontally(m_dataTails2);
        auto&& m_dataFull = m_dataHead.concatenateVertically(m_dataTails1).concatenateVertically(m_dataTails2);
        MICRO_BENCH_STOP(timer);
        FIGARO_LOG_BENCH("Figaro", "main", "computeQRDecompositionHouseholder", "concatenate", MICRO_BENCH_GET_TIMER_LAP(timer));

        MICRO_BENCH_START(timer);
        copyMatrixDTToMatrixEigen(m_dataFull, matEigen);
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

    std::ostream& operator<<(std::ostream& out, const Relation::Attribute& attribute)
    {
        std::string strPk = attribute.m_isPrimaryKey ? "PK, " : "";
        std::string strType = attribute.mapTypeToStr.at(attribute.m_type);
        out << attribute.m_name  << " (" << strPk
            << strType << ")" << "; ";
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Relation& relation)
    {
        out << std::endl;
        out << "Relation " << relation.m_name << std::endl;
        out << "[ ";
        for (const auto& attribute: relation.m_attributes)
        {
            out << attribute;
        }
        out << "]" << std::endl;
        out << relation.m_data;
        return out;
    }
}
