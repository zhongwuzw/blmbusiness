#include "AisSvc.h"
#include "blmcom_head.h"
#include "UserRegSvc.h"
#include "VesselSvc.h"
#include "MessageService.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "SmsMail.h"
#include "Util.h"
#include "NotifyService.h"
#include "AisSvc.h"
#include <math.h>
#include "PortSvc.h"
#include "SmsMailSvc.h"
#include "Geom.h"
#include "Util.h"

//根据MMSI列表获取对应的AIS明细信息
AisSvc::AisSvc() : m_LastUpdate(0)
{
}

AisSvc::~AisSvc()
{
}

bool AisSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_AIS, this))
        return false;

	if(g_MainConfig::instance()->GetFamilyNotify())
	{
		if(!loadFamilyUser())
			return false;

		int interval = g_MainConfig::instance()->GetFamilyInterval();
		if(interval == 0) // 定点发，否则就是间隔发
			interval = 60;
		int timerId = g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(interval), ACE_Time_Value(interval));
		if(timerId <= 0)
			return false;

		checkConnect();
	}

    SERVICE_MAP(SID_AISDETAILSBYMMSILIST,AisSvc,GetDetailOfMmsList);
	SERVICE_MAP(SID_AIS_UPD_USERCONSHIP,AisSvc,updConShip);
	SERVICE_MAP(SID_AIS_UPD_USERTELNO,AisSvc,updUserTelno);
	SERVICE_MAP(SID_AIS_GET_TELNOANDBAL,AisSvc,getTelnoandBal);
	SERVICE_MAP(SID_AIS_GET_SHIPINFO,AisSvc,getShipInfo);
	SERVICE_MAP(SID_MODIFY_DEST,AisSvc,modifyDest);

	DEBUG_LOG("[AisSvc::Start] OK......................................");
    return true;
}

bool AisSvc::loadFamilyUser(MySql* psql, const char* sql, MapFamilyUser& familyUserMap)
{
	CHECK_MYSQL_STATUS(psql->Query(sql), false);

	while (psql->NextRow())
	{
		int tt;
		_FamilyUserInfo userInfo;
		char telno[1024];
		READMYSQL_STR(user_id, userInfo.userid);
		READMYSQL_STR(telno, telno);	
		Tokens telnos = StrSplit(telno, "|");
		for (int i = 0; i < (int)telnos.size(); i++)
		{
			userInfo.telnos.push_back(telnos[i]);
		}
		//去重
		sort(userInfo.telnos.begin(), userInfo.telnos.end());
		userInfo.telnos.erase(unique( userInfo.telnos.begin(), userInfo.telnos.end() ), userInfo.telnos.end());

		READMYSQL_STR(email, userInfo.email);
		READMYSQL_INT(usertype, userInfo.usertype, -1);
		READMYSQL_STR(shipid, userInfo.shipid);
		READMYSQL_INT(mmsi, userInfo.mmsi, 0);
		READMYSQL_INT(last_evt, userInfo.last_evt, 0);
		READMYSQL_INT(tt, tt, 0);
		if(userInfo.mmsi==0)
			continue;
		if(tt > m_LastUpdate)
			m_LastUpdate = tt;

		familyUserMap.insert(MapFamilyUser::value_type(userInfo.mmsi, userInfo));
	}
	return true;
}

bool AisSvc::loadFamilyUser()
{
	SYSTEM_LOG("[AisSvc::loadFamilyUser] begin ==========================================");

	MySql* psql = CREATE_MYSQL;
	char sql[1024];
	MapFamilyUser familyUserMap;

	sprintf(sql, "SELECT t1.user_id, t1.shipid, t1.mmsi, t1.last_evt, t2.email, t2.usertype, UNIX_TIMESTAMP(t1.last_upd_dt) AS tt, t3.telno \
					FROM t00_user_conship AS t1 INNER JOIN t00_user AS t2 ON t1.user_id = t2.user_id\
					LEFT JOIN t00_user_telno AS t3 ON t1.user_id = t3.user_id LEFT JOIN t02_product_balance t4 ON t1.user_id = t4.user_id\
					WHERE t1.is_flag = '1' AND t3.telno IS NOT NULL AND t3.telno != '' AND t4.BALANCE > 0 AND PRODUCT_ID IN ('BLMFAMILY_GLOBAL', 'BLMFAMILY_TRADE')");
	if (!loadFamilyUser(psql, sql, familyUserMap))
		RELEASE_MYSQL_RETURN(psql, false);

	sprintf(sql, "SELECT t1.user_id, t1.shipid, t1.mmsi, t1.last_evt, t2.email, t2.usertype, UNIX_TIMESTAMP(t1.last_upd_dt) AS tt, t3.telno \
					FROM t00_user_conship AS t1 INNER JOIN t00_user AS t2 ON t1.user_id = t2.user_id\
					LEFT JOIN t00_user_telno AS t3 ON t1.user_id = t3.user_id \
					WHERE t1.is_flag = '1' AND t3.telno IS NOT NULL AND t3.telno != '' AND t2.usertype = '22'");
	if (!loadFamilyUser(psql, sql, familyUserMap))
		RELEASE_MYSQL_RETURN(psql, false);

	sprintf(sql, "SELECT t1.user_id, t1.shipid, t1.mmsi, t1.last_evt, t2.email, t2.usertype, UNIX_TIMESTAMP(t1.last_upd_dt) AS tt, t3.telno \
					FROM t00_user_conship AS t1 INNER JOIN t00_user AS t2 ON t1.user_id = t2.user_id\
					LEFT JOIN t00_user_telno AS t3 ON t1.user_id = t3.user_id \
					WHERE t1.is_flag = '1' AND t3.telno IS NOT NULL AND t3.telno != '' AND t2.usertype IN ('21', '10') AND t2.FAMILY_EXPIRE_FLAG = '0'");
	if (!loadFamilyUser(psql, sql, familyUserMap))
		RELEASE_MYSQL_RETURN(psql, false);
	
	ACE_Guard<ACE_Thread_Mutex> guard(m_Lock);
	m_FamilyUsers.clear();
	m_FamilyUsers = familyUserMap;

	SYSTEM_LOG("[AisSvc::loadFamilyUser] end, count:%d ==========================================", (int)m_FamilyUsers.size());
	RELEASE_MYSQL_RETURN(psql, true);
}

int AisSvc::checkExpired()
{
	MySql* psql = CREATE_MYSQL;

	if(g_MainConfig::instance()->GetFamilyNotify())
	{
		SYSTEM_LOG("[AisSvc::handle_timeout] family expired notification ==============");

		char sql[1024];
		sprintf(sql, "select user_id, mobile, family_first_dt as time from t00_user where usertype in('21','10') and mobile_verified = '1' and family_first_dt is not null and family_expire_flag != 2");
		CHECK_MYSQL_STATUS(psql->Query(sql), 0);

		Tokens expiredUsers;
		while(psql->NextRow())
		{
			const char* pUser = psql->GetField("user_id");
			const char* pTel = psql->GetField("mobile");
			if(!pTel)
				continue;
			int familyFirstTime = atoi(NOTNULL(psql->GetField("family_first_dt")));

			char content[1024] = "";
			string szTel(pTel);
			string iso2;
			if(!g_MobileIllegal(szTel, iso2))
				continue;

			if(iso2 == "CN")
				strcpy(content, "感谢您试用博懋船员亲情版（BLM-Family），您的试用期已过，请您购买后继续使用。 博懋国际");
			else
				strcpy(content, "Thank you for using BLM-Family, your trial period has expired, please continue to use after purchase. From BoLooMo");

			//改成2012-10-30过期
			ACE_Date_Time today;
			if(today.year() >= 2012 && today.month() >= 10 && today.day() >= 30)
			{
				string szUser = "'" + string(pUser) + "'";
				expiredUsers.push_back(szUser);
				CNotifyMsg* pSmsMsg = new CNotifyMsg;
				pSmsMsg->m_szType = "sms";
				pSmsMsg->m_SmsFromUser = "";
				pSmsMsg->m_SmsToUser = pUser;
				pSmsMsg->m_SmsToTel = pTel;
				pSmsMsg->m_SmsMsg = content;
				pSmsMsg->m_SmsType = 0;
				g_NotifyService::instance()->PutMessage(pSmsMsg);
			}
		}

		if(!expiredUsers.empty())
		{
			stringstream ss;
			ss << "update t00_user set family_expire_flag = '2' where user_id in (" << ToStr(expiredUsers, ",") << ")";
			psql->Execute(ss.str().c_str(), 0);
		}

		SYSTEM_LOG("[AisSvc::handle_timeout] end family expired notification, count:%d ==============", (int)expiredUsers.size());
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}

bool AisSvc::checkConnect()
{
    ACE_Guard<ACE_Thread_Mutex> guard(m_ConnectLock);

    ACE_Time_Value tv(0, 500*1000);

    if(!m_bTrackConnected)
    {
        ACE_INET_Addr& addrTrack = g_MainConfig::instance()->GetTrackAddr();
        if(m_Connector.connect(m_TrackPeer, addrTrack, &tv) == 0)
        {
            m_bTrackConnected = true;
        }
    }
    return m_bTrackConnected;
}

IMPLEMENT_SERVICE_MAP(AisSvc)

int AisSvc::GetDetailOfMmsList(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[AisSvc::OnAisMsg]bad format:", jsonString, 1);

    string szMmsi = root.getv("mmsilist", "");
    Tokens mmsis = StrSplit(szMmsi, "|");
    if(mmsis.empty())
        return 1;

	char oneRecord[1024];
	int total = 0;
    out << '[';
	for(int i=0; i<(int)mmsis.size(); i++)
	{
		if(total != 0)
			out << ",";

		ShipData* pShip = g_VesselSvc::instance()->aisShip.FindShip(atoi(szMmsi.c_str()));
		AisRealtime* pAis = g_VesselSvc::instance()->aisShip.FindAis(atoi(szMmsi.c_str()));
		if(!pShip && !pAis)
			continue;
		total++;

		sprintf(oneRecord, "{imo:\"%s\",na:\"%s\",st:\"%d\",dest:\"%s\",mmsi:\"%d\",cs:\"%s\",eta:\"%d\",drau:\"%.2f\",dwt:\"%d\"}",
			pShip?pShip->imo:"", pShip?pShip->name:"", CAisShip::GetType(pShip, pAis), pAis?pAis->dest:"", pShip?pShip->mmsi:0, pShip?pShip->callsign:"", pAis?pAis->eta:0, pShip?pShip->draft:0, pShip?pShip->dwt:0);

		out << oneRecord;
    }
    out << ']';
	return 0;
}

//zhuxj
//{uid:"",shipid:"",mmsi:""}
int AisSvc::updConShip(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AisSvc::updConShip]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	string shipid = root.getv("shipid", "");
	string mmsi = root.getv("mmsi", "");

	if (mmsi.empty())
		return 1;

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	char gmt0now[20];
	GmtNow(gmt0now);

	sprintf (sql, "REPLACE INTO t00_user_conship (user_id, shipid, mmsi, last_evt, is_flag, last_upd_dt) VALUES ('%s', '%s', '%s', '0', '1', '%s')", uid.c_str(), shipid.c_str(), mmsi.c_str(), gmt0now);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

void AisSvc::OnAisEvent(const char* eventString)
{
	if(!checkConnect())
	{
		DEBUG_LOG("[AisSvc::OnAisEvent] checkConnect failed.");
		return;
	}

	Tokens tokens = StrSplit(eventString, "@");
	if(tokens.size() != 10)
		return;

	int packet = atoi(tokens[0].c_str());
	int nav_status = atoi(tokens[1].c_str());
	int eventid = atoi(tokens[2].c_str());
	int mmsi = atoi(tokens[3].c_str());
	int occurtime = atoi(tokens[4].c_str());
	double startlon = atof(tokens[5].c_str());
	double startlat = atof(tokens[6].c_str());
	int srcid = atoi(tokens[9].c_str());

	if(eventid != 0 && eventid != 5)
		return;

	char gmtNow[20];
	GmtNow(gmtNow);

	MapUserFamily users;
	MapFamilyUserRange range = m_FamilyUsers.equal_range(mmsi);
	for(MapFamilyUserIter iter = range.first; iter != range.second; iter++)
	{
		_FamilyUserInfo userInfo = (*iter).second;

		char sql[1024];
		if(eventid == 0)		//动力航行
		{
			if(userInfo.last_evt == 5)
			{
				dieBoy(userInfo, users, 0);
				userInfo.last_evt = eventid;
				sprintf(sql, "update t00_user_conship set last_evt = '%d', last_upd_dt = '%s' where user_id = '%s' and mmsi = '%d'", eventid, gmtNow, userInfo.userid, userInfo.mmsi);
				DB_LOG(sql);
			}
		}
		else if(eventid == 5)	//系泊
		{
			dieBoy(userInfo, users, 5);
			userInfo.last_evt = eventid;
			sprintf(sql, "update t00_user_conship set last_evt = '%d', last_upd_dt = '%s' where user_id = '%s' and mmsi = '%d'", eventid, gmtNow, userInfo.userid, userInfo.mmsi);
			DB_LOG(sql);
		}
	}
	notifyNoMoney(users);
}

//zhuxj
int AisSvc::sendShipsMsg()
{
	SYSTEM_LOG("[AisSvc::sendShipsMsg] begin ==========================================");

	if(!checkConnect())
	{
		SYSTEM_LOG("[AisSvc::sendShipsMsg] checkConnect failed.");
		return -1;
	}

	ACE_Guard<ACE_Thread_Mutex> guard(m_Lock);

	MapUserFamily users;
	MapFamilyUserIter iter;
	for(iter=m_FamilyUsers.begin(); iter!=m_FamilyUsers.end(); iter++)
	{
		_FamilyUserInfo userInfo = (*iter).second;
		dieBoy(userInfo, users);
	}
	notifyNoMoney(users);
	
	SYSTEM_LOG("[AisSvc::sendShipsMsg] end ==========================================");
	return 0;
}

int AisSvc::dieBoy(_FamilyUserInfo userInfo, MapUserFamily& users, int type)
{
	DEBUG_LOG("[AisSvc::dieBoy] type:%d", type);

	// 取船舶实时数据
	ShipData* pShip = g_VesselSvc::instance()->aisShip.FindShip(userInfo.mmsi);
	AisRealtime* pAis = g_VesselSvc::instance()->aisShip.FindAis(userInfo.mmsi);
	if(!pAis)
	{
		SYSTEM_LOG("[AisSvc::dieBoy] not found mmsi:%d, user:%s", userInfo.mmsi, userInfo.userid);
		return -1;
	}

	time_t ais_t = (time_t)pAis->time;
	struct tm *ptr;
	ptr = localtime(&ais_t);
	int ais_hour = ptr->tm_hour;
	int ais_min = ptr->tm_min;
	int ais_mday = ptr->tm_mday;
	int ais_mon = ptr->tm_mon+1;

	time_t now = time(0);
	ptr = localtime(&now);
	int now_hour = ptr->tm_hour;
	int now_min = ptr->tm_min;
	int now_sec = ptr->tm_sec;
	int now_mday = ptr->tm_mday;
	int now_mon = ptr->tm_mon+1;

	// 计算最近港口（国家）距离以及目的港口(国家)
	double dist, degree;
	PortInfo* pNearPort = g_PortSvc::instance()->FindNearestPort(pAis->lon, pAis->lat, dist, degree, true);
	if(!pNearPort)
	{
		SYSTEM_LOG("[AisSvc::dieBoy] not found nearest port, user:%s", userInfo.userid);
		return -1;
	}
	_Country* pNearCountry = g_PortSvc::instance()->m_country.FindCountry(pNearPort->iso3);

	PortInfo* pDestPort = g_PortSvc::instance()->FindPortByID(pAis->toPort);
	_Country* pDestCountry = NULL;
	if(pDestPort)
	{
		pDestCountry = g_PortSvc::instance()->m_country.FindCountry(pDestPort->iso3);
	}

	char msg[2048];
	if (pDestPort) //有目的港
	{		
		double dist_dest = 0.0;
		string retJsonString;
		{
			ACE_Guard<ACE_Thread_Mutex> guard(m_ConnectLock);
			char* pRes;
			char pReq[1024] = "";
			ACE_Time_Value tv(0, 3000*1000);
			sprintf(pReq, "{mmsi:\"%d\",time:\"%d\",x:%f,y:%f,portid:\"%d\",port:\"%s\"}", pAis->mmsi, pAis->time, pAis->lon, pAis->lat, pDestPort->portid, pDestPort->portname.c_str());
			SYSTEM_LOG("[AisSvc::dieBoy] req trackserver:%s", pReq);
			if(!ReqPostServer(m_TrackPeer, 0x0108, pReq, tv, pRes))
			{
				m_TrackPeer.close();
				m_bTrackConnected = false;

				DEBUG_LOG("[AisSvc::dieBoy] request trackserver failed.");
				return 2;
			}
			retJsonString = string(pRes+16);
			delete pRes;
		}
		SYSTEM_LOG("[AisSvc::dieBoy] retJsonString:%s", retJsonString.c_str());

		if(retJsonString.find("eid:0") == string::npos)
		{
			//TODO
		}

		JSON_PARSE_RETURN("[AisSvc::dieBoy]bad format:", retJsonString.c_str(), 1);
		int eta = atoi(root.getv("eta", ""));
		dist_dest = atof(root.getv("distance", ""));
		if(type == -1)
		{
			time_t Eta_Unixtime = (time_t)eta;
			ptr = localtime(&Eta_Unixtime);
			int eta_hour = ptr->tm_hour;
			int eta_min = ptr->tm_min;
			int eta_sec = ptr->tm_sec;

			int diff = (int)(Eta_Unixtime - eta_sec - eta_min * 60 - eta_hour * 3600) - (int)(now - now_sec - now_min * 60 - now_hour * 3600);
			diff /= 86400; 
			if (!diff && Eta_Unixtime < now)
				diff = -1;
			
			char *(date_eta[3]) = {"今日","明日","后天"};
			if (diff < 0)
				sprintf (msg, (ais_mon == now_mon && ais_mday == now_mday)?"%s今日 %02d:%02d 在%s%s附近":"%s昨日 %02d:%02d 在%s%s附近",
				(pShip?pShip->name:""), ais_hour, ais_min, pNearCountry?( CodeConverter::Utf8ToGb2312(pNearCountry->namecn).c_str() ):"", (pNearPort->portnamecn.empty()?pNearPort->portname.c_str():CodeConverter::Utf8ToGb2312(pNearPort->portnamecn.c_str()).c_str()));
			else if (diff <= 2)
				sprintf (msg, (ais_mon == now_mon && ais_mday == now_mday)?"%s今日 %02d:%02d 在%s%s附近，距离%s%s %.3f海里，预计%s %02d:%02d 到达":"%s昨日 %02d:%02d 在%s%s附近，距离%s%s %.3f海里，预计%s %02d:%02d 到达",
				(pShip?pShip->name:""), ais_hour, ais_min, pNearCountry?(CodeConverter::Utf8ToGb2312(pNearCountry->namecn).c_str()):"", (pNearPort->portnamecn.empty()?pNearPort->portname.c_str():CodeConverter::Utf8ToGb2312(pNearPort->portnamecn.c_str()).c_str()), pDestCountry?(CodeConverter::Utf8ToGb2312(pDestCountry->namecn).c_str()):"", (pDestPort->portnamecn.empty()?pDestPort->portname.c_str():CodeConverter::Utf8ToGb2312(pDestPort->portnamecn.c_str()).c_str()), dist_dest, date_eta[diff], eta_hour, eta_min);
			else
				sprintf (msg, (ais_mon == now_mon && ais_mday == now_mday)?"%s今日 %02d:%02d 在%s%s附近，距离%s%s %.3f海里，预计%d 天后到达":"%s昨日 %02d:%02d 在%s%s附近，距离%s%s %.3f海里，预计%d 天后到达",
				(pShip?pShip->name:""), ais_hour, ais_min, pNearCountry?(CodeConverter::Utf8ToGb2312(pNearCountry->namecn).c_str()):"", (pNearPort->portnamecn.empty()?pNearPort->portname.c_str():CodeConverter::Utf8ToGb2312(pNearPort->portnamecn.c_str()).c_str()), pDestCountry?(CodeConverter::Utf8ToGb2312(pDestCountry->namecn).c_str()):"", (pDestPort->portnamecn.empty()?pDestPort->portname.c_str():CodeConverter::Utf8ToGb2312(pDestPort->portnamecn.c_str()).c_str()), dist_dest, diff);
		}
		else if(type == 0)
		{
			sprintf (msg, "%s %02d月%02d日%02d:%02d 离开%s%s，开往%s%s",
				(pShip?pShip->name:""), ais_mon, ais_mday, ais_hour, ais_min, pNearCountry?(CodeConverter::Utf8ToGb2312(pNearCountry->namecn).c_str()):"", (pNearPort->portnamecn.empty()?pNearPort->portname.c_str():CodeConverter::Utf8ToGb2312(pNearPort->portnamecn.c_str()).c_str()), pDestCountry?(CodeConverter::Utf8ToGb2312(pDestCountry->namecn).c_str()):"", (pDestPort->portnamecn.empty()?pDestPort->portname.c_str():CodeConverter::Utf8ToGb2312(pDestPort->portnamecn.c_str()).c_str()));
		}
		else if(type == 5)
		{
			sprintf (msg, "%s %02d月%02d日%02d:%02d 停靠在%s%s",
				(pShip?pShip->name:""), ais_mon, ais_mday, ais_hour, ais_min, pNearCountry?(CodeConverter::Utf8ToGb2312(pNearCountry->namecn).c_str()):"", (pNearPort->portnamecn.empty()?pNearPort->portname.c_str():CodeConverter::Utf8ToGb2312(pNearPort->portnamecn.c_str()).c_str()));
		}
	}
	else
	{
		//无目的港
		if(type == -1)
		{
			sprintf (msg, (ais_mon == now_mon && ais_mday == now_mday)?"%s今日 %02d:%02d 在%s%s附近":"%s昨日 %02d:%02d 在%s%s附近",
				(pShip?pShip->name:""), ais_hour, ais_min, pNearCountry?(CodeConverter::Utf8ToGb2312(pNearCountry->namecn).c_str()):"", (pNearPort->portnamecn.empty()?pNearPort->portname.c_str():CodeConverter::Utf8ToGb2312(pNearPort->portnamecn.c_str()).c_str()));
		}
		else if(type == 0)
		{
			sprintf (msg, "%s %02d月%02d日%02d:%02d 离开%s%s",
				(pShip?pShip->name:""), ais_mon, ais_mday, ais_hour, ais_min, pNearCountry?(CodeConverter::Utf8ToGb2312(pNearCountry->namecn).c_str()):"", (pNearPort->portnamecn.empty()?pNearPort->portname.c_str():CodeConverter::Utf8ToGb2312(pNearPort->portnamecn.c_str()).c_str()));
		}
		else if(type == 5)
		{
			sprintf (msg, "%s %02d月%02d日%02d:%02d 停靠在%s%s",
				(pShip?pShip->name:""), ais_mon, ais_mday, ais_hour, ais_min, pNearCountry?(CodeConverter::Utf8ToGb2312(pNearCountry->namecn).c_str()):"", (pNearPort->portnamecn.empty()?pNearPort->portname.c_str():CodeConverter::Utf8ToGb2312(pNearPort->portnamecn.c_str()).c_str()));
		}		
	}
	string smsMsg = SmsMsg(msg);
	char jsonString[1024 * 2] = "";
	std::stringstream out;
	sprintf (jsonString, "{id:\"blm_system\",to:\"%s\",cc:\"\",subject:\"%s\",msg:\"%s\",type:\"0\"}", userInfo.email, CodeConverter::Gb2312ToUtf8(smsMsg.c_str()).c_str(), CodeConverter::Gb2312ToUtf8(smsMsg.c_str()).c_str());
	g_SmsMailSvc::instance()->sendMail("blm_system", jsonString, out);

	if(userInfo.telnos.empty())
	{
		SYSTEM_LOG("[AisSvc::dieBoy] empty telno");
		return 0;
	}

	for (Tokens::iterator vecit = userInfo.telnos.begin(); vecit != userInfo.telnos.end(); vecit++)
	{
		out.str("");
		SYSTEM_LOG("[AisSvc::dieBoy] test =============== userid:%s, telno:%s", userInfo.userid, (*vecit).c_str());
		std::string jsonString = FormatString("{id:\"%s\",tel:\"%s\",msg:\"%s %s\"}", userInfo.userid, (*vecit).c_str(), CodeConverter::Gb2312ToUtf8(smsMsg.c_str()).c_str(), CodeConverter::Gb2312ToUtf8("来自 博懋国际").c_str());
		SYSTEM_LOG("[AisSvc::dieBoy] json:%s", jsonString.c_str());

		int t_flg = g_SmsMailSvc::instance()->sendSms("blm_system", jsonString.c_str(), out);
		SYSTEM_LOG("[AisSvc::dieBoy] t_flg:%d", t_flg);
		string tout = out.str();
		if(tout.length() > 7 && tout[5] == '5')
		{
			users.insert(make_pair(string(userInfo.userid),userInfo));
		}
	}
	
	SYSTEM_LOG("[AisSvc::dieBoy] send msg ok:%s, telnos:%s", userInfo.userid, ToStr(userInfo.telnos).c_str());
	return 0;
}

int AisSvc::notifyNoMoney(MapUserFamily& users)
{
	if(users.empty())
		return 0;

	MySql* psql = CREATE_MYSQL;

	char gmtNow[20];
	GmtNow(gmtNow);

	char sql[20*1024];
	int len = sprintf (sql, "UPDATE t00_user_conship SET is_flag = '0', last_upd_dt = '%s' WHERE user_id IN (", gmtNow);
	for (MapUserFamilyIter iTer = users.begin(); iTer != users.end(); iTer++)
	{
		int tl = (*iTer).first.length();

		char content[1024] = "";
		if ((*iTer).second.usertype == 9 || (*iTer).second.usertype == 8)
		{
			sprintf (content, "短信服务余额不足(from BLM_Shipping)");
		}
		else
		{
			//改成2012-10-12过期
			ACE_Date_Time today;
			if(today.year() >= 2012 && today.month() >= 10 && today.day() >= 12)
			{
				sprintf (content, "短信服务余额不足,您的亲情版试用期已到期，购买亲情版服务即可免费获赠短信(from BLM_Shipping)");
			}
			else
			{
				sprintf (content, "短信服务余额不足,您的亲情版试用期到2012-10-12，购买亲情版服务即可免费获赠短信(from BLM_Shipping)");
			}
		}
		
		for(int i=0; i<(int)(*iTer).second.telnos.size(); i++)
		{
			CNotifyMsg* pSmsMsg = new CNotifyMsg;
			pSmsMsg->m_szType = "sms";
			pSmsMsg->m_SmsFromUser = "";
			pSmsMsg->m_SmsToUser = (*iTer).first;
			pSmsMsg->m_SmsToTel = (*iTer).second.telnos[i];
			pSmsMsg->m_SmsMsg = content;
			pSmsMsg->m_SmsType = 0;
			g_NotifyService::instance()->PutMessage(pSmsMsg);
		}
		
		len += sprintf (sql + len, (iTer == users.begin())?"'%s'":",'%s'", (*iTer).first.c_str());
	}
	sprintf (sql + len, ")");
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, -1);

	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
int AisSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	SYSTEM_LOG("[AisSvc::handle_timeout] begin ================");
	
	checkExpired();
	loadFamilyUser();

	time_t t = time(NULL);
	struct tm *ptr = localtime(&t);

	if(g_MainConfig::instance()->GetFamilyInterval() != 0)
	{
		 //间隔发
		g_AisSvc::instance()->sendShipsMsg();
	}
	else if ((ptr->tm_wday > 0 && ptr->tm_wday < 6 && ptr->tm_hour == 8 && ptr->tm_min == 0) || ((ptr->tm_wday == 0 || ptr->tm_wday == 6) && ptr->tm_hour == 9 && ptr->tm_min == 0) || (ptr->tm_hour == 20 && ptr->tm_min == 0))
	{
		 //定点发
		g_AisSvc::instance()->sendShipsMsg();	
	}
	
	SYSTEM_LOG("[AisSvc::handle_timeout] end ==============");
	return 0;
}

//zhuxj
//{uid:"",telno:""}
int AisSvc::updUserTelno(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AisSvc::updUserTelno]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	string telno = root.getv("telno", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	
	sprintf (sql, "REPLACE INTO t00_user_telno (user_id, telno) VALUES ('%s', '%s')", uid.c_str(), telno.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	
	out << "{eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:""}
int AisSvc::getTelnoandBal(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AisSvc::getTelnoandBal]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";

	int local_bal = 0;
	int inter_bal = 0;
	char telno[1024] = {0};

	sprintf (sql, "SELECT t1.telno, t2.local_bal, t2.inter_bal FROM t00_user_telno AS t1 LEFT JOIN t02_user_sms_bal AS t2 ON t1.user_id = t2.user_id WHERE t1.user_id = '%s'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		READMYSQL_STR(telno, telno);
		READMYSQL_INT(local_bal, local_bal, 0);
		READMYSQL_INT(inter_bal, inter_bal, 0);
	}
	
	out << FormatString("{telno:\"%s\",bal:%d}", telno, (local_bal+inter_bal));
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{uid:""}
int AisSvc::getShipInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AisSvc::getShipInfo]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	
	char sid[32] = "";
	int mmsi = 0;
	char name[400] = "";

	sprintf (sql, "SELECT t1.shipid, t1.mmsi, t2.name FROM t00_user_conship AS t1 \
					INNER JOIN t41_ship AS t2 ON t1.shipid = t2.shipid WHERE t1.user_id = '%s'", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		READMYSQL_STR(shipid, sid);
		READMYSQL_INT(mmsi, mmsi, 0);
		READMYSQL_STR(name, name);
	}
	
	out << "{sid:\"" << sid << "\",mmsi:" << mmsi << ",name:\"" << name <<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//7@mmsi@modifydest@srcdest@user@lastupddt
//{uid:"",modidest:"",srcdest:"",mmsi:""}
int AisSvc::modifyDest(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[AisSvc::modifyDest]bad format:", jsonString, 1);
	string uid = root.getv("uid", "");
	string modidest = root.getv("modidest", "");
	string srcdest = root.getv("srcdest", "");
	string mmsi = root.getv("mmsi", "");

	MySql *psql = CREATE_MYSQL;	
	char sql[1024] = "";

	sprintf (sql, "SELECT COUNT(1) AS num FROM t00_user WHERE user_id = '%s' AND (usertype = 8 OR usertype = '9')", uid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	int num = 0;
	if (psql->NextRow())
		READMYSQL_INT(num, num, 0);
	if (num == 0)
		RELEASE_MYSQL_RETURN(psql, 1);

	char dest[1024] = "";
	sprintf(dest, "7@%s@%s@%s@%s@%d", mmsi.c_str(), modidest.c_str(), srcdest.c_str(), uid.c_str(), (int)time(NULL));
	size_t destLen = strlen(dest) + 1;

	ACE_INET_Addr ais_udp_addr(10010);
	ACE_INET_Addr& addrTrack_udp = g_MainConfig::instance()->GetTrackAddr_udp();
	ACE_SOCK_Dgram	ais_udp_endpoint(ais_udp_addr);
	ais_udp_endpoint.enable(ACE_NONBLOCK);

	size_t nSend = ais_udp_endpoint.send(dest, destLen, addrTrack_udp);
	ais_udp_endpoint.close();
	if(nSend == -1)
		out << "{eid:0,flag:0}";
	else
		out << "{eid:0,flag:1}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
