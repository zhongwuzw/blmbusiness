#include "blmcom_head.h"
#include "BDSvc.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "kSQL.h"
#include "ObjectPool.h"
#include "MainConfig.h"
#include "Util.h"
#include "xbase64.h"
#include "json.h"

//#define BD_IP "10.134.2.112"
#define BD_IP "122.227.170.98"

IMPLEMENT_SERVICE_MAP(BDSvc)

BDSvc::BDSvc()
{

}

BDSvc::~BDSvc()
{

}

bool BDSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_BEIDOU, this))
        return false;

	int seconds = 5;
	g_TimerManager::instance()->schedule(this, (void* )NULL, ACE_OS::gettimeofday() + ACE_Time_Value(seconds), ACE_Time_Value(seconds));

    SERVICE_MAP(SID_BD_ZHJ_GROUP,BDSvc,zhjGroup);
    SERVICE_MAP(SID_BD_CARD_DETAIL,BDSvc,cardDetail);
	SERVICE_MAP(SID_BD_ALL_YHJ,BDSvc,allYhj);
	SERVICE_MAP(SID_BD_ALL_POS,BDSvc,allPos);
	SERVICE_MAP(SID_BD_POSITION,BDSvc,position);
	SERVICE_MAP(SID_BD_TRACK,BDSvc,track);
	SERVICE_MAP(SID_BD_TX,BDSvc,tx);
	SERVICE_MAP(SID_BD_TXHZ,BDSvc,txhz);
	SERVICE_MAP(SID_BD_SEARCH,BDSvc,search);
	SERVICE_MAP(SID_BD_REMARK,BDSvc,remark);
	SERVICE_MAP(SID_BD_P2P,BDSvc,p2p);
	SERVICE_MAP(SID_BD_BROADCAST,BDSvc,broadcast);
	SERVICE_MAP(SID_BD_ALL_YJBJ,BDSvc,allYjbj);
	SERVICE_MAP(SID_BD_SMS,BDSvc,recvSms);

    DEBUG_LOG("[BDSvc::Start] OK......................................");
    return true;
}

int BDSvc::handle_timeout(const ACE_Time_Value &tv, const void *arg)
{
	return 0;
}

bool BDSvc::checkConnect(ACE_SOCK_Connector& m_Connector, ACE_SOCK_Stream& m_Peer)
{
	ACE_Time_Value tv(0, 2000*1000);

	ACE_INET_Addr addrPeer = ACE_INET_Addr(6005, BD_IP);
	if(m_Connector.connect(m_Peer, addrPeer, &tv) == 0) {
		DEBUG_LOG("[BDSvc::checkConnect] SUCC connected to BeiDou Peer");
		return true;
	}

	DEBUG_LOG("[BDSvc::checkConnect] FAILED connected to BeiDou Peer");
	return false;
}

int BDSvc::zhjGroup(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BDSvc::zhjGroup]bad format:", jsonString, 1);
	int seq = root.getv("seq", 0);
	std::string szUserID = root.getv("userid", "");

    MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query("SELECT zhjaddr, GROUP_CONCAT(yhjaddr) AS yhjaddrs FROM t_bd_zhj_yhj GROUP BY zhjaddr"), 3);
    
	bool bFirst = true;
	out << FormatString("{seq:%d,results:[", seq);
	while(psql->NextRow())
	{
		if(bFirst)
			bFirst = false;
		else
			out << ",";
		out << FormatString("{zhjAddr:%s,yhjAddrs:[", psql->GetField("zhjaddr"));
		out << psql->GetField("yhjaddrs");
		out << "]}";
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::cardDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BDSvc::cardDetail]bad format:", jsonString, 1);
	int seq = root.getv("seq", 0);
	int addr = root.getv("addr", 0);
	if(addr <= 0) return 1;

	char sql[1024];
	sprintf(sql, "select addr,baddr,type,frequency,commlevel,cnfdflag,usersnum from t_bd_ic where addr='%d'", addr);
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
	
	out << FormatString("{seq:%d,addr:%s,brdcstAddr:%s,type:%s,frequency:%s,level:%s,flag:%s,usersNum:%s}",
						seq, psql->GetField("addr"),
							psql->GetField("baddr"),
							psql->GetField("type"),
							psql->GetField("frequency"),
							psql->GetField("commlevel"),
							psql->GetField("cnfdflag"),
							psql->GetField("usersnum"));

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::allYhj(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BDSvc::allYhj]bad format:", jsonString, 1);
	int seq = root.getv("seq", 0);
	int zhjAddr = root.getv("zhjAddr", 0);
	if(zhjAddr <= 0) return 1;

	IntVector yhjList;

	char sql[1024];
	sprintf(sql, "SELECT yhjaddr FROM t_bd_zhj_yhj WHERE zhjaddr = %d", zhjAddr);
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow()) {
		yhjList.push_back(atoi(psql->GetField("yhjaddr")));
	}
	yhjList.push_back(zhjAddr); // 把指挥卡信息也放进去

	bool bFirst = true;
	out << FormatString("{seq:%d,zhjAddr:%d,yhjAddrs:[", seq, zhjAddr);
	for(int i=0; i<(int)yhjList.size(); i++)
	{
		if(bFirst)
			bFirst = false;
		else
			out << ",";
		sprintf(sql, "SELECT t1.addr,t1.baddr,t1.type,t1.frequency,t1.commlevel,t1.cnfdflag,t1.usersnum from t_bd_ic t1 where addr=%d", yhjList[i]);
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if(!psql->NextRow()) {
			out << FormatString("{addr:%d,brdcstAddr:0,type:1,frequency:0,level:0,flag:0,usersNum:0}", yhjList[i]);
		} else {
			out << FormatString("{addr:%s,brdcstAddr:%s,type:%s,frequency:%s,level:%s,flag:%s,usersNum:%s}",
				psql->GetField("addr"),
				psql->GetField("baddr"),
				psql->GetField("type"),
				psql->GetField("frequency"),
				psql->GetField("commlevel"),
				psql->GetField("cnfdflag"),
				psql->GetField("usersnum"));
		}
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::allPos(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BDSvc::allPos]bad format:", jsonString, 1);
	int seq = root.getv("seq", 0);
	int zhjAddr = root.getv("zhjAddr", 0);
	if(zhjAddr <= 0) return 1;

	char sql[1024];
	sprintf(sql, "SELECT t1.addr,t1.time,t1.longitude,t1.latitude,t1.course,t1.speed FROM t_bd_position t1, t_bd_zhj_yhj t2 WHERE t2.ZhjAddr = '%d' AND t1.Addr = t2.YhjAddr", zhjAddr);
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	bool bFirst = true;
	out << FormatString("{seq:%d,zhjAddr:%d,positions:[", seq, zhjAddr);
	while(psql->NextRow())
	{
		if(bFirst)
			bFirst = false;
		else
			out << ",";
		out << FormatString("{addr:%s,time:%s,longitude:%.6f,latitude:%.6f,course:%.6f,speed:%.6f}",
			psql->GetField("addr"),
			psql->GetField("time"),
			atof(psql->GetField("longitude")),
			atof(psql->GetField("latitude")),
			atof(NOTNULL(psql->GetField("course"))),
			atof(NOTNULL(psql->GetField("speed"))));
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::position(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BDSvc::position]bad format:", jsonString, 1);
	int seq = root.getv("seq", 0);
	Json* jAddrs = root.get("addrs");

	Tokens addrList;
	for(int i=0; i<jAddrs->size(); i++) {
		addrList.push_back(IntToStr(jAddrs->getv(i,0)));
	}
	string szAddrs = ToStr(addrList, ",");
	if(szAddrs.empty())
		return 1;

	char sql[1024];
	sprintf(sql, "SELECT t1.addr,t1.time,t1.longitude,t1.latitude,t1.course,t1.speed FROM t_bd_position t1 WHERE addr IN (%s)", szAddrs.c_str());
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	bool bFirst = true;
	out << FormatString("{seq:%d,addrs:[", seq);
	while(psql->NextRow())
	{
		if(bFirst)
			bFirst = false;
		else
			out << ",";
		out << FormatString("{addr:%s,time:%s,longitude:%.6f,latitude:%.6f,course:%.6f,speed:%.6f}",
			psql->GetField("addr"),
			psql->GetField("time"),
			atof(psql->GetField("longitude")),
			atof(psql->GetField("latitude")),
			atof(NOTNULL(psql->GetField("course"))),
			atof(NOTNULL(psql->GetField("speed"))));
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

typedef struct _STRUCT_POS {
	int time;
	double longitude;
	double latitude;
	double course;
	double speed;
} STRUCT_POS;
typedef vector<STRUCT_POS> PosList;

int BDSvc::track(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BDSvc::track]bad format:", jsonString, 1);
	int seq = root.getv("seq", 0);
	Json* jAddrs = root.get("addrs");
	int start = root.getv("start", 0);
	int end = root.getv("end", 0);
	if(end == 0) end = (int)time(NULL);

	Tokens addrList;
	for(int i=0; i<jAddrs->size(); i++) {
		addrList.push_back(IntToStr(jAddrs->getv(i,0)));
	}
	string szAddrs = ToStr(addrList, ",");
	if(szAddrs.empty())
		return 1;

	char sql[1024];
	sprintf(sql, "SELECT addr,TIME,longitude,latitude,course,speed FROM t_bd_position_history WHERE addr IN (%s) AND TIME BETWEEN %d AND %d ORDER BY addr,time", szAddrs.c_str(), start, end);
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	map<int, PosList> addrPosMap;
	map<int, PosList>::iterator iter;
	while(psql->NextRow())
	{
		STRUCT_POS pos;
		pos.time = atoi(psql->GetField("time"));
		pos.longitude = atof(psql->GetField("longitude"));
		pos.latitude = atof(psql->GetField("latitude"));
		pos.course = atof(NOTNULL(psql->GetField("course")));
		pos.speed = atof(NOTNULL(psql->GetField("speed")));

		int addr = atoi(psql->GetField("addr"));
		if((iter=addrPosMap.find(addr)) == addrPosMap.end()) {
			PosList posList;
			posList.push_back(pos);
			addrPosMap[addr] = posList;
		} else {
			iter->second.push_back(pos);
		}
	}

	bool bFirst = true;
	out << FormatString("{seq:%d,tracks:[", seq);
	for(iter=addrPosMap.begin(); iter!=addrPosMap.end(); iter++)
	{
		if(bFirst)
			bFirst = false;
		else
			out << ",";
		out << FormatString("{addr:%d,positions:[", iter->first);
		for(int i=0; i<(int)iter->second.size(); i++) {
			if(i!=0) out << ",";
			out << FormatString("{time:%d,longitude:%.6f,latitude:%.6f,course:%.6f,speed:%.6f}", 
							iter->second[i].time,
							iter->second[i].longitude,
							iter->second[i].latitude,
							iter->second[i].course,
							iter->second[i].speed);
		}
		out << "]}";
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::tx(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BDSvc::track]bad format:", jsonString, 1);
	int seq = root.getv("seq", 0);
	Json* jAddrs = root.get("addrs");
	Json* jMsgTypes = root.get("msg_types");
	int start = root.getv("start", 0);
	int end = root.getv("end", 0);
	if(end == 0) end = (int)time(NULL);

	Tokens addrList;
	for(int i=0; i<jAddrs->size(); i++) {
		addrList.push_back(IntToStr(jAddrs->getv(i,0)));
	}
	string szAddrs = ToStr(addrList, ",");
	if(szAddrs.empty())
		return 1;

	Tokens typeList;
	for(int i=0; i<jMsgTypes->size(); i++) {
		typeList.push_back(IntToStr(jMsgTypes->getv(i,0)));
	}
	string szTypes = ToStr(typeList, ",");

	char sql[1024];
	if(!szTypes.empty())
		sprintf(sql, "SELECT addr,time,postaddr,posttime,infotype,msgtype,msgcode,message,bjflag,succ FROM t_bd_message WHERE (addr IN (%s) OR postaddr IN (%s)) AND msgType IN (%s) AND TIME BETWEEN %d AND %d ORDER BY addr,time", szAddrs.c_str(), szAddrs.c_str(), szTypes.c_str(), start, end);
	else
		sprintf(sql, "SELECT addr,time,postaddr,posttime,infotype,msgtype,msgcode,message,bjflag,succ FROM t_bd_message WHERE (addr IN (%s) OR postaddr IN (%s)) AND TIME BETWEEN %d AND %d ORDER BY addr,time", szAddrs.c_str(), szAddrs.c_str(), start, end);
	
	DEBUG_LOG("[BDSvc::tx]%s", sql);

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	bool bFirst = true;
	out << FormatString("{seq:%d,messages:[", seq);
	while(psql->NextRow())
	{
		if(bFirst)
			bFirst = false;
		else
			out << ",";
		out << FormatString("{addr:%s,time:%s,postAddr:%s,postTime:%s,infoType:%s,msgType:%s,msgCode:%s,msg:\"%s\",flag:%d,succ:%s}",
			psql->GetField("addr"),
			psql->GetField("time"),
			psql->GetField("postaddr"),
			psql->GetField("posttime"),
			psql->GetField("infotype"),
			psql->GetField("msgtype"),
			psql->GetField("msgcode"),
			psql->GetField("message"),
			atoi(NOTNULL(psql->GetField("bjflag"))),
			psql->GetField("succ"));
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::txhz(const char* pUid, const char* jsonString, std::stringstream& out)
{
	return 2;
}

int BDSvc::search(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BDSvc::search]bad format:", jsonString, 1);
	int seq = root.getv("seq", 0);
	const char* szText = root.getv("text", "");
	if(strlen(szText) == 0)
		return 1;

	char sql[1024];
	sprintf(sql, "SELECT addr,type FROM t_bd_ic WHERE addr LIKE '%s%%' UNION SELECT yhjAddr as addr, '1' as type FROM t_bd_zhj_yhj WHERE yhjAddr LIKE '%s%%'", szText, szText);
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	bool bFirst = true;
	out << FormatString("{seq:%d,results:[", seq);
	while(psql->NextRow())
	{
		if(bFirst)
			bFirst = false;
		else
			out << ",";
		out << FormatString("{addr:%s,type:%s}",
			psql->GetField("addr"),
			psql->GetField("type"));
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::remark(const char* pUid, const char* jsonString, std::stringstream& out)
{
	return 2;
}

void writeSql(const char* sql)
{
	MySql* psql = CREATE_MYSQL;
	psql->Execute(sql);
	RELEASE_MYSQL_NORETURN(psql);
}

int BDSvc::p2p(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BDSvc::p2p]bad format:", jsonString, 1);
	int seq = root.getv("seq", 0);
	Json* jAddrs = root.get("addrs");
	int infoType = root.getv("infoType", 0);
	int msgType = root.getv("msgType", 0);
	int msgCode = root.getv("msgCode", 0);
	string msg = root.getv("msg", "");
	msg = CodeConverter::Utf8ToGb2312(msg.c_str());

	IntVector addrList;
	for(int i=0; i<jAddrs->size(); i++) {
		int addr = jAddrs->getv(i, 0);
		if(addr != 0)
			addrList.push_back(addr);
	}
	if(addrList.empty())
		return 1;

	time_t now = time(NULL);
	char failedSql[1024*10];
	char succSql[1024*10];
	sprintf(failedSql, "insert into t_bd_message(addr,time,postaddr,posttime,infotype,msgtype,msgcode,message,crc,succ) values \
				 (%d,%d,%d,%d,%d,%d,%d,'%s',0,0)",
				 addrList[0],
				 (int)now,
				 root.getv("postAddr",0),
				 (int)now,
				 root.getv("infoType",0),
				 root.getv("msgType",0),
				 root.getv("msgCode",0),
				 root.getv("msg",""));

	ACE_SOCK_Connector m_Connector;
	ACE_SOCK_Stream m_Peer;
	if(!checkConnect(m_Connector, m_Peer)) {
		out << FormatString("{seq:%d,eid:1,msg:\"not connect to bd\"}", seq);
		writeSql(failedSql);
		return 0;
	}

	int nLen = 0;
	ACE_Time_Value tv(10);
	for(int i=0; i<(int)addrList.size(); i++) {
		char buff[1024*10];
		sprintf(buff, "{seq:%d,cmd:1,info_type:%d,msg_code:%d,msg_type:%d,addr:%d,msg:\"%s\"}##",seq,infoType,msgCode,msgType,addrList[i], msg.c_str());
		DEBUG_LOG("[BDSvc::p2p]========= %s", buff);

		// 屏蔽掉这两个失效的协议
		if(msgType != 9 && msgType != 11) {
			nLen = m_Peer.send(buff, strlen(buff), &tv);
			if(nLen<=0) {
				DEBUG_LOG("[BDSvc::p2p] ============================== send failed");
				m_Peer.close();
				writeSql(failedSql);
				return 2;
			}
			memset(buff, 0, sizeof(buff));
			nLen = m_Peer.recv(buff, sizeof(buff), &tv);
			if(nLen<=0) {
				DEBUG_LOG("[BDSvc::p2p] ============================== recv failed");
				m_Peer.close();
				writeSql(failedSql);
				return 2;
			}
			if(strstr(buff, ",succ:0")) {
				out << FormatString("{seq:%d,eid:1,msg:\"p2p send failed\"}", seq);
				m_Peer.close();
				writeSql(failedSql);
				return 0;
			}

			sprintf(succSql, "insert into t_bd_message(addr,time,postaddr,posttime,infotype,msgtype,msgcode,message,crc,succ) values \
							 (%d,%d,%d,%d,%d,%d,%d,'%s',0,1)",
							 addrList[i],
							 (int)now,
							 root.getv("postAddr",0),
							 (int)now,
							 root.getv("infoType",0),
							 root.getv("msgType",0),
							 root.getv("msgCode",0),
							 root.getv("msg",""));
			writeSql(succSql);
		}
		else {
			sprintf(buff, "update t_bd_message set bjflag=%d where postaddr=%d and bjid=%s", (msgType==9?1:3), addrList[i], msg.c_str());
			DEBUG_LOG("[BDSvc::p2p] ============================== UPDATE BJ Status:%s", buff);

			writeSql(buff);
		}
	}

	out << FormatString("{seq:%d,eid:0,msg:\"succ\"}", seq);
	m_Peer.close();
	return 0;
}

int BDSvc::broadcast(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BDSvc::broadcast]bad format:", jsonString, 1);
	int seq = root.getv("seq", 0);
	int zhjAddr = root.getv("zhjAddr", 0);
	int infoType = root.getv("infoType", 0);
	int msgType = root.getv("msgType", 0);
	int msgCode = root.getv("msgCode", 0);
	string msg = root.getv("msg", "");
	msg = CodeConverter::Utf8ToGb2312(msg.c_str());

	if(zhjAddr <= 0 || msg.empty())
		return 1;

	ACE_SOCK_Connector m_Connector;
	ACE_SOCK_Stream m_Peer;
	if(!checkConnect(m_Connector, m_Peer)) {
		out << FormatString("{seq:%d,eid:1,msg:\"not connect to bd\"}", seq);
		return 0;
	}

	int nLen = 0;
	ACE_Time_Value tv(10);
	char buff[1024*10];
	sprintf(buff, "{seq:%d,cmd:2,info_type:%d,msg_code:%d,msg_type:%d,msg:\"%s\"}##",seq,infoType,msgCode,msgType,msg.c_str());
	DEBUG_LOG("[BDSvc::broadcast]========= %s", buff);

	nLen = m_Peer.send(buff, strlen(buff), &tv);
	if(nLen<=0) {
		DEBUG_LOG("[BDSvc::broadcast] ============================== send failed");
		m_Peer.close();
		return 2;
	}
	memset(buff, 0, sizeof(buff));
	nLen = m_Peer.recv(buff, sizeof(buff), &tv);
	if(nLen<=0) {
		DEBUG_LOG("[BDSvc::broadcast] ============================== recv failed");
		m_Peer.close();
		return 2;
	}

	if(strstr(buff, ",succ:0")) {
		out << FormatString("{seq:%d,eid:1,msg:\"broadcast send failed\"}", seq);
		m_Peer.close();
		return 0;
	}

	out << FormatString("{seq:%d,eid:0,msg:\"succ\"}", seq);
	m_Peer.close();
	return 0;
}

typedef struct _STRUCT_MSG {
	int addr;
	int time;
	int infoType;
	int msgType;
	int msgCode;
	string msg;
} STRUCT_MSG;
typedef vector<STRUCT_MSG> MsgList;

int BDSvc::allYjbj(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BDSvc::allYjbj]bad format:", jsonString, 1);
	int seq = root.getv("seq", 0);
	int zhjAddr = root.getv("zhjAddr", 0);
	if(zhjAddr <= 0) return 1;

	char sql[1024];
	sprintf(sql, "SELECT PostAddr,PostTime,InfoType,MsgType,MsgCode,Message FROM t_bd_message WHERE Addr = '%d' AND MsgType=8 AND (bjflag IS NULL OR bjflag IN ('0','1'))", zhjAddr);
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	map<int, MsgList> addMsgMap;
	map<int, MsgList>::iterator iter;
	while(psql->NextRow())
	{
		STRUCT_MSG sMsg;
		sMsg.addr = atoi(psql->GetField("PostAddr"));
		sMsg.time = atoi(psql->GetField("PostTime"));
		sMsg.infoType = atoi(psql->GetField("InfoType"));
		sMsg.msgType = atoi(psql->GetField("MsgType"));
		sMsg.msgCode = atoi(psql->GetField("MsgCode"));
		sMsg.msg = psql->GetField("Message");
	
		if((iter=addMsgMap.find(sMsg.addr)) == addMsgMap.end()) {
			MsgList msgList;
			msgList.push_back(sMsg);
			addMsgMap[sMsg.addr] = msgList;
		} else {
			iter->second.push_back(sMsg);
		}
	}

	bool bFirst = true;
	out << FormatString("{seq:%d,results:[", seq);
	for(iter=addMsgMap.begin(); iter!=addMsgMap.end(); iter++)
	{
		if(bFirst)
			bFirst = false;
		else
			out << ",";
		out << FormatString("{addr:%d,messages:[", iter->first);
		for(int i=0; i<(int)iter->second.size(); i++) {
			if(i!=0) out << ",";
			out << FormatString("{time:%d,postAddr:%d,postTime:%d,infoType:%d,msgType:%d,msgCode:%d,msg:\"%s\"}", 
				iter->second[i].time,
				iter->second[i].addr,
				iter->second[i].time,
				iter->second[i].infoType,
				iter->second[i].msgType,
				iter->second[i].msgCode,
				iter->second[i].msg.c_str());
		}
		out << "]}";
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::recvSms(const char* pUid, const char* jsonString, std::stringstream& out)
{
	static string lastSrcMobile;
	static string lastAddr;

	JSON_PARSE_RETURN("[BDSvc::recvSms]bad format:", jsonString, 1);
	string smsType = root.getv("SmsType", "");
	string srcMobile = root.getv("SrcMobile", "");
	string appendID = root.getv("AppendID", "");
	string content = root.getv("Content", "");
	string recvTime = root.getv("RecvTime", "");
	string sendTime = root.getv("SendTime", "");
	string status = root.getv("Status", "");

	string utf8Content = CodeConverter::Gb2312ToUtf8(content.c_str());
	DEBUG_LOG("[BDSvc::recvSms]=========Sms Content(utf8): %s", utf8Content.c_str());

	if(smsType != "0")
		return 1;

	// 需要对短信网关进行的分包进行处理
	Tokens addrAndMsg;
	string::size_type idx1, idx2;
	if((idx1=content.find("("))!=string::npos && (idx2=content.find(")"))!=string::npos) {
		string seqStr = content.substr(idx1, idx2-idx1+1);
		string tmp = content.substr(idx2+1);
		addrAndMsg = StrSplit(tmp, "#"); if(addrAndMsg.size() != 2) addrAndMsg = StrSplit(tmp, "＃");
		if(addrAndMsg.size() == 2) {
			lastSrcMobile = srcMobile;
			lastAddr = addrAndMsg[0];
			addrAndMsg[1] = seqStr + addrAndMsg[1];
		} else {
			addrAndMsg.push_back(addrAndMsg[0]);
			addrAndMsg[0] = lastAddr;
			addrAndMsg[1] = seqStr + addrAndMsg[1];
		}
	} else {
		addrAndMsg = StrSplit(content, "#"); if(addrAndMsg.size() != 2) addrAndMsg = StrSplit(content, "＃");
		if(addrAndMsg.size() != 2 || addrAndMsg[0].empty() || addrAndMsg[1].empty() || srcMobile.empty()) 
			return 1;
	}


	string addr = addrAndMsg[0];
	content = "86&" + srcMobile + "&" + addrAndMsg[1];

	ACE_SOCK_Connector m_Connector;
	ACE_SOCK_Stream m_Peer;
	if(!checkConnect(m_Connector, m_Peer)) {
		out << "{eid:1,msg:\"not connect to bd\"}";
		return 0;
	}

	int nLen = 0;
	ACE_Time_Value tv(10);
	char buff[1024*10];
	sprintf(buff, "{seq:0,cmd:1,info_type:0,msg_code:0,msg_type:2,addr:%s,msg:\"%s\"}##",addr.c_str(), content.c_str());
	DEBUG_LOG("[BDSvc::recvSms]========= %s", buff);

	nLen = m_Peer.send(buff, strlen(buff), &tv);
	if(nLen<=0) {
		DEBUG_LOG("[BDSvc::recvSms] ============================== send failed");
		m_Peer.close();
		return 2;
	}
	memset(buff, 0, sizeof(buff));
	nLen = m_Peer.recv(buff, sizeof(buff), &tv);
	if(nLen<=0) {
		DEBUG_LOG("[BDSvc::recvSms] ============================== recv failed");
		m_Peer.close();
		return 2;
	}
	if(strstr(buff, ",succ:0")) {
		out << "{eid:1,msg:\"p2p send failed\"}";
		m_Peer.close();
		return 0;
	}

	out << "{eid:0,msg:\"succ\"}";
	m_Peer.close();
	return 0;
}
