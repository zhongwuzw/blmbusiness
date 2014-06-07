#ifndef __COMPANY_SEARCH_H__
#define __COMPANY_SEARCH_H__
#include "define.h"
#include "kSQL.h"
#include "Util.h"

struct PortSet
{
	std::set<int> ports;
};

struct CompanyCond
{
	bool filter;
	bool details;
	int svrtype;
	int port;
	char *iso3;

	//�޹�������
	bool nofilter() const
	{
		return ((!filter) || ((svrtype==0) && (*iso3==0) && (port==0)));
	}
};

struct CompanyData
{
	char id[64];
	char name[128];
	char trimname[128];	//��name�������ַ���
	int  trimnamelen;	//�����ַ����ĳ���
	char addr[128];
	char iso3[10];

	PortSet ports;
	std::set<int> svrs;	//֧�ֵķ����б�

	CompanyData():trimnamelen(0)
	{
		id[0] = name[0] = trimname[0] = addr[0] = iso3[0] = 0;
	}

	void calctrimname()
	{
		int len = 0;
		char *p, *q = trimname;
		for(p=name; *p; ++p)
			if(*p!=' ') {
				*q++=*p;
				len++;
			}
		*q = 0;
		trimnamelen = len;
		strlwr(trimname);
	}
	~CompanyData()
	{
		
	}
	void setsvrs(char *pv);
};

typedef std::set<CompanyData *> COMPANYSET;

class CompanySearch
{
public:
	CompanySearch();
	~CompanySearch();

	//װ��һϵ����صı�
	bool LoadDatabase();
	void OnUpdateCompanyData(CompanySearch *s);

	//likeƥ��
	uint32 Like_Trimname(const char *tname, COMPANYSET &ss);

	//���ƶ�ƥ��
	uint32 DistanceTrimname(const char *tname, COMPANYSET &ss);

	//search����
	bool Search(char *text, CompanyCond *pcond, COMPANYSET &fullss, COMPANYSET &ss);

	//���˽����
	bool FilterCompanySet(COMPANYSET &fullss, COMPANYSET &ss, CompanyCond *pcond);

	uint32 size() const
	{
		return ctable.size();
	}

public:
	typedef std::map<std::string, CompanyData *, NoCaseCmp> COMPANY_TABLE;
	typedef std::map<std::string, PortSet, NoCaseCmp> PORTSET;

private:
	bool LoadCompany(MySql *psql);
	bool LoadPortset(MySql *psql, PORTSET &ps);
	bool MergePortset(PORTSET &ps);
	void clear();

	//������ͺ�״̬�Ƿ�һ��
	inline bool Valify(CompanyData *pdata, CompanyCond *pcond);

private:
	COMPANY_TABLE ctable;
};

#endif
