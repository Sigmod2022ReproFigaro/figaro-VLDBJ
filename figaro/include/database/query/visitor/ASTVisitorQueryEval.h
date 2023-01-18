#ifndef _FIGARO_AST_VISITOR_QUERY_EVAL_H_
#define _FIGARO_AST_VISITOR_QUERY_EVAL_H_

#include "database/query/visitor/figaro/qr/ASTVisitorQRFigaroAbs.h"
#include "database/query/visitor/result/ASTVisitorResultJoin.h"
#include "database/query/visitor/result/ASTVisitorResultQR.h"
#include "database/query/visitor/result/ASTVisitorResultSVD.h"


namespace Figaro
{
    class ASTVisitorQueryEval: public ASTVisitor
    {
        MemoryLayout m_memoryLayout;
        Database* m_pDatabase;
        bool m_saveResult;
        bool m_saveMemory;
        std::map<std::string, bool> m_mFlagPhases;
        std::map<std::string, uint32_t> m_mIntOpts;

        bool isFlagOn(const std::string& flagName) const
        {
            if (!m_mFlagPhases.contains(flagName))
            {
                return false;
            }
            return m_mFlagPhases.at(flagName);
        }

    public:
        ASTVisitorQueryEval(
            Database* pDatabase,
            Figaro::MemoryLayout memoryLayout,
            bool saveResult,
            bool saveMemory,
            const std::map<std::string, bool>& mFlagPhases,
            const std::map<std::string, uint32_t>& mIntOpts
            ): ASTVisitor(pDatabase), m_memoryLayout(memoryLayout),
                m_pDatabase(pDatabase),
                m_saveResult(saveResult),
                m_saveMemory(saveMemory),
                m_mFlagPhases(mFlagPhases),
                m_mIntOpts(mIntOpts){}

        ASTVisitorResultJoin* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultAbs* visitNodeJoin(ASTNodeJoin* pElement) override { return nullptr; }
        ASTVisitorResultQR* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;
        ASTVisitorResultQR* visitNodeQRDecAlg(ASTNodeQRAlg* pElement) override;
        ASTVisitorResultSVD* visitNodeSVDDecAlg(ASTNodeSVDAlgDec* pElement) override;
        ASTVisitorResultSVD* visitNodePCADecAlg(ASTNodePCAAlgDec* pElement) override;
        ASTVisitorResultQR* visitNodeLUDecAlg(ASTNodeLUAlg* pElement) override;
        ASTVisitorResultQR* visitNodeLUThin(ASTNodeLUThin* pElement) override;
        ASTVisitorResultQR* visitNodeLUFigaro(ASTNodeLUFigaro* pElement) override;
        ASTVisitorResultSVD* visitNodeSVDFigaro(ASTNodeSVDFigaro* pElement) override;
        ASTVisitorResultSVD* visitNodePCAFigaro(ASTNodePCAFigaro* pElement) override;
        ASTVisitorResultJoin* visitNodeLinReg(ASTNodeLinReg* pElement) override;
        ASTVisitorResultAbs* visitNodeAssign([[maybe_unused]] ASTNodeAssign* pElement) override { return nullptr; }
        ASTVisitorResultJoin* visitNodeEvalJoin(ASTNodeEvalJoin* pElement) override;
        ASTVisitorResultJoin* visitNodeRightMultiply(ASTNodeRightMultiply* pElement) override;
        ASTVisitorResultJoin* visitNodeInverse(ASTNodeInverse* pElement) override;
         ASTVisitorResultJoin* visitNodeSVDSVTInverse(ASTNodeSVDSVTInverse* pElement) override;

        virtual ~ASTVisitorQueryEval() override {}
    };
}

#endif