#include "blmcom_head.h"
#include "PortSvc.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "Util.h"
#include "Geom.h"
#include <sstream>

CCountry::CCountry()
{
}

CCountry::~CCountry()
{
}

bool CCountry::LoadAll()
{
	SYSTEM_LOG("[PortSvc::loadCountry] begin ======================== ");

    MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query("select ISO3,name,namecn from t91_country"), false);

	_Country cty;
	while(psql->NextRow())
	{
		const char* pKey = psql->GetField("ISO3");
		READMYSQL_STR(name, cty.nameen);
		READMYSQL_STR(namecn, cty.namecn);
		if(cty.namecn[0] == '\0')
			strcpy(cty.namecn, cty.nameen);

		country_table[pKey] = cty;
	}

	SYSTEM_LOG("[PortSvc::loadCountry] end, total:%d ======================== ", (int)country_table.size());
	RELEASE_MYSQL_RETURN(psql, true);
}

_Country *CCountry::FindCountry(const char *countrycode)
{
    COUNTRY_TABLE::iterator p = country_table.find(countrycode);
    if (p != country_table.end())
        return &(p->second);
    return NULL;
}

CPortAlias::CPortAlias()
{
}

CPortAlias::~CPortAlias()
{
}

bool CPortAlias::LoadAll()
{
	SYSTEM_LOG("[PortSvc::loadPortAlias] begin ======================== ");

    MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query("select POINT_ID,ORIG_POINT_ID from T14_PORT_ALIAS"), false);
   
	const char *pv, *pe;
	while(psql->NextRow())
	{
		pv = psql->GetField("POINT_ID");
		pe = psql->GetField("ORIG_POINT_ID");

		if (!pe)
			pe = pv;
		portalias_table[pv] = pe;
	}

	SYSTEM_LOG("[PortSvc::loadPortAlias] end, total:%d ======================== ", (int)portalias_table.size());
	RELEASE_MYSQL_RETURN(psql, true);
}

char* CPortAlias::FindOrig(const char *aliasID)
{
    PORTALIAS_TABLE::iterator p = portalias_table.find(aliasID);
    if (p != portalias_table.end())
        return (char*)p->second.c_str();
    return NULL;
}

IMPLEMENT_SERVICE_MAP(PortSvc)

PortSvc::PortSvc()
{
}

PortSvc::~PortSvc()
{
}

bool PortSvc::Start()
{
    if(!loadPorts() || !m_alias.LoadAll() || !m_country.LoadAll()||!loadNavigations())
        return false;
	

    if(!g_MessageService::instance()->RegisterCmd(MID_PORT, this))
        return false;

    SERVICE_MAP(SID_PORTANCHORS,PortSvc,GetAllAnchors);
    SERVICE_MAP(SID_PORTREGIONCHANGE,PortSvc,GetChangeRegion);
    SERVICE_MAP(SID_PORTBASEINFO,PortSvc,GetPortBaseInfo2);
    SERVICE_MAP(SID_PORTLOGOS,PortSvc,GetPortSmallLogos);
    SERVICE_MAP(SID_PORTPIC,PortSvc,GetPortBigLogo);
    SERVICE_MAP(SID_PORTVHF,PortSvc,GetPortVhf);
    SERVICE_MAP(SID_PORTMAXSIZE,PortSvc,GetPortMaxSize);
    SERVICE_MAP(SID_PORTTERMINAL,PortSvc,GetPortAllTers);
    SERVICE_MAP(SID_PORTTRAFFIC,PortSvc,GetPortTraffic);
    SERVICE_MAP(SID_PORTTERLOGOS,PortSvc,GetTerSmallLogos);
    SERVICE_MAP(SID_PORTTERPIC,PortSvc,GetTerBigLogo);
    SERVICE_MAP(SID_PORTOPCONDITION,PortSvc,GetPortOperationCondtion);
	SERVICE_MAP(SID_ALLNAVIGATION,PortSvc,GetAllNavigation);
	SERVICE_MAP(SID_NAVIGATEION_DETAIL,PortSvc,GetNavigationDetail);

    DEBUG_LOG("[PortSvc::Start] OK......................................");

    return true;
}

int PortSvc::GetPortOperationCondtion(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PortSvc::GetPortOperationCondtion]bad format:", jsonString, 1);

	string portid = root.getv("id", "");
	char* orgportid = m_alias.FindOrig(portid.c_str());
	if (orgportid)
		portid = orgportid;
   
	char sql[1024];
	sprintf(sql, "SELECT PORTID,LDCAPACITY,HANDLINGEQUIPMENT,TUG,"\
		" WAREHOUSEYARD,MAINCARGO,"\
		" BULK_FACILITY,CONTAINER_FACILITY,TANK_FACILITY,LPGLNG_FACILITY,"\
		" PASSENGER_FACILITY,WASTE_FACILITY,OTHER_FACILITY,"\
		" OPERATION_REGULATION"\
		" FROM t41_port_opcondition where PORTID='%s'", portid.c_str());

	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql) && psql->NextRow(), 3);
    
	const char* ldcapacity = NOTNULL(psql->GetField("LDCAPACITY"));
	const char* equ = NOTNULL(psql->GetField("HANDLINGEQUIPMENT"));
	const char* tug =	NOTNULL(psql->GetField("TUG"));
	const char* warehouse = NOTNULL(psql->GetField("WAREHOUSEYARD"));
	const char* cargo = NOTNULL(psql->GetField("MAINCARGO"));
	const char* bulk_fac = NOTNULL(psql->GetField("BULK_FACILITY"));
	const char* con_fac = NOTNULL(psql->GetField("CONTAINER_FACILITY"));
	const char* tank_fac = NOTNULL(psql->GetField("TANK_FACILITY"));
	const char* lgp_fac = NOTNULL(psql->GetField("LPGLNG_FACILITY"));
	const char* passen_fac = NOTNULL(psql->GetField("PASSENGER_FACILITY"));
	const char* waste_fac = NOTNULL(psql->GetField("WASTE_FACILITY"));
	const char* other_fac = NOTNULL(psql->GetField("OTHER_FACILITY"));
	const char* oper_reg = NOTNULL(psql->GetField("OPERATION_REGULATION"));

	out<<"[\""<<ldcapacity<<"\","
		<<"\""<<equ<<"\","
		<<"\""<<warehouse<<"\","
		<<"\""<<tug<<"\","
		<<"\""<<cargo<<"\","
		<<"\""<<bulk_fac<<"\","
		<<"\""<<con_fac<<"\","
		<<"\""<<tank_fac<<"\","
		<<"\""<<lgp_fac<<"\","
		<<"\""<<passen_fac<<"\","
		<<"\""<<waste_fac<<"\","
		<<"\""<<other_fac<<"\","
		<<"\""<<oper_reg<<"\"]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int PortSvc::GetTerBigLogo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PortSvc::GetTerBigLogo]bad format:", jsonString, 1);

	string id = root.getv("id", "");
	string pid = root.getv("pid", "");

	char sql[1024];
	sprintf(sql, "SELECT PICKEY,PICTURE FROM T41_SHIP_PICTURES WHERE SHIPID='%s' AND PICKEY='%s'", id.c_str(), pid.c_str());

    MySql *psql = CREATE_MYSQL;
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out << "{\"id\":\"" << id << ",\"pid\":\"" << pid;
    if(psql->NextRow())
    {
        int piclen;
        char* picdata = (char*)psql->GetField("PICTURE",&piclen);
        out<<",len:"<<piclen<<'}'<<string(picdata,piclen);
    }
    else
    {
        out<<'}';
    }
    
    RELEASE_MYSQL_RETURN(psql, 0);
}

int PortSvc::GetTerSmallLogos(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PortSvc::GetTerSmallLogos]bad format:", jsonString, 1);
	string id = root.getv("id", "");
	Tokens includeList;
	Json* filters = root["filter"];

	for(int i=0; i<filters->size(); i++)
	{
		includeList.push_back(filters->getv(i, ""));
	}

	//封装SQL
	stringstream ss;
	ss << "SELECT PICKEY,PICTURE FROM T41_SHIP_PICTURES WHERE SHIPID='" << id << "'";
	for (size_t j=0; j<includeList.size(); j++)
	{
		ss<<" AND PICKEY='"<<includeList[j]<<'\'' ;
	}

    MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(ss.str().c_str()), 3);

	Tokens picDataList;

	int total = 0;
	out << "{\"id\":\" "<< id << "\",pics:[";
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char *pickey = psql->GetField("PICKEY");
		int piclen;
		char* picdata = (char*)psql->GetField("PICTURE",&piclen);
		out<<"{\"pid\":\""<<(pickey?pickey:"\"\"")<<"\","
			<<"len:"<<piclen<<'}';
		picDataList.push_back(string(picdata,piclen));
	}
	out<<"]}";

	for (size_t k=0; k<picDataList.size(); k++)
	{
		out<<picDataList[k];
	}

    RELEASE_MYSQL_RETURN(psql, 0);
}

int PortSvc::GetPortAllTer2(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[PortSvc::GetPortAllTer2]bad format:", jsonString, 1);
	string portid = root.getv("id", "");
	char* orgportid = m_alias.FindOrig(portid.c_str());
	if (orgportid)
		portid = orgportid;

	string strterminalid;

	char sql[4*1024];
	MySql *psql = CREATE_MYSQL;

	int total = 0;
	out << '{';
	sprintf(sql, "SELECT Terminal_id,Name FROM t41_port_terminal where PORTID='%s'", portid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<<"terlist:[";
	while(psql->NextRow())
	{
		const char* terid = NOTNULL(psql->GetField("Terminal_id"));
		const char* tername = NOTNULL(psql->GetField("NAME"));

		if(total != 0)
		{
			out << ",";
			strterminalid += ",\"" + string(terid) + "\"";
		}
		else
		{
			strterminalid += "\"" + string(terid) + "\"";
		}
		total++;

		char temp[1024];
		sprintf(temp,"[\"%s\",\"%s\"]",terid,tername);
		out<<temp;
	}
	out<<']';

	sprintf(sql, "SELECT T2.BERTH_ID,T2.TERMINAL_ID,T2.NAME,T2.LENGTH,T2.DEPTH,T2.DRAUGHT,"\
		" T2.DWT,T2.CARGOTYPE,T2.LOADSPEED,T2.DISCHARGESPEED,T2.REMARK "\
		" FROM T41_PORT_BERTH T2 WHERE T2.TERMINAL_ID in (%s)", strterminalid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	total = 0;
	out<<",berlist:[";
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* terid = NOTNULL(psql->GetField("TERMINAL_ID"));
		const char* name = NOTNULL(psql->GetField("NAME"));
		const char* type = NOTNULL(psql->GetField("CARGOTYPE"));
		const char* dwt = NOTNULL(psql->GetField("DWT"));
		const char* length = NOTNULL(psql->GetField("LENGTH"));
		const char* draft = NOTNULL(psql->GetField("DRAUGHT"));
		const char* depth = NOTNULL(psql->GetField("DEPTH"));
		const char* description = NOTNULL(psql->GetField("REMARK"));
		out<<"[\""<<name<<"\","
			<<"\""<<type<<"\","
			<<"\""<<dwt<<"\","
			<<"\""<<length<<"\","
			<<"\""<<draft<<"\","
			<<"\""<<depth<<"\","
			<<"\""<<description<<"\","
			<<"\""<<terid<<"\"]";
	}
	out<<"]}";    

	RELEASE_MYSQL_RETURN(psql, 0);
}

int PortSvc::GetPortAllTers(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PortSvc::GetPortAllTers]bad format:", jsonString, 1);
	string portid = root.getv("id", "");
	string terminalid = root.getv("terminalid", "");
	string isall = root.getv("all", "");
	string strterminalid;

	if(!isall.empty())
		return GetPortAllTer2(pUid, jsonString, out);

	char* orgportid = m_alias.FindOrig(portid.c_str());
	if (orgportid)
		portid = orgportid;
	
	char sql[1024];
    MySql *psql = CREATE_MYSQL;
    
	int total = 0;
	out << '{';
    if(!terminalid.empty())
    {
        out<<"terlist:[]";
		strterminalid = terminalid;
    }
    else
    {
        sprintf(sql, "SELECT Terminal_id,Name FROM t41_port_terminal where PORTID='%s'", portid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

        out<<"terlist:[";
		while(psql->NextRow())
		{
			if(total != 0)
				out << ",";
			total++;

			const char* terid = NOTNULL(psql->GetField("Terminal_id"));
			const char* tername = NOTNULL(psql->GetField("NAME"));
			if (total==1)
				strterminalid = terid;
			char temp[512];
			sprintf(temp,"[\"%s\",\"%s\"]",terid,tername);
			out<<temp;
		}
        out<<']';
    }

    sprintf(sql, "SELECT T2.BERTH_ID,T2.TERMINAL_ID,T2.NAME,T2.LENGTH,T2.DEPTH,T2.DRAUGHT,"\
             " T2.DWT,T2.CARGOTYPE,T2.LOADSPEED,T2.DISCHARGESPEED,T2.REMARK "\
             " FROM T41_PORT_BERTH T2 WHERE T2.TERMINAL_ID = '%s'", strterminalid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	total = 0;
	out<<",berlist:[";
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* name = NOTNULL(psql->GetField("NAME"));
		const char* type = NOTNULL(psql->GetField("CARGOTYPE"));
		const char* dwt = NOTNULL(psql->GetField("DWT"));
		const char* length = NOTNULL(psql->GetField("LENGTH"));
		const char* draft = NOTNULL(psql->GetField("DRAUGHT"));
		const char* depth = NOTNULL(psql->GetField("DEPTH"));
		const char* description = NOTNULL(psql->GetField("REMARK"));
		out<<"[\""<<name<<"\","
			<<"\""<<type<<"\","
			<<"\""<<dwt<<"\","
			<<"\""<<length<<"\","
			<<"\""<<draft<<"\","
			<<"\""<<depth<<"\","
			<<"\""<<description<<"\"]";
	}
    out<<']';    
	
    sprintf(sql, "SELECT T1.COMPANY_KEY,T2.Name FROM t41_port_terminal T1"\
                     " LEFT OUTER JOIN t41_company T2 ON T1.COMPANY_KEY = T2.COMPANY_KEY"\
                     " WHERE T1.Terminal_id='%s'", strterminalid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    out<<",comp:[";
    if(psql->NextRow())
    {
		const char* compkey = NOTNULL(psql->GetField("COMPANY_KEY"));
		const char* compname = NOTNULL(psql->GetField("Name"));
		out<<"\""<<compkey<<"\",\""<<compname<<"\"";
    }
    out<<"]}";

    RELEASE_MYSQL_RETURN(psql, 0);
}
int PortSvc::GetPortTraffic(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PortSvc::GetPortTraffic]bad format:", jsonString, 1);
	string portid = root.getv("id", "");
	char* orgportid = m_alias.FindOrig(portid.c_str());
	if (orgportid)
		portid = orgportid;

	char sql[1024];
	sprintf(sql, "SELECT PORTID,CHANNEL,PILOT,DOCUMENT,QUARANTINE,IMMIGRATIONFORMALITY,SECURITYMANAGE,"\
				 " SHIPPING_REGULATION,CUSTOM,STOWAWAY,VTS_RADAR"\
				 " FROM t41_port_traffic  where PORTID='%s'", portid.c_str());

    MySql *psql = CREATE_MYSQL;
    CHECK_MYSQL_STATUS(psql->Query(sql) && psql->NextRow(), 3);
    
	const char* vts =	NOTNULL(psql->GetField("VTS_RADAR"));
	const char* channel = NOTNULL(psql->GetField("CHANNEL"));
	const char* pilot =	NOTNULL(psql->GetField("PILOT"));
	const char* document = NOTNULL(psql->GetField("DOCUMENT"));;
	const char* quaran =	NOTNULL(psql->GetField("QUARANTINE"));
	const char* securitymanage = NOTNULL(psql->GetField("SECURITYMANAGE"));
	const char* ship_reg = NOTNULL(psql->GetField("SHIPPING_REGULATION"));
	const char* customs = NOTNULL(psql->GetField("CUSTOM"));
	const char* immigra = NOTNULL(psql->GetField("IMMIGRATIONFORMALITY"));
	const char* stowawy = NOTNULL(psql->GetField("STOWAWAY"));

	out << "[\"" << vts << "\",\"" << 
		channel << "\",\"" << 
		pilot << "\",\"" << 
		document << "\",\"" << 
		quaran << "\",\"" << 
		securitymanage << "\",\"" << 
		ship_reg << "\",\"" << 
		customs << "\",\"" << 
		immigra << "\",\"" << 
		stowawy << "\"]";
	
    RELEASE_MYSQL_RETURN(psql, 0);
}

int PortSvc::GetPortMaxSize(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PortSvc::GetPortMaxSize]bad format:", jsonString, 1);
	string portid = root.getv("id", "");
	char* orgportid = m_alias.FindOrig(portid.c_str());
	if (orgportid)
		portid = orgportid;

	char sql[1024];
	sprintf(sql, "SELECT t1.SHIPTYPE CARGOTYPE,t1.LOA,t1.DWT,t1.BEAM,t1.DRAFT,t1.DEPTH,t1.REMARK FROM t41_port_cargo_maxsize t1 WHERE t1.PORTID='%s'", portid.c_str());
    
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
   
	int total = 0;
	out<<"[";
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* type = NOTNULL(psql->GetField("CARGOTYPE"));
		const char* len = NOTNULL(psql->GetField("LOA"));
		const char* beam = NOTNULL(psql->GetField("BEAM"));
		const char* depth = NOTNULL(psql->GetField("DEPTH"));
		const char* dwt = NOTNULL(psql->GetField("DWT"));
		const char* draft = NOTNULL(psql->GetField("DRAFT"));
		const char* rmk = NOTNULL(psql->GetField("REMARK"));
		out << FormatString("[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]",type,len,beam,depth,dwt,draft,rmk);
	}
	out<<"]";
    
    RELEASE_MYSQL_RETURN(psql, 0);
}

int PortSvc::GetPortVhf(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PortSvc::GetPortVhf]bad format:", jsonString, 1);
	string portid = root.getv("id", "");
	char* orgportid = m_alias.FindOrig(portid.c_str());
	if (orgportid)
		portid = orgportid;

	char sql[1024];
    sprintf(sql, "SELECT T1.CHANNEL,T1.REMARK, T2.SENDHZ,T2.RECEIVEHZ,T2.STATUS,T1.STATUS WLA FROM T41_port_vhf T1"\
                    " LEFT OUTER JOIN t91_vhf_channel T2 ON T1.CHANNEL=T2.CHANNELNO "\
                    " where T1.PORTID='%s' ORDER BY T1.CHANNEL ASC,T2.STATUS ASC", portid.c_str());

	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int total = 0;
	out << "[";
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char* name = NOTNULL(psql->GetField("CHANNEL"));
		const char* working = NOTNULL(psql->GetField("SENDHZ"));
		const char* listening = NOTNULL(psql->GetField("RECEIVEHZ"));
		const char* rmk = NOTNULL(psql->GetField("REMARK"));
		const char* status = NOTNULL(psql->GetField("STATUS"));
		const char* wla = NOTNULL(psql->GetField("WLA"));
		out << "[\""<< name << "\",\"" << working << "\",\"" << listening << "\",\"" << status << "\",\"" << rmk << "\",\"" << wla << "\"]";
	}
	out << "]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int PortSvc::GetPortBigLogo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PortSvc::GetPortBigLogo]bad format:", jsonString, 1);
	string id = root.getv("id", "");
	string pid = root.getv("pid", "");

	char sql[1024];
	sprintf(sql, "SELECT PICKEY,PICTURE FROM T41_SHIP_PICTURES WHERE SHIPID='%s' AND PICKEY='%s'", id.c_str(), pid.c_str());

    MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
   
    out << "{id:\"" << id << "\",pid:\"" << pid << '"';

	if(psql->NextRow())
	{
		int piclen;
		char* picdata = (char*)psql->GetField("PICTURE",&piclen);
		out << ",len:" << piclen << '}' << string(picdata,piclen);
	}
	else
	{
		out << "}";
	}
    
    RELEASE_MYSQL_RETURN(psql, 0);
}

int PortSvc::GetPortSmallLogos(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PortSvc::GetPortSmallLogos]bad format:", jsonString, 1);
	string id = root.getv("id", "");
	Json* filter = root["filter"];

	stringstream sql;
	sql << "SELECT PICKEY,PICTURE FROM T41_SHIP_PICTURES WHERE SHIPID='" << id << "'";

	for(int i=0; i<filter->size(); i++)
		sql << " AND PICKEY='" << filter->getv(i, "") << "'";

    MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql.str().c_str()), 3);

	out<<"{id:\""<< id << "\",pics:[";

	int total = 0;
	Tokens picDataList;
	while(psql->NextRow())
	{
		if(total != 0)
			out << ",";
		total++;

		const char *pickey = psql->GetField("PICKEY");
		int piclen;
		const char* picdata = (char*)psql->GetField("PICTURE",&piclen);
		out << "{pid:\"" << (pickey?pickey:"\"\"") << "\",len:" << piclen << "}";
		picDataList.push_back(string(picdata,piclen));
	}

	out << "]}";
	for (size_t k=0; k<picDataList.size(); k++)
	{
		out << picDataList[k];
	}

    RELEASE_MYSQL_RETURN(psql, 0);
}

int PortSvc::GetPortBaseInfo2(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PortSvc::GetPortBaseInfo2]bad format:", jsonString, 1);
	string portid = root.getv("id", "");
	char* orgportid = m_alias.FindOrig(portid.c_str());
	if (orgportid)
		portid = orgportid;

	char sql[1024];
    MySql *psql = CREATE_MYSQL;

    //查询港口基本信息
    char* sql_base = "SELECT T.TIMEZONE,T.CHARTNO,T.ZONEMOVE,T.LATITUDE,"\
                     " T.LONGITUDE,T.PORTID,T.NAME,T.ISO3,"\
                     " T.COMPANY_KEY,T1.NAME COMPNAY_NAME FROM T41_PORT T "\
                     " LEFT OUTER JOIN t41_company T1 ON T.COMPANY_KEY = T1.COMPANY_KEY WHERE T.PortID = '%s'";
    //查询别名信息
    char* sql_alias = "SELECT T.RELA_PORTID ALIASID FROM t41_port_relation T WHERE T.PORTID='%s'"\
                      " UNION "\
                      " SELECT T1.POINT_ID ALIASID FROM t14_port_alias T1 where ORIG_POINT_ID='%s' limit 1";
    //查询码头数量信息
    char* sql_ternum = "SELECT COUNT(1) ternum FROM t41_port_terminal WHERE portid='%s'";

    //查询基本信息
	sprintf(sql, sql_base, portid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql) && psql->NextRow(), 3);
    
	const char* timezone	= psql->GetField("TIMEZONE");
	const char* chartno		= psql->GetField("CHARTNO");
	const char* lat			= NOTNULL(psql->GetField("LATITUDE"));
	const char* lon			= NOTNULL(psql->GetField("LONGITUDE"));
	const char* id			= psql->GetField("PORTID");
	const char* name		= NOTNULL(psql->GetField("NAME"));
	const char* iso3		= psql->GetField("ISO3");
	const char* compid		= psql->GetField("COMPANY_KEY");
	const char* compna		= NOTNULL(psql->GetField("COMPNAY_NAME"));

	string szLat = JsonReplace(lat);
	string szLon = JsonReplace(lon);
	string szName = JsonReplace(name);
	string szComp = JsonReplace(compna);

	out<<"{\"id\":\""<<NOTNULL(id)<<'\"'
		<<",\"na\":\""<<szName<<'\"'
		<<",\"iso\":\""<<NOTNULL(iso3)<<'\"'
		<<",\"cno\":\""<<NOTNULL(chartno)<<'\"'
		<<",\"tz\":\""<<NOTNULL(timezone)<<'\"'
		<<",\"lat\":\""<<szLat<<'\"'
		<<",\"lon\":\""<<szLon<<'\"'
		<<",\"compid\":\""<<NOTNULL(compid)<<'\"'
		<<",\"compna\":\""<<szComp<<'\"';

	//查询别名信息
	const char* aliasid = NULL;
	sprintf(sql, sql_alias, portid.c_str(), portid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow())
	{
		aliasid = psql->GetField("ALIASID");
	}
	out<<",\"aliasid\":\""<<NOTNULL(aliasid)<<'\"';

	//查询码头信息
	const char* ternum = NULL;
	sprintf(sql, sql_ternum, portid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if(psql->NextRow())
	{
		ternum = psql->GetField("ternum");
	}
	out<<",\"ternum\":\""<<NOTNULL(ternum)<<"\"}";

    RELEASE_MYSQL_RETURN(psql, 0);
}

int PortSvc::GetChangeRegion(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PortSvc::GetChangeRegion]bad format:", jsonString, 1);
	string time = root.getv("time", "");
	string type = root.getv("type", "");

	char sql[1024];
    if (time.empty())
        sprintf(sql, "SELECT T1.regionid,MAX(T1.time) MT FROM t41_port_region_change T1 WHERE T1.TYPE = '%s' GROUP BY T1.REGIONID", type.c_str());
    else
        sprintf(sql, "SELECT T1.regionid,MAX(T1.time) MT FROM t41_port_region_change T1 WHERE T1.TIME > '%s' AND T1.TYPE = '%s' GROUP BY T1.REGIONID",time.c_str(), type.c_str());
	
	MySql *psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	std::stringstream ss;
	const char* tm = NULL;
	int total = 0;
	while(psql->NextRow())
	{
		if(total == 0)
			tm = psql->GetField("MT");
		else
			ss << "|";
		total++;

		const char* regionid = psql->GetField("regionid");
		ss << regionid;
	}

	out << "{\"time\":\"" << NOTNULL(tm) << "\",\"regions\":\"" << ss.str() << "\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

int PortSvc::GetAllAnchors(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[PortSvc::GetAllAnchors]bad format:", jsonString, 1);
	string text = root.getv("regions", "");
	Tokens regions = StrSplit(text, "|");

	char sql[10240];
    MySql *psql = CREATE_MYSQL;
  
	int total = 0;
    out << '[';
    for (int i=0; i<(int)regions.size(); i++)
    {
        sprintf(sql, "SELECT T.PORTID,T.ANCHORAGEID,T.NAME,T.LOCTYPE,T.LOCPOINTSET FROM T41_PORT_ANCHORAGE T WHERE T.REGIONSET LIKE '%%%s%%'", regions[i].c_str());
        CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		while(psql->NextRow())
		{
			if(total != 0)
				out << ",";
			total++;

			const char* portid = psql->GetField("PORTID");
			const char* anchorid = psql->GetField("ANCHORAGEID");
			const char* anchorname = psql->GetField("NAME");
			const char* anchorLocType = NOTNULL(psql->GetField("LOCTYPE"));
			const char* anchorLocation = psql->GetField("LOCPOINTSET");

			if (strcasecmp(anchorLocType,"3") == 0 || strcasecmp(anchorLocType,"4")==0)
			{
				anchorLocType = "0";
			}
			out << "{\"portid\":\"" << NOTNULL(portid) << '\"'
				<< ",\"aid\":\"" << NOTNULL(anchorid) << '\"'
				<< ",\"ana\":\"" << NOTNULL(anchorname) << '\"'
				<< ",\"alt\":\"" << NOTNULL(anchorLocType) << '\"'
				<< ",\"aloc\":\"" << NOTNULL(anchorLocation) << '\"'
				<< ",\"areg\":\"" << regions[i].c_str() << '\"'
				<< '}';
		}
    }

    out << ']';
    RELEASE_MYSQL_RETURN(psql, 0);
}

int PortSvc::GetAllNavigation(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[PortSvc::GetAllNavigation]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");

	map<string,NavigationInfo>::iterator it=m_mapNavigation.begin();


	out<<"{seq:\""<<seq<<"\",data:[";

	for (;it!=m_mapNavigation.end();it++)
	{

		NavigationInfo naviInfo=it->second;

		if (it!=m_mapNavigation.begin())
		{
			out<<",";
		}

		out<<"{num:\""<<naviInfo.navi_id<<"\",na:\""<<naviInfo.navi_name<<"\",lon:"<<naviInfo.xpos<<",lat:"<<naviInfo.ypos<<"}";
	}
	out<<"]}";


	return 0;

}
int PortSvc::GetNavigationDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[PortSvc::GetNavigationDetail]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
    string navi_id = root.getv("mmsi", "");

	NavigationInfo naviInfo;
    
	if (m_mapNavigation.find(navi_id)!=m_mapNavigation.end())
	{
		naviInfo=m_mapNavigation[navi_id];
	}

	out<<"{seq:\""<<seq<<"\",num:\""<<naviInfo.navi_id<<"\",type:\""<<naviInfo.navi_type<<"\",etype:\""<<naviInfo.equip_type<<"\",enum:\""<<naviInfo.euqip_id;
	
	out<<"\",lane:\""<<naviInfo.navi_channel<<"\",sea:\""<<naviInfo.sea_area<<"\",office:\""<<naviInfo.navi_office<<"\",station:\""<<naviInfo.navi_station<<"\",factory:\""<<naviInfo.conpany<<"\"}";
   

	return 0;
}

bool PortSvc::loadNavigations()
{
	SYSTEM_LOG("[PortSvc::loadNavigations] begin ======================== ");

	MySql* psql = CREATE_MYSQL;
	std::string sql = "select Navig_id,Navig_name,Navig_type,equip_type,equip_id,navig_station,navig_in,navig_channel,sea_area,company,latitude_dd,longitude_dd from t99_navigation";
	CHECK_MYSQL_STATUS(psql->Query(sql.c_str()), false);

	while(psql->NextRow())
	{
		NavigationInfo naviInfo;
		READMYSQL_STRING(Navig_id,naviInfo.navi_id)
		READMYSQL_STRING(Navig_name,naviInfo.navi_name)
		READMYSQL_STRING(Navig_type,naviInfo.navi_type)
		READMYSQL_STRING(equip_type,naviInfo.equip_type)
		READMYSQL_STRING(equip_id,naviInfo.euqip_id)
		READMYSQL_STRING(navig_station,naviInfo.navi_station)
		READMYSQL_STRING(navig_in,naviInfo.navi_office)
		READMYSQL_STRING(navig_channel,naviInfo.navi_channel)
		READMYSQL_STRING(sea_area,naviInfo.sea_area)
		READMYSQL_STRING(company,naviInfo.conpany)
		READMYSQL_STRING(latitude_dd,naviInfo.ypos)
		READMYSQL_STRING(longitude_dd,naviInfo.xpos)
		m_mapNavigation.insert(std::make_pair(naviInfo.navi_id, naviInfo));
	}

	SYSTEM_LOG("[PortSvc::loadNavigations] end, total:%d ======================== ", (int)m_mapNavigation.size());
	
	RELEASE_MYSQL_RETURN(psql, true);

}

bool PortSvc::loadPorts()
{
	SYSTEM_LOG("[PortSvc::loadPorts] begin ======================== ");

    MySql* psql = CREATE_MYSQL;
    std::string sql = "select portid,name,namecn,iso3,longitude_dd,latitude_dd  from t41_port";
    CHECK_MYSQL_STATUS(psql->Query(sql.c_str()), false);

    while(psql->NextRow())
    {
        PortInfo *oneportdata = new PortInfo;
        char namechr[100];
        READMYSQL_STR(name,namechr);
		oneportdata->portname = namechr;
		READMYSQL_STR(namecn,namechr);
		oneportdata->portnamecn = namechr;
		READMYSQL_STR(iso3, oneportdata->iso3);
        READMYSQL_INT(portid, oneportdata->portid, 0);
        READMYSQL_DOUBLE(longitude_dd,oneportdata->x, 0);
        READMYSQL_DOUBLE(latitude_dd,oneportdata->y, 0);
        int _x = int((oneportdata->x + 180)/2);
        int _y = int((oneportdata->y + 90)/2);
        if(_x>=180)_x=179;
        if(_y>=90)_y=89;
        if(grid[_x][_y].empty())
        {
            VecPortInfo tempVec;
            tempVec.push_back(oneportdata);
            grid[_x][_y] = tempVec;
        }
        else
        {
            grid[_x][_y].push_back(oneportdata);
        }
        m_mapPort.insert(std::make_pair(oneportdata->portid, oneportdata));
    }

	SYSTEM_LOG("[PortSvc::loadPorts] end, total:%d ======================== ", (int)m_mapPort.size());
    RELEASE_MYSQL_RETURN(psql, true);
}

void PortSvc::GetGridIndex(double lon, double lat, int& i, int& j)
{
    if(180 - lon <= EP) i = 179;
    else
        i = (int)((lon + 180) / 2);
    j = (int)((lat + 90) / 2);
}

PortInfo* PortSvc::FindNearestPort(double lon, double lat, double& dist, double& degree, bool extra)
{
    int try_count = 1;
    double try_rate = 0.0;
    int min_i, max_i, min_j, max_j;
    double xmin, xmax, ymin, ymax, _xmin, _xmax;
    bool cross180;
    double _finalx, _finaly;
    PortInfo* p = NULL;
    VecPortInfo portList;
    VecPortInfoIter iter;
    while(!p)
    {
        xmin = lon - 1*(try_count + try_rate);
        xmax = lon + 1*(try_count + try_rate);
        if((ymin = lat - 0.5*(try_count+try_rate)) < -90) ymin = -90;
        if((ymax = lat + 0.5*(try_count+try_rate)) > 90) ymax = 90;
        // consider the longitude 180 problem
        cross180 = false;
        if(xmin < -180)
        {
            cross180 = true;
            _xmin = xmin + 360;
            _xmax = 180;
            xmin = -180;
        }
        else if(xmax > 180)
        {
            cross180 = true;
            _xmin = -180;
            _xmax = xmax - 360;
            xmax = 180;
        }
        GetGridIndex(xmin, ymin, min_i, min_j);
        GetGridIndex(xmax, ymax, max_i, max_j);
        for(int i=min_i; i<=max_i; i++)
        {
            for(int j=min_j; j<=max_j; j++)
            {
                VecPortInfoIter iter;
                for(iter = grid[i][j].begin(); iter != grid[i][j].end(); iter++)
                {
                    PT pt((*iter)->x, (*iter)->y);
                    if(PtInBox(pt, xmin, ymin, xmax, ymax))
                        portList.push_back(*iter);
                }
            }
        }
        if(cross180)
        {
            GetGridIndex(_xmin, ymin, min_i, min_j);
            GetGridIndex(_xmax, ymax, max_i, max_j);
            for(int i=min_i; i<=max_i; i++)
            {
                for(int j=min_j; j<=max_j; j++)
                {
                    for(iter = grid[i][j].begin(); iter != grid[i][j].end(); iter++)
                    {
                        PT pt((*iter)->x, (*iter)->y);
                        if(PtInBox(pt, _xmin, ymin, _xmax, ymax))
                            portList.push_back(*iter);
                    }
                }
            }
        }
        if(!portList.empty())
            break;
        try_count++;
        try_rate += 0.5;
    }
    if(portList.size() == 1)
    {
        p = portList[0];
        dist = CircleLineUtl::CalculateDistance(p->x, p->y, lon, lat);
        _finalx = lon - p->x;
        _finaly = lat - p->y;
    }
    else
    {
        size_t min_idx = 0;
        double _dist, min_dist = CircleLineUtl::CalculateDistance(lon, lat, portList[0]->x, portList[0]->y);
        for(int i=1; i<(int)portList.size(); i++)
        {
            if((_dist = CircleLineUtl::CalculateDistance(lon, lat, portList[i]->x, portList[i]->y)) < min_dist)
            {
                min_idx = i;
                min_dist = _dist;
            }
        }
        p = portList[min_idx];
        dist = min_dist;
        _finalx = lon - p->x;
        _finaly = lat - p->y;
    }
    if(!extra)
        return p;
    dist /= 1852; //转换成海里
    //根据 _finalx， _finaly 得到夹角
    if(fabs(_finalx) <= 0.0001 && fabs(_finaly) <= 0.0001 )
    {
        degree = 0;
    }
    else if(_finalx > 0 && _finaly > 0 )
    {
        degree = int(90 - atan(_finaly/_finalx)/PER_DEGREE);
    }
    else if(_finalx > 0 && _finaly < 0 )
    {
        degree = int(90 + atan(fabs(_finaly)/_finalx)/PER_DEGREE);
    }
    else if(_finalx < 0 && _finaly < 0 )
    {
        degree = int(270 - atan(fabs(_finaly/_finalx))/PER_DEGREE);
    }
    else if(_finalx < 0 && _finaly >0 )
    {
        degree = int(270 + atan(fabs(_finaly/_finalx))/PER_DEGREE);
    }
    else if(_finalx == 0)
    {
        degree = _finaly>=0?0:180;
    }
    else if(_finaly == 0)
    {
        degree = _finalx >=0 ? 90: 270;
    }
    return p;
}

PortInfo* PortSvc::FindPortByID(int portid)
{
    MapPortInfoIter iter = m_mapPort.find(portid);
    if(iter == m_mapPort.end())
        return NULL;
    return iter->second;
}
