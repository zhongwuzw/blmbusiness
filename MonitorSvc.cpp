#include "blmcom_head.h"
#include "MonitorSvc.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "Util.h"
#include "kSQL.h"
#include "ObjectPool.h"

using namespace std;

IMPLEMENT_SERVICE_MAP(MonitorSvc)

MonitorSvc::MonitorSvc()
{

}

MonitorSvc::~MonitorSvc()
{

}

bool MonitorSvc::Start()
{
    if(!g_MessageService::instance()->RegisterCmd(MID_MONITOR, this))
        return false;

    SERVICE_MAP(SID_MONITOR_ENTITY,MonitorSvc,getMonitorsOfEntify);
    SERVICE_MAP(SID_MONITOR_ADD,MonitorSvc,addMonitor);
    SERVICE_MAP(SID_MONITOR_UPD_AUTHORITY,MonitorSvc,updateAuthority);
    SERVICE_MAP(SID_MONITOR_DEAL_APPLY,MonitorSvc,dealApply);
    SERVICE_MAP(SID_MONITOR_APPLY,MonitorSvc,apply);
    SERVICE_MAP(SID_PORT_BERTHS,MonitorSvc,getBerthsOfPort);
    SERVICE_MAP(SID_MONITOR_REGION,MonitorSvc,getMonitorsOfRegion);
	SERVICE_MAP(SID_ENTITY_MONITOR_APPLY,MonitorSvc,applyEntityMonitors);
	SERVICE_MAP(SID_UPD_ENTITY_MONITORS_IP,MonitorSvc,updEntityMonitorsIP);
	
    DEBUG_LOG("[MonitorSvc::Start] OK......................................");
	//string jsonString = "{tp:0,id:\"S8714\",ip:\"123.125.18.142\",num:8}";
	//std::stringstream out;
	//updEntityMonitorsIP("cjtest4",jsonString.c_str(), out);
	
    return true;
}

//{ tp: 0,id: "2011120800" }
int MonitorSvc::getMonitorsOfEntify(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MonitorSvc::getMonitorsOfEntify]bad format:", jsonString, 1);

    int tp = root.getv("tp", 0);
    string id = root.getv("id", "");

    MySql* psql = CREATE_MYSQL;
	
    char sql[1024] = "";
    //sprintf (sql, "SELECT t1.camera_id, t1.name, t1.is_public, t1.user_id, t1.address, t1.port, t1.width, t1.height, UNIX_TIMESTAMP(t1.update_dt) AS owntm,	t3.user_id AS applyer,		\
				//	t3.accept, UNIX_TIMESTAMP(t3.APPLY_DT) as appdt, t4.user_id AS viewer, UNIX_TIMESTAMP(t4.UPDATE_DT) as updt, t1.status AS ntp, t1.PROTOCOL_TYPE, t1.url, t1.is_seipc\
				//	FROM t20_camera AS t1 LEFT JOIN t20_camera_entity AS t2 ON t1.camera_id = t2.camera_id																				\
				//	LEFT JOIN t20_camera_apply AS t3 ON t1.camera_id = t3.camera_id																										\
				//	LEFT JOIN t20_camera_viewer AS t4 ON t1.camera_id = t4.camera_id																									\
				//	WHERE t2.entity_type = %d AND t2.entity_param = '%s'", tp, id.c_str());
    sprintf (sql, "SELECT t1.camera_id, t1.name, t1.is_public, t1.user_id, t1.address, t1.port, t1.width, t1.height, UNIX_TIMESTAMP(t1.update_dt) AS owntm, t3.user_id AS applyer,			\
						t3.accept, UNIX_TIMESTAMP(t3.APPLY_DT) AS appdt, t4.user_id AS viewer, t4.status AS updt, t1.status AS ntp, t1.PROTOCOL_TYPE, t1.url, t1.is_seipc\
						FROM t20_camera AS t1 LEFT JOIN t20_camera_entity AS t2 ON t1.camera_id = t2.camera_id																				\
						LEFT JOIN t41_authen_apply AS t3 ON t1.camera_id = t3.BUSID AND t3.BUSTYPE = '3'																					\
						LEFT JOIN t20_camera_viewer AS t4 ON t1.camera_id = t4.camera_id																									\
						WHERE t2.entity_type = '%d' AND t2.entity_param = '%s'", tp, id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	CameraInfoMap CIMap;
    CameraInfoMap::iterator ciIter;
	while (psql->NextRow())
	{
		CameraInfo cameraInfo;
		//VIEWPERSON tmpVp;
		char viewer[32];
		int appdt;
		int updt;
		char applyer[32];
		int accept;
		//int bseipc;
		READMYSQL_INT(camera_id, cameraInfo.id, 0);
		READMYSQL_STR(name, cameraInfo.name);
		READMYSQL_INT(is_public, cameraInfo.is_public, 0);
		READMYSQL_STR(user_id, cameraInfo.owner);
		READMYSQL_STR(address, cameraInfo.addr);
		READMYSQL_INT(port, cameraInfo.port, 80);
		READMYSQL_STR(url, cameraInfo.path);
		READMYSQL_INT(width, cameraInfo.x, 0);
		READMYSQL_INT(height, cameraInfo.y, 0);
		READMYSQL_INT(owntm, cameraInfo.owntm, 0);
		READMYSQL_STR(applyer, applyer);
		READMYSQL_INT(accept, accept, -1);
		READMYSQL_INT(appdt, appdt, -1);
		READMYSQL_INT(updt, updt, -1);
		READMYSQL_STR(viewer, viewer);
		READMYSQL_INT(ntp, cameraInfo.ntp, -1);
		READMYSQL_INT(PROTOCOL_TYPE, cameraInfo.nconnect, -1);
		READMYSQL_INT(is_seipc, cameraInfo.bseipc, -1);

		if ((ciIter = CIMap.find(cameraInfo.id)) == CIMap.end())
		{
			if (cameraInfo.viewers.find(string(viewer)) == cameraInfo.viewers.end())
				cameraInfo.viewers.insert(make_pair(string(viewer), updt));

			if (accept == 0 && cameraInfo.applyers.find(string(applyer)) == cameraInfo.applyers.end())
				cameraInfo.applyers.insert(make_pair(string(applyer), appdt));
			else if (accept == 2 && cameraInfo.refusers.find(string(applyer)) == cameraInfo.refusers.end())
				cameraInfo.refusers.insert(make_pair(string(applyer), appdt));

			CIMap.insert(make_pair(cameraInfo.id, cameraInfo));
		}
		else
		{
			if (cameraInfo.viewers.find(string(viewer)) == cameraInfo.viewers.end())
				(*ciIter).second.viewers.insert(make_pair(string(viewer), updt));

			if (accept == 0 && cameraInfo.applyers.find(string(applyer)) == cameraInfo.applyers.end())
				(*ciIter).second.applyers.insert(make_pair(string(applyer), appdt));
			else if (accept == 2 && cameraInfo.refusers.find(string(applyer)) == cameraInfo.refusers.end())
				(*ciIter).second.refusers.insert(make_pair(string(applyer), appdt));
		}
	}

    out << "[";
    for (ciIter = CIMap.begin(); ciIter != CIMap.end(); ciIter++)
    {
        out << (ciIter == CIMap.begin()?"":",");
        out << "{id:\"" << (*ciIter).second.id << "\", na:\"" << JsonReplace(string((*ciIter).second.name)) << "\", bpublic:" << (*ciIter).second.is_public << ", viewers:[" << (*ciIter).second.toJsonViewers() << "], refusers:[" << (*ciIter).second.toJsonRefusers() << "],  applyers:[" << (*ciIter).second.toJsonApplyers() 
			<< "], ip: \"" << (*ciIter).second.addr << "\",port:\"" << (*ciIter).second.port << "\", x:" << (*ciIter).second.x << ", y:" << (*ciIter).second.y << ", ntp:" << (*ciIter).second.ntp << ",nconnect:" << (*ciIter).second.nconnect << ",path:\"" << (*ciIter).second.path << "\",bseipc:" << (*ciIter).second.bseipc << "}";
    }
    out << "]";
    RELEASE_MYSQL_RETURN(psql, 0);
}


//{id:"camer1",na:"camer1",bpublic:true, viewers: "user1,user2", ip: "http://www.boloomo.com",port:,"",x:320,y:480,nprotocol:0,nconnect:0,objtp:0,objid:"111111",objna:"123",path: "/dev/camra",bseipc:0}
int MonitorSvc::addMonitor(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MonitorSvc::addMonitor]bad format:", jsonString, 1);

	string cid = root.getv("id", "");
    string naString = root.getv("na", "");
	string na = SqlReplace(naString);
    int bpublic = root.getv("bpublic", 0);
    string viewers = root.getv("viewers", "");
    string addr = root.getv("ip", "");
	string port = root.getv("port", "");
    int x = root.getv("x", 0);
    int y = root.getv("y", 0);
    int nprotocol = root.getv("nprotocol", 0);
    int nconnect = root.getv("nconnect", 0);
    int objtp = root.getv("objtp", 0);
    string objid = root.getv("objid", "");
	string objna = root.getv("objna", "");
	string path = root.getv("path", "");
	int bseipc = root.getv("bseipc", 0);

	char gmt0now[20];
	GmtNow(gmt0now);

	MySql* psql = CREATE_MYSQL;

    double longitude = 0.0;
    double latitude = 90.0;
	bool idFlg = (cid.length() > 0);//判断操作为插入还是修改

    int camera_id = 0;

    char sql[1024 * 100] = "";

    switch(objtp)
    {
		case 0:
		{
			sprintf(sql, "SELECT longitude, latitude FROM aisdb.t41_ais_ship_realtime AS t1, t41_ship AS t2 WHERE t1.mmsi = t2.mmsi AND SHIPID = '%s'", objid.c_str());
		}
		break;
		case 1:
		{
			sprintf (sql, "SELECT  LONGITUDE_dd as longitude, LATITUDE_dd as latitude FROM t41_port WHERE PortID = '%s'", objid.c_str());
		}
		break;
		case 2:
		{
			sprintf (sql, "SELECT  LONGITUDE_dd as longitude, LATITUDE_dd as latitude FROM t41_port AS t1, t41_port_berth AS t2 WHERE t1.PortID = t2.PortID AND t2.Berth_id = '%s'", objid.c_str());
		}
		break;
		case 3:
		{
			Tokens longlat = StrSplit(objid, ",");
			longitude = atof(longlat[0].c_str());
			latitude = atof(longlat[1].c_str());
		}
		break;
    }

    if (objtp >=0 && objtp <= 2)
    {
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		if (psql->NextRow())
		{
			READMYSQL_DOUBLE(longitude, longitude, 0.0);
			READMYSQL_DOUBLE(latitude, latitude, 90.0);
		}
    }

    //INSERT INTO t20_camera
	if(idFlg)
		sprintf (sql, "UPDATE t20_camera SET t20_camera.NAME = '%s', user_id = '%s', PROTOCOL = '%d', PROTOCOL_TYPE = '%d', ADDRESS = '%s', \
					  						WIDTH = '%d', HEIGHT = '%d', IS_PUBLIC = '%d', update_DT = '%s', URL = '%s', port = '%s', IS_SEIPC = '%d' WHERE camera_id = '%s'"
						, na.c_str(), pUid, nprotocol, nconnect, addr.c_str(), x, y, bpublic, gmt0now,path.c_str(), port.c_str(), bseipc, cid.c_str());
	else
		sprintf (sql, "INSERT INTO t20_camera																					\
						(t20_camera.NAME, user_id, PROTOCOL, PROTOCOL_TYPE, ADDRESS, WIDTH, HEIGHT, IS_PUBLIC, update_DT, URL, port, IS_SEIPC)		\
						VALUES	('%s', '%s', %d, %d, '%s', %d, %d, %d, '%s', '%s', '%s', '%d')"
				 , na.c_str(), pUid, nprotocol, nconnect, addr.c_str(), x, y, bpublic, gmt0now,path.c_str(), port.c_str(), bseipc);
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    //查询插入
	camera_id = idFlg?atoi(cid.c_str()):psql->GetInsertId();

    //INSERT INTO t20_camera_entity
    sprintf (sql, "REPLACE INTO t20_camera_entity (CAMERA_ID, ENTITY_TYPE, ENTITY_PARAM, ENTITY_NAME, UPDATE_DT,  LONGITUDE,  LATITUDE)	\
													VALUES (%d, %d, '%s', '%s',  '%s', %f, %f)", camera_id, objtp, objid.c_str(), objna.c_str(), gmt0now,longitude, latitude);
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    Tokens viewersOfCamera = StrSplit(viewers, ",");
    int len = sprintf (sql, "REPLACE INTO t20_camera_viewer (USER_ID, CAMERA_ID, UPDATE_DT) VALUES ('%s', %d, '%s')", pUid, camera_id,gmt0now);
    for (Tokens::iterator iTer = viewersOfCamera.begin(); iTer != viewersOfCamera.end(); iTer++)
    {
        if (strcmp(pUid, (*iTer).c_str()))
            len += sprintf (sql + len, ", ('%s', %d, '%s')", (*iTer).c_str(), camera_id, gmt0now);
    }

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out << "{eid:0, id:\"" << camera_id << "\"}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//{uid:"gsf",cid: "camer1" ,ntp:0}
int MonitorSvc::updateAuthority(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MonitorSvc::updateAuthority]bad format:", jsonString, 1);

    string uid = root.getv("uid", "");
    string cid = root.getv("cid", "");
    int ntp = root.getv("ntp", 0);

	char gmt0now[20];
	GmtNow(gmt0now);

    MySql* psql = CREATE_MYSQL;
    char sql[1024] = "";
    //查询是否camera的拥有者
    char owner[32] = "";
    sprintf (sql, "SELECT user_id FROM t20_camera  WHERE camera_id = '%s'", cid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		READMYSQL_STR(user_id, owner);
	}

    if (strcmp(owner, pUid))
        RELEASE_MYSQL_RETURN(psql, 1);

	if (ntp)
		sprintf (sql, "REPLACE INTO t20_camera_viewer (USER_ID, CAMERA_ID, UPDATE_DT) VALUES ('%s', '%s', '%s')", uid.c_str(), cid.c_str(), gmt0now);
	else
		sprintf (sql, "DELETE FROM t20_camera_viewer WHERE user_id = '%s' AND camera_id = '%s'", uid.c_str(), cid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    out << "{eid:0}";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//{uid:"gsf",cid: "camer1" ,ntp:0}
int MonitorSvc::dealApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MonitorSvc::dealApply]bad format:", jsonString, 1);

    string uid = root.getv("uid", "");
    string cid = root.getv("cid", "");
    int ntp = root.getv("ntp", 0);

	char gmt0now[20];
	GmtNow(gmt0now);

    MySql* psql = CREATE_MYSQL;
    char sql[1024] = "";
    //查询是否camera的拥有者
    char owner[32] = "";
    sprintf (sql, "SELECT user_id FROM t20_camera  WHERE camera_id = '%s'", cid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		READMYSQL_STR(user_id, owner);
	}

    if (strcmp(owner, pUid))
        RELEASE_MYSQL_RETURN(psql, 1);

    int accept = 3;

	if (ntp == 2)
		//sprintf (sql, "DELETE FROM t20_camera_apply WHERE camera_id = '%s' AND user_id = '%s'", cid.c_str(), uid.c_str());
		sprintf (sql, "DELETE FROM t41_authen_apply WHERE BUSID = '%s' AND user_id = '%s' AND BUSTYPE = '3'", cid.c_str(), uid.c_str());
	else
	{
		accept = ntp?2:1;
		//sprintf (sql, "UPDATE t20_camera_apply SET accept = %d WHERE camera_id = '%s' AND user_id = '%s'", accept, cid.c_str(), uid.c_str());	
		sprintf (sql, "UPDATE t41_authen_apply SET accept = '%d' WHERE BUSID = '%s' AND user_id = '%s' AND BUSTYPE = '3'", accept, cid.c_str(), uid.c_str());	
	}
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    if (!ntp)
    {
        sprintf (sql, "REPLACE INTO t20_camera_viewer (USER_ID, CAMERA_ID, UPDATE_DT) VALUES ('%s', '%s', '%s')", uid.c_str(), cid.c_str(), gmt0now);
        CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
    }

    RELEASE_MYSQL_RETURN(psql, 0);
}

//{uid:"gsf",cid: "camer1" }
int MonitorSvc::apply(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MonitorSvc::apply]bad format:", jsonString, 1);

    string uid = root.getv("uid", "");
    string cid = root.getv("cid", "");

	char gmt0now[20];
	GmtNow(gmt0now);

    MySql* psql = CREATE_MYSQL;
    char sql[1024] = "";

	sprintf (sql, "SELECT COUNT(1) AS num FROM t20_camera_viewer WHERE user_id = '%s' AND camera_id = '%s'", uid.c_str(), cid.c_str());
	int num = 0;
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
	READMYSQL_INT(num, num, 0);

	if (num)
	{
		RELEASE_MYSQL_RETURN(psql, 0);
	}

    //sprintf (sql, "REPLACE INTO t20_camera_apply (USER_ID, CAMERA_ID, APPLY_DT, ACCEPT, UPDATE_DT) VALUES ('%s', '%s', '%s', 0, '%s')", uid.c_str(), cid.c_str(),gmt0now,gmt0now);
	sprintf (sql, "REPLACE INTO t41_authen_apply (USER_ID, BUSTYPE, BUSID, APPLY_DT, ACCEPT, UPDATE_DT) VALUES ('%s', '3', '%s', '%s', 0, '%s')", uid.c_str(), cid.c_str(),gmt0now,gmt0now);
    CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

//{id:"46094"}
int MonitorSvc::getBerthsOfPort(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MonitorSvc::getBerthsOfPort]bad format:", jsonString, 1);
    string id = root.getv("id", "");

    MySql* psql = CREATE_MYSQL;
    char sql[1024] = "";
    sprintf (sql, "SELECT berth_id, t41_port_berth.Name FROM t41_port_berth WHERE portid = '%s'", id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	berthVec bVec;
	BerthOfPort berth;
	while (psql->NextRow())
	{
		READMYSQL_STR(berth_id, berth.id);
		READMYSQL_STR(Name, berth.name);
		bVec.push_back(berth);
	}

    out << "[";
    for (berthVec::iterator iTer = bVec.begin(); iTer != bVec.end(); iTer++)
    {
        if (iTer == bVec.begin())
            out << "{id:\"" << (*iTer).id <<"\", na:\"" << (*iTer).name << "\"}";
        else
            out << ",{id:\"" << (*iTer).id <<"\", na:\"" << (*iTer).name << "\"}";
    }
    out << "]";

    RELEASE_MYSQL_RETURN(psql, 0);
}

//{xmin:16.6,xmax:17.6,ymin:1,ymax:2.7}
int MonitorSvc::getMonitorsOfRegion(const char* pUid, const char* jsonString, std::stringstream& out)
{
    JSON_PARSE_RETURN("[MonitorSvc::getMonitorsOfRegion]bad format:", jsonString, 1);
    double xmin = root.getv("xmin", 0.0);
    double xmax = root.getv("xmax", 0.0);
    double ymin = root.getv("ymin", 0.0);
    double ymax = root.getv("ymax", 0.0);

    MySql* psql = CREATE_MYSQL;
    char sql[1024] = "";
    sprintf (sql, "SELECT entity_type, entity_param, ENTITY_NAME, longitude, latitude FROM t20_camera_entity WHERE longitude >= %f AND longitude <= %f AND latitude >= %f AND latitude <= %f", xmin, xmax, ymin, ymax);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

    posCMAP pcMap;
	posCMAP::iterator posIter;
	while (psql->NextRow())
	{
		PositionOfCamera posOfCam;
		READMYSQL_INT(entity_type, posOfCam.type, 0);
		READMYSQL_STR(entity_param, posOfCam.id);
		READMYSQL_STR(ENTITY_NAME, posOfCam.name);
		READMYSQL_DOUBLE(longitude, posOfCam.x, 0.0);
		READMYSQL_DOUBLE(latitude, posOfCam.y, 90.0);

		if (posOfCam.type == 0)			
			continue;

		if ((posIter = pcMap.find(string(posOfCam.id))) == pcMap.end() || (*posIter).second.type != posOfCam.type)
			pcMap.insert(make_pair(string(posOfCam.id),posOfCam));
	}

	sprintf (sql, "SELECT ENTITY_PARAM FROM t20_camera_entity WHERE ENTITY_TYPE = '0'");
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	Tokens shipids;
	while (psql->NextRow())
	{
		char tmp[64];
		READMYSQL_STR(ENTITY_PARAM, tmp);
		shipids.push_back(tmp);
	}

	if (shipids.size() > 0)
	{
		string sqlString = FormatString("SELECT shipid, t2.name, longitude, latitude FROM aisdb.t41_ais_ship_realtime AS t1, t41_ship AS t2 WHERE t1.mmsi = t2.mmsi AND SHIPID IN ('%s') \
						AND longitude >= '%f' AND longitude <= '%f' AND latitude >= '%f' AND latitude <= '%f'", ToStr(shipids, ",").c_str(), xmin, xmax, ymin, ymax);
		CHECK_MYSQL_STATUS(psql->Query(sqlString.c_str()), 3);

		while (psql->NextRow())
		{
			PositionOfCamera posOfCam;
			posOfCam.type = 0;
			READMYSQL_STR(shipid, posOfCam.id);
			READMYSQL_STR(name, posOfCam.name);
			READMYSQL_DOUBLE(longitude, posOfCam.x, 0.0);
			READMYSQL_DOUBLE(latitude, posOfCam.y, 90.0);

			pcMap.insert(make_pair(string(posOfCam.id),posOfCam));
		}
	}

    out << "[";
    for (posCMAP::iterator iTer = pcMap.begin(); iTer != pcMap.end(); iTer++)
    {
		out << (iTer == pcMap.begin()?"":",");
		out << "{tp:" << (*iTer).second.type << ",id:\"" << (*iTer).second.id << "\", x:" << (*iTer).second.x << ", y:" << (*iTer).second.y << ",name:\"" << (*iTer).second.name << "\"}";
    }
    out << "]";
    RELEASE_MYSQL_RETURN(psql, 0);
}

//{ uid:"gsf",tp: 0,id: "2011120800" }
int MonitorSvc::applyEntityMonitors(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[MonitorSvc::applyEntityMonitors]bad format:", jsonString, 1);
	
	string uid = root.getv("uid", "");
	int tp = root.getv("tp", 0);
	string id = root.getv("id", "");
    
	char gmt0now[20];
	GmtNow(gmt0now);

    MySql* psql = CREATE_MYSQL;
    char sql[1024 * 10] = "";
    sprintf (sql, "SELECT camera_id FROM t20_camera_entity WHERE entity_type = '%d' AND entity_param = '%s'", tp, id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
    set<int> cidSet;
	while (psql->NextRow())
	{
		int camera_id;
		READMYSQL_INT(camera_id, camera_id, -1);
		cidSet.insert(camera_id);
	}

	if (!cidSet.size())
	{
		RELEASE_MYSQL_RETURN(psql, 0);
	}

	int len;
	len = sprintf (sql, "SELECT camera_id FROM t20_camera_viewer WHERE user_id = '%s' AND camera_id IN (", uid.c_str());
	for (set<int>::iterator iTer = cidSet.begin(); iTer != cidSet.end(); iTer++)
	{
		len += sprintf (sql + len, iTer == cidSet.begin()?"'%d'":",'%d'", (*iTer));
	}
	len += sprintf (sql + len, ")");
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while (psql->NextRow())
	{
		int tmpCid;
		READMYSQL_INT(camera_id, tmpCid, -1);
		set<int>::iterator iTer;
		if ((iTer = cidSet.find(tmpCid)) != cidSet.end())
			cidSet.erase(iTer);
	}

	if (!cidSet.size())
	{
		RELEASE_MYSQL_RETURN(psql, 0);
	}

    //len = sprintf (sql, "REPLACE INTO t20_camera_apply (USER_ID, CAMERA_ID, APPLY_DT, ACCEPT, UPDATE_DT) VALUES ");
	len = sprintf (sql, "REPLACE INTO t41_authen_apply (USER_ID, BUSTYPE, BUSID, APPLY_DT, ACCEPT, UPDATE_DT) VALUES ");
	for (set<int>::iterator iTer = cidSet.begin(); iTer != cidSet.end(); iTer++)
	{
		len += sprintf (sql + len, iTer == cidSet.begin()?"('%s', '3', '%d', '%s', 0, '%s')":",('%s', '3', '%d', '%s', 0, '%s')", uid.c_str(), (*iTer),gmt0now,gmt0now);
	}
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    RELEASE_MYSQL_RETURN(psql, 0);
}

//zhuxj
//{tp:0,id:"2011120800",ip:"212.199.11.112", num:}
int MonitorSvc::updEntityMonitorsIP(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[MonitorSvc::applyEntityMonitors]bad format:", jsonString, 1);
	
	int tp = root.getv("tp", 0);
	string id = root.getv("id", "");
	string ip = root.getv("ip", "");
	int num = root.getv("num", 0);
    
	char gmt0now[20];
	GmtNow(gmt0now);

    MySql* psql = CREATE_MYSQL;

	char sql[1024 * 3] = "";
	int cnum = 0;
	sprintf (sql, "SELECT COUNT(1) AS num FROM t20_camera AS t1, t20_camera_entity AS t2 WHERE t1.camera_id = t2. camera_id AND t2.ENTITY_TYPE = '%d' AND t2.ENTITY_PARAM = '%s' AND t1.IS_SEIPC = '1'", tp, id.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
	READMYSQL_INT(num, cnum, 0);

	char admin[64] = "";
	sprintf (sql, "SELECT USER_ID FROM t41_seipc_user WHERE SEIPC_ID = '%s' and IS_VALID = '1'", pUid);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if (psql->NextRow())
	{
		READMYSQL_STR(USER_ID, admin);
	}

	if (!strlen(admin)) strcpy(admin, pUid);

	if (cnum)
	{
		sprintf (sql, "UPDATE t20_camera AS t1, t20_camera_entity AS t2 SET t1.ADDRESS = '%s', t1.user_id = '%s', t1.UPDATE_DT = '%s', t2.UPDATE_DT = '%s' WHERE t1.CAMERA_ID = t2.CAMERA_ID AND t2.ENTITY_TYPE = '%d' AND t2.ENTITY_PARAM = '%s' AND t1.IS_SEIPC = '1'", ip.c_str(), admin,gmt0now,gmt0now, tp, id.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}
	else
	{
		int len = sprintf (sql, "INSERT INTO t20_camera (t20_camera.NAME, user_id, PROTOCOL, PROTOCOL_TYPE, ADDRESS, WIDTH, HEIGHT, IS_PUBLIC, update_DT, URL, `port`, IS_SEIPC) VALUES");																										\
		for (int i = 1; i <= num; i++)
		{
			if (i > 1) 
				len += sprintf (sql + len, ",");
			len += sprintf (sql + len, " ('c_%d_%d', '%s', '0', '0', '%s', '640', '480', '0', '%s', '/', '2000%d', '1')", tp, i, admin, ip.c_str(), gmt0now,i);
		}
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		//int camera_id = psql->GetInsertId();
		vector<int> camera_ids;		
		sprintf (sql, "SELECT camera_id FROM t20_camera ORDER BY camera_id DESC LIMIT %d", num);
		
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
		while (psql->NextRow())
		{
			int tmpId;
			READMYSQL_INT(camera_id, tmpId, -1);
			if (tmpId != -1)
				camera_ids.push_back(tmpId);
		}
		double longitude = 0.0, latitude = 90.0;
		char name[100] = "";

		switch(tp)
		{
			case 0:
			{
				sprintf(sql, "SELECT t2.name, longitude, latitude FROM aisdb.t41_ais_ship_realtime AS t1, t41_ship AS t2 WHERE t1.mmsi = t2.mmsi AND SHIPID = '%s'", id.c_str());
			}
			break;
			case 1:
			{
				sprintf (sql, "SELECT t41_port,name, LONGITUDE_dd as longitude, LATITUDE_dd as latitude FROM t41_port WHERE PortID = '%s'", id.c_str());
			}
			break;
			case 2:
			{
				sprintf (sql, "SELECT t2.name, LONGITUDE_dd as longitude, LATITUDE_dd as latitude FROM t41_port AS t1, t41_port_berth AS t2 WHERE t1.PortID = t2.PortID AND t2.Berth_id = '%s'", id.c_str());
			}
			break;
			case 3:
			{
				Tokens longlat = StrSplit(id, ",");
				longitude = atof(longlat[0].c_str());
				latitude = atof(longlat[1].c_str());
			}
			break;
		}

		if (tp >=0 && tp <= 2)
		{
			CHECK_MYSQL_STATUS(psql->Query(sql), 3);
			if (psql->NextRow())
			{
				READMYSQL_STR(name, name);
				READMYSQL_DOUBLE(longitude, longitude, 0.0);
				READMYSQL_DOUBLE(latitude, latitude, 90.0);
			}
		}

		len = sprintf (sql, "REPLACE INTO t20_camera_entity (CAMERA_ID, ENTITY_TYPE, ENTITY_PARAM, ENTITY_NAME, UPDATE_DT,  LONGITUDE,  LATITUDE) VALUES");
		
		for (int i = 0; i < (int)camera_ids.size(); i++)
		{
			if (i)
				len += sprintf(sql + len, ",");
			len += sprintf (sql + len, " (%d, %d, '%s', '%s',  '%s', %f, %f)", camera_ids[i], tp, id.c_str(), name, gmt0now,longitude, latitude);		
		}
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		len = sprintf (sql, "REPLACE INTO t20_camera_viewer (USER_ID, CAMERA_ID, UPDATE_DT) VALUES");

		for (int i = 0; i < (int)camera_ids.size(); i++)
		{
			if (i)
				len += sprintf(sql + len, ",");
			len += sprintf (sql + len, "  ('%s', %d, '%s'), ('%s', %d, '%s')", pUid, camera_ids[i], gmt0now, admin, camera_ids[i], gmt0now);		
		}
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}

    RELEASE_MYSQL_RETURN(psql, 0);
}
