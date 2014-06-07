#include "GpsSvc.h"
#include "blmcom_head.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "Util.h"
#include "json.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "NotifyService.h"
#include "SmsMail.h"

IMPLEMENT_SERVICE_MAP(GpsSvc)

SendWarnSmsInfoMap GpsSvc::m_sendWarnSmsInfo;

GpsSvc::GpsSvc()
{
   
}

GpsSvc::~GpsSvc()
{
   
}

bool GpsSvc::Start()
{
	if(!loadSeipcUser())
		return false;

    if(!g_MessageService::instance()->RegisterCmd(MID_GPS, this))
        return false;

	int interval = 3600;
	int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
	if(timerId <= 0)
		return false;

    SERVICE_MAP(SID_UPLOADUSERGPSINFO,GpsSvc,UploadGpsPositionInfo);
    SERVICE_MAP(SID_USERGPSINFO,GpsSvc,SetUserGpsInfo);
	SERVICE_MAP(SID_USERSHIPINFO,GpsSvc,GetUserShipInfo);
	SERVICE_MAP(SID_APPLYGPSAUTHORITY,GpsSvc,ApplyGpsAuthority);
	SERVICE_MAP(SID_DEALWITHUSERAPLICATION,GpsSvc,DealWithUserApplication);
    SERVICE_MAP(SID_GETGPSOWNER,GpsSvc,GetGpsOwner);
    SERVICE_MAP(SID_SHIPGPSINFO ,GpsSvc,GetShipGpsInfo);
	SERVICE_MAP(SID_UPD_SENSOR_INFO,GpsSvc,updSensorInfo);
	SERVICE_MAP(SID_UPD_TANK_INFO,GpsSvc,updTankInfo);
	SERVICE_MAP(SID_DEL_TANK_INFO,GpsSvc,delTankInfo);
	SERVICE_MAP(SID_GET_TANK_REALTIME_INFO,GpsSvc,getTankRealtimeInfo);
	SERVICE_MAP(SID_UPD_ENGINE_INFO,GpsSvc,updEngineInfo);
	SERVICE_MAP(SID_DEL_ENGINE_INFO,GpsSvc,delEngineInfo);
	SERVICE_MAP(SID_GET_ENGINE_REALTIME_INFO,GpsSvc,getEngineRealtimeInfo);
	SERVICE_MAP(SID_UPD_QUANT_INFO,GpsSvc,updQuantInfo);
	SERVICE_MAP(SID_DEL_QUANT_INFO,GpsSvc,delQuantInfo);
	SERVICE_MAP(SID_GET_QUANT_REALTIME_INFO,GpsSvc,getQuantRealtimeInfo);
	SERVICE_MAP(SID_UPD_SENSOR_REALTIME_INFO,GpsSvc,updSensorRealtimeInfo);
	SERVICE_MAP(SID_GET_SENSOR_ADMIN,GpsSvc,getSensorAdmin);
	SERVICE_MAP(SID_GET_SENSOR_P_AUTH,GpsSvc,getSensorAuth);
	SERVICE_MAP(SID_APPLY_SENSOR_P_AUTH,GpsSvc,applySensorAuth);
	SERVICE_MAP(SID_DEAL_SENSOR_APPLY,GpsSvc,dealSensorApply);
	SERVICE_MAP(SID_UPD_SEIPC_ALERT,GpsSvc,updShipAlertInfo);
	SERVICE_MAP(SID_GET_SEIPC_ALERTLIST,GpsSvc,getShipAlertList);

    DEBUG_LOG("[GpsSvc::Start] OK......................................");
	//std::stringstream out;
	//updSensorRealtimeInfo("6EA6A45806B87467", "{shipid:\"S992010102496042\",tm:1355964268,val:\"|||||||||||||||\"}", out);
	//updSensorRealtimeInfo("6EA6A45806B87467", "{shipid:\"S992010102496042\",tm:1355966474,val:\"|||||||||||||||\"}", out); 
	return true;
}

int GpsSvc::JudgeZhuFu(string name)
{
	if (ACE_OS::strcmp(name.c_str(), "左辅机") == 0)
		return 1;
	if (ACE_OS::strcmp(name.c_str(), "右辅机") == 0)
		return 2;
	return 0;
}

//检查这个id（可以是seipcid也可以是管理员id也可以是普通用户）对这条船是否有权限
//同时得到这条船对应的seipcid
bool GpsSvc::Verify(const std::string& id, const std::string& shipid, std::string& seipcid, MySql* psql)
{
	//获取id对应的两个用户
	bool isSeipc1 = false;
	string uid1 = SeipcId2UserId(id, isSeipc1);
	
	//获取shipid对应的两个用户
	char sql[1024];
	sprintf(sql, "SELECT t1.seipc_id,t2.user_id FROM t41_seipc_connection t1,t41_seipc_user t2 \
		WHERE t1.ship_id = '%s' AND t1.is_valid = '1' AND t1.seipc_id = t2.SEIPC_ID AND t2.IS_VALID = '1'", shipid.c_str());
	if(!(psql->Query(sql)&&psql->NextRow()))
		return false;

	seipcid = psql->GetField("seipc_id");
	string uid2 = psql->GetField("user_id");

	//id是工控机
	if(isSeipc1 && id == seipcid)
		return true;
	//id是这个shipid的工控机管理者
	if(!isSeipc1 && id == uid2)
		return true;
	//id是普通用户(查权限表)
	//sprintf(sql, "SELECT COUNT(1) AS num FROM t41_seipc_apply WHERE ship_id = '%s' AND user_id = '%s' AND accept = '1'", shipid.c_str(), id.c_str());
	sprintf(sql, "SELECT COUNT(1) AS num FROM t41_authen_apply WHERE BUSID = '%s' AND user_id = '%s' AND accept = '1' AND BUSTYPE = '1'", shipid.c_str(), id.c_str());
	if(!(psql->Query(sql)&&psql->NextRow()))
		return false;

	int ret = atoi(psql->GetField("num"));
	return (ret!=0);
}

std::string GpsSvc::SeipcId2UserId(const std::string& id, bool& isSeipc)
{
	ACE_Read_Guard<ACE_RW_Thread_Mutex> guard(m_Lock);

	StrStrMapIter iter = m_mapSeipcUser.find(id);
	if(iter != m_mapSeipcUser.end()) {
		isSeipc = true;
		return iter->second;
	}

	isSeipc = false;
	return id;
}

bool GpsSvc::loadSeipcUser()
{
	ACE_Write_Guard<ACE_RW_Thread_Mutex> guard(m_Lock);

	const char* sql = "SELECT seipc_id, user_id FROM t41_seipc_user WHERE is_valid = '1'";
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), false);

	m_mapSeipcUser.clear();
	while(psql->NextRow())
	{
		const char* seipc_id = psql->GetField("seipc_id");
		const char* user_id = psql->GetField("user_id");
		m_mapSeipcUser[string(seipc_id)] = string(user_id);
	}

	RELEASE_MYSQL_RETURN(psql, true);
}

//zhuxj
void GpsSvc::dealUnitValue(string key, int t, double value, double low, double up, WarnSmsBaseInfo _info, int flag)
{
	if (low < 0.0 && up < 0.0)
		return;
	//SYSTEM_LOG("[GpsSvc::dealUnitValue] end warning: key:%s, t:%d, value:%.3f low:%.3f, up:%.3f ==============", key.c_str(), t, value, low, up);
	SendWarnSmsInfoMap::iterator mIter = m_sendWarnSmsInfo.find(key);

	if (mIter != m_sendWarnSmsInfo.end())
	{		
		(*mIter).second.cnt++;
		switch(flag)
		{
		case 0:
			{
				(*mIter).second.cnt_h = value<up?(*mIter).second.cnt_h:(up > 0.0?(*mIter).second.cnt_h+1:(*mIter).second.cnt_h);
				(*mIter).second.cnt_l = value>low?(*mIter).second.cnt_l:(low > 0.0?(*mIter).second.cnt_l+1:(*mIter).second.cnt_l);
			}break;
		case 1:// 16_18 主机滑油温度报警
			{
				(*mIter).second.cnt_h = value<75.0?(*mIter).second.cnt_h:(*mIter).second.cnt_h+1;
			}break;
		case 2:// 16_18 主机淡水温度报警
			{
				(*mIter).second.cnt_h = value<95.0?(*mIter).second.cnt_h:(*mIter).second.cnt_h+1;
			}break;
		case 3:// 16_18_12 主机超速报警
			{
				(*mIter).second.cnt_h = value<760?(*mIter).second.cnt_h:(*mIter).second.cnt_h+1;
			}break;
		case 4:// 16_18 滑油压力低压报警
			{
				(*mIter).second.cnt_l = value>0.2?(*mIter).second.cnt_l:(*mIter).second.cnt_l+1;
			}break;
		case 5:// 16_18_12 右/左辅机滑油压力低报警
			{
				double _f_p_l = -1.0;
				double _f_p_u = -1.0;
				size_t s = (*mIter).second.p_f_l.size();
				if (s != -1)
				{
					_f_p_l = (*mIter).second.p_f_l[s -1]; 
					_f_p_u = (*mIter).second.p_f_u[s - 1];					
				}
				(*mIter).second.cnt_p_f += (_f_p_l<0.08 && _f_p_u>2.0);
				//五分钟内小于0.08并且大于2.0的数据超过70%
				(*mIter).second.cnt_l = value<0.15&&((*mIter).second.cnt_p_f*1.0/(*mIter).second.cnt>0.8)?1:0;
				//printf ("%d %d\n", (*mIter).second.cnt_l, (*mIter).second.cnt_p_f);
			}break;
		case 6:// 12 主机滑油温度报警
			{
				(*mIter).second.cnt_h = value<65.0?(*mIter).second.cnt_h:(*mIter).second.cnt_h+1;
			}break;
		case 7:// 12 主机淡水温度报警
			{
				(*mIter).second.cnt_h = value<85.0?(*mIter).second.cnt_h:(*mIter).second.cnt_h+1;
			}break;
		case 8:// 12 滑油压力低压报警
			{
				(*mIter).second.cnt_l = value>0.25?(*mIter).second.cnt_l:(*mIter).second.cnt_l+1;
			}break;
		default:
			{
				
			}break;
		}
		(*mIter).second.value_time.push(t);

		while (t - (*mIter).second.value_time.top() > 300)//储存5分钟之内的数据
		{
			(*mIter).second.value_time.pop();
			(*mIter).second.cnt--;
			if (flag == 5)
			{
				vector<double>::iterator iterl = (*mIter).second.p_f_l.begin();
				vector<double>::iterator iteru = (*mIter).second.p_f_u.begin();
				(*mIter).second.cnt_p_f -= ((*iterl)>2.0&&(*iteru)<0.08);
				(*mIter).second.p_f_l.erase(iterl);
				(*mIter).second.p_f_u.erase(iteru);
			}
			else
			{				
				(*mIter).second.cnt_h--;
				(*mIter).second.cnt_l--;
			}
		}
		bool lower = ((*mIter).second.cnt_l*1.0/(*mIter).second.cnt >= 0.5);
		bool over = ((*mIter).second.cnt_h*1.0/(*mIter).second.cnt >= 0.5);
		if (flag == 5)
			lower = (*mIter).second.cnt_l;
		if (t - (*mIter).second.last_time >= 300 && ( lower || over ))
			//距离上次发短信超过5分钟,并且一分钟内超过50%的数据需要预警
		{
			char content[1024] = "";
			time_t _t = (time_t)t;
			struct tm *ptr = localtime(&_t);
			sprintf (content, "%d:%d %s %s 的 %s 的值为%.3f，%s %.3f",
				ptr->tm_hour, ptr->tm_min, _info.shipName.c_str(), _info.name.c_str(), _info.v_name.c_str(), value, over?"超过":"低于", over?up:low);
			
			string szTel = _info.mobile;
			string iso2;

			if(!g_MobileIllegal(szTel, iso2))
			{
				DEBUG_LOG("[AccountSvc::autoRegister] illegal mobile:%s", szTel.c_str());
				return;
			}
			CNotifyMsg* pMsg = new CNotifyMsg;
			pMsg->m_szType = "sms";
			pMsg->m_SmsFromUser = "";
			pMsg->m_SmsToUser = _info.uid;
			pMsg->m_SmsToTel = _info.mobile;
			pMsg->m_SmsMsg = content;
			pMsg->m_SmsType = 0;
			bool testFlg = g_NotifyService::instance()->PutMessage(pMsg);
			SYSTEM_LOG("[GpsSvc::dealUnitValue] end warning:%d ============== key:%s, t:%d, value:%.3f low:%.3f, up:%.3f ", testFlg, key.c_str(), t, value, low, up);
			(*mIter).second.last_time = t;
		}
	}
	else
	{
		SendWarnSmsInfo _sendWarnSmsInfo;
		_sendWarnSmsInfo.cnt = 1;
		switch(flag)
		{
		case 0:
			{
				_sendWarnSmsInfo.cnt_h = value<up?0:(up > 0.0?1:0);
				_sendWarnSmsInfo.cnt_l = value>low?0:(low > 0.0?1:0);
			}break;
		case 1:// 16_18 主机滑油温度报警
			{
				_sendWarnSmsInfo.cnt_h = value<75.0?0:1;
				_sendWarnSmsInfo.cnt_l = 0;
			}break;
		case 2:// 16_18 主机淡水温度报警
			{
				_sendWarnSmsInfo.cnt_h = value<95.0?0:1;
				_sendWarnSmsInfo.cnt_l = 0;
			}break;
		case 3:// 16_18_12 主机超速报警
			{
				_sendWarnSmsInfo.cnt_h = value<760?0:1;
				_sendWarnSmsInfo.cnt_l = 0;
			}break;
		case 4:// 16_18 滑油压力低压报警
			{
				_sendWarnSmsInfo.cnt_l = value>0.2?0:1;
				_sendWarnSmsInfo.cnt_h = 0;
			}break;
		case 5:// 16_18_12 右/左辅机滑油压力低报警
			{
				_sendWarnSmsInfo.cnt_l = value>0.15?0:1;
				_sendWarnSmsInfo.cnt_p_f = 0;
				_sendWarnSmsInfo.cnt_h = 0;
			}break;
		case 6:// 12 主机滑油温度报警
			{
				_sendWarnSmsInfo.cnt_h = value<65.0?0:1;
				_sendWarnSmsInfo.cnt_l = 0;
			}break;
		case 7:// 12 主机淡水温度报警
			{
				_sendWarnSmsInfo.cnt_h = value<85.0?0:1;
				_sendWarnSmsInfo.cnt_l = 0;
			}break;
		case 8:// 12 滑油压力低压报警
			{
				_sendWarnSmsInfo.cnt_l = value>0.25?0:1;
				_sendWarnSmsInfo.cnt_h = 0;
			}break;
		default:
			{
				
			}break;
		}		
		_sendWarnSmsInfo.last_time = t;
		_sendWarnSmsInfo.value_time.push(t);
		m_sendWarnSmsInfo.insert(make_pair(key,_sendWarnSmsInfo));

		if ((_sendWarnSmsInfo.cnt_h || _sendWarnSmsInfo.cnt_l) && flag != 5)
		{
			char content[1024] = "";
			time_t _t = (time_t)t;
			struct tm *ptr = localtime(&_t);
			sprintf (content, "%d:%d %s %s 的 %s 的值为%.3f，%s %.3f",
				ptr->tm_hour, ptr->tm_min, _info.shipName.c_str(), _info.name.c_str(), _info.v_name.c_str(), value, _sendWarnSmsInfo.cnt_h?"超过":"低于", _sendWarnSmsInfo.cnt_h?up:low);
			
			string szTel = _info.mobile;
			string iso2;

			if(!g_MobileIllegal(szTel, iso2))
			{
				DEBUG_LOG("[AccountSvc::autoRegister] illegal mobile:%s", szTel.c_str());
				return;
			}
			CNotifyMsg* pMsg = new CNotifyMsg;
			pMsg->m_szType = "sms";
			pMsg->m_SmsFromUser = "";
			pMsg->m_SmsToUser = _info.uid;
			pMsg->m_SmsToTel = _info.mobile;//"+86 13601376849";//
			pMsg->m_SmsMsg = content;
			pMsg->m_SmsType = 0;
			bool testFlg = g_NotifyService::instance()->PutMessage(pMsg);
			DEBUG_LOG("[GpsSvc::dealUnitValue]content:%s", content);
			SYSTEM_LOG("[GpsSvc::dealUnitValue] first end warning:%d ============== key:%s, t:%d, value:%.3f low:%.3f, up:%.3f", testFlg, key.c_str(), t, value, low, up);
		}
	}
}
//flag 0:普通 1:16_18 2：12
bool GpsSvc::dealRealtimeValue(string shipid, int t, string value, int flag)
{
	char sql[1024] = "";
	MySql* psql = CREATE_MYSQL;

	Tokens values = StrSplit(value, "|");

	sprintf (sql, "SELECT  d.user_id, b.ship_name, b.sms_mobile, e.mobile,  c.engine_id, c.name, c.rpm_channel, c.rpm_threshold0, c.rpm_threshold1, c.oilpress_channel, c.oilpress_threshold0, \
					c.oilpress_threshold1, c.oiltemp_channel, c.oiltemp_threshold0, c.oiltemp_threshold1, c.waterpress_channel, c.waterpress_threshold0, c.waterpress_threshold1,\
					c.watertemp_channel, c.watertemp_threshold0, c.watertemp_threshold1, c.slave_channel, c.slave_threshold0, c.slave_threshold1 \
					FROM t41_seipc_realtime a JOIN t41_seipc_connection b ON a.SHIP_ID = b.ship_id\
					JOIN t41_seipc_engine c ON b.seipc_id = c.seipc_id JOIN t41_seipc_user d ON c.seipc_id = d.seipc_id\
					JOIN t00_user e ON d.user_id = e.user_id WHERE a.SHIP_ID = '%s' AND b.is_valid = '1' AND d.is_valid = '1'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), false);

	double left_p = -1.0;
	double right_p = -1.0;
	char key_l[256] = "";
	char key_r[256] = "";
	WarnSmsBaseInfo info_l;
	WarnSmsBaseInfo info_r;

	while(psql->NextRow())
	{
		int channel;
		char _key[256];
		double low;
		double up;
		double hywd = -1.0, dswd = -1.0, zjzs = -1.0;
		
		string engineId = NOTNULL(psql->GetField("engine_id"));

		WarnSmsBaseInfo _info;
		_info.uid = NOTNULL(psql->GetField("user_id"));
		_info.mobile = NOTNULL(psql->GetField("sms_mobile"));
		if (!_info.mobile.length())
			_info.mobile = NOTNULL(psql->GetField("mobile"));
		_info.shipName = CodeConverter::Utf8ToGb2312(SmsMsg(NOTNULL(psql->GetField("ship_name"))).c_str());
		_info.name = CodeConverter::Utf8ToGb2312(SmsMsg(NOTNULL(psql->GetField("name"))).c_str());
		
		int zfFlg = JudgeZhuFu(_info.name);
		channel = atoi(NOTNULL(psql->GetField("rpm_channel")));
		if(channel && !zfFlg)
		{
			sprintf (_key, "%s@%s@%s@%s", engineId.c_str(), "rpm", _info.uid.c_str(), _info.name.c_str());
			low = atof(NOTNULL(psql->GetField("rpm_threshold0")));
			up = atof(NOTNULL(psql->GetField("rpm_threshold1")));
			_info.v_name = "转速";
			if (channel -1 < (int)values.size())
			{	
				zjzs = atof(values[channel -1].c_str());
				dealUnitValue(_key, t, zjzs, low, up, _info, flag?3:0);
			}
		}

		channel = atoi(NOTNULL(psql->GetField("oiltemp_channel")));
		if(channel && !zfFlg)
		{
			sprintf (_key, "%s@%s@%s@%s", engineId.c_str(), "oiltemp", _info.uid.c_str(), _info.name.c_str());
			low = atof(NOTNULL(psql->GetField("oiltemp_threshold0")));
			up = atof(NOTNULL(psql->GetField("oiltemp_threshold1")));
			_info.v_name = "滑油温度";
			if (channel -1 < (int)values.size())
			{
				hywd = atof(values[channel -1].c_str());
				dealUnitValue(_key, t, hywd, low, up, _info, flag?(flag==1?1:6):0);
			}
		}

		channel = atoi(NOTNULL(psql->GetField("waterpress_channel")));
		if(channel && !zfFlg)
		{
			sprintf (_key, "%s@%s@%s@%s", engineId.c_str(), "waterpress", _info.uid.c_str(), _info.name.c_str());
			low = atof(NOTNULL(psql->GetField("waterpress_threshold0")));
			up = atof(NOTNULL(psql->GetField("waterpress_threshold1")));
			_info.v_name = "淡水压力";
			if (channel -1 < (int)values.size())
				dealUnitValue(_key, t, atof(values[channel -1].c_str()), low, up, _info, 0);
		}

		channel = atoi(NOTNULL(psql->GetField("watertemp_channel")));
		if(channel && !zfFlg)
		{
			sprintf (_key, "%s@%s@%s@%s", engineId.c_str(), "watertemp", _info.uid.c_str(), _info.name.c_str());
			low = atof(NOTNULL(psql->GetField("watertemp_threshold0")));
			up = atof(NOTNULL(psql->GetField("watertemp_threshold1")));
			_info.v_name = "淡水温度";
			if (channel -1 < (int)values.size())
			{
				dswd = atof(values[channel -1].c_str());
				dealUnitValue(_key, t, dswd, low, up, _info, flag?(flag==1?2:7):0);
			}
		}

		channel = atoi(NOTNULL(psql->GetField("slave_channel")));
		if(channel && !zfFlg)
		{
			sprintf (_key, "%s@%s@%s@%s", engineId.c_str(), "slave", _info.uid.c_str(), _info.name.c_str());
			low = atof(NOTNULL(psql->GetField("slave_threshold0")));
			up = atof(NOTNULL(psql->GetField("slave_threshold1")));
			_info.v_name = "副机滑油压力";
			if (channel -1 < (int)values.size())
				dealUnitValue(_key, t, atof(values[channel -1].c_str()), low, up, _info, 0);
		}
		
		channel = atoi(NOTNULL(psql->GetField("oilpress_channel")));
		if(channel)
		{
			sprintf (_key, "%s@%s@%s@%s", engineId.c_str(), "oilpress", _info.uid.c_str(), _info.name.c_str());
			low = atof(NOTNULL(psql->GetField("oilpress_threshold0")));
			up = atof(NOTNULL(psql->GetField("oilpress_threshold1")));
			_info.v_name = "滑油压力";
			if (channel -1 < (int)values.size() && !zfFlg)
			{
				if (flag && hywd>10&&dswd>10&&zjzs>350)
					dealUnitValue(_key, t, atof(values[channel -1].c_str()), low, up, _info, flag==1?4:8);
				else if (!flag)
					dealUnitValue(_key, t, atof(values[channel -1].c_str()), low, up, _info, 0);
			}
			else if (channel -1 < (int)values.size() && zfFlg == 1 && flag)
			{
				left_p = atof(values[channel -1].c_str());
				strcpy(key_l, _key);
				info_l = _info;
			}
			else if (channel -1 < (int)values.size() && zfFlg == 2 && flag)
			{
				right_p = atof(values[channel -1].c_str());
				strcpy(key_r, _key);
				info_r = _info;
			}
		}
	}

	if (left_p > 0.0)
	{
		SendWarnSmsInfoMap::iterator mIter = m_sendWarnSmsInfo.find(key_l);
		if (mIter != m_sendWarnSmsInfo.end())
		{
			(*mIter).second.p_f_l.push_back(right_p);
			(*mIter).second.p_f_u.push_back(left_p);
		}
		dealUnitValue(key_l, t, left_p, 1.0, 1.0, info_l, 5);
	}

	if (right_p > 0.0)
	{
		SendWarnSmsInfoMap::iterator mIter = m_sendWarnSmsInfo.find(key_r);
		if (mIter != m_sendWarnSmsInfo.end())
		{
			(*mIter).second.p_f_l.push_back(left_p);
			(*mIter).second.p_f_u.push_back(right_p);
		}
		dealUnitValue(key_r, t, right_p, 1.0, 1.0, info_r, 5);
	}

	RELEASE_MYSQL_RETURN(psql, true);
}

bool json2gpsdata(Json* jv,gpsdata& d)
{
    d.m_time = jv->getv("tm", 0);
    d.m_xpos = jv->getv("x", 0.0);
    d.m_ypos = jv->getv("y", 0.0);
    d.m_hight= jv->getv("h", 0.0);
	d.m_speed= jv->getv("spd", 0.0);
    d.m_course=jv->getv("course", 0.0);

    return 0;
}

int GpsSvc::UploadGpsPositionInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[GpsSvc::UploadGpsPositionInfo]bad format:", jsonString, 1);

	bool isSeipc = false;
	string uid = SeipcId2UserId(pUid, isSeipc);
    string shipid = root.getv("shipid", "");
    string shipname = root.getv("sname", "");
    string mmsi = root.getv("mmsi", "");

    gpsdata d;
    if(json2gpsdata(root["gps"],d)) return 1;

	char gmt0now[20];
	GmtNow(gmt0now);

	char sql[1024];
    MySql* psql = CREATE_MYSQL;

	//获取gps_id
    sprintf(sql, "select gps_id from  t41_gps_connection where user_id ='%s' and ship_id = '%s' and is_valid = '1'", uid.c_str(), shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
    const char* gps_id = psql->GetField("gps_id");

    sprintf(sql, "insert into t41_gps_track (gps_id, time, mmsi, longitude, latitude, height, sog, cog) values ('%s', '%d', '%s', '%f', '%f', '%f', '%f', '%f')",
				gps_id, d.m_time, mmsi.c_str(), d.m_xpos, d.m_ypos, d.m_hight, d.m_speed, d.m_course);
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	
	sprintf(sql, "replace into t41_gps_realtime(gps_id, time, mmsi, longitude, latitude, height, sog, cog, update_dt) values('%s', '%d', '%s', '%f', '%f', '%f', '%f', '%f', '%s')",
				gps_id, d.m_time, mmsi.c_str(), d.m_xpos, d.m_ypos, d.m_hight, d.m_speed, d.m_course,gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int GpsSvc::SetUserGpsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[GpsSvc::SetUserGpsInfo]bad format:", jsonString, 1);
	bool isSeipc = false;
	string uid = SeipcId2UserId(pUid, isSeipc);
    string shipid = root.getv("shipid", "");
    string mmsi = root.getv("mmsi", "");
    string shipname = root.getv("sname", "");
    int tm = root.getv("tm", 0);

	char sql[1024];
	MySql* psql = CREATE_MYSQL;
    sprintf(sql, "update t41_gps_connection set is_valid = '0',disconnect_time='%d' where user_id = '%s' and is_valid = '1'", tm, uid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    
    sprintf(sql, "insert into t41_gps_connection (ship_id,mmsi,ship_name,user_id ,connect_time,is_valid) values('%s','%s','%s','%s','%d','1')", shipid.c_str(),mmsi.c_str(), shipname.c_str(), uid.c_str(), tm);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int GpsSvc::GetUserShipInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[GpsSvc::GetUserShipInfo]bad format:", jsonString, 1);
	bool isSeipc = false;
	string uid = SeipcId2UserId(pUid, isSeipc);

	char sql[1024];
	MySql* psql = CREATE_MYSQL;
    sprintf(sql, "select ship_id ,mmsi, ship_name from t41_gps_connection where user_id='%s' and is_valid = '1'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	const char* shipid = psql->GetField("ship_id");
	const char* shipnm = psql->GetField("ship_nm");
	out << FormatString("{shipid:\"%s\",shipnm:\"%s\"}", NOTNULL(shipid), NOTNULL(shipnm));

    RELEASE_MYSQL_RETURN(psql, 0);
}

int GpsSvc::GetShipGpsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[GpsSvc::GetShipGpsInfo]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");

	char sql[1024];
	MySql* psql = CREATE_MYSQL;
    sprintf(sql, "select gps_id, user_id from t41_gps_connection where ship_id='%s' and is_valid='1'", shipid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

	const char* gps_id = psql->GetField("gps_id");
	const char* user_id = psql->GetField("user_id");
	if(!strcasecmp(user_id, pUid)) //是owner
	{
		//sprintf(sql, "select t1.user_id, t1.accept, unix_timestamp(t1.update_dt) as time, t2.nickname from t41_gps_apply t1, t00_user t2 where t1.gps_id='%s' and t1.user_id=t2.user_id", gps_id);
		sprintf(sql, "SELECT t1.USER_ID, t1.ACCEPT, UNIX_TIMESTAMP(t1.UPDATE_DT) AS TIME, t2.nickname FROM t41_authen_apply t1, t00_user t2 WHERE t1.BUSID = '%s' AND t1.user_id=t2.user_id AND BUSTYPE = '0'", gps_id);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);

		int total = 0;
		out << "{appstate:\"-1\",appliers:[";
		while(psql->NextRow())
		{
			if(total != 0)
				out << ",";
			total++;
			out << FormatString("{userId:\"%s\",usrnk:\"%s\",state:\"%s\",updt:%s}",
				psql->GetField("user_id"),psql->GetField("nickname"),psql->GetField("accept"),psql->GetField("time"));
		}
		out << "]}";
	}
	else
	{
		//sprintf(sql, "select accept, unix_timestamp(update_dt) as time from t41_gps_apply where gps_id='%s' and user_id='%s'", gps_id, pUid);
		sprintf(sql, "SELECT accept, UNIX_TIMESTAMP(update_dt) AS TIME FROM t41_authen_apply WHERE BUSID = '%s' AND user_id = '%s' AND BUSTYPE = '0'", gps_id, pUid);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(psql->NextRow())
		{
			int accept = atoi(psql->GetField("accept"));
			const char* time = psql->GetField("time");
			out<<FormatString("{appstate:\"%d\",appliers:[],updt:%s}", accept, (time?time:"0"));
		}
		else
		{
			out<<"{appstate:\"-1\",appliers:[],updt:0}";
		}
	}


    RELEASE_MYSQL_RETURN(psql, 0);
}

int GpsSvc::ApplyGpsAuthority(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[GpsSvc::ApplyGpsAuthority]bad format:", jsonString, 1);
    string ownerid = root.getv("ownerid", "");
    string uid = root.getv("uid", "");
    string shipid = root.getv("shipid", "");

	char gmt0now[20];
	GmtNow(gmt0now);

	char sql[1024];
	MySql* psql = CREATE_MYSQL;
	sprintf(sql, "select gps_id from t41_gps_connection where user_id='%s' and ship_id='%s' and is_valid='1'",ownerid.c_str(), shipid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

    const char* gps_id = psql->GetField("gps_id");
    //sprintf(sql, "insert into  t41_gps_apply ( user_id ,gps_id,accept,apply_dt,update_dt ) values( '%s','%s',0,'%s','%s')", uid.c_str(), gps_id,gmt0now,gmt0now);
	sprintf(sql, "INSERT INTO  t41_authen_apply (user_id, BUSTYPE, BUSID, accept, apply_dt, update_dt) VALUES ( '%s', '0', '%s', 0, '%s', '%s')", uid.c_str(), gps_id,gmt0now,gmt0now);
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int GpsSvc::DealWithUserApplication(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[GpsSvc::DealWithUserApplication]bad format:", jsonString, 1);
    string ownerid = root.getv("ownerid", "");
    string uid = root.getv("uid", "");
    string shipid = root.getv("shipid", "");
    int accept = atoi(root.getv("accept", ""));

	char gmt0now[20];
	GmtNow(gmt0now);

	char sql[1024];
	MySql* psql = CREATE_MYSQL;
    sprintf(sql, "select gps_id from t41_gps_connection where user_id='%s' and ship_id='%s' and is_valid='1'", ownerid.c_str(), shipid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);

    const char* gps_id = psql->GetField("gps_id");
    //sprintf(sql, "update t41_gps_apply set accept=%d, update_dt='%s' where user_id='%s' and gps_id='%s'", (accept==0? 2:1), gmt0now,uid.c_str(), gps_id);
	if (accept == 2)
		sprintf(sql, "DELETE FROM t41_authen_apply WHERE user_id='%s' AND BUSID='%s' AND BUSTYPE = '0'", uid.c_str(), gps_id);
	else
		sprintf(sql, "UPDATE t41_authen_apply SET ACCEPT='%d', UPDATE_DT='%s' WHERE user_id='%s' AND BUSID='%s' AND BUSTYPE = '0'", (accept==0? 2:1), gmt0now,uid.c_str(), gps_id);
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

int GpsSvc::GetGpsOwner(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[GpsSvc::GetGpsOwner]bad format:", jsonString, 1);
    string shipid = root.getv("shipid", "");

	char sql[1024];
	MySql* psql = CREATE_MYSQL;
    sprintf(sql, "select user_id from t41_gps_connection where ship_id='%s' and is_valid = '1'", shipid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    int total = 0;
    out<<'[';
    while(psql->NextRow())
    {
        if(total != 0)
			out<<',';
		total++;

        out<<'\"'<<NOTNULL(psql->GetField("user_id"))<<'\"';
    }
    out<<']';

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"jiangtao",shipid:"",mmsi:"",sname:"",tm:139094332}
int GpsSvc::updSensorInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::updSensorInfo]bad format:", jsonString, 1);

	bool isSeipc = false;
	string uid = SeipcId2UserId(pUid, isSeipc);
	if(!isSeipc)
	{
		//非工控机
		return 2;
	}
	string shipid = root.getv("shipid", "");
	string mmsi = root.getv("mmsi", "");
	string sname = root.getv("sname", "");
	int tm = root.getv("tm", 0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024];
	sprintf (sql, "UPDATE t41_seipc_connection SET disconnect_time = '%d',is_valid = 0 WHERE seipc_id = '%s' AND is_valid = 1", tm, pUid);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	sprintf (sql, "insert INTO t41_seipc_connection (seipc_id, ship_id, mmsi, ship_name, connect_time, is_valid) VALUES	('%s', '%s', '%s', '%s', '%d', '1')",
				pUid, shipid.c_str(), mmsi.c_str(), sname.c_str(), tm);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{type:0,shipid:"S100021",tank:{id:"jiangtao_0_1",nm:"fuel1",loc:"1",ch:"2",unit:"0",cap:"32",thr0:"",thr1:""}}
int GpsSvc::updTankInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::updTankInfo]bad format:", jsonString, 1);
	bool isSeipc = false;
	string uid = SeipcId2UserId(pUid, isSeipc);
	if(!isSeipc)
	{
		//非工控机
		return 2;
	}
	
	int type = root.getv("type", 0);
	Json* tank = root["tank"];
	string id = tank->getv("id", "");
	string nm = tank->getv("nm", "");
	string loc = tank->getv("loc", "");
	string ch = tank->getv("ch", "");
	string unit = tank->getv("unit", "");
	string cap = tank->getv("cap", "");
	string thr0String = tank->getv("thr0", "");
	string thr1String = tank->getv("thr1", "");

	char gmt0now[20];
	GmtNow(gmt0now);

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	double thr0 = -1.0;
	double thr1 = -1.0;

	if (thr0String.length())
		thr0 = atof(thr0String.c_str());
	if (thr1String.length())
		thr1 = atof(thr1String.c_str());

	sprintf (sql, "REPLACE INTO t41_seipc_tank (tank_id, seipc_id, name, location, channel, unit, capacity, threshold0, threshold1, is_valid, update_dt)  VALUES	\
					('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%f', '%f', '1', '%s')", 
					id.c_str(), pUid, nm.c_str(), loc.c_str(), ch.c_str(), unit.c_str(), cap.c_str(), thr0, thr1,gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{shipid:"S100021",tankid:"jiangtao_0_1"}
int GpsSvc::delTankInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::delTankInfo]bad format:", jsonString, 1);
	bool isSeipc = false;
	string uid = SeipcId2UserId(pUid, isSeipc);
	if(!isSeipc)
	{
		//非工控机
		return 2;
	}

	string tankid = root.getv("tankid", "");
	MySql* psql = CREATE_MYSQL;
	
	char sql[1024] = "";
	sprintf (sql, "update t41_seipc_tank set is_valid = '0' WHERE tank_id = '%s'", tankid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"jiangtao",shipid:"S1000019"}
//[{id:"jiangtao_0_1",nm:"tank1",loc:"",ch:"",totalcap:"",unit:"",thr0:"",thr1:"",tm:13923232,curcap:"",consump:""},...]
int GpsSvc::getTankRealtimeInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::getTankRealtimeInfo]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	string shipid = root.getv("shipid", "");

	if (uid.empty())
		uid = pUid;

	MySql* psql = CREATE_MYSQL;

	string seipcid;
	if(!Verify(uid, shipid, seipcid, psql))
		RELEASE_MYSQL_RETURN(psql, 2);

	Tokens vals;
	int time_v = 0;
	char sql[1024] = "";
	sprintf (sql, "SELECT t1.time, t1.value FROM t41_seipc_realtime t1 WHERE t1.ship_id = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		string tmp = NOTNULL(psql->GetField("value"));
		vals = StrSplit(tmp, "|");
		time_v = atoi(NOTNULL(psql->GetField("time")));
	}

	sprintf (sql, "SELECT tank_id, name, location, channel, capacity, unit, threshold0, threshold1 FROM t41_seipc_tank WHERE seipc_id = '%s' and is_valid = '1'", seipcid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	TankRTMInfos tankRtmInfos;
	while (psql->NextRow())
	{
		TankRealtimeInfo tankRtmInfo;
		READMYSQL_STR(tank_id, tankRtmInfo.id);
		READMYSQL_STR(name, tankRtmInfo.nm);
		READMYSQL_INT(location, tankRtmInfo.loc, -1);
		READMYSQL_INT(channel, tankRtmInfo.ch, -1);
		READMYSQL_DOUBLE(capacity, tankRtmInfo.totalcap, -1.0);
		READMYSQL_STR(unit, tankRtmInfo.unit);
		READMYSQL_DOUBLE(threshold0, tankRtmInfo.thr0, -1.0);
		READMYSQL_DOUBLE(threshold1, tankRtmInfo.thr1, -1.0);
		//READMYSQL_INT(tm, tankRtmInfo.time, -1);	
		tankRtmInfo.time = time_v;

		tankRtmInfos.push_back(tankRtmInfo);
	}

	out << "[";
	for (TankRTMInfos::iterator iTer = tankRtmInfos.begin(); iTer != tankRtmInfos.end(); iTer++)
	{
		out << (iTer == tankRtmInfos.begin()?"":",");
		out << "{id:\"" << (*iTer).id << "\",nm:\"" << (*iTer).nm << "\",loc:\"" << (*iTer).loc << "\",ch:\"" << (*iTer).ch << "\",totalcap:\"" << (*iTer).totalcap << "\",unit:\"" << (*iTer).unit 
			<< "\",thr0:\"" << (*iTer).thr0 << "\",thr1:\"" << (*iTer).thr1 << "\",tm:" << (*iTer).time << ",curcap:\"" << (((*iTer).ch != -1 && ((int)vals.size() + 1) > (*iTer).ch)?vals[(*iTer).ch - 1]:"") << "\",consump:\"\"}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
/*
{
type:0,shipid:\"S100021\",id:\"jiangtao_1_1\",enginenm:\"Engine1\",loc:\"\",
rpm:{ bexist:0,ch:\"\",unit:\"\",thr0:\"\",thr1:\"\"},
oilpres:{bexist:0,ch:\"\",unit:\"\", thr0:\"\",thr1:\"\"},
oiltemp:{bexist:0,ch:\"\",unit:\"\", thr0:\"\",thr1:\"\"},
waterpres:{bexist:0,ch:\"\",unit:\"\", thr0:\"\",thr1:\"\"},
watertemp:{bexist:0,ch:\"\",unit:\"\", thr0:\"\",thr1:\"\"},
slaveoilpres: {bexist:0,ch:\"\",unit:\"\", thr0:\"\",thr1:\"\"}
}
*/
int GpsSvc::updEngineInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::updEngineInfo]bad format:", jsonString, 1);
	bool isSeipc = false;
	string uid = SeipcId2UserId(pUid, isSeipc);
	if(!isSeipc)
	{
		//非工控机
		return 2;
	}
	int type = root.getv("type", 0);
	string shipid = root.getv("shipid", "");
	string id = root.getv("id", "");
	string enginenm = root.getv("enginenm", "");
	string loc = root.getv("loc", "");

	Json* rpm = root["rpm"];
	int rmpExist = rpm->getv("bexist", 0);
	string rpmCh = rpm->getv("ch", "");
	string rpmUnit = rpm->getv("unit", "");
	string rpmThr0 = rpm->getv("thr0", "");
	string rpmThr1 = rpm->getv("thr1", "");

	Json* oilpres = root["oilpres"];
	int oilpresExist = oilpres->getv("bexist", 0);
	string oilpresCh = oilpres->getv("ch", "");
	string oilpresUnit = oilpres->getv("unit", "");
	string oilpresThr0 = oilpres->getv("thr0", "");
	string oilpresThr1 = oilpres->getv("thr1", "");

	Json* oiltemp = root["oiltemp"];
	int oiltempExist = oiltemp->getv("bexist", 0);
	string oiltempCh = oiltemp->getv("ch", "");
	string oiltempUnit = oiltemp->getv("unit", "");
	string oiltempThr0 = oiltemp->getv("thr0", "");
	string oiltempThr1 = oiltemp->getv("thr1", "");

	Json* waterpres = root["waterpres"];
	int waterpresExist = waterpres->getv("bexist", 0);
	string waterpresCh = waterpres->getv("ch", "");
	string waterpresUnit = waterpres->getv("unit", "");
	string waterpresThr0 = waterpres->getv("thr0", "");
	string waterpresThr1 = waterpres->getv("thr1", "");

	Json* watertemp = root["watertemp"];
	int watertempExist = watertemp->getv("bexist", 0);
	string watertempCh = watertemp->getv("ch", "");
	string watertempUnit = watertemp->getv("unit", "");
	string watertempThr0 = watertemp->getv("thr0", "");
	string watertempThr1 = watertemp->getv("thr1", "");

	Json* slaveoilpres = root["slaveoilpres"];
	int slaveoilpresExist = slaveoilpres->getv("bexist", 0);
	string slaveoilpresCh = slaveoilpres->getv("ch", "");
	string slaveoilpresUnit = slaveoilpres->getv("unit", "");
	string slaveoilpresThr0 = slaveoilpres->getv("thr0", "");
	string slaveoilpresThr1 = slaveoilpres->getv("thr1", "");

	MySql* psql = CREATE_MYSQL;

	char gmt0now[20];
	GmtNow(gmt0now);

	char sql[1024*10] = "";
	int len;
	len = sprintf (sql, "REPLACE INTO t41_seipc_engine (engine_id, seipc_id, `name`, location, is_valid, update_dt");
	if (rmpExist)
		len += sprintf (sql + len, ", rpm_channel, rpm_unit, rpm_threshold0, rpm_threshold1");
	if (oilpresExist)
		len += sprintf (sql + len, ", oilpress_channel, oilpress_unit, oilpress_threshold0, oilpress_threshold1");
	if (oiltempExist)
		len += sprintf (sql + len, ", oiltemp_channel, oiltemp_unit, oiltemp_threshold0, oiltemp_threshold1");
	if (waterpresExist)
		len += sprintf (sql + len, ", waterpress_channel, waterpress_unit, waterpress_threshold0, waterpress_threshold1");
	if (watertempExist)
		len += sprintf (sql + len, ", watertemp_channel, watertemp_unit, watertemp_threshold0, watertemp_threshold1");
	if (slaveoilpresExist)
		len += sprintf (sql + len, ", slave_channel, slave_unit, slave_threshold0, slave_threshold1");

	len += sprintf (sql + len, ") VALUES ('%s', '%s', '%s', '%s', '1', '%s'", id.c_str(), pUid, enginenm.c_str(), loc.c_str(),gmt0now);
	if (rmpExist)
		len += sprintf (sql + len, ", '%s', '%s', '%s', '%s'", rpmCh.c_str(), rpmUnit.c_str(), rpmThr0.c_str(), rpmThr1.c_str());
	if (oilpresExist)
		len += sprintf (sql + len, ", '%s', '%s', '%s', '%s'", oilpresCh.c_str(), oilpresUnit.c_str(), oilpresThr0.c_str(), oilpresThr1.c_str());
	if (oiltempExist)
		len += sprintf (sql + len, ", '%s', '%s', '%s', '%s'", oiltempCh.c_str(), oiltempUnit.c_str(), oiltempThr0.c_str(), oiltempThr1.c_str());
	if (waterpresExist)
		len += sprintf (sql + len, ", '%s', '%s', '%s', '%s'", waterpresCh.c_str(), waterpresUnit.c_str(), waterpresThr0.c_str(), waterpresThr1.c_str());
	if (watertempExist)
		len += sprintf (sql + len, ", '%s', '%s', '%s', '%s'", watertempCh.c_str(),watertempUnit.c_str(), watertempThr0.c_str(), watertempThr1.c_str());
	if (slaveoilpresExist)
		len += sprintf (sql + len, ", '%s', '%s', '%s', '%s'", slaveoilpresCh.c_str(), slaveoilpresUnit.c_str(), slaveoilpresThr0.c_str(), slaveoilpresThr1.c_str());

	sprintf (sql + len, ")");
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{shipid:"S100021",engineid:"jiangtao_0_1"}
int GpsSvc::delEngineInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::delEngineInfo]bad format:", jsonString, 1);
	bool isSeipc = false;
	string uid = SeipcId2UserId(pUid, isSeipc);
	if(!isSeipc)
	{
		//非工控机
		return 2;
	}
	string engineid = root.getv("engineid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "update t41_seipc_engine set is_valid = '0' WHERE engine_id = '%s'", engineid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"jiangtao",shipid:"S1000019"}
int GpsSvc::getEngineRealtimeInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::getEngineRealtimeInfo]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	string shipid = root.getv("shipid", "");

	if(!uid.length())
		uid = pUid;

	MySql* psql = CREATE_MYSQL;

	string seipcid;
	if(!Verify(uid, shipid, seipcid, psql))
		RELEASE_MYSQL_RETURN(psql, 2);

	Tokens vals;
	char sql[1024] = "";
	int time_v = 0;
	sprintf (sql, "SELECT time, value FROM t41_seipc_realtime WHERE ship_id = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		time_v = atoi(NOTNULL(psql->GetField("time")));
		string tmp = NOTNULL(psql->GetField("value"));
		vals = StrSplit(tmp, "|");
	}

	sprintf (sql, "SELECT engine_id, t41_seipc_engine.name, location,								\
							rpm_value, rpm_channel, rpm_unit, rpm_threshold0, rpm_threshold1,										\
							oilpress_value, oilpress_channel, oilpress_unit, oilpress_threshold0, oilpress_threshold1,				\
							oiltemp_value, oiltemp_channel, oiltemp_unit, oiltemp_threshold0, oiltemp_threshold1,					\
							waterpress_value, waterpress_channel, waterpress_unit, waterpress_threshold0, waterpress_threshold1,	\
							watertemp_value, watertemp_channel, watertemp_unit, watertemp_threshold0, watertemp_threshold1,			\
							slave_value, slave_channel, slave_unit, slave_threshold0, slave_threshold1								\
							FROM t41_seipc_engine WHERE seipc_id = '%s' and is_valid = '1'", seipcid.c_str());

	EngineRTMInfoVEC eRTMInfoVec;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while (psql->NextRow())
	{
		EngineRealtimeInfo engineRtmInfo;
		READMYSQL_STR(engine_id, engineRtmInfo.engineid);
		READMYSQL_STR(name, engineRtmInfo.name);
		READMYSQL_INT(location,engineRtmInfo.location,-1);
		engineRtmInfo.time = time_v;
		engineRtmInfo.rpmExit = 0;
		READMYSQL_INT(rpm_value,engineRtmInfo.rpm_value,-1);
		//engineRtmInfo.rpmExit = (engineRtmInfo.rpm_value != -1)?1:engineRtmInfo.rpmExit;

		READMYSQL_INT(rpm_channel,engineRtmInfo.rpm_channel,-1);
		engineRtmInfo.rpmExit = (engineRtmInfo.rpm_channel != -1)?1:engineRtmInfo.rpmExit;

		READMYSQL_STR(rpm_unit, engineRtmInfo.rpm_unit);
		//engineRtmInfo.rpmExit = strlen(engineRtmInfo.rpm_unit)?1:engineRtmInfo.rpmExit;

		READMYSQL_DOUBLE(rpm_threshold0,engineRtmInfo.rpm_threshold0, -1.0);
		//engineRtmInfo.rpmExit = (engineRtmInfo.rpm_threshold0 < 0)?1:engineRtmInfo.rpmExit;

		READMYSQL_DOUBLE(rpm_threshold1,engineRtmInfo.rpm_threshold1, -1.0);
		//engineRtmInfo.rpmExit = (engineRtmInfo.rpm_threshold1 < 0)?1:engineRtmInfo.rpmExit;

		engineRtmInfo.oilpressExit = 0;
		READMYSQL_INT(oilpress_value,engineRtmInfo.oilpress_value,-1);
		//engineRtmInfo.oilpressExit = (engineRtmInfo.oilpress_value != -1)?1:engineRtmInfo.oilpressExit;

		READMYSQL_INT(oilpress_channel,engineRtmInfo.oilpress_channel,-1);
		engineRtmInfo.oilpressExit = (engineRtmInfo.oilpress_channel != -1)?1:engineRtmInfo.oilpressExit;

		READMYSQL_STR(oilpress_unit, engineRtmInfo.oilpress_unit);
		//engineRtmInfo.oilpressExit = strlen(engineRtmInfo.oilpress_unit)?1:engineRtmInfo.oilpressExit;

		READMYSQL_DOUBLE(oilpress_threshold0,engineRtmInfo.oilpress_threshold0, -1.0);
		//engineRtmInfo.oilpressExit = (engineRtmInfo.oilpress_threshold0  < 0)?1:engineRtmInfo.oilpressExit;

		READMYSQL_DOUBLE(oilpress_threshold1,engineRtmInfo.oilpress_threshold1, -1.0);
		//engineRtmInfo.oilpressExit = (engineRtmInfo.oilpress_threshold1 < 0)?1:engineRtmInfo.oilpressExit;

		engineRtmInfo.oiltempExit = 0;
		READMYSQL_INT(oiltemp_value,engineRtmInfo.oiltemp_value,-1);
		//engineRtmInfo.oiltempExit = (engineRtmInfo.oiltemp_value != -1)?1:engineRtmInfo.oiltempExit;

		READMYSQL_INT(oiltemp_channel,engineRtmInfo.oiltemp_channel,-1);
		engineRtmInfo.oiltempExit = (engineRtmInfo.oiltemp_channel != -1)?1:engineRtmInfo.oiltempExit;

		READMYSQL_STR(oiltemp_unit, engineRtmInfo.oiltemp_unit);
		//engineRtmInfo.oiltempExit = strlen(engineRtmInfo.oiltemp_unit)?1:engineRtmInfo.oiltempExit;

		READMYSQL_DOUBLE(oiltemp_threshold0,engineRtmInfo.oiltemp_threshold0, -1.0);
		//engineRtmInfo.oiltempExit = (engineRtmInfo.oiltemp_threshold0 < 0)?1:engineRtmInfo.oiltempExit;

		READMYSQL_DOUBLE(oiltemp_threshold1,engineRtmInfo.oiltemp_threshold1, -1.0);
		//engineRtmInfo.oiltempExit = (engineRtmInfo.oiltemp_threshold1 < 0)?1:engineRtmInfo.oiltempExit;

		engineRtmInfo.waterpressExit = 0;
		READMYSQL_INT(waterpress_value,engineRtmInfo.waterpress_value,-1);
		//engineRtmInfo.waterpressExit = (engineRtmInfo.waterpress_value != -1)?1:engineRtmInfo.waterpressExit;

		READMYSQL_INT(waterpress_channel,engineRtmInfo.waterpress_channel,-1);
		engineRtmInfo.waterpressExit = (engineRtmInfo.waterpress_channel != -1)?1:engineRtmInfo.waterpressExit;

		READMYSQL_STR(waterpress_unit, engineRtmInfo.waterpress_unit);
		//engineRtmInfo.waterpressExit = strlen(engineRtmInfo.waterpress_unit)?1:engineRtmInfo.waterpressExit;

		READMYSQL_DOUBLE(waterpress_threshold0,engineRtmInfo.waterpress_threshold0,  -1.0);
		//engineRtmInfo.waterpressExit = (engineRtmInfo.waterpress_threshold0 < 0)?1:engineRtmInfo.waterpressExit;

		READMYSQL_DOUBLE(waterpress_threshold1,engineRtmInfo.waterpress_threshold1, -1.0);
		//engineRtmInfo.waterpressExit = (engineRtmInfo.waterpress_threshold1 < 0)?1:engineRtmInfo.waterpressExit;

		engineRtmInfo.watertempExit = 0;
		READMYSQL_INT(watertemp_value,engineRtmInfo.watertemp_value,-1);
		//engineRtmInfo.watertempExit = (engineRtmInfo.watertemp_value != -1)?1:engineRtmInfo.watertempExit;

		READMYSQL_INT(watertemp_channel,engineRtmInfo.watertemp_channel,-1);
		engineRtmInfo.watertempExit = (engineRtmInfo.watertemp_channel != -1)?1:engineRtmInfo.watertempExit;

		READMYSQL_STR(watertemp_unit, engineRtmInfo.watertemp_unit);
		//engineRtmInfo.watertempExit = strlen(engineRtmInfo.watertemp_unit)?1:engineRtmInfo.watertempExit;

		READMYSQL_DOUBLE(watertemp_threshold0,engineRtmInfo.watertemp_threshold0, -1.0);
		//engineRtmInfo.watertempExit = (engineRtmInfo.watertemp_threshold0 < 0)?1:engineRtmInfo.watertempExit;

		READMYSQL_DOUBLE(watertemp_threshold1,engineRtmInfo.watertemp_threshold1,-1.0);
		//engineRtmInfo.watertempExit = (engineRtmInfo.watertemp_threshold1 < 0)?1:engineRtmInfo.watertempExit;

		engineRtmInfo.slaveExit = 0;
		READMYSQL_INT(slave_value,engineRtmInfo.slave_value,-1);
		//engineRtmInfo.slaveExit = (engineRtmInfo.slave_value != -1)?1:engineRtmInfo.slaveExit;

		READMYSQL_INT(slave_channel,engineRtmInfo.slave_channel,-1);
		engineRtmInfo.slaveExit = (engineRtmInfo.slave_channel != -1)?1:engineRtmInfo.slaveExit;

		READMYSQL_STR(slave_unit, engineRtmInfo.slave_unit);
		//engineRtmInfo.slaveExit = strlen(engineRtmInfo.slave_unit)?1:engineRtmInfo.slaveExit;

		READMYSQL_DOUBLE(slave_threshold0,engineRtmInfo.slave_threshold0,-1.0);
		//engineRtmInfo.slaveExit = (engineRtmInfo.slave_threshold0 < 0)?1:engineRtmInfo.slaveExit;

		READMYSQL_DOUBLE(slave_threshold1,engineRtmInfo.slave_threshold1,-1.0);
		//engineRtmInfo.slaveExit = (engineRtmInfo.slave_threshold1 < 0)?1:engineRtmInfo.slaveExit;

		eRTMInfoVec.push_back(engineRtmInfo);
	}

	out << "[";
	for (EngineRTMInfoVEC::iterator iTer = eRTMInfoVec.begin(); iTer != eRTMInfoVec.end(); iTer++)
	{
		out << (iTer == eRTMInfoVec.begin()?"":",");
		out << "{id:\"" << (*iTer).engineid << "\",nm:\"" << (*iTer).name << "\",loc:\"" << (*iTer).location << "\",tm:" << (*iTer).time << ",";
		if ((*iTer).rpmExit)
			out << "rpm:{bexist:1,ch:\"" << (*iTer).rpm_channel << "\",unit:\"" << (*iTer).rpm_unit << "\",thr0:\"" << (*iTer).rpm_threshold0<< "\",thr1:\"" << (*iTer).rpm_threshold1 << "\",curspd:\"" << (((*iTer).rpm_channel!=-1 && ((int)vals.size() + 1)> (*iTer).rpm_channel)?vals[(*iTer).rpm_channel - 1]:"") << "\"},";
		else
			out << "rpm:{bexist:0,ch:\"\",unit:\"\",thr0:\"\",thr1:\"\",curspd:\"\"},";

		if ((*iTer).oilpressExit)
			out << "oilpres:{bexist:1,ch:\"" << (*iTer).oilpress_channel << "\",unit:\"" << (*iTer).oilpress_unit << "\",thr0:\"" << (*iTer).oilpress_threshold0 << "\",thr1:\"" << (*iTer).oilpress_threshold1 << "\",curpres:\"" << (((*iTer).oilpress_channel!=-1  && ((int)vals.size() + 1) > (*iTer).oilpress_channel)?vals[(*iTer).oilpress_channel - 1]:"") << "\"},";
		else
			out << "oilpres:{bexist:0,ch:\"\",unit:\"\",thr0:\"\",thr1:\"\",curpres:\"\"},";

		if ((*iTer).oiltempExit)
			out << "oiltemp:{bexist:1,ch:\"" << (*iTer).oiltemp_channel << "\",unit:\"" << (*iTer).oiltemp_unit << "\",thr0:\"" << (*iTer).oiltemp_threshold0 << "\",thr1:\"" << (*iTer).oiltemp_threshold1 << "\",curtemp:\"" << (((*iTer).oiltemp_channel!=-1 && ((int)vals.size() + 1) > (*iTer).oiltemp_channel)?vals[(*iTer).oiltemp_channel - 1]:"") << "\"},";
		else
			out << "oiltemp:{bexist:0,ch:\"\",unit:\"\",thr0:\"\",thr1:\"\",curtemp:\"\"},";

		if ((*iTer).waterpressExit)
			out << "waterpres:{bexist:1,ch:\"" << (*iTer).waterpress_channel << "\",unit:\"" << (*iTer).waterpress_unit << "\",thr0:\"" << (*iTer).waterpress_threshold0 << "\",thr1:\"" << (*iTer).waterpress_threshold1 << "\",curpres:\"" << (((*iTer).waterpress_channel!=-1 && ((int)vals.size() + 1) > (*iTer).waterpress_channel)?vals[(*iTer).waterpress_channel - 1]:"") << "\"},";
		else
			out << "waterpres:{bexist:0,ch:\"\",unit:\"\",thr0:\"\",thr1:\"\",curpres:\"\"},";
				
		if ((*iTer).watertempExit)
			out << "watertemp:{bexist:1,ch:\"" << (*iTer).watertemp_channel << "\",unit:\"" << (*iTer).watertemp_unit << "\",thr0:\"" << (*iTer).watertemp_threshold0 << "\",thr1:\"" << (*iTer).watertemp_threshold1 << "\",curtemp:\"" << (((*iTer).watertemp_channel!=-1 && ((int)vals.size() + 1) > (*iTer).watertemp_channel)?vals[(*iTer).watertemp_channel - 1]:"") << "\"},";
		else
			out << "watertemp:{bexist:0,ch:\"\",unit:\"\",thr0:\"\",thr1:\"\",curtemp:\"\"},";
		
		if ((*iTer).slaveExit)
			out <<"slaveoilpres:{bexist:1,ch:\"" << (*iTer).slave_channel << "\",unit:\"" << (*iTer).slave_unit << "\", thr0:\"" << (*iTer).slave_threshold0 << "\",thr1:\"" << (*iTer).slave_threshold1 << "\", curpres:\"" << (((*iTer).slave_channel!=-1 && ((int)vals.size() + 1) > (*iTer).slave_channel)?vals[(*iTer).slave_channel - 1]:"") << "\"}";
		else
			out <<"slaveoilpres:{bexist:0,ch:\"\",unit:\"\", thr0:\"\",thr1:\"\", curpres:\"\"}";
				
		out << "}";
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
/*
"{\
type:0,shipid:\"S100021\",proid:\"jiangtao_2_0\",propelnm:\"\",loc:\"\",\
loilpres:{bexist:1,ch:\"\",unit:\"\",thr0:\"\",thr1:\"\"},\
hoilpres:{bexist:1, ch:\"\",unit:\"\", thr0:\"\",thr1:\"\"}\
}";
*/
int GpsSvc::updQuantInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::updQuantInfo]bad format:", jsonString, 1);
	bool isSeipc = false;
	string uid = SeipcId2UserId(pUid, isSeipc);
	if(!isSeipc)
	{
		//非工控机
		return 2;
	}
	int type = root.getv("type", 0);
	string shipid = root.getv("shipid", "");
	string proid = root.getv("proid", "");
	string propelnm = root.getv("propelnm", "");
	string loc = root.getv("loc", "");

	Json* loilpres = root["loilpres"];
	int loilExist = loilpres->getv("bexist", 0);
	string loilCh = loilpres->getv("ch", "");
	string loilUnit = loilpres->getv("unit", "");
	string loilThr0 = loilpres->getv("thr0", "");
	string loilThr1 = loilpres->getv("thr1", "");

	Json* hoilpres = root["hoilpres"];
	int hoilExist = hoilpres->getv("bexist", 0);
	string hoilCh = hoilpres->getv("ch", "");
	string hoilUnit = hoilpres->getv("unit", "");
	string hoilThr0 = hoilpres->getv("thr0", "");
	string hoilThr1 = hoilpres->getv("thr1", "");

	char gmt0now[20];
	GmtNow(gmt0now);

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	int len  = 0;
	len = sprintf (sql, "REPLACE INTO t41_seipc_rudder (RUDDER_ID, seipc_id, `NAME`, LOCATION, is_valid, UPDATE_DT");
	if (loilExist)
		len += sprintf (sql + len, ", LOILPRESS_CHANNEL, LOILPRESS_UNIT, LOILPRESS_THRESHOLD0, LOILPRESS_THRESHOLD1");
	if (hoilExist)
		len += sprintf (sql + len, ", HOILPRESS_CHANNEL, HOILPRESS_UNIT, HOILPRESS_THRESHOLD0, HOILPRESS_THRESHOLD1");
	len += sprintf (sql + len, ") VALUES ('%s', '%s', '%s', '%s', '1', '%s'", proid.c_str(), pUid, propelnm.c_str(), loc.c_str(),gmt0now);
	if (loilExist)
		len += sprintf (sql + len, ", '%s', '%s', '%s', '%s'", loilCh.c_str(), loilUnit.c_str(), loilThr0.c_str(), loilThr1.c_str());
	if (hoilExist)
		len += sprintf (sql + len, ", '%s', '%s', '%s', '%s'", hoilCh.c_str(), hoilUnit.c_str(), hoilThr0.c_str(), hoilThr1.c_str());
	sprintf (sql + len, ")");
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{shipid:"S100021",proid:"jiangtao_0_1"}
int GpsSvc::delQuantInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::delQuantInfo]bad format:", jsonString, 1);
	bool isSeipc = false;
	string uid = SeipcId2UserId(pUid, isSeipc);
	if(!isSeipc)
	{
		//非工控机
		return 2;
	}
	//string shipid = root.getv("shipid", "");
	string proid = root.getv("proid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "update t41_seipc_rudder set is_valid = '0' WHERE rudder_id = '%s'", proid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:”jiangtao”,shipid:”S1000019”}
int GpsSvc::getQuantRealtimeInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::getQuantRealtimeInfo]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	string shipid = root.getv("shipid", "");

	if (uid.empty())
		uid = pUid;

	MySql* psql = CREATE_MYSQL;

	string seipcid;
	if(!Verify(uid, shipid, seipcid, psql))
		RELEASE_MYSQL_RETURN(psql, 2);

	Tokens vals;
	int time_v = 0;
	char sql[1024] = "";
	sprintf (sql, "SELECT time, value FROM t41_seipc_realtime WHERE ship_id = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		string tmp = NOTNULL(psql->GetField("value"));
		vals = StrSplit(tmp, "|");
		time_v = atoi(NOTNULL(psql->GetField("time")));
	}

	sprintf (sql, "SELECT RUDDER_ID, t41_seipc_rudder.NAME, LOCATION,															\
							LOILPRESS_VALUE, LOILPRESS_CHANNEL, LOILPRESS_UNIT, LOILPRESS_THRESHOLD0, LOILPRESS_THRESHOLD1,		\
							HOILPRESS_VALUE, HOILPRESS_CHANNEL, HOILPRESS_UNIT, HOILPRESS_THRESHOLD0, HOILPRESS_THRESHOLD1		\
							FROM t41_seipc_rudder WHERE seipc_id = '%s' and is_valid = '1'", seipcid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	RudderRTMInfos rudderRtmInfos;
	while (psql->NextRow())
	{
		RudderRealTimeInfo rudderRtmInfo;
		READMYSQL_STR(RUDDER_ID, rudderRtmInfo.id);
		READMYSQL_STR(NAME, rudderRtmInfo.name);
		READMYSQL_INT(LOCATION, rudderRtmInfo.loc, -1);
		//READMYSQL_INT(tm, rudderRtmInfo.tm, -1);
		rudderRtmInfo.tm = time_v;

		rudderRtmInfo.loilpreExit = 0;
		READMYSQL_INT(LOILPRESS_VALUE, rudderRtmInfo.loilpre_val, -1);
		//rudderRtmInfo.loilpreExit = (rudderRtmInfo.loilpre_val != -1)?1:rudderRtmInfo.loilpreExit;

		READMYSQL_INT(LOILPRESS_CHANNEL, rudderRtmInfo.loilpre_ch, -1);
		rudderRtmInfo.loilpreExit = (rudderRtmInfo.loilpre_ch != -1)?1:rudderRtmInfo.loilpreExit;

		READMYSQL_STR(LOILPRESS_UNIT, rudderRtmInfo.loilpre_unit);
		//rudderRtmInfo.loilpreExit = strlen(rudderRtmInfo.loilpre_unit)?1:rudderRtmInfo.loilpreExit;

		READMYSQL_DOUBLE(LOILPRESS_THRESHOLD0, rudderRtmInfo.loilpre_thr0, -1.0);
		//rudderRtmInfo.loilpreExit = (rudderRtmInfo.loilpre_thr0 < 0)?1:rudderRtmInfo.loilpreExit;

		READMYSQL_DOUBLE(LOILPRESS_THRESHOLD1, rudderRtmInfo.loilpre_thr1, -1.0);
		//rudderRtmInfo.loilpreExit = (rudderRtmInfo.loilpre_thr1 < 0)?1:rudderRtmInfo.loilpreExit;

		rudderRtmInfo.hoilpreExit = 0;
		READMYSQL_INT(HOILPRESS_VALUE, rudderRtmInfo.hoilpre_val, -1);
		//rudderRtmInfo.hoilpreExit = (rudderRtmInfo.hoilpre_val != -1)?1:rudderRtmInfo.hoilpreExit;

		READMYSQL_INT(HOILPRESS_CHANNEL, rudderRtmInfo.hoilpre_ch, -1);
		rudderRtmInfo.hoilpreExit = (rudderRtmInfo.hoilpre_ch != -1)?1:rudderRtmInfo.hoilpreExit;

		READMYSQL_STR(HOILPRESS_UNIT, rudderRtmInfo.hoilpre_unit);
		//rudderRtmInfo.hoilpreExit = strlen(rudderRtmInfo.hoilpre_unit)?1:rudderRtmInfo.hoilpreExit;

		READMYSQL_DOUBLE(HOILPRESS_THRESHOLD0, rudderRtmInfo.hoilpre_thr0, -1.0);
		//rudderRtmInfo.hoilpreExit = (rudderRtmInfo.hoilpre_thr0 < 0)?1:rudderRtmInfo.hoilpreExit;

		READMYSQL_DOUBLE(HOILPRESS_THRESHOLD1, rudderRtmInfo.hoilpre_thr1, -1.0);
		//rudderRtmInfo.hoilpreExit = (rudderRtmInfo.hoilpre_thr1 < 0)?1:rudderRtmInfo.hoilpreExit;

		rudderRtmInfos.push_back(rudderRtmInfo);
	}
	out << "[";
	for (RudderRTMInfos::iterator iTer = rudderRtmInfos.begin(); iTer != rudderRtmInfos.end(); iTer++)
	{
		out << (iTer == rudderRtmInfos.begin()?"":",");
		out << "{id:\"" << (*iTer).id << "\",nm:\"" << (*iTer).name << "\",loc:\"" << (*iTer).loc << "\",tm:" << (*iTer).tm << ",";
		if ((*iTer).loilpreExit)
			out << "loilpres:{ bexist:1, ch:\"" << (*iTer).loilpre_ch << "\", unit:\"" << (*iTer).loilpre_unit << "\",thr0:\"" << (*iTer).loilpre_thr0 << "\", thr1:\"" << (*iTer).loilpre_thr1 << "\", curpres:\"" << (( (*iTer).loilpre_ch != -1 && ((int)vals.size() + 1) > (*iTer).loilpre_ch)?vals[(*iTer).loilpre_ch - 1]:"") << "\"},";
		else
			out << "loilpres:{ bexist:0, ch:\"\", unit:\"\",thr0:\"\", thr1:\"\", curpres:\"\"},";
		if ((*iTer).hoilpreExit)
			out << "hoilpres:{ bexist:1, ch:\"" << (*iTer).hoilpre_ch << "\",unit:\"" << (*iTer).hoilpre_unit << "\", thr0:\"" << (*iTer).hoilpre_thr0 << "\", thr1:\"" << (*iTer).hoilpre_thr1 << "\", curpres:\"" << (( (*iTer).hoilpre_ch != -1 && ((int)vals.size() + 1) > (*iTer).hoilpre_ch)?vals[(*iTer).hoilpre_ch - 1]:"") << "\"}";
		else 
			out << "hoilpres:{ bexist:0, ch:\"\",unit:\"\", thr0:\"\", thr1:\"\", curpres:\"\"}";
		out << "}";
	}
	out << "]";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{shipid:"S100021",tm:13912323,val:""}
int GpsSvc::updSensorRealtimeInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::updSensorRealtimeInfo]bad format:", jsonString, 1);
	bool isSeipc = false;
	string uid = SeipcId2UserId(pUid, isSeipc);

	if(!isSeipc)
	{
		//非工控机
		return 2;
	}
	string shipid = root.getv("shipid", "");
	int tm = root.getv("tm", 0);
	string val = root.getv("val", "");

	MySql* psql = CREATE_MYSQL;
	
	char sql[1024] = "";
	sprintf (sql, "REPLACE INTO t41_seipc_history(SHIP_ID, TIME, VALUE) SELECT ship_id, `time`, `value` FROM t41_seipc_realtime WHERE ship_id = '%s'", shipid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	sprintf (sql, "REPLACE INTO t41_seipc_realtime(SHIP_ID, TIME, VALUE) VALUES ('%s', '%d', '%s')", shipid.c_str(), tm, val.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	//工控机预警
	Tokens _vals = StrSplit(val, "|");
	int _valsLen = 0;
	for (size_t i = 0; i < _vals.size(); i++)
		_valsLen += _vals[i].length();
	if (_valsLen)
	{
		int _flag = strcmp(shipid.c_str(), "S992010102496042")?(!strcmp(shipid.c_str(),"S992010100792811")||!strcmp(shipid.c_str(),"S992010102496051")?1:0):2;
		dealRealtimeValue(shipid, tm, val, _flag);
	}

	//send seip data to trackserver
	char dest[1024] = "";
	sprintf(dest, "s@%s %d@%s", shipid.c_str(), tm, val.c_str());
	size_t destLen = strlen(dest) + 1;

	ACE_INET_Addr ais_udp_addr(10011);
	//ACE_INET_Addr& addrTrack_udp = g_MainConfig::instance()->GetTrackAddr_udp();	
	ACE_SOCK_Dgram	ais_udp_endpoint(ais_udp_addr);
	ais_udp_endpoint.enable(ACE_NONBLOCK);

	ACE_INET_Addr m_addrTrack_udp_beijing = ACE_INET_Addr(5001, "192.168.0.179");
	size_t nSend = ais_udp_endpoint.send(dest, destLen, m_addrTrack_udp_beijing);

	ACE_INET_Addr m_addrTrack_udp_shanghai = ACE_INET_Addr(5001, "192.168.1.179");
	nSend = ais_udp_endpoint.send(dest, destLen, m_addrTrack_udp_shanghai);

	ACE_INET_Addr m_addrTrack_udp_usa = ACE_INET_Addr(5001, "192.168.6.179");
	nSend = ais_udp_endpoint.send(dest, destLen, m_addrTrack_udp_usa);

	ACE_INET_Addr m_addrTrack_udp_jiangyin = ACE_INET_Addr(5001, "192.168.7.179");
	nSend = ais_udp_endpoint.send(dest, destLen, m_addrTrack_udp_jiangyin);
	ais_udp_endpoint.close();

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{shipid:"S1000019"}
int GpsSvc::getSensorAdmin(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::getSensorAdmin]bad format:", jsonString, 1);
	string shipid = root.getv("shipid", "");
	bool isSeipc = false;
	string seipcid;

	MySql* psql = CREATE_MYSQL;
	Verify(pUid, shipid, seipcid, psql);
	string owner = SeipcId2UserId(seipcid, isSeipc);
	
	out << "{owner:\"" << owner << "\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"jiangtao",shipid:"S1000019"}
int GpsSvc::getSensorAuth(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::getSensorpAuth]bad format:", jsonString, 1);
	bool isSeipc = false;
	//string uid = SeipcId2UserId(pUid, isSeipc);
	string uid = root.getv("uid", "");
	string shipid = root.getv("shipid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	int flg = 0;
	sprintf (sql, "SELECT COUNT(1) AS num FROM t41_seipc_connection AS t1, t41_seipc_user AS t2 WHERE t1.seipc_id = t2.SEIPC_ID AND t1.SHIP_ID = '%s' AND t2.USER_ID = '%s' AND t1.is_valid = '1'", shipid.c_str(), uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		READMYSQL_INT(num, flg, 0);
	}
	
	if (flg)
	{
		//sprintf (sql, "SELECT t1.user_id, t1.accept, UNIX_TIMESTAMP(t1.UPDATE_DT) AS updt, t2.nickname FROM t41_seipc_apply AS t1 LEFT JOIN t00_user AS t2 ON t1.user_id = t2.user_id WHERE ship_id = '%s'", shipid.c_str());
		sprintf (sql, "SELECT t1.user_id, t1.accept, UNIX_TIMESTAMP(t1.UPDATE_DT) AS updt, t2.nickname \
							FROM t41_authen_apply AS t1 LEFT JOIN t00_user AS t2 ON t1.user_id = t2.user_id WHERE BUSID = '%s' AND t1.BUSTYPE = '1'", shipid.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		UserInfos userInfos;
		while (psql->NextRow())
		{
			UserInfo userInfo;
			READMYSQL_STR(user_id, userInfo.id);
			READMYSQL_INT(updt, userInfo.tm, 0);
			READMYSQL_INT(accept, userInfo.status, -1);
			READMYSQL_STR(nickname, userInfo.nickname);
			userInfos.push_back(userInfo);
		}
		out << "{appstate:\"-1\",appliers:[";
		for (UserInfos::iterator iTer = userInfos.begin(); iTer != userInfos.end(); iTer++)
		{
			if (!strcmp((*iTer).id, uid.c_str()))
				continue;
			out << (iTer == userInfos.begin()?"":",");
			out << "{usrId:\"" << (*iTer).id << "\",usrnk:\"" << (*iTer).nickname << "\",state:\"" << (*iTer).status << "\",updt:" << (*iTer).tm << "}";
		}
		out << "]}";
	}
	else
	{
		//sprintf (sql, "SELECT ACCEPT FROM t41_seipc_apply WHERE USER_ID = '%s' AND SHIP_ID = '%s'", uid.c_str(), shipid.c_str());
		sprintf (sql, "SELECT ACCEPT FROM t41_authen_apply WHERE USER_ID = '%s' AND BUSID = '%s' AND BUSTYPE = '1'", uid.c_str(), shipid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		flg = -1;
		if (psql->NextRow())
		{
			READMYSQL_INT(ACCEPT, flg, -1);
		}
		out << "{appstate:\"" << flg << "\",appliers:[]}";
	}
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:"jiangtao",shipid:"S1000019",ownerid:"sunliang"}
int GpsSvc::applySensorAuth(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::applySensorpAuth]bad format:", jsonString, 1);

	string uid = root.getv("uid", "");
	string shipid = root.getv("shipid", "");
	//string ownerid = root.getv("ownerid", "");
	//ownerid = SeipcId2UserId(ownerid);

	char gmt0now[20];
	GmtNow(gmt0now);

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	//sprintf (sql, "REPLACE INTO t41_seipc_apply (USER_ID, SHIP_ID, APPLY_DT, ACCEPT, UPDATE_DT)	VALUES ('%s', '%s', '%s', 0, '%s')", uid.c_str(), shipid.c_str(),gmt0now,gmt0now);
	sprintf (sql, "REPLACE INTO t41_authen_apply (USER_ID, BUSTYPE, BUSID, APPLY_DT, ACCEPT, UPDATE_DT)	\
												VALUES ('%s', '1', '%s', '%s', 0, '%s')", uid.c_str(), shipid.c_str(),gmt0now,gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{ownerid:"",uid:"jiangtao",shipid:"S1000019",accept:"0"}
int GpsSvc::dealSensorApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[GpsSvc::dealSensorApply]bad format:", jsonString, 1);
	//string ownerid = root.getv("ownerid", "");
	//ownerid = SeipcId2UserId(ownerid);
	string uid = root.getv("uid", "");
	string shipid = root.getv("shipid", "");
	int accept = atoi(root.getv("accept", ""));

	char gmt0now[20];
	GmtNow(gmt0now);

	MySql* psql = CREATE_MYSQL;
	char sql[1024];
	//sprintf (sql, "UPDATE t41_seipc_apply SET accept = '%d',update_dt = '%s' WHERE ship_id = '%s' AND user_id = '%s'", (accept==0)?2:1, gmt0now,shipid.c_str(), uid.c_str());
	if (accept == 2)
		sprintf (sql, "DELETE FROM t41_authen_apply WHERE user_id='%s' AND BUSID='%s' AND BUSTYPE = '1'", uid.c_str(), shipid.c_str());
	else	
		sprintf (sql, "UPDATE t41_authen_apply SET accept = '%d',update_dt = '%s' WHERE BUSID = '%s' AND user_id = '%s' AND BUSTYPE = '1'", (accept==0)?2:1, gmt0now,shipid.c_str(), uid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);
}


int GpsSvc::updShipAlertInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[GpsSvc::updShipAlertInfo]bad format:", jsonString, 1);

	string shipid = root.getv("shipid", "");
	string alertname = root.getv("alertname", "");
	long alerttime = root.getv("alerttm", 0);
	float curvalue =atof(root.getv("value", ""));
	float th0 =atof(root.getv("th0", ""));
	float th1 =atof(root.getv("th1", ""));


	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "INSERT INTO t41_seipc_alert (SHIP_ID, ALERT_TIME, ALERT_NAME, VALUE, THRESHOLD0, THRESHOLD1)	\
				  VALUES ('%s', %d, '%s', '%.1f', '%.1f', '%.1f')", shipid.c_str(),alerttime, alertname.c_str(),curvalue,th0,th1);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	RELEASE_MYSQL_RETURN(psql, 0);

   
}
int GpsSvc::getShipAlertList(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[GpsSvc::getShipAlertList]bad format:", jsonString, 1);
	
	string shipid = root.getv("shipid", "");
	long startdt = root.getv("startdt", 0);
	long enddt = root.getv("enddt", 0);

	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	sprintf (sql, "SELECT ALERT_TIME, ALERT_NAME, VALUE, THRESHOLD0, THRESHOLD1 FROM t41_seipc_alert WHERE SHIP_ID = '%s' and ALERT_TIME>=%d and ALERT_TIME<=%d ORDER BY ALERT_TIME desc ", shipid.c_str(),startdt,enddt);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int index=0;

	long alerttime=0;
	char alertname[128]="";
	float alertvalue=0.0;
    float thr0=0.0;
	float thr1=0.0;

	out <<"[";
	while (psql->NextRow())
	{
		
		if (index>0)
		{
			out <<",";
		}

		READMYSQL_STR(ALERT_NAME, alertname);
		READMYSQL_INT(ALERT_TIME, alerttime, 0);
		READMYSQL_DOUBLE(VALUE, alertvalue, 0.0);
		READMYSQL_DOUBLE(THRESHOLD0, thr0, 0.0);
		READMYSQL_DOUBLE(THRESHOLD1, thr1, 0.0);

		out << "{alertname:\"" << alertname << "\",alerttm:"<<alerttime<<",alertval:\""<<alertvalue<<"\",th0:\""<<thr0<<"\",th1:\""<<thr1<<"\"}";

		index++;
	}

	out <<"]";
	
	RELEASE_MYSQL_RETURN(psql, 0);
}

int GpsSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	SYSTEM_LOG("[GpsSvc::handle_timeout] re loadSeipcUser begin ============= ");
	loadSeipcUser();
	SYSTEM_LOG("[GpsSvc::handle_timeout] re loadSeipcUser end ============= ");

	return 0;
}
