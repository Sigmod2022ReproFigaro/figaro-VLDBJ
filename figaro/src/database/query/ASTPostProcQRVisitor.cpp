#include "database/query/ASTPostProcQRVisitor.h"

namespace Figaro
{
    ASTVisitorAbsResult* ASTPostProcQRVisitor::visitNodePostProcQR(ASTNodePostProcQR* pElement)
    {
        m_pDatabase->evalPostprocessing(pElement->getRelationOrder().at(0),
            m_qrHintType, m_memoryLayout, pElement->isComputeQ(), m_pResult);
        return nullptr;
    }
}