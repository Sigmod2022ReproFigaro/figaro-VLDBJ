#ifndef _FIGARO_AST_VISITOR_QUERY_EVAL_H_
#define _FIGARO_AST_VISITOR_QUERY_EVAL_H_

#include "./figaro/qr/ASTVisitorQRFigaroAbs.h"
#include "./result/ASTVisitorResultJoin.h"
#include "./result/ASTVisitorResultQR.h"


namespace Figaro
{
    class ASTVisitorQueryEval: public ASTVisitor
    {
        MemoryLayout m_memoryLayout;
        QRHintType m_qrHintType;
        Database* m_pDatabase;
        bool m_saveResult;
        bool m_saveMemory;
        std::map<std::string, bool> m_mFlagPhases;

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
            Figaro::QRHintType qrHintType,
            bool saveResult,
            bool saveMemory,
            const std::map<std::string, bool>& mFlagPhases
            ): ASTVisitor(pDatabase), m_memoryLayout(memoryLayout),
                m_pDatabase(pDatabase), m_qrHintType(qrHintType),
                m_saveResult(saveResult),
                m_saveMemory(saveMemory),
                m_mFlagPhases(mFlagPhases){}
        ASTVisitorResultJoin* visitNodeRelation(ASTNodeRelation* pElement) override;
        ASTVisitorResultAbs* visitNodeJoin(ASTNodeJoin* pElement) override { return nullptr; }
        ASTVisitorResultQR* visitNodeQRFigaro(ASTNodeQRFigaro* pElement) override;
        ASTVisitorResultQR* visitNodeQRPostProc(ASTNodeQRPostProc* pElement) override;
        ASTVisitorResultQR* visitNodeSVDLapack(ASTNodeSVDLapack* pElement) override;
        ASTVisitorResultQR* visitNodeLULapack(ASTNodeLULapack* pElement) override;
        ASTVisitorResultQR* visitNodeLUThin(ASTNodeLUThin* pElement) override;
        ASTVisitorResultQR* visitNodeLUFigaro(ASTNodeLUFigaro* pElement) override;
        ASTVisitorResultJoin* visitNodeLinReg(ASTNodeLinReg* pElement) override;
        ASTVisitorResultAbs* visitNodeAssign([[maybe_unused]] ASTNodeAssign* pElement) override { return nullptr; }
        ASTVisitorResultJoin* visitNodeEvalJoin(ASTNodeEvalJoin* pElement) override;
        ASTVisitorResultJoin* visitNodeRightMultiply(ASTNodeRightMultiply* pElement) override;
        ASTVisitorResultJoin* visitNodeInverse(ASTNodeInverse* pElement) override;

        virtual ~ASTVisitorQueryEval() override {}
    };
}

#endif