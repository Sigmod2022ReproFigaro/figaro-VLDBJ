#include "database/query/ASTPostProcQRVisitor.h"

namespace Figaro
{
    void ASTPostProcQRVisitor::visitNodePostProcQR(ASTNodePostProcQR* pElement)
    {
        m_pDatabase->evalPostprocessing(pElement->getRelationOrder().at(0),
            m_qrHintType, m_memoryLayout, m_pResult);
    }
}