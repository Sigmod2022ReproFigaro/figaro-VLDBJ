#include "database/query/visitor/ASTVisitor.h"
namespace Figaro
{
    std::string ASTVisitor::getFormateJoinAttributeNames(
        const std::vector<std::string>& vJoinAttributeNames)
    {
        std::string formatedStr = "";

        for (uint32_t idx = 0; idx < vJoinAttributeNames.size(); idx++)
        {
            if (idx > 0)
            {
                formatedStr += ",";
            }
            formatedStr += vJoinAttributeNames[idx];
        }
        return formatedStr;
    }

}