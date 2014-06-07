#ifndef __GPS_SVC_H__
#define __GPS_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"
#include "define.h"
#include <stdio.h>
#include <algorithm>
#include <queue>
#include <vector>
using namespace std;

struct gpsdata
{
    long m_time;//unixTime
    double m_xpos;
    double m_ypos;
    double m_hight;
    double m_speed;
    double m_course;
};

struct q_cmp{
	bool operator() (int& a, int& b)
	{
		return a > b;
	}
};

typedef struct _SendWarnSmsInfo
{
	int cnt_p_f;
	int cnt_l;  										//小于等于预警值下限数
	int cnt_h;  										//大于等于预警值上限数
	int cnt;    										//收到的数据的总数
	int last_time; 										//上次发送sms的时间戳
	priority_queue<int, vector<int>, q_cmp> value_time;	//五分钟内收到信息的时间,发送一次短信清空一次
	std::vector<double> p_f_l;
	std::vector<double> p_f_u;
}SendWarnSmsInfo;

typedef map<std::string, SendWarnSmsInfo> SendWarnSmsInfoMap; 
//key: engineid&(rmp|oilpress|oiltemp|waterpress|watertemp|slave)

typedef struct _WarnSmsBaseInfo
{
	string uid;
	string mobile;
	string shipName;
	string name;
	string v_name;

	_WarnSmsBaseInfo& operator = (const _WarnSmsBaseInfo& _tmp) 
	{
		uid = _tmp.uid;
		mobile = _tmp.mobile;
		shipName = _tmp.shipName;
		name = _tmp.name;
		v_name = _tmp.v_name;
		return (*this);
	}
}WarnSmsBaseInfo;

class GpsSvc : IBusinessService, ACE_Event_Handler
{
public:
    GpsSvc();
    ~GpsSvc();

	std::string SeipcId2UserId(const std::string& id, bool& isSeipc);
	bool Verify(const std::string& id, const std::string& shipid, std::string& seipcid, MySql* psql);

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(GpsSvc)

private:
    int UploadGpsPositionInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int SetUserGpsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetUserShipInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int GetShipGpsInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int ApplyGpsAuthority(const char* pUid, const char* jsonString, std::stringstream& out);
    int DealWithUserApplication(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetGpsOwner(const char* pUid, const char* jsonString, std::stringstream& out);

	int updSensorInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int updTankInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int delTankInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getTankRealtimeInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int updEngineInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int delEngineInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getEngineRealtimeInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int updQuantInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int delQuantInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getQuantRealtimeInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int updSensorRealtimeInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getSensorAdmin(const char* pUid, const char* jsonString, std::stringstream& out);
	int getSensorAuth(const char* pUid, const char* jsonString, std::stringstream& out);
	int applySensorAuth(const char* pUid, const char* jsonString, std::stringstream& out);
	int dealSensorApply(const char* pUid, const char* jsonString, std::stringstream& out);
    int updShipAlertInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int getShipAlertList(const char* pUid, const char* jsonString, std::stringstream& out);
private:
	int JudgeZhuFu(string name);
	bool loadSeipcUser();
	bool dealRealtimeValue(string shipid, int t, string value, int flag = 0);	
	void dealUnitValue(string key, int t, double value, double low, double up, WarnSmsBaseInfo _info, int flag);

private:
	StrStrMap			m_mapSeipcUser;
	ACE_RW_Thread_Mutex	m_Lock;
	static SendWarnSmsInfoMap m_sendWarnSmsInfo;
};

typedef struct _TankRealtimeInfo
{
	char id[32];
	char nm[64];
	int loc;
	int ch;
	double totalcap;
	char unit[2];
	double thr0;
	double thr1;
	int time;
	double consump;		//保留
}TankRealtimeInfo;

typedef std::vector<TankRealtimeInfo> TankRTMInfos;

typedef struct _RudderRealTimeInfo
{
	char id[64];
	char name[64];
	int loc;
	int tm;
	int loilpreExit;
	int loilpre_val;
	int loilpre_ch;
	char loilpre_unit[2];
	double loilpre_thr0;
	double loilpre_thr1;

	int hoilpreExit;
	int hoilpre_val;
	int hoilpre_ch;
	char hoilpre_unit[2];
	double hoilpre_thr0;
	double hoilpre_thr1;
}RudderRealTimeInfo;

typedef std::vector<RudderRealTimeInfo> RudderRTMInfos;

typedef struct _EngineRealtimeInfo
{
	char engineid[32];
	char name[64];
	int location;
	int time;
	int rpmExit;
	int rpm_value;
	int rpm_channel;
	char rpm_unit[2];
	double rpm_threshold0;
	double rpm_threshold1;	

	int oilpressExit;
	int oilpress_value;
	int oilpress_channel;
	char oilpress_unit[2];
	double oilpress_threshold0;
	double oilpress_threshold1;

	int oiltempExit;
	int oiltemp_value;
	int oiltemp_channel;
	char oiltemp_unit[2];
	double oiltemp_threshold0;
	double oiltemp_threshold1;	

	int waterpressExit;
	int waterpress_value;
	int waterpress_channel;
	char waterpress_unit[2];
	double waterpress_threshold0;
	double waterpress_threshold1;	

	int watertempExit;
	int watertemp_value;
	int watertemp_channel;
	char watertemp_unit[2];
	double watertemp_threshold0;
	double watertemp_threshold1;

	int slaveExit;
	int slave_value;
	int slave_channel;
	char slave_unit[2];
	double slave_threshold0;
	double slave_threshold1;
}EngineRealtimeInfo;

typedef std::vector<EngineRealtimeInfo> EngineRTMInfoVEC;

typedef struct _UserInfo
{
	char id[64];
	char nickname[255];
	int status;
	int tm;
}UserInfo;

typedef std::vector<UserInfo> UserInfos;

typedef ACE_Singleton<GpsSvc, ACE_Null_Mutex> g_GpsSvc;

#endif
