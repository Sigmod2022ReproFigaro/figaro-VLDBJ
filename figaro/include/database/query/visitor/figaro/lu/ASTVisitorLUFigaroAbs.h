#ifndef _FIGARO_AST_VISITOR_LU_FIGARO_ABS_H_
#define _FIGARO_AST_VISITOR_LU_FIGARO_ABS_H_

#include "../../ASTVisitor.h"
#include "../../result/ASTVisitorResultAbs.h"

namespace Figaro
{
    class ASTVisitorLUFigaroAbs: public ASTVisitor
    {
    public:
        ASTVisitorLUFigaroAbs(Database* pDatabase):
            ASTVisitor(pDatabase)
         {}
        /*
        virtual ASTVisitorResultJoin* visitNodeRelation(ASTNodeRelation* pElement) override;

        virtual ASTVisitorResultJoin* visitNodeJoin(ASTNodeJoin* pElement) override;

        virtual ASTVisitorResultAbs* visitNodeLUFigaro(
             ASTNodeLUFigaro* pElement) override;
        */

        virtual ASTVisitorResultAbs* visitNodeQRFigaro([[maybe_unused]] ASTNodeQRFigaro* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
            }

        virtual ASTVisitorResultAbs* visitNodeQRPostProc([[maybe_unused]] ASTNodeQRAlg* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeSVDLapack([[maybe_unused]] ASTNodeSVDLapack* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeLULapack([[maybe_unused]] ASTNodeLUAlg* pElement) override {
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

        virtual  ASTVisitorResultAbs* visitNodeRightMultiply([[maybe_unused]] ASTNodeRightMultiply* pElement) override

        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }


        virtual ~ASTVisitorLUFigaroAbs() override {}


    };
}

#endif