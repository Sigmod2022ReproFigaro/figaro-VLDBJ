#ifndef _FIGARO_AST_VISITOR_RIGHT_MULTIPLY_H_
#define _FIGARO_AST_VISITOR_RIGHT_MULTIPLY_H_

#include "ASTVisitor.h"
#include "./result/ASTVisitorResultJoin.h"

namespace Figaro
{
    class ASTVisitorRightMultiply: public ASTVisitor
    {
        std::string m_relName;
        uint32_t startRowIdx = 0;
        bool m_useLFTJoin;
        std::vector<std::string> m_vRelNames;
        std::vector<std::string> m_vParRelNames;
        std::vector<std::string> m_vPreOrderRelNames;
        std::vector<std::string> m_vPreOrderParRelNames;
        std::vector<std::vector<std::string> > m_vvJoinAttrNames;
        std::vector<std::vector<std::string> > m_vvParJoinAttrNames;
        std::vector<uint32_t> m_vDownCountsSize;
        std::vector<uint32_t> m_vBlockSizes;
    public:
        ASTVisitorRightMultiply(
            Database* pDatabase,
            const std::string& relName,
            bool useLFTJoin = false,
            const std::vector<std::string>& vPreOrderRelNames = {},
            const std::vector<std::string>& vPreOrderParRelNames = {},
            const std::vector<std::vector<std::string> >& vvJoinAttrNames = {},
            const std::vector<std::vector<std::string> >& vvParJoinAttrNames = {},
            const std::vector<uint32_t>& vDownCountsSize = {},
            const std::vector<uint32_t>& vBlockSizes = {}): ASTVisitor(pDatabase), m_relName(relName),
                m_useLFTJoin(useLFTJoin), m_vPreOrderRelNames(vPreOrderRelNames),
                m_vPreOrderParRelNames(vPreOrderParRelNames),
                m_vvJoinAttrNames(vvJoinAttrNames),
                m_vvParJoinAttrNames(vvParJoinAttrNames),
                m_vDownCountsSize(vDownCountsSize),
                m_vBlockSizes(vBlockSizes){}
        virtual ASTVisitorResultJoin* visitNodeRelation(ASTNodeRelation* pElement) override;
        virtual ASTVisitorResultJoin* visitNodeJoin(ASTNodeJoin* pElement) override;
        virtual  ASTVisitorResultAbs* visitNodeRightMultiply(ASTNodeRightMultiply* pElement) override;

        virtual ASTVisitorResultAbs* visitNodeQRFigaro([[maybe_unused]] ASTNodeQRFigaro* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
            }

        virtual ASTVisitorResultAbs* visitNodeLUFigaro([[maybe_unused]] ASTNodeLUFigaro* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
            }

        virtual ASTVisitorResultAbs* visitNodeQRDecAlg([[maybe_unused]] ASTNodeQRAlg* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeSVDFigaro([[maybe_unused]] ASTNodeSVDFigaro* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
            }

        virtual ASTVisitorResultAbs* visitNodeSVDDecAlg([[maybe_unused]] ASTNodeSVDAlgDec* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

         virtual ASTVisitorResultAbs* visitNodePCAFigaro([[maybe_unused]] ASTNodePCAFigaro* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
            }

        virtual ASTVisitorResultAbs* visitNodePCADecAlg([[maybe_unused]] ASTNodePCAAlgDec* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeLUDecAlg([[maybe_unused]] ASTNodeLUAlg* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

         virtual ASTVisitorResultAbs* visitNodeLUThin([[maybe_unused]] ASTNodeLUThin* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeAssign([[maybe_unused]] ASTNodeAssign* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeEvalJoin([[maybe_unused]] ASTNodeEvalJoin* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeInverse([[maybe_unused]]  ASTNodeInverse* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

         virtual ASTVisitorResultAbs* visitNodeLinReg([[maybe_unused]] ASTNodeLinReg* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeLeastSquares([[maybe_unused]]ASTNodeLeastSquares* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeSVDSVTInverse([[maybe_unused]] ASTNodeSVDSVTInverse* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ~ASTVisitorRightMultiply() override {}
    };
}

#endif