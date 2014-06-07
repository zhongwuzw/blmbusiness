#ifndef __PORT_SVC_H__
#define __PORT_SVC_H__

#include "IBusinessService.h"

using namespace std;

class CPortAlias
{
public:
    CPortAlias();
    ~CPortAlias();

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

struct _Country
{
	char namecn[64];
	char nameen[64];
	_Country() {
		memset(this, 0, sizeof(_Country));
	}
};
class CCountry
{
public:
    CCountry();
    ~CCountry();

    bool LoadAll();
    _Country *FindCountry(const char *countrycode);
    uint32 size() const
    {
        return country_table.size();
    }

private:
    typedef std::map<std::string, _Country> COUNTRY_TABLE;
    COUNTRY_TABLE country_table;
};

struct PortInfo
{
    double x;
    double y;
    int portid;
	char iso3[10];
    std::string portname;
	std::string portnamecn;
};

struct NavigationInfo
{
	string navi_id; //航标编号
	string navi_name; //航标名称
	string navi_type; //航标类型
	string equip_type; //设备类型
	string euqip_id;  //设备编号
	string navi_station; //所属航标站
	string navi_office;  //所属航标处
	string navi_channel; //所属航道
	string sea_area;  //所属海区
	string xpos;
	string ypos;
	string conpany; 
};

typedef std::vector<PortInfo*> VecPortInfo;
typedef VecPortInfo::iterator VecPortInfoIter;

typedef std::map<int, PortInfo*> MapPortInfo;
typedef MapPortInfo::iterator MapPortInfoIter;

class PortSvc : IBusinessService
{
public:
    PortSvc();
    ~PortSvc();
	CPortAlias m_alias;
	CCountry m_country;

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(PortSvc)

public:
    static void GetGridIndex(double lon, double lat, int& i, int& j);
    PortInfo*   FindNearestPort(double lon, double lat, double& dist/*NM*/, double& degree, bool extra=false);
    PortInfo*   FindPortByID(int portid);

private:
    int GetAllAnchors(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetChangeRegion(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetPortBaseInfo2(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetPortSmallLogos(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetPortBigLogo(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetPortVhf(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetPortMaxSize(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetPortTraffic(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetPortAllTers(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetPortAllTer2(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetTerSmallLogos(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetTerBigLogo(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetPortOperationCondtion(const char* pUid, const char* jsonString, std::stringstream& out);

	int GetAllNavigation(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetNavigationDetail(const char* pUid, const char* jsonString, std::stringstream& out);


    bool loadPorts();
	bool loadNavigations();

    VecPortInfo grid[180][90];
    MapPortInfo m_mapPort;
	std::map<string,NavigationInfo> m_mapNavigation;
};

struct PortSerCompany
{
    string  id;
    string  name;
    string  addr;
    string  telno;
    string  fax;
    string  email;
    string  website;
    string  sername;
    string  cryname;
    string  imcount;
};

typedef ACE_Singleton<PortSvc, ACE_Null_Mutex> g_PortSvc;

#endif
