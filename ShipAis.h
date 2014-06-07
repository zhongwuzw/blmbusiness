#ifndef __SHIP_SEARCH_H__
#define __SHIP_SEARCH_H__

#include "define.h"
#include "kSQL.h"
#include "Util.h"
#include <set>


struct ShipCondition 
{
	bool filter;	//是否过滤
	int type;		//船舶类型 2~9
	int state;		//船舶状态 1实时 2非实时 3没有信号
	char country[20];//船旗
	int built_l;	//建造年代下限
	int built_u;    //建造年代上限
	float speed_l;	//船速下限
	float speed_u;  //船速上限
	int speedopt;	//船速比较操作
	int dwt;		//载重吨
	int dwtopt;		//载重吨比较操作

	ShipCondition():filter(true),speedopt(-1),dwtopt(-1)
	{
 
	 }

    bool IsValid()
	{
		if (!filter)
		{
			return false;
		}

		if ((type>=2&&type<=9)||(state>=1&&state<=3)||strlen(country)>0||built_l>0||speed_u>0)
		{
			return true;
		}

		return false;
	}


};


struct ShipData
{
	long  mmsi;			//mmsi
	char  shipid[21];	//shipid
	char  imo[21];		//imo
	char  name[64];		//船名
	char  trimname[64];	//预先strlwr
	char  callsign[21];	//呼号
	char  trimcs[21];   //处理过的呼号
	float length;		//长度
	float beam;			//宽度
	float depth;		//深度
	float draft;		//吃水
	float speed;		//船速
	short built;		//建造年代
	int	  dwt;			//载重吨
	char  country[20];  //船旗
	char  shipTypeKey[20];	//船舶类型key

	ShipData()
	{
		memset(this, 0, sizeof(ShipData));
	}
	ShipData(const ShipData &o)
	{
		memcpy(this, &o, sizeof(o));
	}

	bool operator < (const ShipData& sd) const
	{
		return strcmp(trimname, sd.trimname) <= 0;
	}

	bool match_mmsiOrImo(const char* text)
	{
		char mmsiTmp[64];
		sprintf(mmsiTmp, "%ld", mmsi);

		return strstr(mmsiTmp, text) != NULL || strstr(imo, text) != NULL;
	}

	bool match_nameOrCallsign(const char* text)
	{
		return strstr(trimname, text) != NULL || strstr(trimcs, text) != NULL;
	}

	int getType()
	{
		if(shipTypeKey[0] == '\0')
			return 0;

		char tmp[3] = {'\0'};
		memcpy(tmp, shipTypeKey, 2);
		return atoi(tmp);
	}

	// 数据清理
	void clean();
};

typedef std::vector<ShipData> SHIPLIST;
typedef std::map<long, ShipData> SHIPMAP;
typedef std::map<string, ShipData> SHIPIDMAP;

struct AisRealtime
{
	int srcid;
	long mmsi;
	long imo;
	int time;
	char name[64];
	char callsign[32];
	char trimname[64];
	char trimcs[32];

	double lon, lat;
	int  nav_status;
	int  sog;
	int  true_head;
	int  cog;
	char dest[64];
	int toPort;
	int  eta;
	int length;
	int width;
	int draught;
	double avgspeed;
	int ship_type;

	AisRealtime()
	{
		memset(this, 0, sizeof(AisRealtime));
	}

	bool operator < (const AisRealtime& ar) const
	{
		return strcmp(trimname, ar.trimname)<0;
		
	}

	//1 实时 2非实时
	int getState() 
	{
		time_t now = ::time(NULL);
		
		if(time >= now-45*60 || (time>=now-24*3600 && srcid==103))
			return 1;

		/*if(time >= now-60*60 || (time>=now-36*3600 && srcid==103))
			return 2;*/
		
		return 2;
	}

	bool match_mmsiOrImo(const char* text)
	{
		char mmsiTmp[64];
		char imoTmp[64];
		sprintf(mmsiTmp, "%ld", mmsi);
		sprintf(imoTmp, "%ld", imo);

		return strstr(mmsiTmp, text) != NULL || strstr(imoTmp, text) != NULL;
	}

	bool match_nameOrCallsign(const char* text)
	{
		return strstr(trimname, text) != NULL || strstr(trimcs, text) != NULL;
	}

	// 数据清理
	void clean();
};

typedef std::vector<AisRealtime> AISLIST;
typedef std::map<long, AisRealtime> AISMAP;

class CAisShip
{
public:
	CAisShip();
	~CAisShip();

	void Refresh()
	{
		//1.加载静态数据 2.加载动态数据  1,2顺序不能颠倒
		
		refreshShip();
		refreshAis();
		
	}

	static int GetType(ShipData* pShip, AisRealtime* pAis)
	{
		if(!pShip && !pAis) 
			return 0;
		if(!pShip)	return pAis->ship_type;
		if(!pAis) return pShip->getType();
		return pAis->ship_type==0?pShip->getType():pAis->ship_type;
	}

	AisRealtime* FindAis(long mmsi);
	ShipData* FindShip(long mmsi);
	ShipData* FindShip_imo(long imo);
	void  GetRealAis(AISLIST& aisList, int seconds=900);
	int GetAisState( long mmsi, time_t& lastTm );

	int Search_MmsiImo(AISLIST& aisList, const char* text);
	int Search_MmsiImo(SHIPLIST& shipList, const char* text);
	int Search_NameCallsign(AISLIST& aisList, const char* text);
	int Search_NameCallsign(SHIPLIST& shipList, const char* text);

	int AdvSearchWithCondition(AISLIST &aisList,const char* text,ShipCondition *filter);
	int AdvSearchWithCondition(SHIPLIST &shipList,const char* text,ShipCondition *filter,bool bonlycnss=false);

private:
	void readShipData(MySql *psql, ShipData &sd);
	void readAisData(MySql *psql, AisRealtime &ad);
	bool refreshAis();
	bool refreshShip();
    void loadCnssShip();
	bool filterShip(ShipData *pShipdata,ShipCondition *filter,bool bonlycnss=false);
	bool filterShip(AisRealtime *pShipAis,ShipCondition *filter);
	

private:
	AISMAP m_AisMap;
	SHIPMAP m_ShipMap;
	SHIPIDMAP m_ShipIdMap;
	std::set<string> m_CnssShipSet;

	ACE_RW_Thread_Mutex m_aisLock;
	ACE_RW_Thread_Mutex m_shipLock;
	time_t m_LastAisTime;
	time_t m_LastShipTime;

	bool bAisLoading;
	bool bShipLoading;
};

#endif
