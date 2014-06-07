#ifndef _SERVERMANAGER_H
#define _SERVERMANAGER_H

#include "define.h"

class CServerMgr
{
public:
    CServerMgr(void);
    ~CServerMgr(void);

    bool Init();
    bool Start();
};

typedef ACE_Singleton<CServerMgr, ACE_Null_Mutex> g_ServerMgr;

#endif
