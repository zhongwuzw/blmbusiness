#include "ServerMgr.h"
#include "iostream"
using namespace std;
int main(int argc, char* argv[])
{
    bool ret = (g_ServerMgr::instance()->Init() && g_ServerMgr::instance()->Start());
    getchar();
    return ret;
}
