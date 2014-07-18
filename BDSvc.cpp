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
#include "curl/curl.h"
#include "SmsMail.h"

//#define BD_IP "10.134.2.112"
#define BD_IP "122.227.170.98"

////////////////////////////////////////////////////// 一些辅助函数 //////////////////////
inline unsigned char char_to_hex(unsigned char x)
{
	return (unsigned char)(x > 9 ? x + 55 : x + 48);
}

inline int is_alpha_number_char(unsigned char c)
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
		return 1;
	return 0;
}

std::string UrlEncode(unsigned char * src, int  src_len)
{
	char dest[4 * 1024];
	int dest_len = 4 * 1024;
	char *pDest = dest;

	unsigned char ch;
	int  len = 0;

	while (len < (dest_len - 4) && *src)
	{
		ch = (unsigned char)*src;
		if (*src == ' ')
		{
			*pDest++ = '+';
		}
		else if (is_alpha_number_char(ch) || strchr("-_.!~*'()", ch))
		{
			*pDest++ = *src;
		}
		else
		{
			*pDest++ = '%';
			*pDest++ = char_to_hex((unsigned char)(ch >> 4));
			*pDest++ = char_to_hex((unsigned char)(ch % 16));
		}
		++src;
		++len;
	}
	*pDest = 0;
	return std::string(dest);
}

int bd_SendSms(const char* toMobile, const char* msg)
{
	char url[2048];
	sprintf(url, "http://221.179.180.158:9007/QxtSms/QxtFirewall?OperID=zgcbjt&OperPass=zgcbjt001&SendTime=&ValidTime=&AppendID=&DesMobile=%s&Content=%s&ContentType=8", toMobile, UrlEncode((unsigned char*)msg, strlen(msg)).c_str());

	CURL* pCurl = curl_easy_init();
	curl_easy_setopt(pCurl, CURLOPT_URL, url);
	int nCode = curl_easy_perform(pCurl);
	if (nCode != CURLE_OK)
	{
		DEBUG_PRT((LM_ERROR, "[g_SendSms] curl_easy_perform error:%d.\n", nCode));
	}
	else
	{

	}
	curl_easy_cleanup(pCurl);

	return nCode;
}
////////////////////////////////////////////////////// 一些辅助函数 END //////////////////////

int bd_SendSms(Tokens& toMobiles, const char* msg)
{
	string strMobiles = ToStr(toMobiles);
	return bd_SendSms(strMobiles.c_str(), msg);
}

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
	SERVICE_MAP(0x20,BDSvc,getGroupListInfo);
	SERVICE_MAP(0x21,BDSvc,AddOrEditGroupPeople);
	SERVICE_MAP(0x22,BDSvc,DeleteGroupPeople);
	SERVICE_MAP(0x23,BDSvc,getGroupPeople);
	SERVICE_MAP(0x24,BDSvc,EditPeopleInfo);
	SERVICE_MAP(0x25,BDSvc,AddPeopleInfo);
	SERVICE_MAP(0x26,BDSvc,getPeopleInfo);
	SERVICE_MAP(0x27,BDSvc,getBindCardList);
	SERVICE_MAP(0x28,BDSvc,getContactsGroupListInfo);
	SERVICE_MAP(0x29,BDSvc,AddOrEditGroupContacts);
	SERVICE_MAP(0x2a,BDSvc,DeleteGroupContacts);
	SERVICE_MAP(0x2b,BDSvc,getContactsPeopleInfo);
	SERVICE_MAP(0x2c,BDSvc,AddOrEditContactsInfo);
	SERVICE_MAP(0x2d,BDSvc,deleteContactInfo);
	SERVICE_MAP(0x2e,BDSvc,editUserPassword);
	SERVICE_MAP(0x2f,BDSvc,getManageCardList);
	SERVICE_MAP(0x30,BDSvc,releaseCardBind);
	SERVICE_MAP(0x31,BDSvc,recycleCards);
	SERVICE_MAP(0x32,BDSvc,getCardInfo);
	SERVICE_MAP(0x33,BDSvc,allocCards);
	SERVICE_MAP(0x34,BDSvc,getBindCardUsers);
	SERVICE_MAP(0x35,BDSvc,EditCardInfo);
	SERVICE_MAP(0x36,BDSvc,checkExistUser);
	SERVICE_MAP(0x37,BDSvc,getWhiteUserInfo);
	SERVICE_MAP(0x38,BDSvc,addWhiteUserInfo);
	SERVICE_MAP(0x39,BDSvc,deleteWhiteUserInfo);
	SERVICE_MAP(0x40,BDSvc,getManageCardList);
	SERVICE_MAP(0x41,BDSvc,editWhiteUser);
	SERVICE_MAP(0x42,BDSvc,getPhonePrefix);
	SERVICE_MAP(0x43,BDSvc,getApplyWhiteUser);
	SERVICE_MAP(0x44,BDSvc,agreeOrDisagreeWhiteUser);
	SERVICE_MAP(0x45,BDSvc,getChargingInfo);
	SERVICE_MAP(0x46,BDSvc,addChargingInfo);
	SERVICE_MAP(0x47,BDSvc,editChargingInfo);
	SERVICE_MAP(0x48,BDSvc,deleteChargingInfo);
	SERVICE_MAP(0x49,BDSvc,sendMsgInfo);
	SERVICE_MAP(0x4a,BDSvc,bindBeiDouCard);

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

	// 处理短信命令
	Tokens cmdAndAddr = StrSplit(content, "#");
	if (cmdAndAddr.size() != 2)
		cmdAndAddr = StrSplit(content, "＃");
	if (cmdAndAddr.size() == 2) {
		int addr = atoi(cmdAndAddr[1].c_str());
		if (addr <= 0)
			return 1;

		// (1) 申请添加到北斗卡短信白名单：TJBDTX#北斗卡号
		if (strcasecmp(cmdAndAddr[0].c_str(), "TJBDTX") == 0) {
			if (!checkExistsCard(addr)) {
				bd_SendSms(srcMobile.c_str(), "您查询的卡号不存在,请检查后再试");
				return 1;
			}
		}
		// (2) 申请从北斗卡短信白名单中移除：QXBDTX#北斗卡号
		else if (strcasecmp(cmdAndAddr[0].c_str(), "TJBDTX") == 0) {
			if (!checkExistsCard(addr)) {
				bd_SendSms(srcMobile.c_str(), "您查询的卡号不存在,请检查后再试");
				return 1;
			}
		}
		// (3) 查询剩余短信余量：CXBDYL#北斗卡号
		//  对于套餐用户，反馈剩余短信条数；对于包年用户，反馈到期时间。
		else if (strcasecmp(cmdAndAddr[0].c_str(), "CXBDYL") == 0) {
			if (!checkExistsCard(addr)) {
				bd_SendSms(srcMobile.c_str(), "您查询的卡号不存在,请检查后再试");
				return 1;
			}
		}
		// (4) 查询北斗卡位置信息：CXBDWZ#北斗卡号
		else if (strcasecmp(cmdAndAddr[0].c_str(), "CXBDWZ") == 0) {
			if (!checkExistsCard(addr)) {
				bd_SendSms(srcMobile.c_str(), "您查询的卡号不存在,请检查后再试");
				return 1;
			}
		}
	}

	// 其它命令：发送给北斗卡
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

// TODO
int BDSvc::allWhitelistApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BDSvc::allWhitelistApply]bad format:", jsonString, 1);
	return 1;
}

int BDSvc::dealWhitelistApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BDSvc::dealWhitelistApply]bad format:", jsonString, 1);
	int seq = root.getv("seq", 0);
	string mobile = root.getv("mobile", "");
	int addr = root.getv("addr", 0);
	int status = root.getv("status", 0);
	if (mobile.empty() || addr == 0 || (status != 1 && status != 2))
		return 1;

	bool bSucc = dealWhitelistApply(mobile, addr, status);
	out << FormatString("{seq:%d,succ:%d}", seq, (bSucc ? 1 : 0));
	return 0;
}

bool BDSvc::checkExistsCard(int addr)
{
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(FormatString("select addr from t_bd_ic where addr='%d'", addr).c_str())&&psql->NextRow(), false);
	RELEASE_MYSQL_RETURN(psql, true);
}

int  BDSvc::checkWhiteList(std::string& mobile, int addr)
{
	return 0;
}

bool BDSvc::dealWhitelistApply(std::string& mobile, int addr, int status, bool isApply)
{
	int now = (int)time(NULL);

	char sql[1024];
	if (isApply) {
		sprintf(sql, "INSERT INTO t_bd_whitelist(addr,mobile,STATUS,applytime,dealtime,updatedt) VALUES (%d,'%s','%d',%d,%d,NOW()) \
					ON DUPLICATE KEY UPDATE STATUS = VALUES(STATUS), applytime = VALUES(applytime), dealtime = VALUES(dealtime), updatedt = VALUES(updatedt)",
					addr, mobile.c_str(), status, now, now);
	}
	else {
		sprintf(sql, "INSERT INTO t_bd_whitelist(addr,mobile,STATUS,dealtime,updatedt) VALUES (%d,'%s','%d',%d,NOW()) \
					ON DUPLICATE KEY UPDATE STATUS = VALUES(STATUS), dealtime = VALUES(dealtime), updatedt = VALUES(updatedt)",
					addr, mobile.c_str(), status, now);
	}
	
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Execute(sql) > 0, false);
	RELEASE_MYSQL_RETURN(psql, true);
}

int BDSvc::getGroupListInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getGroupListInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");


	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"SELECT GROUP_ID,GROUP_NA,GROUP_ID_FA FROM boloomodb.T_BD_GROUP_PEOPLE \
				WHERE USER_ID_CREATOR='%s'",strUid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",groups:[";

	string group_id="";
	string group_na="";
	string group_id_fa="";

	int idx = 0;

	while(psql->NextRow())
	{
		if (idx)
		{
			out<<",";
		}
		idx++;

		READMYSQL_STRING(GROUP_ID,group_id);
		READMYSQL_STRING(GROUP_NA,group_na);
		READMYSQL_STRING(GROUP_ID_FA,group_id_fa);

		out<<"{gid:\""<<group_id<<"\",gname:\""<<group_na<<"\",fid:\""<<group_id_fa<<"\"}";
	}

	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::AddOrEditGroupPeople(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::AddOrEditGroupPeople]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");
	string strFid=root.getv("fid","");
	string strGid=root.getv("gid","");
	string strGname=root.getv("gname","");


	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	string strTemp = "G";
	long ltime=0;
	ltime=time(0);
	char szTmp[32];
	sprintf(szTmp,"%d",ltime);
	strTemp+=szTmp;

	if(strGid.empty())
	{
		sprintf(sql,"INSERT INTO boloomodb.T_BD_GROUP_PEOPLE VALUES('%s','%s','%s','%s')",strTemp.c_str(),strGname.c_str(),strFid.c_str(),strUid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
		out<<"{seq:\""<<strSeq<<"\",etype:0,gid:\""<<strTemp<<"\"}";
	}
	else
	{
		sprintf(sql,"UPDATE boloomodb.T_BD_GROUP_PEOPLE SET GROUP_NA='%s',GROUP_ID_FA='%s',USER_ID_CREATOR='%s' \
					WHERE GROUP_ID='%s'",strGname.c_str(),strFid.c_str(),strUid.c_str(),strGid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
		out<<"{seq:\""<<strSeq<<"\",etype:0,gid:\""<<strGid<<"\"}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::DeleteGroupPeople(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::DeleteGroupPeople]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strGid=root.getv("gid","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"DELETE FROM boloomodb.T_BD_GROUP_PEOPLE \
				WHERE GROUP_ID='%s'",strGid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	sprintf(sql,"DELETE FROM boloomodb.T_BD_GROUP_PEOPLE_REL \
				WHERE GROUP_ID='%s'",strGid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",gid:\""<<strGid<<"\",etype:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::getGroupPeople(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getGroupPeople]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");
	string strGid=root.getv("gid","");
	string strWord=root.getv("word","");
	int strPage=root.getv("page",0);
	int strPagesize=root.getv("pageSize",0);

	int offsetTemp = 0;
	offsetTemp = strPagesize*(strPage-1);

	char test[20];
	sprintf(test,"%d",offsetTemp);
	DEBUG_LOG(test);

	char test1[20];
	sprintf(test1,"%d",strPagesize);
	DEBUG_LOG(test1);

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	int idx=0;

	out<<"{seq:\""<<strSeq<<"\",gid:\""<<strGid<<"\",users:[";

	string group_id="";
	string user_id="";
	string user_na=""; 
	string tel="";
	int role=0;
	int create_time=0;
	int state=0;
	string group_na="";


	sprintf(sql,"SELECT COUNT(*) AS SUM FROM boloomodb.T_BD_GROUP_PEOPLE_REL \
				WHERE GROUP_ID='%s'",strGid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	if (psql->NextRow())
	{
		DEBUG_LOG("1");
		sprintf(sql,"SELECT t1.GROUP_ID,t1.USER_ID,t2.USER_NA,t2.TEL, \
					t2.ROLE,t2.CREATE_TIME,t2.STATE,t3.GROUP_NA \
					FROM boloomodb.T_BD_GROUP_PEOPLE_REL t1 \
					JOIN boloomodb.T_BD_PEOPLE_INFO t2 \
					ON t1.USER_ID=t2.USER_ID \
					JOIN boloomodb.T_BD_GROUP_PEOPLE t3 \
					ON t3.GROUP_ID = t1.GROUP_ID \
					WHERE t1.GROUP_ID='%s' AND t1.USER_ID LIKE '%%%s%' ORDER BY t2.CREATE_TIME DESC LIMIT %d,%d ",strGid.c_str(),strWord.c_str(),offsetTemp,strPagesize);
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);


		while(psql->NextRow())
		{
			if(idx)
			{
				out<<",";
			}
			idx++;
			READMYSQL_STRING(GROUP_ID,group_id);
			READMYSQL_STRING(USER_ID,user_id);
			READMYSQL_STRING(USER_NA,user_na);
			READMYSQL_STRING(TEL,tel);
			READMYSQL_INT(ROLE,role,0);
			READMYSQL_INT(CREATE_TIME,create_time,0);
			READMYSQL_INT(STATE,state,0);
			READMYSQL_STRING(GROUP_NA,group_na);
			out<<"{uid:\""<<user_id<<"\",un:\""<<user_na<<"\",ph:\""<<tel<<"\",role:"<<role<<",ct:"<<create_time<<",st:"<<state<<",gi:\""<<group_id<<"\",gn:\""<<group_na<<"\"}";
		}
	}
	else
	{
		sprintf(sql,"SELECT t1.GROUP_ID,t1.USER_ID,t2.USER_NA,t2.TEL, \
					t2.ROLE,t2.CREATE_TIME,t2.STATE,t3.GROUP_NA \
					FROM boloomodb.T_BD_GROUP_PEOPLE_REL t1 \
					JOIN boloomodb.T_BD_PEOPLE_INFO t2 \
					ON t1.USER_ID=t2.USER_ID \
					JOIN boloomodb.T_BD_GROUP_PEOPLE t3 \
					ON t3.GROUP_ID = t1.GROUP_ID \
					WHERE t1.GROUP_ID IN (SELECT GROUP_ID \
					FROM T_BD_GROUP_PEOPLE WHERE GROUP_ID_FA='%s') AND t1.USER_ID LIKE '%%%s%' ORDER BY t2.CREATE_TIME DESC LIMIT %d,%d ",strGid.c_str(),strWord.c_str(),offsetTemp,strPagesize);
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

		while(psql->NextRow())
		{
			if(idx)
			{
				out<<",";
			}
			idx++;
			READMYSQL_STRING(GROUP_ID,group_id);
			READMYSQL_STRING(USER_ID,user_id);
			READMYSQL_STRING(USER_NA,user_na);
			READMYSQL_STRING(TEL,tel);
			READMYSQL_INT(ROLE,role,0);
			READMYSQL_INT(CREATE_TIME,create_time,0);
			READMYSQL_INT(STATE,state,0);
			READMYSQL_STRING(GROUP_NA,group_na);
			out<<"{uid:\""<<user_id<<"\",un:\""<<user_na<<"\",ph:\""<<tel<<"\",role:"<<role<<",ct:"<<create_time<<",st:"<<state<<",gi:\""<<group_id<<"\",gn:\""<<group_na<<"\"}";
		}
	}

	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::EditPeopleInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::EditPeopleInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");
	string strUname=root.getv("uname","");
	string strPhone=root.getv("phone","");
	int strState=root.getv("state",0);
	string strGid=root.getv("gid","");
	string strEmail=root.getv("email","");
	string strCompany=root.getv("company","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";


	sprintf(sql,"UPDATE boloomodb.T_BD_PEOPLE_INFO SET USER_NA='%s',TEL='%s',STATE=%d,EMAIL='%s', \
				COMPANY='%s' WHERE USER_ID='%s'",strUname.c_str(),strPhone.c_str(),strState,strEmail.c_str(),strCompany.c_str(),strUid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	sprintf(sql,"DELETE FROM boloomodb.T_BD_GROUP_PEOPLE_REL \
				WHERE USER_ID='%s'",strUid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	sprintf(sql,"INSERT INTO boloomodb.T_BD_GROUP_PEOPLE_REL VALUES('%s','%s')",strUid.c_str(),strGid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",etype:0,uid:\""<<strUid<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::AddPeopleInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::AddPeopleInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");
	string strUname=root.getv("uname","");
	string strPhone=root.getv("phone","");
	int strRoleid=root.getv("roleId",0);
	int strState=root.getv("state",0);
	string strGid=root.getv("gid","");
	string strEmail=root.getv("email","");
	string strCompany=root.getv("company","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	long ltime=0;
	ltime=time(0);

	sprintf(sql,"INSERT INTO boloomodb.T_BD_PEOPLE_INFO VALUES('%s','','',%d,'%s','%s','%s',%d,%d,0,'','%s')",strUid.c_str(),strRoleid,strUname.c_str(),strPhone.c_str(),strEmail.c_str(),ltime,strState,strCompany.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	sprintf(sql,"INSERT INTO boloomodb.T_BD_GROUP_PEOPLE_REL VALUES('%s','%s')",strUid.c_str(),strGid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",etype:0,uid:\""<<strUid<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::getPeopleInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getPeopleInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"SELECT t1.USER_ID,t1.COMBO_KIND,t1.COMBO_ST,t1.ROLE,t1.USER_NA,t1.TEL,t1.EMAIL,t1.CREATE_TIME, \
				t1.STATE,t1.END_TIME,t1.COMPANY,t2.GROUP_ID,t3.GROUP_NA FROM \
				boloomodb.T_BD_PEOPLE_INFO t1 \
				LEFT JOIN boloomodb.T_BD_GROUP_PEOPLE_REL t2 \
				ON t2.USER_ID=t1.USER_ID \
				LEFT JOIN boloomodb.T_BD_GROUP_PEOPLE t3 \
				ON t3.GROUP_ID=t2.GROUP_ID \
				WHERE t1.USER_ID='%s'",strUid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	string combo_kind="";
	string combo_st="";
	int role=0;
	string user_na="";
	string tel="";
	string email="";
	int create_time=0;
	int state=0;
	int end_time=0;
	string company="";
	string group_id="";
	string group_na="";

	if (psql->NextRow())
	{
		READMYSQL_STRING(COMBO_KIND,combo_kind);
		READMYSQL_STRING(COMBO_ST,combo_st);
		READMYSQL_INT(ROLE,role,0);
		READMYSQL_STRING(USER_NA,user_na);
		READMYSQL_STRING(TEL,tel);
		READMYSQL_STRING(EMAIL,email);
		READMYSQL_INT(CREATE_TIME,create_time,0);
		READMYSQL_INT(STATE,state,0);
		READMYSQL_INT(END_TIME,end_time,0);
		READMYSQL_STRING(COMPANY,company);
		READMYSQL_STRING(GROUP_ID,group_id);
		READMYSQL_STRING(GROUP_NA,group_na);

		out<<"{seq:\""<<strSeq<<"\",uid:\""<<strUid<<"\",uname:\""<<user_na<<"\",phone:\""<<tel<<"\",roleId:\""<<role<<"\",email:\""<<email<<"\",groupId:\""<<group_id<<"\",gname:\""<<group_na<<"\",status:\""<<state<<"\",createTime:\""<<create_time<<"\",tm:\""<<end_time<<"\",pay:\""<<combo_st<<"\",pt:\""<<combo_kind<<"\",company:\""<<company<<"\"}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}


int BDSvc::getBindCardList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getBindCardList]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"SELECT t1.CARD_ID,t1.CARD_ST,t1.BROADCAST_ADDR,t1.SER_FREQ,t1.COMMUNICATE_RATE, \
				t1.IS_CODE,t1.CARD_KIND,t1.END_TIME,t1.COMBO_ST,t1.COMBO_KIND \
				FROM boloomodb.T_BD_CARD_LIST_TABLE t1 \
				JOIN boloomodb.T_BD_USER_BIND_TABLE t2 \
				ON t1.CARD_ID=t2.CARD_ID \
				WHERE t2.USER_ID='%s'",strUid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	string card_id="";
	string card_st="";
	string broadcast_addr="";
	string ser_freq="";
	string communicate_rate="";
	string is_code="";
	string card_kind="";
	int end_time=0;
	string combo_st="";
	string combo_kind="";

	out<<"{seq:\""<<strSeq<<"\",cards:[";

	int idx=0;

	while(psql->NextRow())
	{
		if(idx)
			out<<",";
		idx++;
		READMYSQL_STRING(CARD_ID,card_id);
		READMYSQL_STRING(CARD_ST,card_st);
		READMYSQL_STRING(BROADCAST_ADDR,broadcast_addr);
		READMYSQL_STRING(SER_FREQ,ser_freq);
		READMYSQL_STRING(COMMUNICATE_RATE,communicate_rate);
		READMYSQL_STRING(IS_CODE,is_code);
		READMYSQL_STRING(CARD_KIND,card_kind);
		READMYSQL_INT(END_TIME,end_time,0);
		READMYSQL_STRING(COMBO_ST,combo_st);
		READMYSQL_STRING(COMBO_KIND,combo_kind);

		out<<"{cid:\""<<card_id<<"\",cy:\""<<card_st<<"\",cq:\""<<broadcast_addr<<"\",cs:\""<<ser_freq<<"\",cl:\""<<communicate_rate<<"\",ci:\""<<is_code<<"\",cty:\""<<card_kind<<"\",tm:\""<<end_time<<"\",pay:\""<<combo_st<<"\",pt:\""<<combo_kind<<"\"}";

	}

	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}
int BDSvc::getContactsGroupListInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getContactsGroupListInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");
	string strGid=root.getv("gid","");


	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	out<<"{seq:\""<<strSeq<<"\",groups:[";

	string group_id="";
	string group_na="";
	string group_id_fa="";

	int idx = 0;

	if(strGid.empty())
	{
		sprintf(sql,"SELECT GROUP_ID,GROUP_NA,GROUP_ID_FA FROM boloomodb.T_BD_CONTACTS_GROUP \
					WHERE USER_ID_CREATOR='%s'",strUid.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

		while(psql->NextRow())
		{
			if (idx)
			{
				out<<",";
			}
			idx++;

			READMYSQL_STRING(GROUP_ID,group_id);
			READMYSQL_STRING(GROUP_NA,group_na);
			READMYSQL_STRING(GROUP_ID_FA,group_id_fa);

			out<<"{gid:\""<<group_id<<"\",gname:\""<<group_na<<"\",fid:\""<<group_id_fa<<"\"}";
		}
	}
	else
	{
		sprintf(sql,"SELECT GROUP_ID,GROUP_NA,GROUP_ID_FA FROM boloomodb.T_BD_CONTACTS_GROUP \
					WHERE GROUP_ID='%s' OR GROUP_ID_FA='%s'",strGid.c_str(),strGid.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

		while(psql->NextRow())
		{
			if (idx)
			{
				out<<",";
			}
			idx++;

			READMYSQL_STRING(GROUP_ID,group_id);
			READMYSQL_STRING(GROUP_NA,group_na);
			READMYSQL_STRING(GROUP_ID_FA,group_id_fa);

			out<<"{gid:\""<<group_id<<"\",gname:\""<<group_na<<"\",fid:\""<<group_id_fa<<"\"}";
		}
	}

	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::AddOrEditGroupContacts(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::AddOrEditGroupContacts]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");
	string strFid=root.getv("fid","");
	string strGid=root.getv("gid","");
	string strGname=root.getv("gname","");


	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	string strTemp = "C";
	long ltime=0;
	ltime=time(0);
	char szTmp[32];
	sprintf(szTmp,"%d",ltime);
	strTemp+=szTmp;

	if(strGid.empty())
	{
		sprintf(sql,"INSERT INTO boloomodb.T_BD_CONTACTS_GROUP VALUES('%s','%s','%s','%s')",strTemp.c_str(),strGname.c_str(),strFid.c_str(),strUid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
		out<<"{seq:\""<<strSeq<<"\",etype:0,gid:\""<<strTemp<<"\"}";
	}
	else
	{
		sprintf(sql,"UPDATE boloomodb.T_BD_CONTACTS_GROUP SET GROUP_NA='%s',GROUP_ID_FA='%s',USER_ID_CREATOR='%s' \
					WHERE GROUP_ID='%s'",strGname.c_str(),strFid.c_str(),strUid.c_str(),strGid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
		out<<"{seq:\""<<strSeq<<"\",etype:0,gid:\""<<strGid<<"\"}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::DeleteGroupContacts(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::DeleteGroupContacts]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strGid=root.getv("gid","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"DELETE FROM boloomodb.T_BD_CONTACTS_GROUP \
				WHERE GROUP_ID='%s'",strGid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);


	sprintf(sql,"DELETE FROM boloomodb.T_BD_CONTACTS_INFO \
				WHERE GROUP_ID='%s'",strGid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",gid:\""<<strGid<<"\",etype:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::getContactsPeopleInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getContactsPeopleInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strGid=root.getv("id","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	int idx=0;

	out<<"{seq:\""<<strSeq<<"\",data:[";

	string communicater="";
	string name="";
	string tel=""; 
	string group_id="";
	string group_na="";
	string career="";
	string remark="";

	sprintf(sql,"SELECT t1.COMMUNICATER,t1.NAME,t1.TEL,t1.GROUP_ID,t2.GROUP_NA,t1.CAREER, \
				t1.REMARK FROM boloomodb.T_BD_CONTACTS_INFO t1 \
				JOIN boloomodb.T_BD_CONTACTS_GROUP t2 \
				ON t1.GROUP_ID=t2.GROUP_ID \
				WHERE t2.GROUP_ID='%s' OR t2.GROUP_ID_FA='%s'",strGid.c_str(),strGid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	while(psql->NextRow())
	{
		if (idx)
		{	
			out<<",";
		}
		idx++;

		READMYSQL_STRING(COMMUNICATER,communicater);
		READMYSQL_STRING(NAME,name);
		READMYSQL_STRING(TEL,tel);
		READMYSQL_STRING(GROUP_ID,group_id);
		READMYSQL_STRING(GROUP_NA,group_na);
		READMYSQL_STRING(CAREER,career);
		READMYSQL_STRING(REMARK,remark);

		out<<"{lid:\""<<communicater<<"\",name:\""<<name<<"\",phone:\""<<tel<<"\",gId:\""<<group_id<<"\",gname:\""<<group_na<<"\",post:\""<<career<<"\",rm:\""<<remark<<"\"}";
	}

	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::AddOrEditContactsInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::AddOrEditContactsInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");
	string strName=root.getv("name","");
	string strLid=root.getv("lid","");
	string strPhone=root.getv("phone","");
	string strGid=root.getv("gid","");
	string strPost=root.getv("post","");
	string strRemark=root.getv("remark","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	string strTemp = "C";
	long ltime=0;
	ltime=time(0);
	char szTmp[32];
	sprintf(szTmp,"%d",ltime);
	strTemp+=szTmp;

	if (strLid.empty())
	{
		sprintf(sql,"INSERT INTO boloomodb.T_BD_CONTACTS_INFO VALUES('%s','%s','%s','%s','%s','%s')",strTemp.c_str(),strGid.c_str(),strName.c_str(),strPhone.c_str(),strPost.c_str(),strRemark.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
		out<<"{seq:\""<<strSeq<<"\",etype:0,lid:\""<<strTemp<<"\"}";
	}
	else
	{

		sprintf(sql,"UPDATE boloomodb.T_BD_CONTACTS_INFO SET NAME='%s',TEL='%s',CAREER='%s',REMARK='%s' \
					WHERE COMMUNICATER='%s'",strName.c_str(),strPhone.c_str(),strPost.c_str(),strRemark.c_str(),strLid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		out<<"{seq:\""<<strSeq<<"\",etype:0,lid:\""<<strLid<<"\"}";

	}

	RELEASE_MYSQL_RETURN(psql, 0);
}
int BDSvc::deleteContactInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::deleteContactInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strLids=root.getv("lids","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	Tokens strTempVec = StrSplit(strLids,"|");

	for(int i=0;i < strTempVec.size();i++)
	{
		sprintf(sql,"DELETE FROM boloomodb.T_BD_CONTACTS_INFO WHERE COMMUNICATER='%s'",strTempVec[i].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::editUserPassword(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::editUserPassword]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");
	string strOld_pwd=root.getv("old_pwd","");
	string strNew_pwd=root.getv("new_pwd","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"UPDATE boloomodb.T_BD_PEOPLE_INFO SET PASSWORD='%s' \
				WHERE USER_ID='%s' AND PASSWORD='%s'",strNew_pwd.c_str(),strUid.c_str(),strOld_pwd.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::getManageCardList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getManageCardList]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");
	string strCtype=root.getv("ctype","");
	string strWord=root.getv("word","");
	int strPage=root.getv("page",0);
	int strPagesize=root.getv("pageSize",0);

	int tempNum = strPagesize*(strPage-1);
	int total=0;

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	out<<"{seq:\""<<strSeq<<"\",cards:[";

	if (strCtype.empty())
	{
		sprintf(sql,"SELECT COUNT(*) AS TOTAL FROM boloomodb.T_BD_CARD_LIST_TABLE t1 \
					JOIN boloomodb.T_BD_USER_CARD_MANAGE_REL t2 \
					ON t1.CARD_ID=t2.CARD_ID WHERE t2.USER_ID='%s' AND t2.CARD_ID LIKE '%%%s%' ORDER BY t1.PURCHASE_TIME DESC",strUid.c_str(),strWord.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

		if (psql->NextRow())
		{
			READMYSQL_INT(TOTAL,total,0);
		}

		sprintf(sql,"SELECT t1.CARD_ID,t1.CARD_ST,t1.BROADCAST_ADDR,t1.SER_FREQ,t1.COMMUNICATE_RATE, \
					t1.IS_CODE,t1.CARD_KIND,t1.CARD_HOLDER,t1.PURCHASE_TIME,t1.END_TIME,t1.COMBO_ST, \
					t1.COMBO_KIND FROM boloomodb.T_BD_CARD_LIST_TABLE t1 \
					JOIN boloomodb.T_BD_USER_CARD_MANAGE_REL t2 \
					ON t1.CARD_ID=t2.CARD_ID WHERE t2.USER_ID='%s' AND t2.CARD_ID LIKE '%%%s%' ORDER BY t1.PURCHASE_TIME DESC LIMIT %d,%d",strUid.c_str(),strWord.c_str(),tempNum,strPagesize);

		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
	}
	else
	{
		sprintf(sql,"SELECT COUNT(*) AS TOTAL FROM boloomodb.T_BD_CARD_LIST_TABLE t1 \
					JOIN boloomodb.T_BD_USER_CARD_MANAGE_REL t2 \
					ON t1.CARD_ID=t2.CARD_ID WHERE t2.USER_ID='%s' AND t1.CARD_ST='%s' AND t2.CARD_ID LIKE '%%%s%' ORDER BY t1.PURCHASE_TIME DESC",strUid.c_str(),strCtype.c_str(),strWord.c_str());

		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

		if (psql->NextRow())
		{
			READMYSQL_INT(TOTAL,total,0);
		}

		sprintf(sql,"SELECT t1.CARD_ID,t1.CARD_ST,t1.BROADCAST_ADDR,t1.SER_FREQ,t1.COMMUNICATE_RATE, \
					t1.IS_CODE,t1.CARD_KIND,t1.CARD_HOLDER,t1.PURCHASE_TIME,t1.END_TIME,t1.COMBO_ST, \
					t1.COMBO_KIND FROM boloomodb.T_BD_CARD_LIST_TABLE t1 \
					JOIN boloomodb.T_BD_USER_CARD_MANAGE_REL t2 \
					ON t1.CARD_ID=t2.CARD_ID WHERE t2.USER_ID='%s' AND t1.CARD_ST='%s' AND t2.CARD_ID LIKE '%%%s%' ORDER BY t1.PURCHASE_TIME DESC LIMIT %d,%d",strUid.c_str(),strCtype.c_str(),strWord.c_str(),tempNum,strPagesize);

		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
	}
	string card_id="";
	string card_st="";
	string broadcast_addr="";
	string ser_freq="";
	string communicate_rate="";
	string is_code="";
	string card_kind="";
	string card_holder="";
	int purchase_time=0;
	int end_time=0;
	string combo_st="";
	string combo_kind="";

	int idx=0;
	int num=0;

	while(psql->NextRow())
	{
		num++;
		if (idx)
		{
			out<<",";
		}
		idx++;

		READMYSQL_STRING(CARD_ID,card_id);
		READMYSQL_STRING(CARD_ST,card_st);
		READMYSQL_STRING(BROADCAST_ADDR,broadcast_addr);
		READMYSQL_STRING(SER_FREQ,ser_freq);
		READMYSQL_STRING(COMMUNICATE_RATE,communicate_rate);
		READMYSQL_STRING(IS_CODE,is_code);
		READMYSQL_STRING(CARD_KIND,card_kind);
		READMYSQL_STRING(CARD_HOLDER,card_holder);
		READMYSQL_INT(PURCHASE_TIME,purchase_time,0);
		READMYSQL_INT(END_TIME,end_time,0);
		READMYSQL_STRING(COMBO_ST,combo_st);
		READMYSQL_STRING(COMBO_KIND,combo_kind);

		out<<"{cid:\""<<card_id<<"\",cy:\""<<card_st<<"\",cq:\""<<broadcast_addr<<"\",cs:\""<<ser_freq<<"\",cl:\""<<communicate_rate<<"\",ci:\""<<is_code<<"\",cty:\""<<card_kind<<"\",co:\""<<card_holder<<"\",cm:\""<<purchase_time<<"\",tm:\""<<end_time<<"\",pay:\""<<combo_st<<"\",pt:\""<<combo_kind<<"\"}";
	}

	out<<"],total:\""<<total<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::releaseCardBind(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::releaseCardBind]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");
	string strCid=root.getv("cid","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"DELETE FROM boloomodb.T_BD_USER_BIND_TABLE \
				WHERE USER_ID='%s' AND CARD_ID='%s'",strUid.c_str(),strCid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);


	out<<"{seq:\""<<strSeq<<"\",cid:\""<<strCid<<"\",etype:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::recycleCards(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::recycleCards]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strLids=root.getv("cards","");
	string strRemark=root.getv("remark","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	Tokens strTempVec = StrSplit(strLids,"|");

	for(int i=0;i < strTempVec.size();i++)
	{
		sprintf(sql,"UPDATE boloomodb.T_BD_CARD_LIST_TABLE SET BE_USER_ID='',CARD_HOLDER='',CARD_ST='2' \
					WHERE CARD_ID='%s'",strTempVec[i].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
	}

	string strTemp = "R";
	long ltime=0;
	ltime=time(0);
	char szTmp[32];
	sprintf(szTmp,"%d",ltime);
	strTemp+=szTmp;

	sprintf(sql,"INSERT INTO boloomodb.T_BD_RECYCLE_RECORD VALUES ('%s','%s') \
				",strTemp.c_str(),strRemark.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);


	out<<"{seq:\""<<strSeq<<"\",etype:0,recycleId:\""<<strTemp<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}


int BDSvc::getCardInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getCardInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strCid=root.getv("cid","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	int end_time=0;
	string combo_st="";
	string combo_kind="";

	sprintf(sql,"SELECT END_TIME,COMBO_ST,COMBO_KIND FROM \
				boloomodb.T_BD_CARD_LIST_TABLE \
				WHERE CARD_ID='%s'",strCid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);;

	if (psql->NextRow())
	{
		READMYSQL_INT(END_TIME,end_time,0);
		READMYSQL_STRING(COMBO_ST,combo_st);
		READMYSQL_STRING(COMBO_KIND,combo_kind);

		out<<"{seq:\""<<strSeq.c_str()<<"\",tm:\""<<end_time<<"\",pay:\""<<combo_st<<"\",pt:\""<<combo_kind<<"\"}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}
int BDSvc::allocCards(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::allocCards]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strCards=root.getv("cards","");
	string strAuid=root.getv("aUid","");
	string strRemark=root.getv("remark",""); 

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"SELECT USER_NA FROM T_BD_PEOPLE_INFO \
				WHERE USER_ID='%s'",strAuid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	string user_na="";

	if (psql->NextRow())
	{
		READMYSQL_STRING(USER_NA,user_na);
	}

	Tokens strTempVec = StrSplit(strCards,"|");

	for(int i=0;i < strTempVec.size();i++)
	{
		sprintf(sql,"UPDATE boloomodb.T_BD_CARD_LIST_TABLE SET BE_USER_ID='%s',CARD_HOLDER='%s',CARD_ST='1' \
					WHERE CARD_ID='%s'",strAuid.c_str(),user_na.c_str(),strTempVec[i].c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
	}

	string strTemp = "A";
	long ltime=0;
	ltime=time(0);
	char szTmp[32];
	sprintf(szTmp,"%d",ltime);
	strTemp+=szTmp;

	sprintf(sql,"INSERT INTO boloomodb.T_BD_ALLOC_RECORD VALUES ('%s','%s') \
				",strTemp.c_str(),strRemark.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);


	out<<"{seq:\""<<strSeq<<"\",etype:0,allocId:\""<<strTemp<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}


int BDSvc::getBindCardUsers(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getBindCardUsers]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");
	string strCid=root.getv("cid","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	out<<"{seq:\""<<strSeq<<"\",users:[";

	sprintf(sql,"SELECT t1.USER_ID,t1.USER_NA,t1.ROLE,t3.GROUP_ID,t3.GROUP_NA FROM \
				boloomodb.T_BD_PEOPLE_INFO t1 \
				LEFT JOIN boloomodb.T_BD_GROUP_PEOPLE_REL t2 \
				ON t1.USER_ID=t2.USER_ID \
				LEFT JOIN boloomodb.T_BD_GROUP_PEOPLE t3 \
				ON t3.GROUP_ID=t2.GROUP_ID \
				JOIN T_BD_USER_BIND_TABLE t4 \
				ON t4.USER_ID=t1.USER_ID \
				WHERE t4.CARD_ID='%s'",strCid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	int idx=0;
	string user_id="";
	string user_na="";
	int role=0;
	string group_id="";
	string group_na="";

	while(psql->NextRow())
	{

		if (idx)
		{
			out<<",";
		}
		idx++;

		READMYSQL_STRING(USER_ID,user_id);
		READMYSQL_STRING(USER_NA,user_na);
		READMYSQL_INT(ROLE,role,0);
		READMYSQL_STRING(GROUP_ID,group_id);
		READMYSQL_STRING(GROUP_NA,group_na);

		out<<"{uid:\""<<user_id<<"\",un:\""<<user_na<<"\",roles:\""<<role<<"\",gid:\""<<group_id<<"\",gn:\""<<group_na<<"\"}";

	}
	DEBUG_LOG("1");;

	out<<"]}";

	DEBUG_LOG("2");;

	RELEASE_MYSQL_RETURN(psql, 0);
}



int BDSvc::EditCardInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::EditCardInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");
	string strCid=root.getv("cid","");
	string strCtype=root.getv("ctype","");
	string strCuser=root.getv("cuser","");
	int strTm=root.getv("tm",0);
	string strPay=root.getv("pay","");
	string strPt=root.getv("pt","");


	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	string cuser="";

	sprintf(sql,"SELECT USER_NA FROM boloomodb.T_BD_PEOPLE_INFO \
				WHERE USER_ID='%s'",strCuser.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	if (psql->NextRow())
	{
		READMYSQL_STRING(USER_NA,cuser);
	}

	sprintf(sql,"UPDATE boloomodb.T_BD_CARD_LIST_TABLE SET \
				CARD_ST='%s',BE_USER_ID='%s',CARD_HOLDER='%s',END_TIME=%d, \
				COMBO_ST='%s',COMBO_KIND='%s' WHERE \
				CARD_ID='%s'",strCtype.c_str(),strCuser.c_str(),cuser.c_str(),strTm,strPay.c_str(),strPt.c_str(),strCid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",etype:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::checkExistUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::checkExistUser]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	int sum = 0;

	sprintf(sql,"SELECT COUNT(*) AS SUM FROM boloomodb.T_BD_PEOPLE_INFO \
				WHERE USER_ID='%s'",strUid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	if (psql->NextRow())
	{
		READMYSQL_INT(SUM,sum,0);
		if (sum)
		{
			out<<"{seq:\""<<strSeq<<"\",eid:0,status:1}";
		}
		else
			out<<"{seq:\""<<strSeq<<"\",eid:0,status:0}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::getWhiteUserInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getWhiteUserInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strCardnum = root.getv("card_num","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"SELECT ID,USER_ID,TEL \
				FROM boloomodb.T_BD_WHITE_CARD  \
				WHERE \
				CARD_ID='%s'",strCardnum.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	string id="";
	string user_id="";
	string tel="";
	int idx=0;

	out<<"{seq:\""<<strSeq<<"\",data:[";

	while(psql->NextRow())
	{

		if (idx)
		{
			out<<",";
		}
		idx++;
		READMYSQL_STRING(ID,id);
		READMYSQL_STRING(USER_ID,user_id);
		READMYSQL_STRING(TEL,tel);

		Tokens strTempVec = StrSplit(tel," ");

		out<<"{id:\""<<id<<"\",name:\""<<user_id<<"\",prefix:\""<<strTempVec[0]<<"\",mobile:\""<<strTempVec[1]<<"\"}";
	}

	RELEASE_MYSQL_RETURN(psql, 0);
}


int BDSvc::addWhiteUserInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::addWhiteUserInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strCardnum = root.getv("card_num","");
	string strName=root.getv("name","");
	string strPrefix=root.getv("prefix","");
	string strMobile=root.getv("mobile","");
	int strFlag=root.getv("flag",0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	string finalTel = strPrefix+ ' ' + strMobile;

	string strTemp = "W";
	long ltime=0;
	ltime=time(0);
	char szTmp[32];
	sprintf(szTmp,"%d",ltime);
	strTemp+=szTmp;

	sprintf(sql,"INSERT INTO boloomodb.T_BD_WHITE_CARD VALUES('%s','%s','%s','%s',%d,%d,0)",strTemp.c_str(),strCardnum.c_str(),strName.c_str(),finalTel.c_str(),strFlag,ltime);

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",eid:0,id:\""<<strTemp<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::deleteWhiteUserInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::deleteWhiteUserInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strId=root.getv("id","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"DELETE FROM boloomodb.T_BD_WHITE_CARD WHERE \
				ID='%s'",strId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::getManageCardList1(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getManageCardList]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");;

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	out<<"{seq:\""<<strSeq<<"\",cards:[";

	sprintf(sql,"SELECT t1.CARD_ID FROM boloomodb.T_BD_CARD_LIST_TABLE t1 \
				JOIN boloomodb.T_BD_USER_CARD_MANAGE_REL t2 \
				ON t1.CARD_ID=t2.CARD_ID WHERE t2.USER_ID='%s'",strUid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	string card_id="";
	int idx=0;

	while(psql->NextRow())
	{
		if (idx)
		{
			out<<",";
		}
		idx++;

		READMYSQL_STRING(CARD_ID,card_id);

		out<<"{name:\""<<card_id<<"\"}";
	}

	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::editWhiteUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::editWhiteUser]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strId=root.getv("id","");
	string strCardnum = root.getv("card_num","");
	string strName=root.getv("name","");
	string strPrefix=root.getv("prefix","");
	string strMobile=root.getv("mobile","");

	string finalMobile = strPrefix + ' ' + strMobile;

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"UPDATE boloomodb.T_BD_WHITE_CARD SET CARD_ID='%s',USER_ID='%s', \
				TEL='%s' WHERE ID='%s'",strCardnum.c_str(),strName.c_str(),finalMobile.c_str(),strId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::getPhonePrefix(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getPhonePrefix]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	string prefix="";

	out<<"{seq:\""<<strSeq<<"\",prefix:[";

	sprintf(sql,"SELECT TelCode FROM boloomodb.t91_country");

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	int idx=0;
	string telcode="";
	while(psql->NextRow())
	{
		if (idx)
		{
			out<<",";
		}
		idx++;
		READMYSQL_STRING(TelCode,telcode);
		out<<"{name:\"+"<<telcode<<"\"}";

	}
	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::getApplyWhiteUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getApplyWhiteUser]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strCardnum = root.getv("card_num","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	Tokens strTempVec = StrSplit(strCardnum,"|");

	out<<"{seq:\""<<strSeq<<"\",data:[";

	for(int i=0;i < strTempVec.size();i++)
	{
		if (i)
		{
			out<<",";
		}
		out<<"{card:\""<<strTempVec[i]<<"\",info:[";

		sprintf(sql,"SELECT ID,USER_ID,REMARKS,TEL,TIME WHERE CARD_ID='%s' \
					AND STATUS=1",strTempVec[i].c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

		string id="";
		string user_id="";
		string remarks="";
		int time=0;
		string tel="";

		string prefix="";
		string final_phone="";

		int idx=0;

		while(psql->NextRow())
		{
			if (idx)
			{
				out<<",";
			}
			idx++;
			READMYSQL_STRING(ID,id);
			READMYSQL_STRING(USER_ID,user_id);
			READMYSQL_STRING(REMARKS,remarks);
			READMYSQL_STRING(TEL,tel);
			READMYSQL_INT(TIME,time,0);

			out<<"{id:\""<<id<<"\",name:\""<<user_id<<"\",mobile:\""<<tel<<"\"time:"<<time<<",remarks:\""<<remarks<<"\",status:1}";
		}
		out<<"]}";

	}

	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}


int BDSvc::agreeOrDisagreeWhiteUser(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::agreeOrDisagreeWhiteUser]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strId=root.getv("id","");
	int strStatus=root.getv("status",0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"UPDATE boloomodb.T_BD_WHITE_CARD SET \
				STATUS=%d WHERE ID='%s'",strStatus,strId.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::getChargingInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getChargingInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"SELECT t1.FLAG,t1.CHARGEID,t1.NAME,t1.CONTENT,t1.FEE,t1.TYPE FROM \
				boloomodb.T_BD_CHARGE_INFO t1 WHERE \
				t1.FLAG=1");
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	int flag=0;
	string chargeid="";
	string name="";
	string content="";
	double fee=0.0;
	int type=0;
	int idx=0;

	out<<"{seq:\""<<strSeq<<"\",data:[{flag:1,tactics:[";

	while(psql->NextRow())
	{
		if (idx)
		{
			out<<",";
		}
		idx++;
		READMYSQL_STRING(CHARGEID,chargeid);
		READMYSQL_INT(FLAG,flag,0);
		READMYSQL_STRING(NAME,name);
		READMYSQL_STRING(CONTENT,content);
		READMYSQL_DOUBLE(FEE,fee,0.0);
		READMYSQL_INT(TYPE,type,0);

		out<<"{id:\""<<chargeid<<"\",name:\""<<name<<"\",details:\""<<content<<"\",fee:"<<fee<<",type:"<<type<<"}";

	}

	out<<"]},{flag:2,tactics:[";

	sprintf(sql,"SELECT t1.FLAG,t1.CHARGEID,t1.NAME,t1.CONTENT,t1.FEE,t1.TYPE FROM \
				boloomodb.T_BD_CHARGE_INFO t1 WHERE t1.FLAG=2");
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	idx=0;

	while(psql->NextRow())
	{
		if (idx)
		{
			out<<",";
		}
		idx++;
		READMYSQL_STRING(CHARGEID,chargeid);
		READMYSQL_INT(FLAG,flag,0);
		READMYSQL_STRING(NAME,name);
		READMYSQL_STRING(CONTENT,content);
		READMYSQL_DOUBLE(FEE,fee,0.0);
		READMYSQL_INT(TYPE,type,0);

		out<<"{id:\""<<chargeid<<"\",name:\""<<name<<"\",details:\""<<content<<"\",fee:"<<fee<<",type:"<<type<<"}";

	}

	out<<"]}]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}


int BDSvc::addChargingInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::addChargingInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strName=root.getv("name","");
	string strDetails=root.getv("details","");
	double strFee=root.getv("fee",0.0);
	int strFlag=root.getv("flag",0);
	int strType=root.getv("type",0);

	char tempfk[32];
	sprintf(tempfk,"%f",strFee);
	DEBUG_LOG(tempfk);

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	string strTemp = "C";
	long ltime=0;
	ltime=time(0);
	char szTmp[32];
	sprintf(szTmp,"%d",ltime);
	strTemp+=szTmp;

	sprintf(sql,"INSERT INTO boloomodb.T_BD_CHARGE_INFO VALUES('%s','%s','%s',%f,%d,%d) \
				",strTemp.c_str(),strName.c_str(),strDetails.c_str(),strFee,strType,strFlag);
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",eid:0,id:\""<<strTemp<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::editChargingInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::editChargingInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strId=root.getv("id","");
	string strName=root.getv("name","");
	string strDetails=root.getv("details","");
	double strFee=root.getv("fee",0.0);
	int strFlag=root.getv("flag",0);

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"UPDATE boloomodb.T_BD_CHARGE_INFO SET NAME='%s', \
				CONTENT='%s',FEE=%f,FLAG=%d WHERE CHARGEID='%s'",strName.c_str(),strDetails.c_str(),strFee,strFlag,strId.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}


int BDSvc::deleteChargingInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::deleteChargingInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strId=root.getv("id","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"DELETE FROM boloomodb.T_BD_CHARGE_INFO WHERE CHARGEID='%s'",strId.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::sendMsgInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::sendMsgInfo]bad format:", jsonString, 1);
	DEBUG_LOG("tets");

	string strSeq=root.getv("seq","");
	string strUid=root.getv("uid","");
	string strPhone=root.getv("phone","");
	string strContent=root.getv("content","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	sprintf(sql,"INSERT INTO boloomodb.T_BD_SEND_MSG VALUES('%s','%s','%s')",strUid.c_str(),strPhone.c_str(),strContent.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	Tokens strPhoneVec = StrSplit(strPhone,"|");

	string fromUser="";
	string toUser="";
	int tempType=0;
	bool is_succeed=false;

	for (int i=0;i < strPhoneVec.size();i++)
	{
		is_succeed = g_SendSms(fromUser.c_str(),toUser.c_str(),strPhoneVec[i].c_str(),strContent.c_str(),tempType);
	}

	out<<"{seq:\""<<strSeq<<"\",is_succeed:\""<<is_succeed<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

int BDSvc::bindBeiDouCard(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::bindBeiDouCard]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strCardnum = root.getv("card_num","");
	string strUid = root.getv("uid","");

	MySql* psql = CREATE_MYSQL;
	char sql[1024]="";

	Tokens tempCardnumVec = StrSplit(strCardnum,"|");

	for (int i = 0;i < tempCardnumVec.size();i++)
	{
		sprintf(sql,"INSERT INTO boloomodb.T_BD_USER_BIND_TABLE VALUES('%s','%s')",strUid.c_str(),tempCardnumVec[i].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}