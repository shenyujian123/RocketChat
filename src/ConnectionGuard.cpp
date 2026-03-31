#include "MysqlPool.h"
#include "ConnectionGuard.h"

ConnectionGuard :: ConnectionGuard(MYSQL** c, MysqlPool* p) : pool(p)
{ 
    *c = pool->getConnection(); 
    conn = *c; 
}
ConnectionGuard :: ~ConnectionGuard()
{
    pool->releaseConnection(conn);
}