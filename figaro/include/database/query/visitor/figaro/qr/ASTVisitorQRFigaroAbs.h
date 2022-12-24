#ifndef _FIGARO_AST_VISITOR_QR_FIGARO_ABS_H_
#define _FIGARO_AST_VISITOR_QR_FIGARO_ABS_H_

#include "../../ASTVisitor.h"
#include "../../result/ASTVisitorResultAbs.h"

namespace Figaro
{
    class ASTVisitorQRFigaroAbs: public ASTVisitor
    {
    public:
        ASTVisitorQRFigaroAbs(Database* pDatabase):
            ASTVisitor(pDatabase)
         {}

        virtual ASTVisitorResultAbs* visitNodeQRDecAlg([[maybe_unused]]ASTNodeQRAlg* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }


        virtual ASTVisitorResultAbs* visitNodeLUFigaro([[maybe_unused]]ASTNodeLUFigaro* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;}

        virtual ASTVisitorResultAbs* visitNodeLUDecAlg([[maybe_unused]]ASTNodeLUAlg* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;}

         virtual ASTVisitorResultAbs* visitNodeSVDFigaro([[maybe_unused]]ASTNodeSVDFigaro* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeSVDDecAlg([[maybe_unused]]ASTNodeSVDAlgDec* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;}


        virtual ASTVisitorResultAbs* visitNodeLUThin([[maybe_unused]]ASTNodeLUThin* pElement) override {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;}

        virtual ASTVisitorResultAbs* visitNodeAssign([[maybe_unused]]ASTNodeAssign* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeEvalJoin([[maybe_unused]]ASTNodeEvalJoin* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeRightMultiply([[maybe_unused]]ASTNodeRightMultiply* pElement)  override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeInverse([[maybe_unused]]ASTNodeInverse* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeLinReg([[maybe_unused]]ASTNodeLinReg* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }

        virtual ASTVisitorResultAbs* visitNodeSVDSVTInverse([[maybe_unused]]ASTNodeSVDSVTInverse* pElement) override
        {
            FIGARO_LOG_ASSERT(1!=1)
            return nullptr;
        }


        virtual ~ASTVisitorQRFigaroAbs() override {}


    };
}

#endif