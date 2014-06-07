#include "CompanySearch.h"
#include "ObjectPool.h"
#include "Util.h"
#include "Pinyin4j.h"
#include "LogMgr.h"

CompanySearch::CompanySearch()
{
}

CompanySearch::~CompanySearch()
{
	clear();
}

void CompanySearch::clear()
{
	for(COMPANY_TABLE::iterator iter = ctable.begin(); iter != ctable.end(); iter++)
		delete iter->second;
	ctable.clear();
}

void CompanySearch::OnUpdateCompanyData(CompanySearch *s)
{
	if(s)
	{
		ctable.swap(s->ctable);
	}
}

//װ��һϵ����صı�
bool CompanySearch::LoadDatabase()
{
	SYSTEM_LOG("[CompanySearch::LoadDatabase] begin ========================== ");
	MySql *psql = CREATE_MYSQL;

	PORTSET ps;
	bool bRet = LoadCompany(psql) && LoadPortset(psql, ps);
	if(bRet)
	{
		MergePortset(ps);
		RELEASE_MYSQL_RETURN(psql, true);
	}
	
	SYSTEM_LOG("[CompanySearch::LoadDatabase] end ========================== ");
	RELEASE_MYSQL_RETURN(psql, false);
}

void CompanyData::setsvrs(char *pv)
{
	if(pv && *pv)
	{
		Tokens tokens = StrSplit(pv, ";");
		for(int i=0; i<(int)tokens.size(); ++i)
		{
			svrs.insert(atoi(tokens[i].c_str()));
		}
	}
}

bool CompanySearch::LoadCompany(MySql *psql)
{
	SYSTEM_LOG("[CompanySearch::LoadCompany] begin load =============");
	if(psql->Query("select company_key as id,name,countryname as iso3,address as addr,servicetype as svrs from t41_company"))
	{
		while(psql->NextRow())
		{
			char svrs[1024];
			CompanyData *data = new CompanyData();
			READMYSQL_STR(id, data->id);
			READMYSQL_STR(name, data->name);
			READMYSQL_STR(iso3, data->iso3);
			READMYSQL_STR(addr, data->addr);
			READMYSQL_STR(svrs, svrs);

			data->calctrimname();
			data->setsvrs(svrs);
			ctable.insert(COMPANY_TABLE::value_type(data->id, data));
		}
		SYSTEM_LOG("[CompanySearch::LoadCompany] end load, total:%d =============", psql->GetRowCount());
		return true;
	}

	SYSTEM_LOG("[CompanySearch::LoadCompany] end load, failed =============");
	return false;
}

bool CompanySearch::LoadPortset(MySql *psql, PORTSET &ps)
{
	SYSTEM_LOG("[CompanySearch::LoadPortset] begin load =============");
	if(psql->Query("SELECT company_key,portid FROM t41_port_service_contacts"))
	{
		PORTSET::iterator it;
		while(psql->NextRow())
		{
			char id[64];
			char portid[64];
			READMYSQL_STR(company_key, id);
			READMYSQL_STR(portid, portid);

			it = ps.find(id);
			if(it == ps.end())
			{
				PortSet s;
				s.ports.insert(atoi(portid));
				ps.insert(PORTSET::value_type(id, s));
			}
			else
			{
				it->second.ports.insert(atoi(portid));
			}
		}
		SYSTEM_LOG("[CompanySearch::LoadPortset] end load, total:%d =============", psql->GetRowCount());
		return true;
	}

	SYSTEM_LOG("[CompanySearch::LoadPortset] end load, failed =============");
	return false;
}

bool CompanySearch::MergePortset(PORTSET &ps)
{
	for(PORTSET::iterator it=ps.begin(); it!=ps.end(); ++it)
	{
		COMPANY_TABLE::iterator j=ctable.find(it->first);
		if(j != ctable.end())
		{
			it->second.ports.swap(j->second->ports.ports);
		}
	}
	return true;
}

//likeƥ��
uint32 CompanySearch::Like_Trimname(const char *tname, COMPANYSET &ss)
{
	uint32 totals = 0;
	int tlen = (int)strlen(tname);
	for(COMPANY_TABLE::iterator it=ctable.begin(); it!=ctable.end(); ++it)
	{
		CompanyData *p = it->second;
		if((p->trimnamelen>=tlen) && strstr(p->trimname, tname))
		{
			++totals;
			ss.insert(p);
		}
	}
	return totals;
}

//���ƶ�ƥ��
uint32 CompanySearch::DistanceTrimname(const char *tname, COMPANYSET &ss)
{
	uint32 totals = 0;
	int tlen = (int)strlen(tname);
	for(COMPANY_TABLE::iterator it=ctable.begin(); it!=ctable.end(); ++it)
	{
		CompanyData *p = it->second;
		int l = p->trimnamelen-tlen;
		if(l < 0) l = -l;
		if((l<=1) && EditDistance(p->trimname, p->trimnamelen, tname, tlen)<=2)
		{
			++totals;
			ss.insert(p);
		}
	}
	return totals;
}

//������ͺ�״̬�Ƿ�һ��
inline bool CompanySearch::Valify(CompanyData *pdata, CompanyCond *pcond)
{
	if(!pcond->filter)
		return true;
	//���svrtype
	if(pcond->svrtype > 0)
	{
		if( (pdata->svrs.empty()) || pdata->svrs.find(pcond->svrtype)==pdata->svrs.end())
			return false;
	}
	//���iso3
	if(*pcond->iso3)
	{
		if( (*pdata->iso3==0) ||
			strstr(pdata->iso3, pcond->iso3)==NULL)
			return false;
	}
	//���port
	if(pcond->port)
	{
		if(pdata->ports.ports.find(pcond->port)==pdata->ports.ports.end())
			return false;
	}
	return true;
}

//���˽����
bool CompanySearch::FilterCompanySet(COMPANYSET &fullss, COMPANYSET &ss, CompanyCond *pcond)
{
	if(pcond->nofilter())
	{
		ss = fullss;
		return !ss.empty();
	}
	for(COMPANYSET::iterator it=fullss.begin(); it!=fullss.end(); ++it)
	{
		if(Valify(*it, pcond))
		{
			ss.insert(*it);
		}
	}
	return !ss.empty();
}

//search����
bool CompanySearch::Search(char *text, CompanyCond *pcond, COMPANYSET &fullss, COMPANYSET &ss)
{
	//���������
	DelSpecialChar(text);
	if(*text==0)
	{
		//֧��textΪ�յ����
		for(COMPANY_TABLE::iterator it=ctable.begin(); it!=ctable.end(); ++it)
		{
			CompanyData *p = it->second;
			fullss.insert(p);
		}
		return FilterCompanySet(fullss, ss, pcond);
	}

	/*
	if ((text[0]&0x80)>>7)
	{
		Like_Trimname(text, fullss);
	}
	*/

	const int MAXSHIPNAMELEN = 50;
	//������ת��ƴ��
	int n = 0;
	char **pytxt = g_Pinyin4j::instance()->FindPinYin(CodeConverter::Utf8ToUnicode(text).c_str(), n);
	std::vector<int> vpy;
	if(n) vpy.resize(n, 0);
	//�޶���󳤶�Ϊ50���ַ�
	for(int i=0; i<n; ++i)
	{
		strlwr(pytxt[i]);
		vpy[i] = strlen(pytxt[i]);
		if(vpy[i] > MAXSHIPNAMELEN)
			pytxt[i][MAXSHIPNAMELEN] = 0;
	}
	//trimname like��ѯ������н�����أ��������
	for(int i=0; (i<n) && (vpy[i]<=MAXSHIPNAMELEN); ++i)
	{
		Like_Trimname(pytxt[i], fullss);
	}
	if(FilterCompanySet(fullss, ss, pcond))
	{
		FreeArray(pytxt, n);
		return true;
	}
	//�������ƶ�ƥ�䣬���ؽ��
	for(int i=0; (i<n) && (vpy[i]<=MAXSHIPNAMELEN); ++i)
	{
		DistanceTrimname(pytxt[i], fullss);
	}
	FreeArray(pytxt, n);
	return FilterCompanySet(fullss, ss, pcond);
}
