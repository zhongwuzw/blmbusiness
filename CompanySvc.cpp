#include "CompanySvc.h"
#include "blmcom_head.h"
#include "MessageService.h"
#include "VesselSvc.h"
#include "LogMgr.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "MainConfig.h"
#include "Util.h"
#include "json.h"

IMPLEMENT_SERVICE_MAP(CompanySvc)

CompanySvc::CompanySvc()
{

}

CompanySvc::~CompanySvc()
{

}

bool CompanySvc::Start()
{
    if(!m_companydatas.LoadDatabase())
    {
        return false;
    }

	int interval = 86400;
	int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
	if(timerId <= 0)
		return false;

    if(!g_MessageService::instance()->RegisterCmd(MID_COMPANY, this))
        return false;

    SERVICE_MAP(SID_SEARCHCOMPANY,CompanySvc,OnCompanySearchMsg);
    SERVICE_MAP(SID_COMPOUTLINE,CompanySvc,GetCompanyInfo);
    SERVICE_MAP(SID_SERVEPORTS,CompanySvc,GetServerPort);
    SERVICE_MAP(SID_CONTACTSTAT,CompanySvc,GetContactStat);
    SERVICE_MAP(SID_SHIPCATEGORY,CompanySvc,GetShipCategory);
    SERVICE_MAP(SID_COMPSHIPLIST,CompanySvc,GetShipList);
    SERVICE_MAP(SID_SHIPPOSITION,CompanySvc,GetShipPosition);

	//std::stringstream out;
	//GetCompanyInfo("caiwj", "{id:\"C090420588\"}", out);

    DEBUG_LOG("[CompanySvc::Start] OK......................................");
    return true;
}

int CompanySvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	SYSTEM_LOG("[CompanySvc::handle_timeout] begin ==============");

	CompanySearch* ps = new CompanySearch;
	if(ps->LoadDatabase())
	{
		m_companydatas.OnUpdateCompanyData(ps);
	}
	delete ps;

	SYSTEM_LOG("[CompanySvc::handle_timeout] end ==============");
	return 0;
}

int CompanySvc::GetShipPosition(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[CompanySvc::GetShipPosition]bad format:", jsonString, 1);
	std::string comp_key	= root.getv("id", "");
	std::string szMmsi		= root.getv("mmsi", "");
    int nCat				= root.getv("cat", 0);

    int nState = 0;
    long mmsi;
    time_t lastTm = 0;
    if(!szMmsi.empty())
    {
        mmsi = atol(szMmsi.c_str());
        nState = g_VesselSvc::instance()->aisShip.GetAisState(mmsi, lastTm);
    }
    out << FormatString("{id:\"%s\",mmsi:\"%s\",cat:%d,dt:%ld,st:%d}", comp_key.c_str(), szMmsi.c_str(), nCat, lastTm, nState);
    return 0;
}

int CompanySvc::GetShipList(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[CompanySvc::GetShipList]bad format:", jsonString, 1);
	std::string comp_key	= root.getv("id", "");
    int nCat				= root.getv("cat", 0);

    if(nCat<0 || nCat>2)
        return 4;

    // 根据类别设置条件
    char* condition = (nCat==0) ? (char*)"owner" :((nCat==1) ? (char*)"manager" : (char*)"builder");
    char* sql = "select t1.shipid as id, name, t1.imo, t1.mmsi, t1.callsign as clsn, shiptype_key as shtp, country_code as iso3, substr(built,1,4) as blt, speed as spd, t2.dwt "\
                " from t41_ship t1, t41_ship_tonnage t2 "\
                " where %s='%s' and t1.shipid=t2.shipid";

	MySql *psql = CREATE_MYSQL;
    CHECK_MYSQL_STATUS(psql->Query(FormatString(sql, condition, comp_key.c_str()).c_str()), 3);

    out << FormatString("{id:\"%s\",shps:[", comp_key.c_str());

	char id[64];
	char name[64];
	char imo[64];
	char mmsi[64];
	char clsn[64];
	char shtp[64];
	char iso3[64];
	char dwt[64];
	char blt[64];
	char spd[64];
	int total = 0;

    while(psql->NextRow())
    {
		if(total != 0) {
			out << ",";
		}
		total++;

		READMYSQL_STR(id,id);
		READMYSQL_STR(name,name);
		READMYSQL_STR(imo,imo);
		READMYSQL_STR(mmsi,mmsi);
		READMYSQL_STR(clsn,clsn);
		READMYSQL_STR(shtp,shtp);
		READMYSQL_STR(iso3,iso3);
		READMYSQL_STR(dwt,dwt);
		READMYSQL_STR(blt,blt);
		READMYSQL_STR(spd,spd);
        out << FormatString("{id:\"%s\",name:\"%s\",imo:\"%s\",mmsi:\"%s\",clsn:\"%s\",shtp:\"%s\",iso3:\"%s\"",
                         id, name, imo, mmsi, clsn, shtp, iso3);
        // dwt
        if(strlen(dwt))
            out << FormatString(",dwt:%s", dwt);

        // blt
        if(strlen(blt)>=4)
			out << FormatString(",blt:%s", blt);

        // spd
        if(strlen(spd))
            out << FormatString(",spd:%s", spd);

		out << "}";
    }
    out << FormatString("]}");
    RELEASE_MYSQL_RETURN(psql, 0);
}

int CompanySvc::GetShipCategory(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[CompanySvc::GetShipCategory]bad format:", jsonString, 1);
	std::string comp_key = root.getv("id", "");

    char* sql = "select count(case owner when '%s' then owner end) as oshp,"\
                "count(case manager when '%s' then manager end) as mshp,"\
                "count(case builder when '%s' then builder end) as bshp "\
                " from t41_ship";
    int oCount = 0, mCount = 0, bCount = 0;

	MySql *psql = CREATE_MYSQL;
    CHECK_MYSQL_STATUS(psql->Query(FormatString(sql, comp_key.c_str(), comp_key.c_str(), comp_key.c_str()).c_str()) && psql->NextRow(), 3)

    oCount = atoi(psql->GetField("oshp"));
    mCount = atoi(psql->GetField("mshp"));
    bCount = atoi(psql->GetField("bshp"));

    out << FormatString("{id:\"%s\",cts:[%d,%d,%d]}", comp_key.c_str(), oCount, mCount, bCount);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int CompanySvc::GetContactStat(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[CompanySvc::GetContactStat]bad format:", jsonString, 1);
	std::string comp_key = root.getv("id", "");

    char* sql = "select user_id as uid from t41_comp_contacts "\
                " where company_key='%s' and flag='1'";

	MySql *psql = CREATE_MYSQL;
    CHECK_MYSQL_STATUS(psql->Query(FormatString(sql, comp_key.c_str()).c_str()), 3)

    out << FormatString("{id:\"%s\",usr:[", comp_key.c_str());

	int total = 0;
    while(psql->NextRow())
    {
		if(total != 0) {
			out << ",";
		}
		total++;

        const char* uid = psql->GetField("uid");
        out << FormatString("{uid:\"%s\",on:\"true\"}", uid);
    }
    out << FormatString("]}");

    RELEASE_MYSQL_RETURN(psql, 0);
}

int CompanySvc::GetServerPort(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[CompanySvc::GetServerPort]bad format:", jsonString, 1);
	std::string comp_key = root.getv("id", "");
   
    char* sql = "select distinct(portid) as id "\
                " from t41_port_service_contacts "\
                " where company_key='%s'";

	MySql *psql = CREATE_MYSQL;
    CHECK_MYSQL_STATUS(psql->Query(FormatString(sql, comp_key.c_str()).c_str()), 3)

    out << FormatString("{id:\"%s\",pts:[", comp_key.c_str());
    
	char id[64];
	int total = 0;
    while(psql->NextRow())
    {
		if(total != 0)
			out << ",";
		total++;

		READMYSQL_STR(id,id);
        out << FormatString("\"%s\"",id);
    }

    out << FormatString("]}");
    RELEASE_MYSQL_RETURN(psql, 0);
}

int CompanySvc::GetCompanyInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[CompanySvc::GetCompanyInfo]bad format:", jsonString, 1);
    std::string comp_key = root.getv("id", "");

    char* sql = "select company_key as id, name, countryname as iso3, telno as tel, fax, email, website as web, "\
                "address as addr, servicetype as svrs, detail as det, logo "\
                " from t41_company "\
                " where company_key='%s'";

    MySql *psql = CREATE_MYSQL;
    CHECK_MYSQL_STATUS(psql->Query(FormatString(sql, comp_key.c_str()).c_str()) && psql->NextRow(), 3)

    out << FormatString("{id:\"%s\",", comp_key.c_str())
        << FormatString("name:\"%s\",", NOTNULL(psql->GetField("name")))
        << FormatString("iso3:\"%s\",", NOTNULL(psql->GetField("iso3")))
        << FormatString("tel:\"%s\",", NOTNULL(psql->GetField("tel")))
        << FormatString("fax:\"%s\",", NOTNULL(psql->GetField("fax")))
        << FormatString("email:\"%s\",", NOTNULL(psql->GetField("email")))
        << FormatString("web:\"%s\",", NOTNULL(psql->GetField("web")))
        << FormatString("addr:\"%s\",", NOTNULL(psql->GetField("addr")));

	
    string svrs;
	READMYSQL_STRING(svrs, svrs);
	Tokens svrList;
	
	if(svrs.find("|") != string::npos)
		svrList = StrSplit(svrs, "|");
	else
		svrList = StrSplit(svrs, ";");

	int total = 0;
    out << "svrs:[";
    for(int i=0; i<(int)svrList.size(); i++)
    {
		if(svrList[i].empty())
			continue;

		if(total!=0)
			out << ",";
		total++;
        out << FormatString("\"%s\"", svrList[i].c_str());
    }
    
    out<< FormatString("],det:\"%s\",", NOTNULL(psql->GetField("det")));

    // logo
    int llen = 0;
    const char* picBuf = psql->GetField("logo", &llen);
	out << FormatString("llen:%d}", llen);

    // write binary logo image
    if(llen > 0)
        out << string(picBuf, llen);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int CompanySvc::OnCompanySearchMsg(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[CompanySvc::OnCompanySearchMsg]bad format:", jsonString, 1);

    const char *text	= root.getv("text", "");
    int pagecur			= root.getv("pagecur", 0);
    int pagemax			= root.getv("pagemax", 0);

    CompanyCond cond;
    cond.filter		= root.getv("filter", false);
    cond.details	= root.getv("details", false);
    cond.svrtype	= root.getv("svrtype", 0);
    cond.port		= atoi(root.getv("port", ""));
    cond.iso3		= (char*)root.getv("iso3", "");

    CompanySearch *psp = &m_companydatas;
    uint32 totals;
    char ckey[128];
    strncpy(ckey, text, 128-1);
    strlwr(ckey);
    COMPANYSET ss, fullss;
	psp->Search(ckey, &cond, fullss, ss);

    totals = (uint32)ss.size();
    if(cond.details)
    {
        out << FormatString("{pagecur:%d,pagesize:%d,amount:%d,totals:%d,tk:0,details:[", pagecur, pagemax, totals, psp->size());
        int i=0, j=0, s=(pagecur-1)*pagemax;
        for(COMPANYSET::iterator it=ss.begin(); (it!=ss.end()) && (j<pagemax); ++it, ++i)
        {
            if(i < s)
                continue;
            CompanyData *p = *it;
            if(j++ > 0)
                out << ',';
            out << FormatString("{id:\"%s\",name:\"%s\",iso3:\"%s\",addr:\"%s\",", p->id, p->name, p->iso3, p->addr);
            WriteJsonIntSet2Stream("svrtypes", p->svrs, out);
            out << ',';
            WriteJsonIntSet2Stream("svrports", p->ports.ports, out);
            out << '}';
        }
        out<<"]}";
    }
    else
    {
        std::set<int> svrtypes;
        std::set<int> svrports;
        std::set<char*> iso3s;

        for(COMPANYSET::iterator it=ss.begin(); it!=ss.end(); ++it)
        {
            CompanyData *p = *it;
            if(!p->svrs.empty())
                svrtypes.insert(p->svrs.begin(), p->svrs.end());
            if(!p->ports.ports.empty())
                svrports.insert(p->ports.ports.begin(), p->ports.ports.end());
            if(*p->iso3)
                iso3s.insert(p->iso3);
        }

        //{svrtypes:["16”, ”23”, ”40”],svrports:["30495”,”20764”],iso3:["CHN","USA","GBR"]}
        out << '{';
        WriteJsonIntSet2Stream("svrtypes", svrtypes, out);
        out << ',';
        WriteJsonIntSet2Stream("svrports", svrports, out);
        out << ',';
        WriteJsonStrSet2Stream("iso3", iso3s, out);
        out << '}';
    }

    return 0;
}
