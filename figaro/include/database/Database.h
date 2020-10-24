#ifndef _FIGARO_DATABASE_H_
#define _FIGARO_DATABASE_H_

#include "utils/Utils.h"
#include "Relation.h"

namespace Figaro
{
    class Database
    {
        std::map<std::string, Relation> m_relations; 

        void loadDatabaseRelationsSchema(const json& jsonRelInfos);

        void loadDatabaseSchema(const std::string& schemaConfigPath); 

       
    public:
        Database(const std::string& schemaConfigPath); 

        void loadData(void);
    };
}
#endif 