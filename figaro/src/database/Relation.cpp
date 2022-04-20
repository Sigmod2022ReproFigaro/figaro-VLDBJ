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
#include <tbb/parallel_sort.h>

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


    Relation::Relation(const std::string& name,
        MatrixDT&& data, const std::vector<Attribute>& attributes):
        m_name(name), m_data(std::move(data)),
        m_attributes(attributes), m_dataColumnMajor(0, 0),
         m_dataScales(0, 0),
        m_scales(0, 0),
        m_countsJoinAttrs(0, 0)
    {
        m_isTmp = true;
    }

    Relation::Relation(json jsonRelationSchema):
        m_data(0, 0), m_dataColumnMajor(0, 0),
         m_dataScales(0, 0),
        m_scales(0, 0),
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

    Relation* Relation::createFactorRelation(
            const std::string& extension,
            MatrixDT&& data,
            uint32_t numRightAttrs)
    {
        uint32_t offset = m_attributes.size() - numRightAttrs;
        return new Relation(m_name + "_" + extension, std::move(data),
            {m_attributes.begin() + offset, m_attributes.end()});
    }

    void Relation::resetComputations(void)
    {
        m_scales = std::move(MatrixDT{0, 0});
        m_dataScales = std::move(MatrixDT{0, 0});
        m_allScales.clear();
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

        FIGARO_LOG_INFO("Number of loaded rows", cntLines, "number attributes", numAttributes)

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
        //MICRO_BENCH_INIT(sortData)
        //MICRO_BENCH_START(sortData)
        MatrixDT tmpMatrix(numRows, numCols);
        getAttributesIdxs(vAttributeNames, vAttributesIdxs);

        std::vector<double*> vRowPts(numRows);
        for (uint32_t rowIdx = 0; rowIdx < numRows; rowIdx++)
        {
            vRowPts[rowIdx] = m_data[rowIdx];
        }
        tbb::parallel_sort(
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
        //MICRO_BENCH_STOP(sortData)
        //FIGARO_LOG_BENCH("Sorting", m_name, MICRO_BENCH_GET_TIMER_LAP(sortData))
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
        std::vector<uint32_t> vBeforeNewAttrIdxs;
        std::vector<uint32_t> vAfterNewAttrIdxs;

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
            for (uint32_t idx = 0; idx < vAfterNewAttrIdxs.size(); idx++)
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
        FIGARO_LOG_INFO("Pre-ohe dimensions ", m_data.getNumRows(), m_data.getNumCols())

        FIGARO_LOG_INFO("OHE dimensions ", oneHotEncData.getNumRows(), oneHotEncData.getNumCols())

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
                        // -1 is for offset caused by first variable.
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
        FIGARO_LOG_INFO("Finished schema reordering in relation ", m_name)
    }


    void Relation::renameRelation(const std::string& newName)
    {
        m_name = newName;
    }

    Relation Relation::copyRelation(void) const
    {
        auto dataCopied = m_data.getBlock(0, m_data.getNumRows()-1, 0, m_data.getNumCols() - 1);
        return Relation("COPY_" + m_name, std::move(dataCopied), m_attributes);
    }

    void Relation::persist(void)
    {
        m_isTmp = false;
    }

    Relation Relation::createDummyGenTailRelation(void) const
    {
        MatrixDT data{0, m_attributes.size()};
        return Relation("GEN_TAIL" + m_name, std::move(data), m_attributes);
    }


    void Relation::initHashTableMNJoin(
        const std::vector<uint32_t>& vParAttrIdx,
        void*&  pHashTablePt, Figaro::Relation::MatrixDT& dataRef)
    {
        if (vParAttrIdx.size() == 0)
        {
            // Do not do anything. This is a root node.
        }
        if (vParAttrIdx.size() == 1)
        {
            std::unordered_map<uint32_t, std::vector<uint32_t> >* tpHashTablePt = new std::unordered_map<uint32_t, std::vector<uint32_t> >();
            tpHashTablePt->reserve(dataRef.getNumRows());
            for (uint32_t rowIdx = 0; rowIdx < dataRef.getNumRows(); rowIdx++)
            {
                double curAttrVal = dataRef[rowIdx][vParAttrIdx[0]];
                const auto& insRes =  tpHashTablePt->try_emplace(
                (uint32_t)dataRef[rowIdx][vParAttrIdx[0]], std::vector<uint32_t>());
                auto& inserted = *(insRes.first);
                inserted.second.push_back(rowIdx);
            }
            pHashTablePt = tpHashTablePt;
        }
        else if (vParAttrIdx.size() == 2)
        {
            std::unordered_map<std::tuple<uint32_t, uint32_t>, std::vector<uint32_t> >* tpHashTablePt = new std::unordered_map<std::tuple<uint32_t, uint32_t>,
                std::vector<uint32_t> > ();
            for (uint32_t rowIdx = 0; rowIdx < dataRef.getNumRows(); rowIdx++)
            {
                const auto& insRes =  tpHashTablePt->try_emplace(
                        std::make_tuple((uint32_t)dataRef[rowIdx][vParAttrIdx[0]],
                        (uint32_t)dataRef[rowIdx][vParAttrIdx[1]]), std::vector<uint32_t>());
                auto& inserted = *(insRes.first);
                inserted.second.push_back(rowIdx);
            }
            pHashTablePt = tpHashTablePt;
        }
        else
        {

        }
    }

    void Relation::destroyHashTableMNJoin(
        const std::vector<uint32_t>& vParJoinAttrIdxs,
        void*& pHashTablePt
        )
    {
        if (vParJoinAttrIdxs.size() == 1)
        {
            std::unordered_map<uint32_t, std::vector<uint32_t> >* phtChildOneParAttrs = (std::unordered_map<uint32_t, std::vector<uint32_t> >*)(pHashTablePt);
            delete phtChildOneParAttrs;
        }
        else if (vParJoinAttrIdxs.size() == 2)
        {
            std::unordered_map<std::tuple<uint32_t, uint32_t>,
            std::vector<uint32_t> > *phtChildTwoParAttrs = (std::unordered_map< std::tuple<uint32_t, uint32_t>, std::vector<uint32_t> >*)(pHashTablePt);
            delete phtChildTwoParAttrs;
        }
        else
        {
            // TODO: Consider how to handle this case.
            //const std::vector<double> t =
            FIGARO_LOG_DBG("Damn")
        }
    }

    Relation Relation::joinRelations(
            const std::vector<Relation*>& vpChildRels,
            const std::vector<std::string>& vJoinAttrNames,
            const std::vector<std::string>& vParJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            bool trackProvenance)
    {
         // TODO: Split work per relation
        // 1) Build a hash table for each of the children where type of a key
        // is the size of parent join attributes of the corresponding child and
        // the value is a queue of pointers to the rows with the tuples that correspond
        // 2) Scan tuples in a parent relation. In particular, for each tuple in a parent node
        // find a corresponding tuples in the children relations and output this to the join result.
        // 3)
        std::vector<uint32_t> vJoinAttrIdxs;
        std::vector<uint32_t> vParJoinAttrIdxs;
        std::vector<std::vector<uint32_t> >  vvCurJoinAttrIdxs;
        std::vector<uint32_t> vNonJoinAttrIdxs;
        std::vector<std::vector<uint32_t> > vvJoinAttrIdxs;
        std::vector<std::vector<uint32_t> > vvNonJoinAttrIdxs;
        std::vector<uint32_t> vNumJoinAttrs;
        std::vector<uint32_t> vCumNumNonJoinAttrs;
        std::vector<void*> vpHashTabQueueOffsets;
        uint32_t numParJoinAttrsCurRel;
        tbb::atomic<uint32_t> rowIdxOutAt = -1;
        std::string newRelName = "";
        std::vector<Attribute> attributes;

        numParJoinAttrsCurRel = vParJoinAttrNames.size();
        getAttributesIdxs(vJoinAttrNames, vJoinAttrIdxs);
        getAttributesIdxs(vParJoinAttrNames, vParJoinAttrIdxs);
        getAttributesIdxsComplement(vJoinAttrIdxs, vNonJoinAttrIdxs);
        vNumJoinAttrs.resize(vvJoinAttributeNames.size());
        vvJoinAttrIdxs.resize(vvJoinAttributeNames.size());
        vvNonJoinAttrIdxs.resize(vvJoinAttributeNames.size());
        vvCurJoinAttrIdxs.resize(vvJoinAttributeNames.size());
        vCumNumNonJoinAttrs.resize(vpChildRels.size());
        vpHashTabQueueOffsets.resize(vpChildRels.size());

        newRelName = m_name;
        for (const auto parJoinAttrIdx: vParJoinAttrIdxs)
        {
            attributes.push_back(m_attributes[parJoinAttrIdx]);
        }
        for (const auto nonJoinAttrIdx: vNonJoinAttrIdxs)
        {
            attributes.push_back(m_attributes[nonJoinAttrIdx]);
        }
        for (uint32_t idxRel = 0; idxRel < vvJoinAttributeNames.size(); idxRel++)
        {
            FIGARO_LOG_DBG("Building hash indices for child", idxRel)
            FIGARO_LOG_INFO("Relation name", vpChildRels[idxRel]->m_name,
            vpChildRels[idxRel]->m_attributes)
            vpChildRels[idxRel]->getAttributesIdxs(vvJoinAttributeNames[idxRel],
                vvJoinAttrIdxs[idxRel]);
            vpChildRels[idxRel]->getAttributesIdxsComplement(vvJoinAttrIdxs[idxRel], vvNonJoinAttrIdxs[idxRel]);
            getAttributesIdxs(vvJoinAttributeNames[idxRel], vvCurJoinAttrIdxs[idxRel]);
            vNumJoinAttrs[idxRel] = vvJoinAttributeNames[idxRel].size();
            initHashTableMNJoin(
                vvJoinAttrIdxs[idxRel],
                vpHashTabQueueOffsets[idxRel],
                vpChildRels[idxRel]->m_data);
             if (idxRel == 0)
            {
                vCumNumNonJoinAttrs[idxRel] = vNonJoinAttrIdxs.size();
            }
            else
            {
                vCumNumNonJoinAttrs[idxRel] = vCumNumNonJoinAttrs[idxRel-1] +
                                            vvNonJoinAttrIdxs[idxRel - 1].size();
            }

            newRelName += vpChildRels[idxRel]->m_name;
            for (const auto nonJoinAttrIdx: vvNonJoinAttrIdxs[idxRel])
            {
                attributes.push_back(vpChildRels[idxRel]->m_attributes[nonJoinAttrIdx]);

            }
        }

        MatrixDT dataOutput {130'000'000, (uint32_t)attributes.size()};
        //MatrixDT dataOutput {15, (uint32_t)attributes.size()};
        FIGARO_LOG_INFO("attributes", attributes)
        uint32_t offPar = vJoinAttrIdxs.size() - vParJoinAttrIdxs.size();
        uint32_t glCnt = 0;

        omp_set_num_threads(4);
        #pragma omp parallel for schedule(static)
        for (uint32_t rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
        {
            FIGARO_LOG_DBG("rowIdx", rowIdx, m_data.getNumRows());
            std::vector<uint32_t> naryCartesianProduct(vpChildRels.size());
            std::vector<uint32_t> nCartProdSize(vpChildRels.size());
            std::vector<std::vector<uint32_t> > vChildrenRowIdxs;
            uint32_t cnt = 1;
            for (uint32_t idxRel = 0; idxRel < vpChildRels.size(); idxRel ++)
            {
                // TODO: optimization so I return only pointer to the queue.

                const std::vector<uint32_t>& vChildRowIdxs = getHashTableMNJoin(
                                        vvCurJoinAttrIdxs[idxRel],
                                        vpHashTabQueueOffsets[idxRel],
                                        m_data[rowIdx]);

                //FIGARO_LOG_INFO("Got values from", vpChildRels[idxRel]->m_name);
                vChildrenRowIdxs.push_back(vChildRowIdxs);
                naryCartesianProduct[idxRel] = 0;
                nCartProdSize[idxRel] = vChildRowIdxs.size();
                cnt *= nCartProdSize[idxRel];
            }
            FIGARO_LOG_DBG("cnt", cnt)
            for (uint32_t cntIdx = 0; cntIdx < cnt; cntIdx ++)
            {
                uint32_t rowIdxOut = rowIdxOutAt.fetch_and_increment() + 1;
                for (const auto& joinAttrIdx: vParJoinAttrIdxs)
                {
                    dataOutput[rowIdxOut][joinAttrIdx] = m_data[rowIdx][joinAttrIdx];
                    if (rowIdx == 0)
                    {
                        FIGARO_LOG_INFO("joinAttrIdx", joinAttrIdx, "val", m_data[rowIdx][joinAttrIdx])
                    }
                }
                for (const auto& nonJoinAttrIdx: vNonJoinAttrIdxs)
                {
                    dataOutput[rowIdxOut][nonJoinAttrIdx - offPar] =
                    m_data[rowIdx][nonJoinAttrIdx];
                    if (rowIdx == 0)
                    {
                        FIGARO_LOG_INFO("colIdxOut", nonJoinAttrIdx - offPar, "nonJoinAttrIdx", nonJoinAttrIdx, "val", m_data[rowIdx][nonJoinAttrIdx])

                    }
                }

                for (uint32_t idxRel = 0; idxRel < vpChildRels.size(); idxRel ++)
                {
                    uint32_t childRowIdx =
                        vChildrenRowIdxs[idxRel][naryCartesianProduct[idxRel]];
                    const double* childRowPt = vpChildRels[idxRel]->m_data[childRowIdx];
                    // Copying data from children relations.
                    for (const auto nonJoinAttrIdx: vvNonJoinAttrIdxs[idxRel])
                    {
                        uint32_t colIdxOut = numParJoinAttrsCurRel + vCumNumNonJoinAttrs[idxRel] + nonJoinAttrIdx - vNumJoinAttrs[idxRel];
                        dataOutput[rowIdxOut][colIdxOut] = childRowPt[nonJoinAttrIdx];
                        if (rowIdx == 0)
                        {
                            FIGARO_LOG_INFO("colIdxOut", colIdxOut, "nonJoinAttrIdx", nonJoinAttrIdx, "val", dataOutput[rowIdxOut][colIdxOut])

                        }
                    }
                }
                 // Moving to next combination of tuples in a join result.
                uint32_t carry = 1;
                uint32_t writeNum;
                for (int32_t idxRel = naryCartesianProduct.size() - 1; idxRel >= 0;
                    idxRel --)
                {
                   writeNum = (naryCartesianProduct[idxRel] + carry) % nCartProdSize[idxRel];
                   FIGARO_LOG_DBG("writeNum", writeNum, "carry", carry)
                    carry =
                    (naryCartesianProduct[idxRel] + carry) / nCartProdSize[idxRel];
                    naryCartesianProduct[idxRel] = writeNum;
                }
                FIGARO_LOG_DBG("naryCartesianProduct", naryCartesianProduct)
                FIGARO_LOG_DBG("nCartProdSize", nCartProdSize)
                glCnt ++;
                FIGARO_LOG_DBG("glCnt", glCnt)

            }
            FIGARO_LOG_DBG("rowIdx", rowIdx, dataOutput)
        }
        for (uint32_t idxRel = 0; idxRel < vpChildRels.size(); idxRel++)
        {
            destroyHashTableMNJoin(vvCurJoinAttrIdxs[idxRel], vpHashTabQueueOffsets[idxRel]);
        }
        dataOutput.resize(rowIdxOutAt + 1);
        FIGARO_LOG_DBG("rowIdxOutAt", rowIdxOutAt)
        FIGARO_LOG_INFO("outputSize", dataOutput.getNumRows(), dataOutput.getNumCols())
        FIGARO_LOG_INFO(newRelName, dataOutput.getNumRows(), dataOutput.getNumCols())
        Relation joinRel = Relation("JOIN_" + newRelName, std::move(dataOutput), attributes);
        return joinRel;
    }

    void Relation::iterateOverRootRel(
            const std::vector<Relation*>& vpRels,
            const std::vector<Relation*>& vpParRels,
            uint32_t rowIdx,
            tbb::atomic<uint32_t>& atOutIdx,
            MatrixDT& dataOut,
            const std::vector<std::vector<uint32_t> >& vvJoinAttrIdxs,
            const std::vector<std::vector<uint32_t> >& vvParJoinAttrIdxs,
            const std::vector<std::vector<std::vector<uint32_t> > >& vvvChildJoinAttrIdxs,
            const std::vector<std::vector<uint32_t> >& vvNonJoinAttrIdxs,
            const std::vector<uint32_t>& vCumNonJoinAttrIdxs,
            const std::vector<uint32_t>& vParRelIdxs,
            const std::vector<void*>& vpHashTabQueueOffsets,
            bool addColumns
    )
    {
        // Iterate over
        std::vector<Relation::IteratorJoin> vIts;

        bool thereIsNext = false;
        // initIterators
        vIts.resize(vpRels.size());
         // fillOutIterators(int)
        vIts[0].m_vRowIdxs = {rowIdx};
        for (uint32_t idxRel = 1; idxRel < vIts.size(); idxRel++)
        {
            // getHashTable based on parent node
            uint32_t parIdxRel = vParRelIdxs[idxRel];
            uint32_t rowIdxPar = vIts[parIdxRel].m_vRowIdxs[0];
            // get child idx join attrnames and then pass them vvCurJoinAttrIdx
            vIts[idxRel].m_vRowIdxs = getHashTableMNJoin(vvvChildJoinAttrIdxs[parIdxRel][idxRel],
                vpHashTabQueueOffsets[idxRel], vpParRels[idxRel]->m_data[rowIdxPar]);
        }
        internalOutputTuple(vpRels, dataOut, vvJoinAttrIdxs, vvNonJoinAttrIdxs,
            vCumNonJoinAttrIdxs, vIts, atOutIdx, addColumns);

        for (int32_t idxRel = vIts.size() - 1; idxRel >= 0; idxRel--)
        {
            vIts[idxRel].next();
            if (!vIts[idxRel].isEnd())
            {
                for (uint32_t idxRelRight = (uint32_t)idxRel + 1; idxRelRight < vIts.size();
                 idxRelRight++)
                {
                    uint32_t parIdxRel = vParRelIdxs[idxRelRight];
                    uint32_t rowIdxPar = vIts[parIdxRel].getRowIdx();
                    // get child idx join attrnames and then pass them vvCurJoinAttrIdx
                    vIts[idxRelRight].m_vRowIdxs = getHashTableMNJoin(vvvChildJoinAttrIdxs[parIdxRel][idxRelRight],
                        vpHashTabQueueOffsets[idxRelRight], vpParRels[idxRelRight]->m_data[rowIdxPar]);
                    vIts[idxRelRight].m_idx = 0;
                }
                thereIsNext = true;
                break;
            }
        }
        while (thereIsNext)
        {
            internalOutputTuple(vpRels, dataOut, vvJoinAttrIdxs, vvNonJoinAttrIdxs,
                vCumNonJoinAttrIdxs, vIts, atOutIdx, addColumns);
            thereIsNext = false;
            for (int32_t idxRel = vIts.size() - 1; idxRel >= 0; idxRel--)
            {
                vIts[idxRel].next();
                if (!vIts[idxRel].isEnd())
                {
                    for (uint32_t idxRelRight = (uint32_t)idxRel + 1; idxRelRight < vIts.size();
                    idxRelRight++)
                    {
                        uint32_t parIdxRel = vParRelIdxs[idxRelRight];
                        uint32_t rowIdxPar = vIts[parIdxRel].getRowIdx();
                        // get child idx join attrnames and then pass them vvCurJoinAttrIdx
                        vIts[idxRelRight].m_vRowIdxs = getHashTableMNJoin(vvvChildJoinAttrIdxs[parIdxRel][idxRelRight],
                            vpHashTabQueueOffsets[idxRelRight], vpParRels[idxRelRight]->m_data[rowIdxPar]);
                        vIts[idxRelRight].m_idx = 0;
                    }
                    thereIsNext = true;
                    break;
                }
            }
        }
    }

    Relation Relation::internalJoinRelations(const std::vector<Relation*>& vpRels,
            const std::vector<Relation*>& vpParRels,
            const std::vector<std::vector<std::string> >& vvJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvParJoinAttrNames,
            uint32_t joinSize,
            bool addColumns)
    {
        // Iterate over
        std::vector<IteratorJoin> vIts;
        std::vector<std::vector<uint32_t> > vvJoinAttrIdxs;
        std::vector<std::vector<uint32_t> > vvParJoinAttrIdxs;
        std::vector<std::vector<std::vector<uint32_t> > > vvvChildJoinAttrIdxs;
        std::vector<std::vector<uint32_t> > vvNonJoinAttrIdxs;
        std::vector<uint32_t> vCumNonJoinAttrIdxs;
        std::unordered_map<std::string, uint32_t> mParRelNameIdx;

        std::vector<void*> vpHashTabQueueOffsets;
        std::vector<uint32_t> vParRelIdxs;
        std::string newName;
        std::vector<Attribute> newAttributes;

        tbb::atomic<uint32_t> outIdx;

        vvJoinAttrIdxs.resize(vpRels.size());
        vvParJoinAttrIdxs.resize(vpRels.size());
        vvvChildJoinAttrIdxs.resize(vpRels.size());
        vvNonJoinAttrIdxs.resize(vpRels.size());
        vpHashTabQueueOffsets.resize(vpRels.size());
        vParRelIdxs.resize(vpRels.size());
        vCumNonJoinAttrIdxs.resize(vpRels.size());

        mParRelNameIdx[vpRels[0]->m_name] = 0;
        vCumNonJoinAttrIdxs[0] = 0;
        newName = vpRels[0]->m_name;

        // build hash tables for all relations except root;
        for (uint32_t idxRel = 0; idxRel < vpRels.size(); idxRel++)
        {
            mParRelNameIdx[vpRels[idxRel]->m_name] = idxRel;
            vpRels[idxRel]->getAttributesIdxs(vvJoinAttrNames[idxRel], vvJoinAttrIdxs[idxRel]);
            vpRels[idxRel]->getAttributesIdxsComplement(vvJoinAttrIdxs[idxRel], vvNonJoinAttrIdxs[idxRel]);
            vpRels[idxRel]->getAttributesIdxs(vvParJoinAttrNames[idxRel], vvParJoinAttrIdxs[idxRel]);
            newName += vpRels[idxRel]->m_name;
            if (!addColumns)
            {
                newAttributes.insert(newAttributes.end(),
                    vpRels[idxRel]->m_attributes.begin() + vvNonJoinAttrIdxs[idxRel][0],
                    vpRels[idxRel]->m_attributes.end());
            }
            else if (idxRel == 0)
            {
                newAttributes.insert(newAttributes.end(),
                    vpRels[idxRel]->m_attributes.begin() + vvNonJoinAttrIdxs[idxRel][0],
                    vpRels[idxRel]->m_attributes.end());
            }
            if (idxRel > 0)
            {
                initHashTableMNJoin(
                    vvParJoinAttrIdxs[idxRel],
                    vpHashTabQueueOffsets[idxRel],
                    vpRels[idxRel]->m_data);
                vCumNonJoinAttrIdxs[idxRel] = vCumNonJoinAttrIdxs[idxRel-1] + vvNonJoinAttrIdxs[idxRel-1].size();
            }
            vvvChildJoinAttrIdxs[idxRel].resize(vpRels.size());
            for (uint32_t idxRelChild = 0; idxRelChild < vpRels.size(); idxRelChild++)
            {
                std::string parName = "";
                if (vpParRels[idxRelChild] != nullptr)
                {
                    parName = vpParRels[idxRelChild]->m_name;
                }
                if (parName == vpRels[idxRel]->m_name)
                {
                    vpRels[idxRel]->getAttributesIdxs(vvParJoinAttrNames[idxRelChild],
                        vvvChildJoinAttrIdxs[idxRel][idxRelChild]);
                }
            }

        }


        for (uint32_t idxRel = 1; idxRel < vParRelIdxs.size(); idxRel++)
        {
            vParRelIdxs[idxRel] = mParRelNameIdx[vpParRels[idxRel]->m_name];
        }

        outIdx = -1;
        MatrixDT dataOutput {joinSize, (uint32_t)(newAttributes.size())};

        omp_set_num_threads(4);
        //MICRO_BENCH_INIT(iterateOverRootRelTimer)
        //MICRO_BENCH_START(iterateOverRootRelTimer)
        #pragma omp parallel for schedule(static)
        for (uint32_t rowIdx = 0; rowIdx < vpRels[0]->m_data.getNumRows(); rowIdx++)
        {
            iterateOverRootRel(vpRels, vpParRels, rowIdx, outIdx,
                dataOutput, vvJoinAttrIdxs, vvParJoinAttrIdxs,
                vvvChildJoinAttrIdxs,
                vvNonJoinAttrIdxs, vCumNonJoinAttrIdxs, vParRelIdxs,
                vpHashTabQueueOffsets, addColumns);
        }
        //MICRO_BENCH_STOP(iterateOverRootRelTimer)
        //FIGARO_LOG_BENCH("join and add columns", MICRO_BENCH_GET_TIMER_LAP(iterateOverRootRelTimer))

        for (uint32_t idxRel = 0; idxRel < vpRels.size(); idxRel++)
        {
            destroyHashTableMNJoin(vvParJoinAttrIdxs[idxRel], vpHashTabQueueOffsets[idxRel]);
        }
        return Relation("JOIN_" + newName, std::move(dataOutput), newAttributes);
    }


    Relation Relation::joinRelations(
            const std::vector<Relation*>& vpRels,
            const std::vector<Relation*>& vpParRels,
            const std::vector<std::vector<std::string> >& vvJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvParJoinAttrNames,
            uint32_t joinSize)
    {
        return internalJoinRelations(vpRels, vpParRels,
            vvJoinAttrNames, vvParJoinAttrNames, joinSize, false);
    }


    Relation Relation::joinRelationsAndAddColumns(
            const std::vector<Relation*>& vpChildRels,
            const std::vector<std::string>& vJoinAttrNames,
            const std::vector<std::string>& vParJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
            bool trackProvenance)
    {
         // TODO: Split work per relation
        // 1) Build a hash table for each of the children where type of a key
        // is the size of parent join attributes of the corresponding child and
        // the value is a queue of pointers to the rows with the tuples that correspond
        // 2) Scan tuples in a parent relation. In particular, for each tuple in a parent node
        // find a corresponding tuples in the children relations and output this to the join result.
        // 3)
        std::vector<uint32_t> vJoinAttrIdxs;
        std::vector<uint32_t> vParJoinAttrIdxs;
        std::vector<std::vector<uint32_t> >  vvCurJoinAttrIdxs;
        std::vector<uint32_t> vNonJoinAttrIdxs;
        std::vector<std::vector<uint32_t> > vvJoinAttrIdxs;
        std::vector<std::vector<uint32_t> > vvNonJoinAttrIdxs;
        std::vector<uint32_t> vNumJoinAttrs;
        std::vector<uint32_t> vCumNumNonJoinAttrs;
        std::vector<void*> vpHashTabQueueOffsets;
        uint32_t numParJoinAttrsCurRel;
        tbb::atomic<uint32_t> rowIdxOutAt = -1;
        std::string newRelName = "";
        std::vector<Attribute> attributes;

        numParJoinAttrsCurRel = vParJoinAttrNames.size();
        getAttributesIdxs(vJoinAttrNames, vJoinAttrIdxs);
        getAttributesIdxs(vParJoinAttrNames, vParJoinAttrIdxs);
        getAttributesIdxsComplement(vJoinAttrIdxs, vNonJoinAttrIdxs);
        vNumJoinAttrs.resize(vvJoinAttributeNames.size());
        vvJoinAttrIdxs.resize(vvJoinAttributeNames.size());
        vvNonJoinAttrIdxs.resize(vvJoinAttributeNames.size());
        vvCurJoinAttrIdxs.resize(vvJoinAttributeNames.size());
        vCumNumNonJoinAttrs.resize(vpChildRels.size());
        vpHashTabQueueOffsets.resize(vpChildRels.size());

        newRelName = m_name;
        for (const auto parJoinAttrIdx: vParJoinAttrIdxs)
        {
            attributes.push_back(m_attributes[parJoinAttrIdx]);
        }
        for (const auto nonJoinAttrIdx: vNonJoinAttrIdxs)
        {
            attributes.push_back(m_attributes[nonJoinAttrIdx]);
        }
        for (uint32_t idxRel = 0; idxRel < vvJoinAttributeNames.size(); idxRel++)
        {
            FIGARO_LOG_DBG("Building hash indices for child", idxRel)
            FIGARO_LOG_INFO("Relation name", vpChildRels[idxRel]->m_name,
            vpChildRels[idxRel]->m_attributes)
            vpChildRels[idxRel]->getAttributesIdxs(vvJoinAttributeNames[idxRel],
                vvJoinAttrIdxs[idxRel]);
            vpChildRels[idxRel]->getAttributesIdxsComplement(vvJoinAttrIdxs[idxRel], vvNonJoinAttrIdxs[idxRel]);
            getAttributesIdxs(vvJoinAttributeNames[idxRel], vvCurJoinAttrIdxs[idxRel]);
            vNumJoinAttrs[idxRel] = vvJoinAttributeNames[idxRel].size();
            initHashTableMNJoin(
                vvJoinAttrIdxs[idxRel],
                vpHashTabQueueOffsets[idxRel],
                vpChildRels[idxRel]->m_data);
             if (idxRel == 0)
            {
                vCumNumNonJoinAttrs[idxRel] = vNonJoinAttrIdxs.size();
            }
            else
            {
                vCumNumNonJoinAttrs[idxRel] = vCumNumNonJoinAttrs[idxRel-1] +
                                            vvNonJoinAttrIdxs[idxRel - 1].size();
            }

            newRelName += vpChildRels[idxRel]->m_name;
        }

        MatrixDT dataOutput {130'000'000, (uint32_t)attributes.size()};
        //MatrixDT dataOutput {15, (uint32_t)attributes.size()};
        FIGARO_LOG_INFO("attributes", attributes)
        uint32_t offPar = vJoinAttrIdxs.size() - vParJoinAttrIdxs.size();
        uint32_t glCnt = 0;

        omp_set_num_threads(4);
        #pragma omp parallel for schedule(static)
        for (uint32_t rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
        {
            FIGARO_LOG_DBG("rowIdx", rowIdx, m_data.getNumRows());
            std::vector<uint32_t> naryCartesianProduct(vpChildRels.size());
            std::vector<uint32_t> nCartProdSize(vpChildRels.size());
            std::vector<std::vector<uint32_t> > vChildrenRowIdxs;
            uint32_t cnt = 1;
            for (uint32_t idxRel = 0; idxRel < vpChildRels.size(); idxRel ++)
            {
                // TODO: optimization so I return only pointer to the queue.

                const std::vector<uint32_t>& vChildRowIdxs = getHashTableMNJoin(
                                        vvCurJoinAttrIdxs[idxRel],
                                        vpHashTabQueueOffsets[idxRel],
                                        m_data[rowIdx]);

                //FIGARO_LOG_INFO("Got values from", vpChildRels[idxRel]->m_name);
                vChildrenRowIdxs.push_back(vChildRowIdxs);
                naryCartesianProduct[idxRel] = 0;
                nCartProdSize[idxRel] = vChildRowIdxs.size();
                cnt *= nCartProdSize[idxRel];
            }
            FIGARO_LOG_DBG("cnt", cnt)
            for (uint32_t cntIdx = 0; cntIdx < cnt; cntIdx ++)
            {
                uint32_t rowIdxOut = rowIdxOutAt.fetch_and_increment() + 1;
                for (const auto& joinAttrIdx: vParJoinAttrIdxs)
                {
                    dataOutput[rowIdxOut][joinAttrIdx] = m_data[rowIdx][joinAttrIdx];
                }
                for (const auto& nonJoinAttrIdx: vNonJoinAttrIdxs)
                {
                    dataOutput[rowIdxOut][nonJoinAttrIdx - offPar] =
                    m_data[rowIdx][nonJoinAttrIdx];
                }

                for (uint32_t idxRel = 0; idxRel < vpChildRels.size(); idxRel ++)
                {
                    uint32_t childRowIdx =
                        vChildrenRowIdxs[idxRel][naryCartesianProduct[idxRel]];
                    const double* childRowPt = vpChildRels[idxRel]->m_data[childRowIdx];
                    // Copying data from children relations.
                    for (const auto nonJoinAttrIdx: vvNonJoinAttrIdxs[idxRel])
                    {
                        uint32_t colIdxOut = numParJoinAttrsCurRel + nonJoinAttrIdx - vNumJoinAttrs[idxRel];
                        dataOutput[rowIdxOut][colIdxOut] += childRowPt[nonJoinAttrIdx];
                        if (rowIdx == 0)
                        {
                            FIGARO_LOG_INFO("colIdxOut", colIdxOut, "nonJoinAttrIdx", nonJoinAttrIdx, "val", dataOutput[rowIdxOut][colIdxOut])

                        }
                    }
                }
                 // Moving to next combination of tuples in a join result.
                uint32_t carry = 1;
                uint32_t writeNum;
                for (int32_t idxRel = naryCartesianProduct.size() - 1; idxRel >= 0;
                    idxRel --)
                {
                   writeNum = (naryCartesianProduct[idxRel] + carry) % nCartProdSize[idxRel];
                   FIGARO_LOG_DBG("writeNum", writeNum, "carry", carry)
                    carry =
                    (naryCartesianProduct[idxRel] + carry) / nCartProdSize[idxRel];
                    naryCartesianProduct[idxRel] = writeNum;
                }
                FIGARO_LOG_DBG("naryCartesianProduct", naryCartesianProduct)
                FIGARO_LOG_DBG("nCartProdSize", nCartProdSize)
                glCnt ++;
                FIGARO_LOG_DBG("glCnt", glCnt)

            }
            FIGARO_LOG_DBG("rowIdx", rowIdx, dataOutput)
        }
        for (uint32_t idxRel = 0; idxRel < vpChildRels.size(); idxRel++)
        {
            destroyHashTableMNJoin(vvCurJoinAttrIdxs[idxRel], vpHashTabQueueOffsets[idxRel]);
        }
        dataOutput.resize(rowIdxOutAt + 1);
        FIGARO_LOG_DBG("rowIdxOutAt", rowIdxOutAt)
        FIGARO_LOG_INFO("outputSize", dataOutput.getNumRows(), dataOutput.getNumCols())
        FIGARO_LOG_INFO(newRelName, dataOutput.getNumRows(), dataOutput.getNumCols())
        Relation joinRel = Relation("JOIN_" + newRelName, std::move(dataOutput), attributes);
        return joinRel;
    }

    Relation Relation::joinRelationsAndAddColumns(
            const std::vector<Relation*>& vpRels,
            const std::vector<Relation*>& vpParRels,
            const std::vector<std::vector<std::string> >& vvJoinAttrNames,
            const std::vector<std::vector<std::string> >& vvParJoinAttrNames,
            uint32_t joinSize)
    {
        //uint32_t oldThreads = getNumberOfThreads();
        //omp_set_num_threads(4);
        //MICRO_BENCH_INIT(computeJoinRelationsAndAddColumns)
        //MICRO_BENCH_START(computeJoinRelationsAndAddColumns)
        return internalJoinRelations(vpRels, vpParRels,
            vvJoinAttrNames, vvParJoinAttrNames, joinSize, true);
        //omp_set_num_threads(oldThreads);
        //MICRO_BENCH_STOP(computeJoinRelationsAndAddColumns)
        //FIGARO_LOG_BENCH("Compute joinRelationsAndAddColumns", MICRO_BENCH_GET_TIMER_LAP(computeJoinRelationsAndAddColumns))
        //return r;
    }


    Relation Relation::addRelation(const Relation& second,
            const std::vector<std::string>& vJoinAttrNames1,
            const std::vector<std::string>& vJoinAttrNames2) const
    {
        uint32_t joinAttrSize1;
        uint32_t joinAttrSize2;
        std::vector<Attribute> newAttributes;

        joinAttrSize1 = vJoinAttrNames1.size();
        joinAttrSize2 = vJoinAttrNames2.size();

        auto result = m_data.add(second.m_data, joinAttrSize1, joinAttrSize2);
        for (uint32_t attrIdx = 0; attrIdx < vJoinAttrNames1.size(); attrIdx++)
        {
            newAttributes.push_back(m_attributes.at(attrIdx));
        }
        newAttributes.insert(newAttributes.end(),
        second.m_attributes.begin() + joinAttrSize2, second.m_attributes.end());

        return Relation("ADD_" + getName() + second.getName(), std::move(result),
            newAttributes);
    }

    Relation Relation::subtractRelation(const Relation& second,
            const std::vector<std::string>& vJoinAttrNames1,
            const std::vector<std::string>& vJoinAttrNames2) const
    {
        uint32_t joinAttrSize1;
        uint32_t joinAttrSize2;
        std::vector<Attribute> newAttributes;

        joinAttrSize1 = vJoinAttrNames1.size();
        joinAttrSize2 = vJoinAttrNames2.size();

        auto result = m_data.subtract(second.m_data, joinAttrSize1, joinAttrSize2);
        for (uint32_t attrIdx = 0; attrIdx < vJoinAttrNames1.size(); attrIdx++)
        {
            newAttributes.push_back(m_attributes.at(attrIdx));
        }
        newAttributes.insert(newAttributes.end(),
        second.m_attributes.begin() + joinAttrSize2, second.m_attributes.end());

        return Relation("SUB_" + getName() + second.getName(), std::move(result),
            newAttributes);
    }

    Relation Relation::multiply(const Relation& second,
            const std::vector<std::string>& vJoinAttrNames1,
            const std::vector<std::string>& vJoinAttrNames2,
            uint32_t startRowIdx2) const
    {
        uint32_t joinAttrSize1;
        uint32_t joinAttrSize2;
        std::vector<Attribute> newAttributes;

        joinAttrSize1 = vJoinAttrNames1.size();
        joinAttrSize2 = vJoinAttrNames2.size();

        auto result = m_data.multiply(second.m_data, joinAttrSize1, joinAttrSize2,
             startRowIdx2);
        // TODO: Refactor
        for (uint32_t attrIdx = 0; attrIdx < vJoinAttrNames1.size(); attrIdx++)
        {
            newAttributes.push_back(m_attributes.at(attrIdx));
        }
        newAttributes.insert(newAttributes.end(), second.m_attributes.begin(), second.m_attributes.end());

        return Relation("MUL_" + getName() + second.getName(), std::move(result),
            newAttributes);
    }


    Relation Relation::selfMatrixMultiply(
        const std::vector<std::string>& vJoinAttrNames) const
    {
        auto result = m_data.selfMatrixMultiply(vJoinAttrNames.size());

        return Relation("MUL_" + getName() + getName(), std::move(result),
            m_attributes);
    }


    Relation Relation::linearRegression(
            const std::string& labelName) const
    {
        uint32_t labelIdx = getAttributeIdx(labelName);
        uint32_t numAttrs = m_attributes.size();

        // multiply R by itself.
        auto smmMat = m_data.selfMatrixMultiply(0);
        MatrixDT APrimeB{numAttrs - 1, 1};
        MatrixDT APrimeA{numAttrs - 1, numAttrs - 1};

        FIGARO_LOG_INFO("smmMat", smmMat);
        FIGARO_LOG_INFO("labelIdx", labelIdx);

        // extract A' * b
        for (uint32_t idx = 0, curIdx = 0; idx < smmMat.getNumRows(); idx++)
        {
            if (idx == labelIdx)
            {
                continue;
            }
            APrimeB[curIdx][0] = smmMat[labelIdx][idx];
            curIdx++;
        }

        FIGARO_LOG_INFO("APrimeB", APrimeB);

        // extract A ' * A
        for (uint32_t rowIdx = 0, curRowIdx=0; rowIdx < smmMat.getNumRows(); rowIdx++  )
        {
            if (rowIdx == labelIdx)
            {
                continue;
            }
            for (uint32_t colIdx = 0, curColIdx=0; colIdx < smmMat.getNumCols(); colIdx++)
            {
                if (colIdx == labelIdx)
                {
                    continue;
                }
                APrimeA[curRowIdx][curColIdx] = smmMat[rowIdx][colIdx];
                curColIdx ++;
            }
            curRowIdx ++;
        }

        FIGARO_LOG_INFO("APrimeA", APrimeA);

        // linear regression
        auto resX = APrimeA.computeInverse().multiply(APrimeB, 0, 0);
        FIGARO_LOG_BENCH("resX", resX);

        return Relation("LIN_REG" + getName() + labelName, std::move(resX),
            {Attribute()});


    }

    double Relation::norm(
            const std::vector<std::string>& vJoinAttrNames) const
    {
        auto result = m_data.norm(vJoinAttrNames.size());
        return result;
    }

    double Relation::checkOrthogonality(const std::vector<std::string>& vJoinAttrNames) const
    {
        FIGARO_LOG_INFO("m_attributes", m_attributes)
        auto result = m_data.selfMatrixMultiply(vJoinAttrNames.size());
        auto eye = MatrixDT::identity(result.getNumRows());
        auto diff = eye.subtract(result, 0, vJoinAttrNames.size());
        FIGARO_LOG_INFO("diff norm", eye.norm(0))
        FIGARO_LOG_INFO("diff norm", result)
        FIGARO_LOG_INFO("diff norm", vJoinAttrNames.size())
        FIGARO_LOG_BENCH("Dimensions", m_data.getNumRows(), m_data.getNumCols())
        return diff.norm(vJoinAttrNames.size()) / eye.norm(0);

    }

    Relation Relation::inverse(
        const std::vector<std::string>& vJoinAttrNames
    ) const
    {
        auto result = m_data.computeInverse(vJoinAttrNames.size(), true);
        return Relation("INV_" + getName(), std::move(result), m_attributes);
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
        // TEMPORARY HACK
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
                //MICRO_BENCH_INIT(testStupidLoop)
                //MICRO_BENCH_START(testStupidLoop)
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
                //MICRO_BENCH_STOP(testStupidLoop)
                //FIGARO_LOG_BENCH("Stupid loop", MICRO_BENCH_GET_TIMER_LAP(testStupidLoop))

                distCnt = 0;
                pParPrevAttrVals = &vParPrevAttrVals[0];

                vParBlockStartIdxsAfterFirstPass.resize(vParBlockStartIdxs.size());
                //MICRO_BENCH_INIT(generalLoops)
                //MICRO_BENCH_START(generalLoops)
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
                        for (const auto& joinAttrIdx: vJoinAttrIdxs)
                        {
                            cntJoinVals[rowIdx][joinAttrIdx] = (uint32_t)m_data[rowIdx][joinAttrIdx];
                        }
                        // We have a new block of join attributes. Stores the block size.
                        cntJoinVals[rowIdx][m_cntsJoinIdxV] = 1;
                        cntJoinVals[rowIdx ][m_cntsJoinIdxE] = rowIdx + 1;
                    }
                    // Dummy indices
                }
                //MICRO_BENCH_STOP(generalLoops)
                //FIGARO_LOG_BENCH("General loop", MICRO_BENCH_GET_TIMER_LAP(generalLoops))
                distCnt = m_data.getNumRows();
                vParBlockStartIdxsAfterFirstPass.back() = distCnt;
                //cntJoinVals.resize(distCnt);
                FIGARO_LOG_DBG("cntJoinVals", m_name, cntJoinVals)
            }
            else
            {
                // TODO: build in parallel
                for (rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
                {
                    const double* pCurAttrVals = m_data[rowIdx];
                    bool parAttrsDiff = compareTuples(pCurAttrVals, pParPrevAttrVals, vParAttrIdxs);
                    bool joinAttrsDiff = compareTuples(pCurAttrVals, pPrevAttrVals, vJoinAttrIdxs);

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


    void Relation::initHashTable(const std::vector<uint32_t>& vParAttrIdx,
        uint32_t hashTableSize,
        std::shared_ptr<void>& pSharedPtr)
    {
        if (vParAttrIdx.size() == 0)
        {
            // Do not do anything. This is a root node.
        }
        if (vParAttrIdx.size() == 1)
        {
            pSharedPtr = std::make_unique< tbb::concurrent_unordered_map<uint32_t, DownUpCntT> >();
        }
        else if (vParAttrIdx.size() == 2)
        {

            pSharedPtr = std::make_unique
                <tbb::concurrent_unordered_map<std::tuple<uint32_t, uint32_t>, DownUpCntT > > ();
        }
        else
        {

        }
    }

    // TODO: Convert this to a template.
    void Relation::initHashTableRowIdxs(
        const std::vector<uint32_t>& vParAttrIdx,
        const MatrixDT& data,
        void*& pHashTablePt
        )
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

    void Relation::buildIndices(
        const std::vector<std::string>& vJoinAttrNames,
        const std::vector<std::string>& vParJoinAttrNames,
        bool isRootNode)
    {
        std::vector<uint32_t> vJoinAttrIdxs;
        std::vector<uint32_t> vParJoinAttrIdxs;
        std::vector<uint32_t> vParBlockStartIdxs;
        std::vector<uint32_t> vParBlockStartIdxsAfterFirstPass;

        getAttributesIdxs(vJoinAttrNames, vJoinAttrIdxs);
        getAttributesIdxs(vParJoinAttrNames, vParJoinAttrIdxs);

        MatrixUI32T cntsJoin {m_data.getNumRows(), vJoinAttrNames.size() + 4};
        vParBlockStartIdxs.reserve(m_data.getNumRows());
        vParBlockStartIdxsAfterFirstPass.reserve(m_data.getNumRows());

        m_cntsJoinIdxE = cntsJoin.getNumCols() - 4;
        m_cntsJoinIdxC = cntsJoin.getNumCols() - 3;
        m_cntsJoinIdxD = cntsJoin.getNumCols() - 2;
        m_cntsJoinIdxV = cntsJoin.getNumCols() - 1;

        //MICRO_BENCH_INIT(indicesComp)
        //MICRO_BENCH_START(indicesComp)
        getDistinctValsAndBuildIndices(vJoinAttrIdxs, vParJoinAttrIdxs, cntsJoin,
            vParBlockStartIdxs, vParBlockStartIdxsAfterFirstPass, isRootNode);

        m_countsJoinAttrs = std::move(cntsJoin);
        m_vParBlockStartIdxs = std::move(vParBlockStartIdxs);
        m_vParBlockStartIdxsAfterFirstPass = std::move(vParBlockStartIdxsAfterFirstPass);
        //MICRO_BENCH_STOP(indicesComp)
        //FIGARO_LOG_BENCH("Indices building" + m_name, MICRO_BENCH_GET_TIMER_LAP(indicesComp))
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
        //MICRO_BENCH_INIT(pureDownCnt)
        /*
        ////MICRO_BENCH_INIT(hashTable)
        */
        numDistParVals = m_vParBlockStartIdxs.size() - 1;
        ////MICRO_BENCH_START(hashTable)
        initHashTable(vParJoinAttrIdxs, numDistParVals, m_pHTParCounts);
        ////MICRO_BENCH_STOP(hashTable)
        ////FIGARO_LOG_BENCH("Figaro hashTable computation", m_name, MICRO_BENCH_GET_TIMER_LAP(hashTable))

        //MICRO_BENCH_START(pureDownCnt)
        if (isRootNode)
        {
            #pragma omp parallel for schedule(static)
            for (uint32_t distCnt = 0;
                distCnt < m_countsJoinAttrs.getNumRows(); distCnt++)
            {
                m_countsJoinAttrs[distCnt][m_cntsJoinIdxC] = m_countsJoinAttrs[distCnt][m_cntsJoinIdxV];

                // SELECT COUNT(*) JOIN RELATIONS IN SUBTREE
                // WHERE join_attributes = current join attribute
                uint32_t curDownCnt = m_countsJoinAttrs[distCnt][m_cntsJoinIdxV];
                for (uint32_t idxChild = 0; idxChild < vpChildRels.size(); idxChild++)
                {
                    FIGARO_LOG_DBG("CHILD", vpChildRels[idxChild]->m_name)
                    Figaro::Relation::DownUpCntT& cnts =
                        getParCntFromHashTable(
                            vvCurJoinAttrIdxs[idxChild],
                            vpChildRels[idxChild]->m_pHTParCounts.get(),
                            m_countsJoinAttrs[distCnt]);
                    uint32_t downCnt = std::get<0>(cnts);
                    curDownCnt *= downCnt;
                }
                // J_i
                m_countsJoinAttrs[distCnt][m_cntsJoinIdxD] = curDownCnt;
            }
        }
        else
        {
            #pragma omp parallel for schedule(static)
            for (uint32_t distCntPar = 0; distCntPar < numDistParVals; distCntPar++)
            {
                uint32_t sum = 0;
                uint32_t parCurBlockStartIdx = m_vParBlockStartIdxs[distCntPar];
                uint32_t parNextBlockStartIdx = m_vParBlockStartIdxs[distCntPar + 1];
                // Distinct parent attribute
                for (uint32_t distCnt = parCurBlockStartIdx;
                    distCnt < parNextBlockStartIdx; distCnt++)
                {
                    m_countsJoinAttrs[distCnt][m_cntsJoinIdxC] = m_countsJoinAttrs[distCnt][m_cntsJoinIdxV];


                    // SELECT COUNT(*) JOIN RELATIONS IN SUBTREE
                    // WHERE join_attributes = current join attribute
                    uint32_t curDownCnt = m_countsJoinAttrs[distCnt][m_cntsJoinIdxV];
                    for (uint32_t idxChild = 0; idxChild < vpChildRels.size(); idxChild++)
                    {
                        Figaro::Relation::DownUpCntT& cnts =
                            getParCntFromHashTable(
                                vvCurJoinAttrIdxs[idxChild],
                                vpChildRels[idxChild]->m_pHTParCounts.get(),
                                m_countsJoinAttrs[distCnt]);
                        uint32_t downCnt = std::get<0>(cnts);
                        curDownCnt *= downCnt;
                    }
                    m_countsJoinAttrs[distCnt][m_cntsJoinIdxD] = curDownCnt;
                    sum += curDownCnt;

                }
                insertParDownCntFromHashTable(vParJoinAttrIdxs,
                    m_countsJoinAttrs[parCurBlockStartIdx], sum, m_pHTParCounts);
            }
        }
        //MICRO_BENCH_STOP(pureDownCnt)
        //FIGARO_LOG_BENCH("Figaro down count" + m_name, MICRO_BENCH_GET_TIMER_LAP(pureDownCnt))
    }

    uint32_t Relation::getDownCountSum(void) const
    {
        uint32_t downCountSum = 0;
        for (uint32_t distCnt = 0; distCnt < m_countsJoinAttrs.getNumRows(); distCnt++)
        {
            downCountSum += m_countsJoinAttrs[distCnt][m_cntsJoinIdxD];
        }
        return downCountSum;
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
        //MICRO_BENCH_INIT(pureUpCnt)
        //MICRO_BENCH_START(pureUpCnt)
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
        //MICRO_BENCH_STOP(pureUpCnt)
        //FIGARO_LOG_BENCH("Figaro pure up count" + m_name, MICRO_BENCH_GET_TIMER_LAP(pureUpCnt))
    }

   // We assume join attributes are before nonJoinAttributes.
    std::tuple<Relation, Relation> Relation::computeHeadsAndTails(
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

            headRowIdx = (distCnt == 0) ? 0 : m_countsJoinAttrs[distCnt - 1][m_cntsJoinIdxE];
            nextHeadRowIdx = m_countsJoinAttrs[distCnt][m_cntsJoinIdxE];
            numDistVals =  m_countsJoinAttrs[distCnt][m_cntsJoinIdxV];

             // Copy join attributes to be used as indices.
            for (const uint32_t joinAttrIdx: vJoinAttrIdxs)
            {
                dataHeads[distCnt][joinAttrIdx] = m_data[headRowIdx][joinAttrIdx];
            }

            for (const uint32_t nonJoinAttrIdx: vNonJoinAttrIdxs)
            {
                dataHeads[distCnt][nonJoinAttrIdx] = m_data[headRowIdx][nonJoinAttrIdx];
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
                    prevRowSum = dataHeads[distCnt][nonJoinAttrIdx];
                    dataHeads[distCnt][nonJoinAttrIdx] += m_data[rowIdx][nonJoinAttrIdx];
                    tailVal = (m_data[rowIdx][nonJoinAttrIdx] * (i - 1) - prevRowSum)
                    / std::sqrt(i * (i - 1));
                    dataTails[tailRowIdx][nonJoinAttrIdx - numJoinAttrs] =
                        tailVal * sqrtCircCnt;
                }
            }

            // TODO: Push square root as late as possible.
            scale[distCnt][0] = std::sqrt(numDistVals);
            dataScale[distCnt][0] = 1 / std::sqrt(numDistVals);
            allScales[distCnt] = scale[distCnt][0];
        }

        m_scales = std::move(scale);
        m_dataScales = std::move(dataScale);
        m_allScales = std::move(allScales);

        m_vSubTreeDataOffsets.push_back(vJoinAttrNames.size());
        Relation relHeads("HEAD_" + m_name, std::move(dataHeads), m_attributes);
        Relation relTails("TAIL_" + m_name, std::move(dataTails), m_attributes);

        FIGARO_LOG_INFO("number of rows", relHeads.m_data.getNumRows(), "num cols", relTails.m_data.getNumCols());

        return std::make_tuple(std::move(relHeads), std::move(relTails));
    }

    void Relation::schemaJoins(
        std::vector<Attribute>& attributes,
        const std::vector<Relation*>& vpChildRels,
        const std::vector<Relation*>& vpChildHeadRels,
        const std::vector<std::vector<uint32_t> >& vvJoinAttrIdxs,
        const std::vector<std::vector<uint32_t> >& vvNonJoinAttrIdxs)
    {
        for (uint32_t idxRel = 0; idxRel < vpChildRels.size(); idxRel++)
        {
            uint32_t prevSize = attributes.size();
            uint32_t prevvSTDOffsSize = m_vSubTreeDataOffsets.size();
            for (const auto nonJoinAttrIdx: vvNonJoinAttrIdxs[idxRel])
            {
                attributes.push_back(vpChildHeadRels[idxRel]->m_attributes[nonJoinAttrIdx]);
            }

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
    }

    void Relation::schemaRemoveNonParJoinAttrs(
            std::vector<Attribute>& attributes,
            const std::vector<uint32_t>& vJoinAttrIdxs,
            const std::vector<uint32_t>& vParJoinAttrIdxs)
    {
        // We have assumption here that the parent join attribute will be always the first one.
        attributes.erase(attributes.begin() + vParJoinAttrIdxs.size(),
            attributes.begin() + vJoinAttrIdxs.size());
    }

    Relation Relation::aggregateAwayChildrenRelations(
        Relation* pHeadRel,
        const std::vector<Relation*>& vpChildRels,
        const std::vector<Relation*>& vpChildHeadRels,
        const std::vector<std::string>& vJoinAttributeNames,
        const std::vector<std::vector<std::string> >& vvJoinAttributeNames,
        const std::vector<std::string>& vSubTreeRelNames,
        const std::vector<std::vector<std::string> >& vvSubTreeRelnames)
    {
        uint32_t numJoinAttrsCurRel;
        std::vector<uint32_t> vJoinAttrIdxs;
        std::vector<std::vector<uint32_t> >  vvCurJoinAttrIdxs;
        std::vector<uint32_t> vNonJoinAttrIdxs;
        std::vector<std::vector<uint32_t> > vvJoinAttrIdxs;
        std::vector<std::vector<uint32_t> > vvNonJoinAttrIdxs;
        std::vector<uint32_t> vNumJoinAttrs;
        std::vector<Attribute> vAttrsAggAway;

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

        pHeadRel->getAttributesIdxs(vJoinAttributeNames, vJoinAttrIdxs);
        pHeadRel->getAttributesIdxsComplement(vJoinAttrIdxs, vNonJoinAttrIdxs);
        vAttrsAggAway = pHeadRel->m_attributes;
        //MICRO_BENCH_INIT(aggregateAway)
        //MICRO_BENCH_START(aggregateAway)

        for (uint32_t idxRel = 0; idxRel < vvJoinAttributeNames.size(); idxRel++)
        {
            vpChildHeadRels[idxRel]->getAttributesIdxs(vvJoinAttributeNames[idxRel],
                vvJoinAttrIdxs[idxRel]);
            vpChildHeadRels[idxRel]->getAttributesIdxsComplement(vvJoinAttrIdxs[idxRel], vvNonJoinAttrIdxs[idxRel]);
            pHeadRel->getAttributesIdxs(vvJoinAttributeNames[idxRel], vvCurJoinAttrIdxs[idxRel]);
            initHashTableRowIdxs(vvJoinAttrIdxs[idxRel],
                vpChildHeadRels[idxRel]->m_data, vpHashTabRowPt[idxRel]);
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
                vCumNumRelSubTree[idxRel] = vCumNumRelSubTree[idxRel-1] + vvSubTreeRelnames[idxRel-1].size();
            }
        }
        schemaJoins(vAttrsAggAway, vpChildRels, vpChildHeadRels,
             vvJoinAttrIdxs, vvNonJoinAttrIdxs);


        MatrixDT dataOutput {pHeadRel->m_data.getNumRows(), (uint32_t)vAttrsAggAway.size()};
        MatrixDT scales{pHeadRel->m_data.getNumRows(), vpChildRels.size() + 1};
        MatrixDT dataScales{pHeadRel->m_data.getNumRows(), vSubTreeRelNames.size()};

        #pragma omp parallel for schedule(static)
        for (uint32_t rowIdx = 0; rowIdx < pHeadRel->m_data.getNumRows(); rowIdx++)
        {
            // TODO: Optimization change the way how the values are extracted to tree.
            for (const auto& joinAttrIdx: vJoinAttrIdxs)
            {
                dataOutput[rowIdx][joinAttrIdx] = pHeadRel->m_data[rowIdx][joinAttrIdx];
            }
            // TODO: Add reordering relations based on global order
            for (const auto& nonJoinAttrIdx: vNonJoinAttrIdxs)
            {
                dataOutput[rowIdx][nonJoinAttrIdx] = pHeadRel->m_data[rowIdx][nonJoinAttrIdx];
            }
            for (uint32_t idxRel = 0; idxRel < vpChildRels.size(); idxRel ++)
            {

                uint32_t childRowIdx = getChildRowIdx(rowIdx, vvCurJoinAttrIdxs[idxRel],
                                                      pHeadRel->m_data, vpHashTabRowPt[idxRel]);
                // Copying data from children relations.
                const double* childRowPt = vpChildHeadRels[idxRel]->m_data[childRowIdx];
                for (const auto nonJoinAttrIdx: vvNonJoinAttrIdxs[idxRel])
                {
                    uint32_t idxOut = numJoinAttrsCurRel + vCumNumNonJoinAttrs[idxRel] +
                                    nonJoinAttrIdx - vNumJoinAttrs[idxRel];
                    dataOutput[rowIdx][idxOut] = childRowPt[nonJoinAttrIdx];
                }

                // Updating dataScales data from relations from subtrees.
                for (uint32_t idxSubTreeRel = 0;
                        idxSubTreeRel < vvSubTreeRelnames[idxRel].size();
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
                            idxSubTreeRel < vvSubTreeRelnames[idxRel].size();
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

        m_dataScales = std::move(dataScales);
        m_scales = std::move(scales);

        //MICRO_BENCH_STOP(aggregateAway)
        //FIGARO_LOG_BENCH("Figaro", "aggregate away" + m_name,  MICRO_BENCH_GET_TIMER_LAP(aggregateAway));
        FIGARO_LOG_INFO("Before moving out", dataOutput.getNumRows());
        return Relation("AGG_AWAY_" + m_name, std::move(dataOutput), vAttrsAggAway);
    }

    std::tuple<Relation, Relation>
    Relation::computeAndScaleGeneralizedHeadAndTail(
        Relation* pAggAwayRel,
        const std::vector<std::string>& vJoinAttributeNames,
        const std::vector<std::string>& vParJoinAttributeNames,
        bool isRootNode,
        uint32_t numRelsSubTree
        )
    {
        std::vector<uint32_t> vJoinAttrIdxs;
        std::vector<uint32_t> vParJoinAttrIdxs;
        uint32_t numParDistVals;
        uint32_t numJoinAttrs;
        uint32_t numParJoinAttrs;
        uint32_t numOmittedAttrs;
        uint32_t numNonJoinAttrs;

        std::vector<Attribute> attributes;

        //MICRO_BENCH_INIT(genHT)
        //MICRO_BENCH_START(genHT)
        pAggAwayRel->getAttributesIdxs(vJoinAttributeNames, vJoinAttrIdxs);
        pAggAwayRel->getAttributesIdxs(vParJoinAttributeNames, vParJoinAttrIdxs);
        // TODO: Replace this
        numParDistVals = m_vParBlockStartIdxsAfterFirstPass.size() - 1;
        numJoinAttrs = vJoinAttributeNames.size();
        numNonJoinAttrs = pAggAwayRel->getNumberOfAttributes() - numJoinAttrs;
        numParJoinAttrs = vParJoinAttrIdxs.size();
        numOmittedAttrs = numJoinAttrs - numParJoinAttrs;

        attributes = pAggAwayRel->m_attributes;

        MatrixDT dataHeadOut { numParDistVals,
            pAggAwayRel->getNumberOfAttributes() - numOmittedAttrs};
        MatrixDT dataTailsOut{ pAggAwayRel->m_data.getNumRows() - numParDistVals,
            numNonJoinAttrs};
        MatrixDT scales{numParDistVals, 1};
        MatrixDT dataScales{numParDistVals, m_dataScales.getNumCols()};

        FIGARO_LOG_INFO("Compute Generalized Head and Tail for relation", m_name)

        FIGARO_LOG_DBG("vJoinAttributeNames", vJoinAttributeNames)
        // temporary adds an element to denote the end limit.
        m_vSubTreeDataOffsets.push_back(pAggAwayRel->m_attributes.size());
        //MICRO_BENCH_INIT(genHTMainLoop)
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
                getParCntFromHashTable(vParJoinAttrIdxs, m_pHTParCounts.get(), pAggAwayRel->m_data[startIdx]);
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
                    pAggAwayRel->m_data[startIdx][attrIdx] *= m_dataScales[startIdx][idxRel];
                    vCurScaleSum[attrIdx - numJoinAttrs] = pAggAwayRel->m_data[startIdx][attrIdx] * m_scales[startIdx][0];
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
                        pAggAwayRel->m_data[rowIdx][attrIdx] *= m_dataScales[rowIdx][idxRel];
                        dataTailsOut[tailRowIdx][shiftIdx] =
                            (pAggAwayRel->m_data[rowIdx][attrIdx] * sumSqrScalesPrev
                            - vi * vCurScaleSum[shiftIdx])
                            / std::sqrt(sumSqrScalesCur * sumSqrScalesPrev);
                        // Multiplies generalized tails with up count.
                        dataTailsOut[tailRowIdx][shiftIdx] *= sqrtUpCnt;
                        vCurScaleSum[shiftIdx] += pAggAwayRel->m_data[rowIdx][attrIdx] * vi;
                    }
                }
            }

            scales[distParCnt][0] = sqrtDownCnt;

            // Copies join parent attributes to generalized head.
            for (const auto& parJoinAttrIdx: vParJoinAttrIdxs)
            {
                dataHeadOut[distParCnt][parJoinAttrIdx] = pAggAwayRel->m_data[startIdx][parJoinAttrIdx];
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

        schemaRemoveNonParJoinAttrs(attributes, vJoinAttrIdxs, vParJoinAttrIdxs);

        m_dataScales = std::move(dataScales);
        m_scales = std::move(scales);
        //MICRO_BENCH_STOP(genHT)
        //MICRO_BENCH_STOP(genHTMainLoop)
        //FIGARO_LOG_BENCH("Figaro", "computeAndScaleGeneralizedHeadAndTail " + m_name,  MICRO_BENCH_GET_TIMER_LAP(genHT));
        ////FIGARO_LOG_BENCH("Figaro",  "Generalized head and tail main loop",  MICRO_BENCH_GET_TIMER_LAP(genHTMainLoop));
        FIGARO_LOG_INFO("Before moving out", dataHeadOut.getNumRows(), dataHeadOut.getNumCols())
        return std::make_tuple(
            Relation("GEN_HEAD" + m_name, std::move(dataHeadOut), attributes),
            Relation("GEN_TAIL" + m_name, std::move(dataTailsOut), attributes));
    }


    void Relation::computeQROfGeneralizedHead(
         const std::vector<Relation*>& vpTailRels,
        Figaro::QRHintType qrTypeHint)
    {
        uint32_t numNonJoinAttrs = 0;
        for (const auto pRel: vpTailRels)
        {
            numNonJoinAttrs += pRel->m_data.getNumCols();
        }
        m_data = m_data.getRightCols(numNonJoinAttrs);
        m_data.computeQR(
            getNumberOfThreads(), true, qrTypeHint,
                false /* computeQ*/, false /* saveResult*/);
    }


    void Relation::computeQRInPlace(Figaro::QRHintType qrHintType)
    {
        m_data.computeQR(getNumberOfThreads(), true, qrHintType,
            false /* computeQ*/, false /* saveResult*/);
    }

    std::tuple<Relation*, Relation*>
    Relation::computeQROfConcatenatedGeneralizedHeadAndTails(
        const std::vector<Relation*>& vpRels,
        Relation* pGenHeadRoot,
        const std::vector<Relation*>& vpTailRels,
        const std::vector<Relation*>& vpGenTailRels,
        Figaro::QRHintType qrHintType,
        bool saveResult,
        const Relation* pJoinRel)
    {
        std::vector<uint32_t> vLeftCumNumNonJoinAttrs;
        std::vector<uint32_t> vRightCumNumNonJoinAttrs;
        std::vector<uint32_t> vCumNumRowsUp;
        uint32_t totalNumRows;
        uint32_t totalNumCols;
        uint32_t numRels;
        uint32_t minNumRows;
        Relation* pR = nullptr;
        Relation* pQ = nullptr;
        bool computeQ = pJoinRel != nullptr;
        MatrixDT qData{0, 0};


        numRels = vpRels.size();
        vCumNumRowsUp.resize(vpTailRels.size() + vpGenTailRels.size() + 1);
        vLeftCumNumNonJoinAttrs.resize(numRels + 1);
        vLeftCumNumNonJoinAttrs[0] = 0;
        for (uint32_t idx = 1; idx < vLeftCumNumNonJoinAttrs.size(); idx++)
        {
            vLeftCumNumNonJoinAttrs[idx] = vLeftCumNumNonJoinAttrs[idx - 1] +
                    vpTailRels[idx-1]->m_data.getNumCols();
        }
        totalNumCols = vLeftCumNumNonJoinAttrs[numRels];
        FIGARO_LOG_INFO("Rel", totalNumCols)
        vCumNumRowsUp[0] = pGenHeadRoot->m_data.getNumRows();

        for (uint32_t idx = 1; idx < vpTailRels.size() + 1; idx++)
        {
            vCumNumRowsUp[idx] = vCumNumRowsUp[idx-1] +
            vpTailRels[idx-1]->m_data.getNumRows();
        }

        for (uint32_t idx = vpTailRels.size() + 1; idx < vCumNumRowsUp.size(); idx++)
        {
            vCumNumRowsUp[idx] = vCumNumRowsUp[idx-1] +
                + vpGenTailRels[idx - vpTailRels.size() - 1]->m_data.getNumRows();;
                FIGARO_LOG_INFO("vpGenTailRels[idx - vpTailRels.size() - 1]->m_data.getNumRows()", vpGenTailRels[idx - vpTailRels.size() - 1]->m_data.getNumRows())
        }
        totalNumRows = vCumNumRowsUp.back();

        FIGARO_LOG_INFO("Size", vCumNumRowsUp)
        //MICRO_BENCH_INIT(copyMatrices)
        //MICRO_BENCH_START(copyMatrices)

        MatrixDT catGenHeadAndTails{totalNumRows, totalNumCols};
        FIGARO_LOG_INFO("totalNumRows", totalNumRows)
        FIGARO_LOG_INFO("totalNumCols", totalNumCols)
        FIGARO_LOG_INFO("Number of rows in generalized head", pGenHeadRoot->m_data.getNumRows())
        FIGARO_LOG_INFO("Number of cols in generalized head", pGenHeadRoot->m_data.getNumCols())
        // Copying dataHead.
        for (uint32_t rowIdx = 0; rowIdx < pGenHeadRoot->m_data.getNumRows(); rowIdx++)
        {
            for (uint32_t colIdx = 0; colIdx < pGenHeadRoot->m_data.getNumCols(); colIdx++)
            {
                catGenHeadAndTails[rowIdx][colIdx] = pGenHeadRoot->m_data[rowIdx][colIdx];
            }
        }


        // Vertically concatenating tails and generalized tail to the head.
        for (uint32_t idxRel = 0; idxRel < vpTailRels.size(); idxRel ++)
        {
            uint32_t startTailIdx = vCumNumRowsUp[idxRel];
            uint32_t nextStartIdx = vCumNumRowsUp[idxRel + 1];
            for (uint32_t rowIdx = startTailIdx; rowIdx < nextStartIdx; rowIdx ++)
            {
                // Set Left zeros1
                for (uint32_t colIdx = 0; colIdx < vLeftCumNumNonJoinAttrs[idxRel];
                    colIdx ++)
                {
                    catGenHeadAndTails[rowIdx][colIdx] = 0;
                }
                uint32_t tailsRowIdx = rowIdx - startTailIdx;

                // Set central Relations
                for (uint32_t colIdx = vLeftCumNumNonJoinAttrs[idxRel];
                    colIdx < vLeftCumNumNonJoinAttrs[idxRel+1]; colIdx++ )
                {
                    uint32_t tailsColIdx = colIdx - vLeftCumNumNonJoinAttrs[idxRel];
                    catGenHeadAndTails[rowIdx][colIdx] =
                     vpTailRels[idxRel]->m_data[tailsRowIdx][tailsColIdx];
                }
                // Set Right zeros
                for (uint32_t colIdx = vLeftCumNumNonJoinAttrs[idxRel+1];
                    colIdx < totalNumCols; colIdx ++)
                {
                    catGenHeadAndTails[rowIdx][colIdx] = 0;
                }
            }
        }


        for (uint32_t idxRel = 0; idxRel < vpGenTailRels.size(); idxRel ++)
        {
            //FIGARO_LOG_INFO("genTail", vpGenTailRels[idxRel]->m_data)
            uint32_t startGenTailIdx = vCumNumRowsUp[idxRel + vpTailRels.size()];
            uint32_t endGenTailIdx = vCumNumRowsUp[idxRel + 1 + vpTailRels.size()];
            for (uint32_t rowIdx = startGenTailIdx;
                rowIdx < endGenTailIdx; rowIdx ++)
            {
                // Set Left zeros
                for (uint32_t colIdx = 0; colIdx < vLeftCumNumNonJoinAttrs[idxRel];
                    colIdx ++)
                {
                    catGenHeadAndTails[rowIdx][colIdx] = 0;
                }
                uint32_t tailsRowIdx = rowIdx - startGenTailIdx;

                // Set central Relations
                // Since the the order of relations is preordered,
                // generalized tails will be one after another.
                for (uint32_t colIdx = vLeftCumNumNonJoinAttrs[idxRel];
                    colIdx < vLeftCumNumNonJoinAttrs[idxRel] + vpGenTailRels[idxRel]->m_data.getNumCols();
                    colIdx++ )
                {
                    uint32_t tailsColIdx = colIdx - vLeftCumNumNonJoinAttrs[idxRel];
                    catGenHeadAndTails[rowIdx][colIdx] =
                     vpGenTailRels[idxRel]->m_data[tailsRowIdx][tailsColIdx];
                }
                // Set Right zeros
                for (uint32_t colIdx = vLeftCumNumNonJoinAttrs[idxRel] + vpGenTailRels[idxRel]->m_data.getNumCols();
                    colIdx < totalNumCols; colIdx ++)
                {
                    catGenHeadAndTails[rowIdx][colIdx] = 0;
                }
            }
        }
        FIGARO_LOG_INFO("After", catGenHeadAndTails)

        //MICRO_BENCH_STOP(copyMatrices)
        //FIGARO_LOG_BENCH("Figaro", "Copying matrices",  MICRO_BENCH_GET_TIMER_LAP(copyMatrices));
        FIGARO_LOG_INFO("Computing final R")

        //MICRO_BENCH_INIT(finalQR)
        //MICRO_BENCH_START(finalQR)
        catGenHeadAndTails.computeQR(getNumberOfThreads(), true, qrHintType,
            false /* computeQ*/, false /* saveResult*/);
        //MICRO_BENCH_STOP(finalQR)
        //FIGARO_LOG_BENCH("Figaro", "Final QR",  MICRO_BENCH_GET_TIMER_LAP(finalQR));


        //MICRO_BENCH_INIT(addingZeros)
        //MICRO_BENCH_START(addingZeros)
        minNumRows = std::min(totalNumRows, totalNumCols);
        if (totalNumCols > minNumRows)
        {
            FIGARO_LOG_INFO("Number of apended rows", totalNumCols - minNumRows);
            catGenHeadAndTails = catGenHeadAndTails.concatenateVerticallyScalar(0.0, totalNumCols - minNumRows);
        }
        //MICRO_BENCH_STOP(addingZeros)
        //FIGARO_LOG_BENCH("Figaro", "Adding zeros",  MICRO_BENCH_GET_TIMER_LAP(addingZeros));

        if (computeQ)
        {
            FIGARO_LOG_INFO("Computing Q")
            qData = std::move(
                pJoinRel->m_data * catGenHeadAndTails.computeInverse(0, true));
            FIGARO_LOG_BENCH("join Rel Size", pJoinRel->m_data.getNumRows())
        }

        if (saveResult)
        {
            catGenHeadAndTails.makeDiagonalElementsPositiveInR();
        }

        pR = createFactorRelation("R", std::move(catGenHeadAndTails), totalNumCols);
        FIGARO_LOG_INFO("R", *pR)
        if (computeQ)
        {
            pQ = createFactorRelation("Q", std::move(qData), totalNumCols);
        }
        return std::make_tuple(pR, pQ);
    }

    std::tuple<Relation*, Relation*> Relation::computeQR(
        Figaro::QRHintType qrHintType,
        Figaro::MemoryLayout memoryLayout,
        bool computeQ,
        bool saveResult)
    {
        Relation* pR = nullptr;
        Relation* pQ = nullptr;
        if (memoryLayout == MemoryLayout::ROW_MAJOR)
        {
            MatrixDT matR = MatrixDT{0, 0};
            MatrixDT matQ = MatrixDT{0, 0};

            m_data.computeQR(getNumberOfThreads(), true,
                qrHintType, computeQ, saveResult, &matR, &matQ);

            //matR.computeQRCholesky(computeQ, true, &matR, &matQ);
            if (saveResult)
            {
                FIGARO_LOG_INFO("R before positive diagonal", matR)
                matR.makeDiagonalElementsPositiveInR();
                FIGARO_LOG_INFO("R after positive diagonal", m_data)
                pR = createFactorRelation("R", std::move(matR), m_attributes.size());
                pQ = createFactorRelation("Q", std::move(matQ), m_attributes.size());
            }
        }
        else
        {
            MatrixDColT matR = MatrixDColT{0, 0};
            MatrixDColT matQ = MatrixDColT{0, 0};

            m_dataColumnMajor.computeQR(getNumberOfThreads(), true,
                 qrHintType, computeQ, saveResult, &matR, &matQ);

            //m_dataColumnMajor.computeQRCholesky(computeQ, true, &matR, &matQ);
            if (saveResult)
            {
                matR.makeDiagonalElementsPositiveInR();
                MatrixDT matRR{matR.getNumRows(), matR.getNumCols()};
                MatrixDT matRQ{matQ.getNumRows(), matQ.getNumCols()};
                matRR.copyBlockToThisMatrixFromCol(
                    matR, 0, matRR.getNumRows() - 1,
                    0, matRR.getNumCols() - 1, 0, 0);
                if (computeQ)
                {
                    matRQ.copyBlockToThisMatrixFromCol(
                        matQ, 0, matRQ.getNumRows() - 1,
                        0, matRQ.getNumCols() - 1, 0, 0);
                }
                pR = createFactorRelation("R", std::move(matRR), m_attributes.size());
                pQ = createFactorRelation("Q", std::move(matRQ), m_attributes.size());
            }
        }
        return std::make_tuple(pR, pQ);
    }

    const Relation::MatrixDT& Relation::getHead(void) const
    {
        return MatrixDT{0, 0};
    }

    const Relation::MatrixDT& Relation::getTail(void) const
    {
        return MatrixDT{0, 0};
    }

    const Relation::MatrixDT& Relation::getGeneralizedTail(void) const
    {
        return MatrixDT{0, 0};
    }


    const Relation::MatrixDT& Relation::getScales(void) const
    {
        return m_scales;
    }

    const Relation::MatrixDT& Relation::getDataScales(void) const
    {
        return m_dataScales;
    }

    void Relation::changeMemoryLayout(const Figaro::MemoryLayout& memoryLayout)
    {
        MatrixDColT tmpOut{m_data.getNumRows(), m_data.getNumCols()};
        if (memoryLayout == MemoryLayout::COL_MAJOR)
        {
            for (uint32_t rowIdx = 0; rowIdx < m_data.getNumRows(); rowIdx++)
            {
                for (uint32_t colIdx = 0; colIdx < m_data.getNumCols(); colIdx++)
                {
                    tmpOut(rowIdx, colIdx) = m_data(rowIdx, colIdx);
                }
            }
            m_dataColumnMajor = std::move(tmpOut);
            m_data = std::move(MatrixDT{0, 0});
        }
        else if (memoryLayout == MemoryLayout::COL_MAJOR)
        {
            // TODO: if needed.
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

    void Relation::outputToFile(std::ostream& out, char sep,
        uint32_t precision, bool header) const
    {
         for (uint32_t row = 0; row < m_data.getNumRows(); row ++)
        {
            for (uint32_t col = 0; col < m_data.getNumCols(); col++)
            {

                out << std::setprecision(precision) << std::scientific << m_data(row, col);
                if (col != (m_data.getNumCols() - 1))
                {
                    out << sep;
                }
            }
            out << std::endl;
        }
    }
}

