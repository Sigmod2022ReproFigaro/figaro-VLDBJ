#ifndef _FIGARO_AST_FIGARO_SECOND_PASS_VISITOR_H_
#define _FIGARO_AST_FIGARO_SECOND_PASS_VISITOR_H_

#include "ASTQRVisitor.h"

namespace Figaro
{
    class ASTFigaroSecondPassVisitor: public ASTQRVisitor
    {
        std::string strCountsHeadGeneralized(ASTNodeRelation* pRel);
        bool m_postProcess = false;
        MatrixEigenT* m_pResult = nullptr;
    public:
        ASTFigaroSecondPassVisitor(
            Database* pDatabase,
            const std::map<std::string, ASTNodeRelation*>& mRelNameASTNodeRel,
            bool postProcess, MatrixEigenT* pResult):
                ASTQRVisitor(pDatabase, mRelNameASTNodeRel),
                m_postProcess(postProcess), m_pResult(pResult) {}
        void visitNodeRelation(ASTNodeRelation* pElement) override;
        void visitNodeJoin(ASTNodeJoin* pElement) override;
        void visitNodeQRGivens(ASTNodeQRGivens* pElement) override;
        virtual ~ASTFigaroSecondPassVisitor() override {}


    };
}

#endif