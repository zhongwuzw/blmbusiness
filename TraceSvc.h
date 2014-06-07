#ifndef TRACESVC_H
#define TRACESVC_H

#include "IBusinessService.h"
#include "kSQL.h"

using namespace std;

typedef struct mytrace
{
    std::string m_strSI;
    std::string m_strCI;
    std::string m_strMM;
    std::string m_strTN;
    std::string m_strSN;
    std::string m_crDt;
    int m_iST;
    int m_iET;
    int m_RemarkNum;
    int m_TrashFlag;
    int m_iType;
} _MYTRACE,*PMYTRACE;

typedef struct tkind
{
    char m_btcategoryid[3];
    char m_szCategoryname[64];	//类名
    char updatet_dt[20];		//修改日期
    char create_dt[20];			//创建日期
    int m_btrash_flag;			//删除标记
    int m_RemarkNum;
    std::vector<_MYTRACE> m_TraceList;
} _TKIND,*PTKAND;

typedef struct cjevent
{
    int m_nEventID;
    int m_nMmsi;
    std::string m_szName;
    long m_lStart;
    long m_lEnd;
    int m_nShipType;
    int m_nCargoType;
} _CJEVENT, *PCJEVENT;

class TraceSvc : IBusinessService
{
public:
    TraceSvc();
     ~TraceSvc();
    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(TraceSvc)

private:
    int alltrace_req(const char* pUid, const char* jsonString, std::stringstream& out);
    int tkindopt_req(const char* pUid, const char* jsonString, std::stringstream& out);
    int traceopt_req(const char* pUid, const char* jsonString, std::stringstream& out);
    int cj_event_req(const char* pUid, const char* jsonString, std::stringstream& out);
    int cj_event_confirm(const char* pUid, const char* jsonString, std::stringstream& out);


    // 轨迹分类添加\修改\删除\清空
    bool tracekind_add(const char* i_szUid,const char* i_szCI,const char*i_szCN,MySql *psql);
    bool tracekind_del(const char* i_szUid,const char* i_szCI,MySql *psql);
    bool tracekind_recover(const char* i_szUid,const char* i_szCI,MySql *psql);
    bool tracekind_edit(const char* i_szUid,const char* i_szCI,const char*i_szCN,MySql *psql);
    bool tracekind_clear(const char* i_szUid,const char* i_szCI,MySql *psql);

    // 轨迹添加\修改\删除\恢复\彻底删除
    bool trace_add(const char* i_szUid,_MYTRACE &i_mytrace,MySql *psql);
    bool trace_del(const char* i_szUid,_MYTRACE &i_mytrace,MySql *psql);
    bool trace_edit(const char* i_szUid,_MYTRACE &i_mytrace,MySql *psql);
    bool trace_recover(const char* i_szUid,_MYTRACE &i_mytrace,MySql *psql);
    bool trace_clear(const char* i_szUid,_MYTRACE &i_mytrace,MySql *psql);

};

typedef ACE_Singleton<TraceSvc, ACE_Null_Mutex> g_TraceSvc;

#endif
