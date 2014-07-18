#ifndef __WZH_SVC_H__
#define __WZH_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"

class CPortAliasWZH
{
public:
	CPortAliasWZH();
	~CPortAliasWZH();

	bool LoadAll();
	char *FindOrig(const char *aliasID);
	uint32 size() const
	{
		return portalias_table.size();
	}

private:
	typedef  std::map<std::string, std::string, NoCaseCmp> PORTALIAS_TABLE;
	PORTALIAS_TABLE portalias_table;
};

class WZHSvc:IBusinessService
{
public:
	WZHSvc();
	~WZHSvc();
	CPortAliasWZH m_alias;

	virtual bool Start();
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	DECLARE_SERVICE_MAP(WZHSvc)
protected:
private:
	int getPortInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int setPortDanager(const char* pUid, const char* jsonString, std::stringstream& out);
	int getPortDynamic(const char* pUid, const char* jsonString, std::stringstream& out);
	int getPortAlarmList(const char* pUid, const char* jsonString, std::stringstream& out);
	int getStows(const char* pUid, const char* jsonString, std::stringstream& out);
	int updateStow(const char* pUid, const char* jsonString, std::stringstream& out);
	int deleteStow(const char* pUid, const char* jsonString, std::stringstream& out);
private:
	std::string  getStowID();
};

typedef ACE_Singleton<WZHSvc, ACE_Null_Mutex> g_WZHSvc;

#endif