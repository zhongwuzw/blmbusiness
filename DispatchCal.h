#ifndef __DISPATCH_CAL_H__
#define __DISPATCH_CAL_H__

#include "define.h"
#include "kSQL.h"

class DispatchCal : ACE_Event_Handler
{
public:
    DispatchCal();
    ~DispatchCal();

    bool Start();
    virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);
};

typedef ACE_Singleton<DispatchCal, ACE_Null_Mutex> g_DispatchCal;

#endif
