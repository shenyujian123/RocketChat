#include "RedisMgr.h"

RedisMgr :: RedisMgr() 
{
    context = redisConnect("127.0.0.1", 6379);
    if (context->err) cerr << "Redis Error: " << context->errstr << endl;
}
void RedisMgr :: set_online(string user, int fd)
{
    lock_guard<mutex> lock(mtx);
    freeReplyObject(redisCommand(context, "HSET online_status %s %d", user.c_str(), fd));
}
void RedisMgr :: set_offline(string user) 
{
    lock_guard<mutex> lock(mtx);
    freeReplyObject(redisCommand(context, "HDEL online_status %s", user.c_str()));
}
int RedisMgr :: get_fd(string user) 
{
    lock_guard<mutex> lock(mtx);
    redisReply *reply = (redisReply*)redisCommand(context, "HGET online_status %s", user.c_str());
    int fd = -1;
    if (reply && reply->type == REDIS_REPLY_STRING) fd = stoi(reply->str);
    freeReplyObject(reply);
    return fd;
}
RedisMgr :: ~RedisMgr() 
{ 
    redisFree(context); 
}
