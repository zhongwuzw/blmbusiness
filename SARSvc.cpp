#include "blmcom_head.h"
#include "SARSvc.h"
#include "MessageService.h"
#include "ObjectPool.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "Util.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#define NOTNULL_0(p)			((p) ? (p) : "0")

using namespace std;

IMPLEMENT_SERVICE_MAP(SARSvc)
SARSvc::SARSvc()
{
}

SARSvc::~SARSvc()
{

}



bool SARSvc::Start()
{
	if(!g_MessageService::instance()->RegisterCmd(MID_SAR, this))
		return false;

	SERVICE_MAP(0x01,SARSvc,getAccidents);
	SERVICE_MAP(0x02,SARSvc,modifyAccident);
	SERVICE_MAP(0x03,SARSvc,getAccident);
	SERVICE_MAP(0x04,SARSvc,deleteAccident);
	SERVICE_MAP(0x05,SARSvc,getLKP);
	SERVICE_MAP(0x06,SARSvc,modifyLKP);
	SERVICE_MAP(0x07,SARSvc,deleteLKP);
	SERVICE_MAP(0x08,SARSvc,modifyForecast);
	SERVICE_MAP(0x09,SARSvc,deleteForecast);
	SERVICE_MAP(0x0a,SARSvc,getForecast);
	SERVICE_MAP(0x0b,SARSvc,getForecastTrack);
	SERVICE_MAP(0x0c,SARSvc,modifyForecastTrack);
	SERVICE_MAP(0x0d,SARSvc,customWind);
	SERVICE_MAP(0x13,SARSvc,GetSRUListInfo);
	SERVICE_MAP(0x14,SARSvc,GetONSRUInfo);
	SERVICE_MAP(0x15,SARSvc,AddOrEditSRUInfo);
	SERVICE_MAP(0x16,SARSvc,DeleteONSRUInfo);
	SERVICE_MAP(0x82,SARSvc,EditOrAddPlanTable);
	SERVICE_MAP(0x83,SARSvc,GetRescuePlanTable);
	SERVICE_MAP(0x84,SARSvc,DeleteRescuePlan);
	SERVICE_MAP(0x90,SARSvc,GetRescueShipStatistic);
	SERVICE_MAP(0x91,SARSvc,GetRescuePlaneStatistic);

	DEBUG_LOG("[SARSvc::Start] OK......................................");
	return true;
} 

std::string SARSvc::getAccidentID()
{
	static int _id = 0;
	if(_id == 0) {
		const char* sql = "select max(accident_id+0) as maxid from t41_sar_accident";
		MySql* psql = CREATE_MYSQL;
		if(psql->Query(sql) && psql->NextRow())
			_id = atoi(NOTNULL(psql->GetField("maxid")))+1;
		else
			_id = 1;
		RELEASE_MYSQL_NORETURN(psql);
	} else {
		_id++;
	}
	return FormatString("%d", _id);
}

std::string SARSvc::getLkpID()
{
	static int _id = 0;
	if(_id == 0) {
		const char* sql = "select max(lkp_id+0) as maxid from t41_sar_accident_lkp";
		MySql* psql = CREATE_MYSQL;
		if(psql->Query(sql) && psql->NextRow())
			_id = atoi(NOTNULL(psql->GetField("maxid")))+1;
		else
			_id = 1;
		RELEASE_MYSQL_NORETURN(psql);
	} else {
		_id++;
	}
	return FormatString("%d", _id);
}

std::string SARSvc::getForecastID()
{
	static int _id = 0;
	if(_id == 0) {
		const char* sql = "select max(forcast_id+0) as maxid from t41_sar_forcast_task";
		MySql* psql = CREATE_MYSQL;
		if(psql->Query(sql) && psql->NextRow())
			_id = atoi(NOTNULL(psql->GetField("maxid")))+1;
		else
			_id = 1;
		RELEASE_MYSQL_NORETURN(psql);
	} else {
		_id++;
	}
	return FormatString("%d", _id);
}

std::string SARSvc::getPersionID(const std::string& accid)
{
	static int _id = 0;
	if(_id == 0) {
		const char* sql = FormatString("select max(person_id+0) as maxid from t41_sar_accident_waterperson where accident_id='%s'", accid.c_str()).c_str();
		MySql* psql = CREATE_MYSQL;
		if(psql->Query(sql) && psql->NextRow())
			_id = atoi(NOTNULL(psql->GetField("maxid")))+1;
		else
			_id = 1;
		RELEASE_MYSQL_NORETURN(psql);
	} else {
		_id++;
	}
	return FormatString("%d", _id);
}

// { seq:¡±¡±,versionid:¡±¡±,startdt:133000000,enddt:134000000}
int SARSvc::getAccidents(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getAccidents]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	string vid = root.getv("versionid", "");
	int startdt = root.getv("startdt", 0);
	int enddt = root.getv("enddt", 0);
	if(startdt > enddt) return 1;

	char sql[1024];
	sprintf(sql, "select accident_id,accident_name,pos_type,position,acctime,status,urgency,IS_DANGER_CAGO,alert_time,alert_man,alert_phone from t41_sar_accident where version_id='%s' and (acctime between %d and %d) order by acctime desc", vid.c_str(), startdt, enddt);
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	bool bFirst = true;
	out << FormatString("{seq:\"%s\",accs:[", seq.c_str());
	while(psql->NextRow())
	{
		if(bFirst)
			bFirst = false;
		else
			out << ",";
		out << FormatString("{accid:\"%s\",accnm:\"%s\",status:%s,acctm:%s,pos_tp:%s,pos:\"%s\",ur:%s,isdanger:%s,alerttm:%s,alertman:\"%s\",phone:\"%s\"}",
							psql->GetField("accident_id"),
							NOTNULL(psql->GetField("accident_name")),
							NOTNULL_0(psql->GetField("status")),
							NOTNULL_0(psql->GetField("acctime")),
							NOTNULL_0(psql->GetField("pos_type")),
							NOTNULL(psql->GetField("position")),
							NOTNULL_0(psql->GetField("urgency")),
							NOTNULL_0(psql->GetField("IS_DANGER_CAGO")),
							NOTNULL_0(psql->GetField("alert_time")),
							NOTNULL(psql->GetField("alert_man")),
							NOTNULL(psql->GetField("alert_phone")));
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

typedef struct _SAR_PERSON
{
	char name[64];
	char country[64];
	int age;
	int gender;
	char contact[1024];
	char phone[64];
	int status;
	char remark[1024];

	string toString() {
		return FormatString("{na:\"%s\",cry:\"%s\",age:%d,gender:%d,contact:\"%s\",phone:\"%s\",status:%d,rmk:\"%s\"}", name, country, age, gender, contact, phone, status, remark);
	}
} SAR_PERSON;
typedef vector<SAR_PERSON> PersonList;

typedef struct _SAR_OBJ
{
	char type[64];
	int num;

	string toString() {
		return FormatString("{tp:\"%s\",num:%d}", type, num);
	}
} SAR_OBJ;
typedef vector<SAR_OBJ> ObjList;

typedef struct _SAR_SHIP
{
	char id[64];
	char name[64];
	char mmsi[64];

	string toString() {
		return FormatString("{id:\"%s\",na:\"%s\",mmsi:\"%s\"}", id, name, mmsi);
	}
} SAR_SHIP;
typedef vector<SAR_SHIP> ShipList;

typedef struct _SAR_ACCIDENT
{
	char accid[64];
	char accnm[1024];
	int acctm;
	int status;
	int pos_tp;
	char pos[1024];
	char waterObjDesc[1024];
	char sceneDesc[1024];
	char cargoDesc[1024];
	char alertMan[64];
	char alertPhone[64];
	int alerttm;
	int urgent;
	int cargoDanger;
	char cargoName[64];
	char cargoNum[10];
	int sartm;

	ShipList ships;
	ObjList objs;
	PersonList persons;

	string toString(const string& seq) {
		stringstream out;
		out << FormatString("{seq:\"%s\",accid:\"%s\",accnm:\"%s\",acctm:%d,status:%d,sartm:%d,pos_tp:%d,pos:\"%s\",waterobjdes:\"%s\",scenedes:\"%s\",cargodes:\"%s\",alertman:\"%s\",phone:\"%s\",alerttm:%d,ur:%d,isdanger:%d,cargo_na:\"%s\",cargo_num:\"%s\",",
							seq.c_str(), accid, accnm, acctm, status, sartm, pos_tp, pos, waterObjDesc, sceneDesc, cargoDesc, alertMan, alertPhone, alerttm, urgent,cargoDanger, cargoName, cargoNum);
		out << "ships:[";
		for(int i=0; i<(int)ships.size(); i++) {
			if(i!=0) out << ",";
			out << ships[i].toString();
		}
		out << "],objs:[";
		for(int i=0; i<(int)objs.size(); i++) {
			if(i!=0) out << ",";
			out << objs[i].toString();
		}
		out << "],persons:[";
		for(int i=0; i<(int)persons.size(); i++) {
			if(i!=0) out << ",";
			out << persons[i].toString();
		}
		out << "]}";

		return out.str();
	}
} SAR_ACCIDENT;

typedef struct _SAR_FORECAST_OBJ
{
	char type[10];
	int count;
	string toString() {
		return FormatString("{type:\"%s\",count:%d}", type, count);
	}
} SAR_FORECAST_OBJ;
typedef vector<SAR_FORECAST_OBJ> ForecastObjList;

typedef struct _SAR_FORECAST
{
	char accid[64];
	char lkpid[64];
	char forcastid[64];
	char fnm[64];
	string rmk;

	int st,cal_st,sc, sim_period, step, cal_interval, iscalerr;
	double draft_err, spd_err, wind_err, curr_err;

	int loctp;
	double loc_err, reck_dist,dr_err;

	ForecastObjList objs;

	string toString() {
		stringstream out;
		out << FormatString("{forcastid:\"%s\",fnm:\"%s\",lkpid:\"%s\",\
							st:%d,cal_st:%d,\
							sc:%d,sim_period:%d,step:%d,cal_interval:%d,\
							draft_err:%.2f,iscalerr:%d, spd_err:%.2f, wind_err:%.2f,curr_err:%.2f,\
							pos_err:{loctp:%d,loc_err:%.2f,reck_dist:%.2f,dr_err:%.2f},rmk:\"%s\",objs:[",
							forcastid, fnm, lkpid, st,cal_st,sc, sim_period, step, cal_interval, draft_err, iscalerr, spd_err, wind_err, curr_err,
							loctp, loc_err, reck_dist, dr_err,rmk.c_str());
		for(int i=0; i<(int)objs.size(); i++) {
			if(i!=0) out << ",";
			out << objs[i].toString();
		}
		out << "]}";
		return out.str();
	}

} SAR_FORECAST;

/*
{ seq:¡±¡±,
bflag:1,accid:¡±¡±,versionid:¡±¡±, accnm:¡±¡±,acctm:1330000,status:0,pos_tp:0,pos:¡±¡±,waterobjdes:¡±¡±, scenedes:¡±¡±, cargodes:¡±¡±,alertman:¡±¡±,phone:¡±¡±,alerttm:1333000,ur:0,isdanger:0,cargo_na:¡±¡±,cargo_num:¡±¡±,
ships:[{id:¡±¡±,na:¡±¡±,mmsi:¡±¡±},{¡­}]
objs:[{tp:¡±¡±,num:3},{}]
persons:[{na:¡±¡±,cry:¡±¡±,age:22,gender:0,contact:¡±¡±,phone:¡±¡±,status:0,rmk:¡±¡±},{}]
}
*/
int SARSvc::modifyAccident(const char* pUid, const char* jsonString, std::stringstream& out)
{
	SAR_ACCIDENT accident;

	JSON_PARSE_RETURN("[SARSvc::modifyAccident]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	int bFlag = root.getv("bflag", 0);
	string vid = root.getv("versionid", "");
	strcpy(accident.accid, root.getv("accid", ""));
	strcpy(accident.accnm, root.getv("accnm", ""));
	accident.acctm = root.getv("acctm", 0);
	accident.status = root.getv("status", 0);
	accident.pos_tp = root.getv("pos_tp", 0);
	strcpy(accident.pos, root.getv("pos", ""));
	strcpy(accident.waterObjDesc, root.getv("waterobjdes", ""));
	strcpy(accident.sceneDesc, root.getv("scenedes", ""));
	strcpy(accident.cargoDesc, root.getv("cargodes", ""));
	strcpy(accident.alertMan, root.getv("alertman", ""));
	strcpy(accident.alertPhone, root.getv("phone", ""));
	accident.alerttm = root.getv("alerttm", 0);
	accident.urgent = root.getv("ur", 0);
	accident.cargoDanger = root.getv("isdanger", 0);
	strcpy(accident.cargoName, root.getv("cargo_na", ""));
	strcpy(accident.cargoNum, root.getv("cargo_num", ""));
	Json* jShips = root.get("ships");
	Json* jObjs = root.get("objs");
	Json* jPersons = root.get("persons");
	for(int i=0; i<jShips->size(); i++) {
		SAR_SHIP ship;
		strcpy(ship.id, jShips->get(i)->getv("id", ""));
		strcpy(ship.name, jShips->get(i)->getv("na", ""));
		strcpy(ship.mmsi, jShips->get(i)->getv("mmsi", ""));
		accident.ships.push_back(ship);
	}
	for(int i=0; i<jObjs->size(); i++) {
		SAR_OBJ obj;
		strcpy(obj.type, jObjs->get(i)->getv("tp", ""));
		obj.num = jObjs->get(i)->getv("num", 0);
		accident.objs.push_back(obj);
	}
	for(int i=0; i<jPersons->size(); i++) {
		SAR_PERSON person;
		strcpy(person.name, jPersons->get(i)->getv("na", ""));
		strcpy(person.country, jPersons->get(i)->getv("cry", ""));
		person.age = jPersons->get(i)->getv("age", 0);;
		person.gender = jPersons->get(i)->getv("gender", 0);
		strcpy(person.contact, jPersons->get(i)->getv("contact", ""));
		strcpy(person.phone, jPersons->get(i)->getv("phone", ""));
		person.status = jPersons->get(i)->getv("status", 0);
		strcpy(person.remark, jPersons->get(i)->getv("rmk", ""));
		accident.persons.push_back(person);
	}

	vector<string> sqlList;
	if(bFlag == 0) {
		string accid = getAccidentID();
		strcpy(accident.accid, accid.c_str());
	}

	char sql[1024*5];
	// t41_sar_accident
	sprintf(sql, "replace into t41_sar_accident(ACCIDENT_ID, ACCIDENT_NAME, VERSION_ID, POS_TYPE, POSITION, ACCTIME, STATUS, WATEROBJ_DESC, SCENE_DESC, CARGO_DESC, IS_DANGER_CAGO, CARGO_NAME, CARGO_NUM, URGENCY, ALERT_MAN, ALERT_PHONE, ALERT_TIME, IS_VALID) values \
					('%s', '%s', '%s', %d, '%s', %d, %d, '%s', '%s', '%s', %d, '%s', '%s', %d, '%s', '%s', %d, 1)",
					accident.accid, accident.accnm, vid.c_str(), accident.pos_tp, accident.pos, accident.acctm, accident.status, accident.waterObjDesc, accident.sceneDesc, accident.cargoDesc, accident.cargoDanger, accident.cargoName, accident.cargoNum, accident.urgent, accident.alertMan, accident.alertPhone, accident.alerttm);
	sqlList.push_back(sql);

	// t41_sar_accident_ship
	sqlList.push_back(FormatString("delete from t41_sar_accident_ship where accident_id='%s'", accident.accid));
	if ((int)accident.ships.size()>0)
	{
		strcpy(sql, "replace into t41_sar_accident_ship(ACCIDENT_ID, SHIP_ID, SHIP_NAME, MMSI) values ");
		for(int i=0; i<(int)accident.ships.size(); i++) {
			if(i!=0) strcat(sql, ",");
			strcat(sql, FormatString("('%s','%s','%s','%s')", accident.accid, accident.ships[i].id, accident.ships[i].name, accident.ships[i].mmsi).c_str());
		}
		sqlList.push_back(sql);
	}
	

	// t41_sar_accident_waterobj
	sqlList.push_back(FormatString("delete from t41_sar_accident_waterobj where accident_id='%s'", accident.accid));
	if ((int)accident.objs.size()>0)
	{
		strcpy(sql, "replace into t41_sar_accident_waterobj(ACCIDENT_ID, OBJ_TYPE, OBJ_NUM) values ");
		for(int i=0; i<(int)accident.objs.size(); i++) {
			if(i!=0) strcat(sql, ",");
			strcat(sql, FormatString("('%s','%s','%d')", accident.accid, accident.objs[i].type, accident.objs[i].num).c_str());
		}
		sqlList.push_back(sql);
	}
	

	// t41_sar_accident_waterperson
	sqlList.push_back(FormatString("delete from t41_sar_accident_waterperson where accident_id='%s'", accident.accid));
	if ((int)accident.persons.size()>0)
	{
		strcpy(sql, "replace into t41_sar_accident_waterperson(ACCIDENT_ID,PERSON_ID,NAME,COUNTRY,AGE,GENDER,CONTACT,CONTACT_TEL,STATUS,REMARK) values ");
		for(int i=0; i<(int)accident.persons.size(); i++) {
			if(i!=0) strcat(sql, ",");
			strcat(sql, FormatString("('%s','%s','%s','%s',%d,%d,'%s','%s',%d,'%s')", 
				accident.accid, getPersionID(accident.accid).c_str(), accident.persons[i].name, accident.persons[i].country, 
				accident.persons[i].age, accident.persons[i].gender, accident.persons[i].contact, accident.persons[i].phone, 
				accident.persons[i].status, accident.persons[i].remark).c_str());
		}
		sqlList.push_back(sql);
	}
	

	MySql* psql = CREATE_MYSQL;
	for(int i=0; i<(int)sqlList.size(); i++) {
		psql->Execute(sqlList[i].c_str());
	}

	out << FormatString("{eid:0,seq:\"%s\",accid:\"%s\"}", seq.c_str(), accident.accid);
	RELEASE_MYSQL_RETURN(psql, 0);
}

int SARSvc::getAccident(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getAccident]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	string accid = root.getv("accid", "");
	if(accid.empty()) return 1;

	SAR_ACCIDENT accident;
	strcpy(accident.accid, accid.c_str());

	MySql* psql = CREATE_MYSQL;
	char sql[1024];
	sprintf(sql, "select ACCIDENT_ID, ACCIDENT_NAME, VERSION_ID, POS_TYPE, POSITION, ACCTIME, STATUS, WATEROBJ_DESC, SCENE_DESC, CARGO_DESC, IS_DANGER_CAGO, CARGO_NAME, CARGO_NUM, URGENCY, ALERT_MAN, ALERT_PHONE, ALERT_TIME from t41_sar_accident where accident_id='%s'", accid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	if(psql->NextRow()) {
		strcpy(accident.accnm, NOTNULL(psql->GetField("ACCIDENT_NAME")));
		accident.acctm = atoi(NOTNULL(psql->GetField("ACCTIME")));
		accident.status = atoi(NOTNULL(psql->GetField("STATUS")));
		accident.pos_tp = atoi(NOTNULL(psql->GetField("POS_TYPE")));
		strcpy(accident.pos, NOTNULL(psql->GetField("POSITION")));
		strcpy(accident.waterObjDesc, NOTNULL(psql->GetField("WATEROBJ_DESC")));
		strcpy(accident.sceneDesc, NOTNULL(psql->GetField("SCENE_DESC")));
		strcpy(accident.cargoDesc, NOTNULL(psql->GetField("CARGO_DESC")));
		strcpy(accident.alertMan, NOTNULL(psql->GetField("ALERT_MAN")));
		strcpy(accident.alertPhone, NOTNULL(psql->GetField("ALERT_PHONE")));
		accident.alerttm = atoi(NOTNULL(psql->GetField("ALERT_TIME")));
		accident.urgent = atoi(NOTNULL(psql->GetField("URGENCY")));
		accident.cargoDanger = atoi(NOTNULL(psql->GetField("IS_DANGER_CAGO")));
		strcpy(accident.cargoName, NOTNULL(psql->GetField("CARGO_NAME")));
		strcpy(accident.cargoNum, NOTNULL(psql->GetField("CARGO_NUM")));
	}

	sprintf(sql, "select ACCIDENT_ID, SHIP_ID, SHIP_NAME, MMSI from t41_sar_accident_ship where accident_id = '%s'", accid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow()) {
		SAR_SHIP ship;
		strcpy(ship.id, NOTNULL(psql->GetField("SHIP_ID")));
		strcpy(ship.name, NOTNULL(psql->GetField("SHIP_NAME")));
		strcpy(ship.mmsi, NOTNULL(psql->GetField("MMSI")));
		accident.ships.push_back(ship);
	}

	sprintf(sql, "select ACCIDENT_ID, OBJ_TYPE, OBJ_NUM from t41_sar_accident_waterobj where accident_id = '%s'", accid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow()) {
		SAR_OBJ obj;
		strcpy(obj.type, NOTNULL(psql->GetField("OBJ_TYPE")));
		obj.num = atoi(NOTNULL(psql->GetField("OBJ_NUM")));
		accident.objs.push_back(obj);
	}

	sprintf(sql, "select ACCIDENT_ID,PERSON_ID,NAME,COUNTRY,AGE,GENDER,CONTACT,CONTACT_TEL,STATUS,REMARK from t41_sar_accident_waterperson where accident_id = '%s'", accid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow()) {
		SAR_PERSON person;
		strcpy(person.name, NOTNULL(psql->GetField("NAME")));
		strcpy(person.country, NOTNULL(psql->GetField("COUNTRY")));
		person.age = atoi(NOTNULL(psql->GetField("AGE")));
		person.gender = atoi(NOTNULL(psql->GetField("GENDER")));
		strcpy(person.contact, NOTNULL(psql->GetField("CONTACT")));
		strcpy(person.phone, NOTNULL(psql->GetField("CONTACT_TEL")));
		person.status = atoi(NOTNULL(psql->GetField("STATUS")));
		strcpy(person.remark, NOTNULL(psql->GetField("REMARK")));
		accident.persons.push_back(person);
	}

	out << accident.toString(seq);
	RELEASE_MYSQL_RETURN(psql, 0);
}

int SARSvc::deleteAccident(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::deleteAccident]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	string accid = root.getv("accid", "");
	if(accid.empty()) return 1;

	MySql* psql = CREATE_MYSQL;

	char sql[1024];
	sprintf(sql, "delete from t41_sar_accident_waterperson where accident_id = '%s'", accid.c_str());
	psql->Execute(sql);
	sprintf(sql, "delete from t41_sar_accident_waterobj where accident_id = '%s'", accid.c_str());
	psql->Execute(sql);
	sprintf(sql, "delete from t41_sar_accident_ship where accident_id = '%s'", accid.c_str());
	psql->Execute(sql);
	sprintf(sql, "delete from t41_sar_accident where accident_id = '%s'", accid.c_str());
	psql->Execute(sql);

	out << FormatString("{seq:\"%s\",eid:0}", seq.c_str());
	RELEASE_MYSQL_RETURN(psql, 0);
}

int SARSvc::getLKP(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getLKP]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	string accid = root.getv("accid", "");
	string vid = root.getv("versionid", "");
	if(accid.empty()) return 1;

	char sql[1024];
	sprintf(sql, "select t1.LKP_ID,t1.ACCIDENT_ID,t1.POS_TYPE,t1.POSITION,t1.LKP_TIME,t1.REPORT_TIME,t1.REPORTER,t1.REMARK,t2.forcast_id from t41_sar_accident_lkp t1 left join t41_sar_forcast_task t2 on t1.lkp_id = t2.lkp_id where t1.accident_id = '%s'", accid.c_str());

	bool bFirst = true;
	out << FormatString("{seq:\"%s\",accid:\"%s\",lkps:[", seq.c_str(), accid.c_str());
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow()) {
		if(!bFirst) out << ","; bFirst = false;
		out << FormatString("{lkpid:\"%s\",seetime:%s,reporttime:%s,reportor:\"%s\",description:\"%s\",points:\"%s\",forcastid:\"%s\",areaid:\"%s\"}",
							NOTNULL(psql->GetField("LKP_ID")),
							NOTNULL_0(psql->GetField("LKP_TIME")),
							NOTNULL_0(psql->GetField("REPORT_TIME")),
							NOTNULL(psql->GetField("REPORTER")),
							NOTNULL(psql->GetField("REMARK")),
							NOTNULL(psql->GetField("POSITION")),
							NOTNULL(psql->GetField("forcast_id")),
							"0");
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

// {flag:¡±¡±,lkpid:¡±¡±,accid:"",seetime:133333,reporttime:1333333,reportor:¡±¡±,description:¡±¡±,points:¡±¡±}
int SARSvc::modifyLKP(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::modifyLKP]bad format:", jsonString, 1);
	string flag = root.getv("flag", "");
	string lkpid = root.getv("lkpid", "");
	string accid = root.getv("accid", "");
	int seetime = root.getv("seetime", 0);
	int reporttime = root.getv("reporttime", 0);
	string reportor = root.getv("reportor", "");
	string description = root.getv("description", "");
	string points = root.getv("points", "");
	if(accid.empty()) return 1;
	if(lkpid.empty()) lkpid = getLkpID();

	MySql* psql = CREATE_MYSQL;
	psql->Execute(FormatString("update t41_sar_forcast_task set cal_status='1' where lkp_id='%s'", lkpid.c_str()).c_str());
	psql->Execute(FormatString("insert into t41_sar_accident_lkp(LKP_ID,ACCIDENT_ID,POSITION,LKP_TIME,REPORT_TIME,REPORTER,REMARK) values ('%s','%s','%s',%d,%d,'%s','%s')\
							   on duplicate key update POSITION=values(POSITION),LKP_TIME=values(LKP_TIME),REPORT_TIME=values(REPORT_TIME),REPORTER=values(REPORTER),REMARK=values(REMARK)",
							   lkpid.c_str(), accid.c_str(), points.c_str(), seetime, reporttime, reportor.c_str(), description.c_str()).c_str());

	out << FormatString("{eid:0,lkpid:\"%s\"}", lkpid.c_str());
	RELEASE_MYSQL_RETURN(psql, 0);
}

int SARSvc::deleteLKP(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::deleteLKP]bad format:", jsonString, 1);
	string seq = root.getv("seq", "");
	string lkpid = root.getv("lkpid", "");
	string vid = root.getv("versionid", "");
	if(lkpid.empty()) return 1;

	MySql* psql = CREATE_MYSQL;
	psql->Execute(FormatString("delete from t41_sar_forcast_task where lkp_id='%s'", lkpid.c_str()).c_str());
	psql->Execute(FormatString("delete from t41_sar_accident_lkp where lkp_id='%s'", lkpid.c_str()).c_str());

	out << FormatString("{seq:\"%s\",eid:0,lkpid:\"%s\"}", seq.c_str(), lkpid.c_str());
	RELEASE_MYSQL_RETURN(psql, 0);
}

/*
{
bflag:0,accid:¡±ac10001¡±,lkpid:¡±¡±,forcastid:¡±¡± , fnm:¡±Ô¤²âÃû³Æ1¡± ,
sc:1,sim_period:24,step:3,cal_interval:20,
draft_err:3.0, iscalerr:1,spd_err:1.0,wind_err:1.0,curr_err:1.0
pos_err: {loctp:1,loc_err:1.0,reck_dist:500.0,dr_err:0.5},
objs:[
{type:¡±001¡±,count:5},¡­¡­
]
}
*/
int SARSvc::modifyForecast(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::modifyForecast]bad format:", jsonString, 1);
	int bflag = root.getv("bflag", 0);
	string accid = root.getv("accid", "");
	string lkpid = root.getv("lkpid", "");
	string forcastid = root.getv("forcastid", ""); if(forcastid.empty()) forcastid = getForecastID();
	int st = root.getv("st", 0);
	int cal_st = root.getv("cal_st", 1);
	string fnm = root.getv("fnm", "");
	int sc = root.getv("sc", 0);
	int sim_period = root.getv("sim_period", 0);
	int step = root.getv("step", 0);
	int cal_interval = root.getv("cal_interval", 0);
	int iscalerr = root.getv("iscalerr", 0);
	double draft_err = root.getv("draft_err", 0.0);
	double spd_err = root.getv("spd_err", 0.0);
	double wind_err = root.getv("wind_err", 0.0);
	double curr_err = root.getv("curr_err", 0.0);
	Json* json_pos_err = root.get("pos_err");
	int loctp = json_pos_err->getv("loctp", 0);
	double loc_err = json_pos_err->getv("loc_err", 0.0);
	double reck_dist = json_pos_err->getv("reck_dist", 0.0);
	double dr_err = json_pos_err->getv("dr_err", 0.0);
	string rmk = root.getv("rmk","");
	vector<string> sqlList;

	char sql[2048];
	sprintf(sql, "replace into t41_sar_forcast_task(FORCAST_ID,LKP_ID,ACCIDENT_ID,FORCAST_NAME,STATUS,CAL_STATUS,SCENE,SIMULATION_PERIOD,CAL_STEP,OUTPUT_INTERVAL,\
				 LOCATE_TYPE,LOCATE_ERROR,RECKON_DISTANCE,DR_ERROR,DRAFT_ERROR,ISCAL_ERROR,SPEEDD_ERROR,WIND_ERROR,CURRENT_ERROR,REMARK) values \
				 ('%s','%s','%s','%s','%d','%d','%d','%d','%d','%d','%d','%.2f','%.2f','%.2f','%.2f','%d','%.2f','%.2f','%.2f','%s') ", 
				 forcastid.c_str(),lkpid.c_str(),accid.c_str(),fnm.c_str(),st,cal_st,sc,sim_period,step,cal_interval,
				 loctp,loc_err,reck_dist,dr_err,draft_err,iscalerr,spd_err,wind_err,curr_err,rmk.c_str());
	sqlList.push_back(sql);
	sqlList.push_back(FormatString("DELETE FROM t41_sar_forcast_target WHERE FORCAST_ID='%s'", forcastid.c_str()));

	Json* jObjs = root.get("objs");
	if(jObjs->size()>0) {
		strcpy(sql, "insert into t41_sar_forcast_target(FORCAST_ID,obj_type,obj_count,is_valid) values ");
		for(int i=0; i<jObjs->size(); i++) {
			if(i!=0) strcat(sql, ",");
			strcat(sql, FormatString("('%s','%s','%d','1')", forcastid.c_str(), jObjs->get(i)->getv("type",""), jObjs->get(i)->getv("count",0)).c_str());
		}
	}
	sqlList.push_back(sql);
	
	MySql* psql = CREATE_MYSQL;
	for(int i=0; i<(int)sqlList.size(); i++)
		psql->Execute(sqlList[i].c_str());
	out << FormatString("{eid:0,forcastid:\"%s\"}", forcastid.c_str());
	RELEASE_MYSQL_RETURN(psql, 0);
}

int SARSvc::deleteForecast(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::deleteForecast]bad format:", jsonString, 1);
	string forcastid = root.getv("forcastid", "");
	if(forcastid.empty()) return 1;

	MySql* psql = CREATE_MYSQL;
	psql->Execute(FormatString("DELETE FROM t41_sar_forcast_target WHERE FORCAST_ID='%s'", forcastid.c_str()).c_str());
	psql->Execute(FormatString("DELETE FROM t41_sar_forcast_trace WHERE FORCAST_ID='%s'", forcastid.c_str()).c_str());
	psql->Execute(FormatString("DELETE FROM t41_sar_forcast_task WHERE FORCAST_ID='%s'", forcastid.c_str()).c_str());
	out << FormatString("{eid:0,forcastid:\"%s\"}", forcastid.c_str());
	RELEASE_MYSQL_RETURN(psql, 0);
}

int SARSvc::getForecast(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getForecast]bad format:", jsonString, 1);
	string forcastid = root.getv("forcastid", "");
	if(forcastid.empty()) return 1;

	SAR_FORECAST forecast;
	strcpy(forecast.forcastid, forcastid.c_str());

	MySql* psql = CREATE_MYSQL;
	char sql[1024];
	sprintf(sql, "select FORCAST_ID,LKP_ID,ACCIDENT_ID,FORCAST_NAME,STATUS,CAL_STATUS,SCENE,SIMULATION_PERIOD,CAL_STEP,OUTPUT_INTERVAL,\
				 LOCATE_TYPE,LOCATE_ERROR,RECKON_DISTANCE,DR_ERROR,DRAFT_ERROR,ISCAL_ERROR,SPEEDD_ERROR,WIND_ERROR,CURRENT_ERROR,REMARK from t41_sar_forcast_task where FORCAST_ID='%s'", forcastid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)&&psql->NextRow(), 3);
	strcpy(forecast.accid,NOTNULL(psql->GetField("FORCAST_ID")));
	strcpy(forecast.lkpid,NOTNULL(psql->GetField("LKP_ID")));
	strcpy(forecast.fnm,NOTNULL(psql->GetField("FORCAST_NAME")));
	forecast.st = atoi(NOTNULL(psql->GetField("STATUS")));
	forecast.cal_st = atoi(NOTNULL(psql->GetField("CAL_STATUS")));
	forecast.sc = atoi(NOTNULL(psql->GetField("SCENE")));
	forecast.sim_period = atoi(NOTNULL(psql->GetField("SIMULATION_PERIOD")));
	forecast.step =atoi(NOTNULL(psql->GetField("CAL_STEP")));
	forecast.cal_interval = atoi(NOTNULL(psql->GetField("OUTPUT_INTERVAL")));
	forecast.iscalerr = atoi(NOTNULL(psql->GetField("ISCAL_ERROR")));
	forecast.draft_err = atof(NOTNULL(psql->GetField("DRAFT_ERROR")));
	forecast.spd_err = atof(NOTNULL(psql->GetField("SPEEDD_ERROR")));
	forecast.wind_err = atof(NOTNULL(psql->GetField("WIND_ERROR")));
	forecast.curr_err = atof(NOTNULL(psql->GetField("CURRENT_ERROR")));
	forecast.loctp = atoi(NOTNULL(psql->GetField("LOCATE_TYPE")));
	forecast.loc_err = atof(NOTNULL(psql->GetField("LOCATE_ERROR")));
	forecast.reck_dist = atof(NOTNULL(psql->GetField("RECKON_DISTANCE")));
	forecast.dr_err = atof(NOTNULL(psql->GetField("DR_ERROR")));
	forecast.rmk = psql->GetField("REMARK");

	sprintf(sql, "select FORCAST_ID,obj_type,obj_count from t41_sar_forcast_target where FORCAST_ID='%s'", forcastid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow()) {
		SAR_FORECAST_OBJ obj;
		strcpy(obj.type, NOTNULL(psql->GetField("obj_type")));
		obj.count = atoi(NOTNULL(psql->GetField("obj_count")));
		forecast.objs.push_back(obj);
	}

	out << forecast.toString();
	RELEASE_MYSQL_RETURN(psql, 0);
}

typedef struct _SAR_FORECAST_POINT {
	int time;
	double x, y;
	string toString() {
		return FormatString("{tm:%d,x:\"%.4f\",y:\"%.4f\"}", time, x, y);
	}
} SAR_FORECAST_POINT;
typedef vector<SAR_FORECAST_POINT> ForecastPtList;

typedef struct _SAR_FORECAST_TRACK {
	char trackid[64];
	ForecastPtList ptList;
	string toString() {
		int starttm, endtm;
		if(ptList.empty()) { starttm = endtm = 0;}
		else { starttm = ptList[0].time; endtm = ptList.rbegin()->time; }

		stringstream ss;
		ss << FormatString("{trackid:\"%s\",startm:%d,endtm:%d,points:[", trackid, starttm, endtm);
		for(int i=0; i<(int)ptList.size(); i++) {
			if(i!=0) ss << ",";
			ss << ptList[i].toString();
		}
		ss << "]}";
		return ss.str();
	}
} SAR_FORECAST_TRACK;
typedef map<string, SAR_FORECAST_TRACK> ForecastTrackMap;

typedef struct _SAR_FORECAST_OBJ_TRACKS {
	char objtype[64];
	ForecastTrackMap trackMap;
	string toString() {
		stringstream ss;
		ss << FormatString("{objtype:\"%s\",tracks:[", objtype);
		for(ForecastTrackMap::iterator iter=trackMap.begin(); iter!=trackMap.end(); iter++) {
			if(iter!=trackMap.begin()) ss << ",";
			ss << iter->second.toString();
		}
		ss << "]}";
		return ss.str();
	}
} SAR_FORECAST_OBJ_TRACKS;

int SARSvc::getForecastTrack(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::getForecastTrack]bad format:", jsonString, 1);
	string forcastid = root.getv("forcastid", "");
	if(forcastid.empty()) return 1;

	map<string, SAR_FORECAST_OBJ_TRACKS> objMap;
	map<string, SAR_FORECAST_OBJ_TRACKS>::iterator objIter;

	char sql[1024];
	sprintf(sql, "SELECT FORCAST_ID,ANALYSES_TM,TRACK_ID,OBJ_TYPE,LATITUDE,LONGITUDE FROM t41_sar_forcast_trace WHERE FORCAST_ID='%s' ORDER BY obj_type,track_id,analyses_tm", forcastid.c_str());
	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	while(psql->NextRow()) {
		SAR_FORECAST_POINT pt;
		pt.time = atoi(NOTNULL(psql->GetField("ANALYSES_TM")));
		pt.x = atof(NOTNULL(psql->GetField("LONGITUDE")));
		pt.y = atof(NOTNULL(psql->GetField("LATITUDE")));
		string trackid = NOTNULL(psql->GetField("TRACK_ID"));
		string objtype = NOTNULL(psql->GetField("OBJ_TYPE"));
		
		objIter = objMap.find(objtype);
		if(objIter == objMap.end()) {
			SAR_FORECAST_OBJ_TRACKS objTracks;
			strcpy(objTracks.objtype, objtype.c_str());

			SAR_FORECAST_TRACK track;
			strcpy(track.trackid, trackid.c_str());
			track.ptList.push_back(pt);

			objTracks.trackMap[trackid] = track;
			objMap[objtype] = objTracks;
		} else {
			SAR_FORECAST_OBJ_TRACKS& objTracks = objIter->second;
			ForecastTrackMap::iterator trackIter = objTracks.trackMap.find(trackid);
			if(trackIter == objTracks.trackMap.end()) {
				SAR_FORECAST_TRACK track;
				strcpy(track.trackid, trackid.c_str());
				track.ptList.push_back(pt);

				objTracks.trackMap[trackid] = track;
			} else {
				trackIter->second.ptList.push_back(pt);
			}
		}
	}

	out << "[";
	for(objIter=objMap.begin(); objIter!=objMap.end(); objIter++) {
		if(objIter != objMap.begin()) out << ",";
		out << objIter->second.toString();
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);
}

/*
{forcastid :¡±¡± ,objtype:¡±001¡±, 
tracks:[
{trackid:¡±¡±
points:[ 
{tm: 1373009711,x:¡±¡±,y:¡±¡± },
{ tm: 1373003311,x:¡±¡±,y:¡±¡± }
]
},
{trackid:¡±¡±,¡­}
]
}
*/
int SARSvc::modifyForecastTrack(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::modifyForecastTrack]bad format:", jsonString, 1);
	string forcastid = root.getv("forcastid", "");
	string objtype = root.getv("objtype", "");

	int now = (int)time(NULL);
	int total = 0;
	stringstream ss;
	ss << "insert into t41_sar_forcast_trace(FORCAST_ID,ANALYSES_TM,TRAKC_ID,OBJ_TYPE,LATITUDE,LONGITUDE,IS_VALID) values ";
	Json* jTracks = root.get("tracks");
	for(int i=0; i<jTracks->size(); i++) {
		string trackid = jTracks->get(i)->getv("trackid", "");
		Json* jPts = jTracks->get(i)->get("points");
		for(int j=0; j<jPts->size(); j++) {
			int tm = jPts->get(j)->getv("tm", 0);
			double x = jPts->get(j)->getv("x", 0.0);
			double y = jPts->get(j)->getv("y", 0.0);
			if(total != 0) ss << ","; total++;
			ss << FormatString("('%s',%d,'%s','%s','%.4f','%.4f','1')", forcastid.c_str(), now, trackid.c_str(), objtype.c_str(), y, x);
		}
	}

	MySql* psql = CREATE_MYSQL;
	psql->Execute(FormatString("delete from t41_sar_forcast_trace where FORCAST_ID='%s' and OBJ_TYPE='%s'", forcastid.c_str(), objtype.c_str()).c_str());
	psql->Execute(ss.str().c_str());

	out << "{eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

/*
{forcastid :¡±¡± , 
data:[ 
{starttm: 1373009711,end tm: 1373003311
wd: 1,ws:1.0,cd:1,cs:1.0 },
{¡­}
]
}
*/
int SARSvc::customWind(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::customWind]bad format:", jsonString, 1);
	string forcastid = root.getv("forcastid", "");

	int total = 0;
	stringstream ss;
	ss << "insert into T41_SAR_FORCAST_MANUAL_WC(FORCAST_ID,FROM_TM,TO_TIME,WIND_COURSE,WIND_SPEED,CURRENT_COURSE,CURRENT_SPEED) values ";
	Json* jData = root.get("data");
	for(int i=0; i<jData->size(); i++) {
		if(i!=0) ss << ",";
		int starttm = jData->get(i)->getv("starttm", 0);
		int endtm = jData->get(i)->getv("endtm", 0);
		int wd = jData->get(i)->getv("wd", 0);
		double ws = jData->get(i)->getv("ws", 0.0);
		int cd = jData->get(i)->getv("cd", 0);
		double cs = jData->get(i)->getv("cs", 0.0);
		ss << FormatString("('%s',%d,%d,%d,%.4f,%d,%.4f)", forcastid.c_str(), starttm, endtm, wd, ws, cd, cs);
	}

	MySql* psql = CREATE_MYSQL;
	psql->Execute(ss.str().c_str());

	out << "{eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

int SARSvc::DeleteSRU(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::DeleteSRU]bad format:", jsonString, 1);

	string sruid=root.getv("sruid","");

	MySql* psql = CREATE_MYSQL;

	char delsql[512]="";
	sprintf(delsql,"delete from boloomodb.t41_sar_sru where SRU_ID ='%s'",sruid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(delsql)>=0, 3);

	out<<"{eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//2282
int SARSvc::EditOrAddPlanTable(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::EditOrAddPlanTable]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strVid=root.getv("versionId","");
	int strFlag=root.getv("flag",0);
	string strPlanid=root.getv("planid","");
	int strState = root.getv("state",0);
	string strAccid=root.getv("accid","");
	string strForcastid=root.getv("forcastId","");
	string strDatum_latlons=root.getv("datum","");
	string strDatumbegin=root.getv("datumBegin","");
	string strDatumend=root.getv("datumEnd","");
	double strZoneangle=root.getv("zoneAngle",0.0);
	double strZonewidth=root.getv("zoneWidth",0.0);
	double strZoneheight=root.getv("zoneHeight",0.0);
	string strCircles=root.getv("circles","");
	int strWeather=root.getv("weather",0);
	double strVisibility=root.getv("visibility",0.0);
	int strFatigue=root.getv("fatigue",0);
	int strSartype=root.getv("sarType",0);
	double strSearchdistance=root.getv("searchDistance",0.0);
	double strZoneArea=root.getv("zoneArea",0.0);
	string strZonepoints=root.getv("zonePoints","");
	double strLegdistance=root.getv("legDistance",0.0);
	double strLegangle=root.getv("legAngle",0.0);
	double strLeggap=root.getv("legGap",0.0);
	int strSearchtime=root.getv("searchTime",0);
	int strReturn=root.getv("return",0);
	int strBoatnum=root.getv("boatNum",0);
	int strTurnnum=root.getv("turnNum",0);
	int strDirection=root.getv("direction",0);
	int strEnterdirection=root.getv("enterDirection",0);

	MySql* psql = CREATE_MYSQL;

	char sql[1024]="";

	string strTemp = "SAR";
	long ltime=0;
	ltime=time(0);
	char szTmp[32];
	sprintf(szTmp,"%d",ltime);
	strTemp+=szTmp;

	char buf[10];
	sprintf(buf,"%d",strState);
	string strState1 = buf;

	char buf1[10];
	sprintf(buf1,"%d",strWeather);
	string strWeather1 = buf1;

	char buf2[10];
	sprintf(buf2,"%d",strSartype);
	string strSartype1 = buf2;

	string datumFinal=strDatumbegin+'|'+strDatumend;

	if(strFlag == 0)
	{
		sprintf(sql,"INSERT INTO boloomodb.T41_SAR_PLAN_TABLE VALUES('%s','%s','%s','%s','%s','%s',%f,'%s',%d,%f,%f,%f,'%s','%s',%f,%f,%f,%d,%d,%d,%d,%d,0.0,'',%d,'%s')",strVid.c_str(),strTemp.c_str(),strForcastid.c_str(),strAccid.c_str(),strState1.c_str(),strDatum_latlons.c_str(),strVisibility,strWeather1.c_str(),strFatigue,strZoneangle,strZonewidth,strZoneheight,strCircles.c_str(),strSartype1.c_str(),strLegdistance,strLeggap,strLegangle,strDirection,strTurnnum,strSearchtime,strBoatnum,strReturn,strEnterdirection,datumFinal.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
	}
	else
	{
		sprintf(sql,"UPDATE boloomodb.T41_SAR_PLAN_TABLE \
					SET FORCAST_ID='%s',ACCIDENT_ID='%s',STATUS='%s',DATUM_LATLONS='%s', \
					VISIBILITY=%f,WEATHER='%s'FATIGUE_FACTOR=%d,ZONE_ANGLE=%f,ZONE_WIDTH=%f, \
					ZONE_HEIGHT=%f,CIRCLES='%s',TYPE='%s',LEG_LENGTH=%f,TRACK_SPACE=%f, \
					LEG_ANGLE=%f,COURSE=%d,LEG_NUM=%d,SEARCH_NUM=%d,SHIP_NUM=%d,IS_BACK=%d, \
					ENTER_DIRECTION=%d,DATUM='%s' \
					WHERE PLAN_ID='%s'",strForcastid.c_str(),strAccid.c_str(),strState1.c_str(),strDatum_latlons.c_str(),strVisibility,strWeather1.c_str(),strFatigue,strZoneangle,strZonewidth,strZoneheight,strCircles.c_str(),strSartype1.c_str(),strLegdistance,strLeggap,strLegangle,strDirection,strTurnnum,strSearchtime,strBoatnum,strReturn,strEnterdirection,datumFinal.c_str(),strPlanid.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
	}

	if(strFlag == 0)
		out<<"{eid:0,planid:\""<<strTemp<<"\"}";
	else
		out<<"{eid:0,planid:\""<<strPlanid<<"\"}";

// 	long ltime1=0;
// 	ltime1=time(0);
// 
// 	Tokens rescueVec = StrSplit(strRescuestatus,"|");
// 	for(Tokens::iterator it=rescueVec.begin();it!=rescueVec.end();it++)
// 	{
// 		Tokens rescueFinalVec=StrSplit(*it,",");
// 		sprintf(sql,"UPDATE boloomodb.T41_SAR_ACCIDENT_WATEROBJ \
// 					SET t1.OBJ_NUM=%d WHERE ACCIDENT_ID='%s' AND OBJ_TYPE='%s'",strAccid.c_str(),rescueFinalVec[0].c_str());
// 
// 		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
// 		
// 		sprintf(sql,"INSERT INTO boloomodb.T41_SAR_RESCUE_STUFF \
// 					VALUES('%s','%s','%s','%s')")
// 
// 	}

	RELEASE_MYSQL_RETURN(psql, 0);
}



//2283
int SARSvc::GetRescuePlanTable(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::GetRescuePlanTable]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strPlanid=root.getv("planid","");
	string strVid=root.getv("versionid","");


	MySql* psql = CREATE_MYSQL;

	string plan_id="";
	string status="";
	string datum_latlons="";
	double zone_angle=0.0;
	double zone_width=0.0;
	double zone_height=0.0;
	string circles="";
	string weather="";
	double visibility=0.0;
	int fatigue_factor=0;
	string type="";
	double leg_length=0.0;
	double leg_angle=0.0;
	double track_space=0.0;
	int search_num=0;
	int is_back=0;
	int ship_num=0;
	int leg_num=0;
	int course=0;
	int enter_direction=0;
	string accident_id="";
	string forcast_id="";
	string datum="";

	char sql[1024]="";

	sprintf(sql,"SELECT PLAN_ID,STATUS,DATUM_LATLONS,ZONE_ANGLE,ZONE_WIDTH, \
				ZONE_HEIGHT,CIRCLES,WEATHER,VISIBILITY,FATIGUE_FACTOR,TYPE, \
				LEG_LENGTH,LEG_ANGLE,TRACK_SPACE,SEARCH_NUM,IS_BACK,SHIP_NUM, \
				LEG_NUM,COURSE,ENTER_DIRECTION,ACCIDENT_ID,FORCAST_ID,DATUM \
				FROM boloomodb.T41_SAR_PLAN_TABLE \
				WHERE PLAN_ID='%s' AND VERSION_ID='%s'",strPlanid.c_str(),strVid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	if(psql->NextRow())
	{
		READMYSQL_STRING(PLAN_ID,plan_id);
		READMYSQL_STRING(STATUS,status);
		READMYSQL_STRING(DATUM_LATLONS,datum_latlons);
		READMYSQL_STRING(CIRCLES,circles);
		READMYSQL_STRING(WEATHER,weather);
		READMYSQL_STRING(TYPE,type);
		READMYSQL_DOUBLE(ZONE_ANGLE,zone_angle,0.0);
		READMYSQL_DOUBLE(ZONE_WIDTH,zone_width,0.0);
		READMYSQL_DOUBLE(ZONE_HEIGHT,zone_height,0.0);
		READMYSQL_DOUBLE(VISIBILITY,visibility,0.0);
		READMYSQL_DOUBLE(LEG_LENGTH,leg_length,0.0);
		READMYSQL_DOUBLE(LEG_ANGLE,leg_angle,0.0);
		READMYSQL_DOUBLE(TRACK_SPACE,track_space,0.0);
		READMYSQL_INT(FATIGUE_FACTOR,fatigue_factor,0);
		READMYSQL_INT(SEARCH_NUM,search_num,0);
		READMYSQL_INT(IS_BACK,is_back,0);
		READMYSQL_INT(SHIP_NUM,ship_num,0);
		READMYSQL_INT(COURSE,course,0);
		READMYSQL_INT(ENTER_DIRECTION,enter_direction,0);
		READMYSQL_STRING(ACCIDENT_ID,accident_id);
		READMYSQL_STRING(FORCAST_ID,forcast_id);
		READMYSQL_STRING(DATUM,datum);
	}

	string datumBegin="";
	string datumEnd="";

	DEBUG_LOG("1");
	
	Tokens datumVec = StrSplit(datum,"|");
	DEBUG_LOG("3");

	if(datumVec.size() == 2)
	{	
		datumBegin = datumVec[0];
		datumEnd = datumVec[1];
	}

	DEBUG_LOG("2");


	out<<"{planid:\""<<plan_id<<"\",state:"<<status<<",datum:\""<<datum_latlons<<"\",datumBegin:\""<<datumBegin<<"\",datumEnd:\""<<datumEnd<<"\",zoneAngle:"<<zone_angle<<",zoneWidth:"<<zone_width<<",zoneHeight:"<<zone_height<<",circles:\""<<circles<<"\",weather:"<<weather<<",visibility:"<<visibility<<",fatigue:"<<fatigue_factor<<",sarType:"<<type<<",legDistance:"<<leg_length<<",legAngle:"<<leg_angle<<",legGap:"<<track_space<<",searchTime:"<<search_num<<",return:"<<is_back<<",boatNum:"<<ship_num<<",turnNum:"<<leg_num<<",direction:"<<course<<",enterDirection:"<<enter_direction<<",";

	sprintf(sql,"SELECT MAX(ETA) AS ETA \
				FROM boloomodb.T41_SAR_STRENGTH_DEPLOY \
				WHERE PLAN_ID='%s'",strPlanid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	int eta=0;
	if(psql->NextRow())
	{
		READMYSQL_INT(ETA,eta,0);
	}

	out<<"planStart:"<<eta<<",";

	sprintf(sql,"SELECT MAX(ETD) AS ETD \
				FROM boloomodb.T41_SAR_STRENGTH_DEPLOY \
				WHERE PLAN_ID='%s'",strPlanid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	int etd=0;
	if(psql->NextRow())
	{
		READMYSQL_INT(ETD,etd,0);
	}

	out<<"planEnd:"<<etd<<",";

	sprintf(sql,"SELECT MAX(ATA) AS ATA \
				FROM boloomodb.T41_SAR_STRENGTH_DEPLOY \
				WHERE PLAN_ID='%s'",strPlanid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	int ata=0;
	if(psql->NextRow())
	{
		READMYSQL_INT(ATA,ata,0);
	}

	out<<"actualStart:"<<ata<<",";

	sprintf(sql,"SELECT MAX(ATD) AS ATD \
				FROM boloomodb.T41_SAR_STRENGTH_DEPLOY \
				WHERE PLAN_ID='%s'",strPlanid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	int atd=0;
	if(psql->NextRow())
	{
		READMYSQL_INT(ATD,atd,0);
	}

	out<<"actualEnd:"<<atd<<",";

	string accident_name="";
	sprintf(sql,"SELECT ACCIDENT_NAME FROM boloomodb.T41_SAR_ACCIDENT \
				WHERE ACCIDENT_ID='%s'",accident_id.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
	if(psql->NextRow())
	{
		READMYSQL_STRING(ACCIDENT_NAME,accident_name);
	}
	
	string forcast_name="";
	sprintf(sql,"SELECT FORCAST_NAME FROM boloomodb.T41_SAR_FORCAST_TASK \
				WHERE FORCAST_ID='%s'",forcast_id.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
	if(psql->NextRow())
	{
		READMYSQL_STRING(FORCAST_NAME,forcast_name);
	}

	out<<"accid:\""<<accident_id<<"\",accName:\""<<accident_name<<"\",forcastid:\""<<forcast_id<<"\",forcastName:\""<<forcast_name<<"\",";

	sprintf(sql,"SELECT COUNT(*) AS COUNT_NUM FROM boloomodb.T41_SAR_STRENGTH_BOAT t1 \
				JOIN boloomodb.T41_SAR_STRENGTH_DEPLOY t2 \
				ON t1.DEPLOY_ID=t2.DEPLOY_ID \
				JOIN  boloomodb.T41_SAR_PLAN_TABLE t3 \
				ON t3.PLAN_ID=t2.PLAN_ID \
				WHERE t2.STATE=1 AND t3.PLAN_ID='%s'",strPlanid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	int currentSarBoatNum = 0;
	if(psql->NextRow())
	{
		READMYSQL_INT(COUNT_NUM,currentSarBoatNum,0);
	}

	sprintf(sql,"SELECT COUNT(*) AS COUNT_NUM FROM boloomodb.T41_SAR_STRENGTH_BOAT t1 \
				JOIN boloomodb.T41_SAR_STRENGTH_DEPLOY t2 \
				ON t1.DEPLOY_ID=t2.DEPLOY_ID \
				JOIN  boloomodb.T41_SAR_PLAN_TABLE t3 \
				ON t3.PLAN_ID=t2.PLAN_ID \
				WHERE t3.PLAN_ID='%s'",strPlanid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	int totalSarBoatNum = 0;
	if(psql->NextRow())
	{
		READMYSQL_INT(COUNT_NUM,totalSarBoatNum,0);
	}

	out<<"currentSarBoatNum:"<<currentSarBoatNum<<",totalSarBoatNum:"<<totalSarBoatNum<<",";

	sprintf(sql,"SELECT COUNT(*) AS COUNT_NUM FROM boloomodb.T41_SAR_STRENGTH_PLANE t1 \
				JOIN boloomodb.T41_SAR_STRENGTH_DEPLOY t2 \
				ON t1.DEPLOY_ID=t2.DEPLOY_ID \
				JOIN  boloomodb.T41_SAR_PLAN_TABLE t3 \
				ON t3.PLAN_ID=t2.PLAN_ID \
				WHERE t2.STATE=1 AND t3.PLAN_ID='%s'",strPlanid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	int currentSarPlaneNum = 0;
	if(psql->NextRow())
	{
		READMYSQL_INT(COUNT_NUM,currentSarPlaneNum,0);
	}

	sprintf(sql,"SELECT COUNT(*) AS COUNT_NUM FROM boloomodb.T41_SAR_STRENGTH_PLANE t1 \
				JOIN boloomodb.T41_SAR_STRENGTH_DEPLOY t2 \
				ON t1.DEPLOY_ID=t2.DEPLOY_ID \
				JOIN  boloomodb.T41_SAR_PLAN_TABLE t3 \
				ON t3.PLAN_ID=t2.PLAN_ID \
				WHERE t3.PLAN_ID='%s'",strPlanid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	int totalSarPlaneNum = 0;
	if(psql->NextRow())
	{
		READMYSQL_INT(COUNT_NUM,totalSarPlaneNum,0);
	}

	out<<"currentSarPlaneNum:"<<currentSarPlaneNum<<",totalSarPlaneNum:"<<totalSarPlaneNum<<",";

	sprintf(sql,"SELECT SUM(PEOPLE_NUM) AS PEOPLE_NUM FROM boloomodb.T41_SAR_STRENGTH_DEPLOY \
				WHERE PLAN_ID='%s' AND STATE=1",strPlanid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	int currentSarPersonNum = 0;
	if(psql->NextRow())
	{
		READMYSQL_INT(PEOPLE_NUM,currentSarPersonNum,0);
	}

	sprintf(sql,"SELECT SUM(PEOPLE_NUM) AS PEOPLE_NUM FROM boloomodb.T41_SAR_STRENGTH_DEPLOY \
				WHERE PLAN_ID='%s'",strPlanid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	int totalSarPersonNum = 0;
	if(psql->NextRow())
	{
		READMYSQL_INT(PEOPLE_NUM,totalSarPersonNum,0);
	}

	out<<"currentSarPersonNum:"<<currentSarPersonNum<<",totalSarPersonNum:"<<totalSarPersonNum<<",";

	sprintf(sql,"SELECT t1.OBJ_TYPE,t1.OBJ_NUM FROM \
				boloomodb.T41_SAR_ACCIDENT_WATEROBJ t1 \
				JOIN boloomodb.T41_SAR_ACCIDENT t2 \
				ON t1.ACCIDENT_ID=t2.ACCIDENT_ID \
				JOIN boloomodb.T41_SAR_PLAN_TABLE t3 \
				ON t3.ACCIDENT_ID=t2.ACCIDENT_ID \
				WHERE t3.PLAN_ID='%s'",strPlanid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	string fall_id="";
	int obj_num=0;
	int idx = 0;

	out<<"rescueStatus:\"";

	DEBUG_LOG("3");


	while (psql->NextRow())
	{
		READMYSQL_STRING(OBJ_TYPE,fall_id);
		READMYSQL_INT(OBJ_NUM,obj_num,0);

		if(idx>0)
			out<<"|";
		
		int quantity=0;
		sprintf(sql,"SELECT QUANTITY FROM \
					boloomodb.T41_SAR_RESCUE_STUFF \
					WHERE FALL_ID='%s' AND PLAN_ID='%s'",fall_id.c_str(),strPlanid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
		if(psql->NextRow())
		{
			READMYSQL_INT(QUANTITY,quantity,0);
		}
		out<<fall_id<<","<<obj_num<<","<<quantity;

	}
	out<<"\"]";
	
	RELEASE_MYSQL_RETURN(psql, 0);
}

//2284
int SARSvc::DeleteRescuePlan(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::DeleteRescuePlan]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strPlanid=root.getv("planid","");

	MySql* psql = CREATE_MYSQL;

	char sql[1024]="";

	sprintf(sql,"DELETE FROM boloomodb.T41_SAR_PLAN_TABLE \
				WHERE PLAN_ID='%s'",strPlanid.c_str());

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

	out<<"{eid:0,planid:\""<<strPlanid<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//2290
int SARSvc::GetRescueShipStatistic(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::GetRescueShipStatistic]bad format:", jsonString, 1);
	
	string strSeq=root.getv("seq","");
	string strCompany=root.getv("place","");
	string strShipKind=root.getv("shipKind","");
	string strVid=root.getv("vid","");

   
	MySql* psql = CREATE_MYSQL;

	char sql[1024]="";
	std::stringstream shipSql;

	shipSql<<" WHERE PLACE IN (";
	if(strCompany.empty())
	{
		sprintf(sql,"SELECT PLACE FROM boloomodb.T41_RESCUE_BOAT_STATISTIC");
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
		int idx=0;
		char company_name[30];
		while(psql->NextRow())
		{
			if(idx>0)
				shipSql<<",";
			idx++;
			READMYSQL_STR(PLACE,company_name);
			shipSql<<"'"<<company_name<<"'";
		}
	}
	else
	{
		
		shipSql<<"'"<<strCompany<<"'";
	}

	shipSql<<")";
	
	std::stringstream shipKindSql;
	shipKindSql<<" AND SHIP_KIND IN (";
//	shipKindSql<<" WHERE SHIP_KIND IN (";
	if(strShipKind.empty())
	{
		sprintf(sql,"SELECT SHIP_KIND FROM boloomodb.T41_RESCUE_BOAT_STATISTIC");
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
		int idx = 0;
		char ship_kind[30];
		while (psql->NextRow())
		{
			if(idx>0)
				shipKindSql<<",";
			idx++;
			READMYSQL_STR(SHIP_KIND,ship_kind);
			shipKindSql<<"'"<<ship_kind<<"'";
		}
	}
	else
	{
		shipKindSql<<"'"<<strShipKind<<"'";
	}
	shipKindSql<<")";

	sprintf(sql,"SELECT SHIPID,MMSI,PLACE,COMPANY,SHIP_NA,SHIP_AREA,SHIP_KIND,SHIP_LENGTH, \
 				TOTAL_WEIGH,SHIP_POWER,VELOCITY,RESISTANCE,WIND_RATE, \
 				FIRE_EQUIP,COMMUNICATE_EQUIP,LOCATION,REMARK \
 				FROM T41_RESCUE_BOAT_STATISTIC \
 				%s %s AND VERSION_ID='%s'",shipSql.str().c_str(),shipKindSql.str().c_str(),strVid.c_str());
//	sprintf(sql,"SELECT PLACE,COMPANY,SHIP_NA,SHIP_AREA,SHIP_KIND,SHIP_LENGTH, \
//							TOTAL_WEIGH,SHIP_POWER,VELOCITY,RESISTANCE,WIND_RATE, \
//							FIRE_EQUIP,COMMUNICATE_EQUIP,LOCATION,REMARK \
//							FROM T41_RESCUE_BOAT_STATISTIC %s",shipKindSql.str().c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	DEBUG_LOG(sql);
	
	string shipid="";
	string place="";
	string company="";
	string ship_na="";
	string ship_area="";
	string ship_kind="";
	double ship_length=0;
	double total_weigh=0;
	double ship_power=0;
	double velocity=0.0;
	int resistance=0;
	string wind_rate="";
	string fire_equip="";
	string communicate_equip="";
	string location="";
	string remark="";
	string tel="";
	string mmsi="";

	out<<"{seq:\""<<strSeq<<"\",info:[";
	int idx=0;

	while(psql->NextRow())
	{
		DEBUG_LOG("sa");
		if(idx>0)
			out<<",";
		  idx++;
		  READMYSQL_STRING(SHIPID,shipid);
		  READMYSQL_STRING(MMSI,mmsi);
		  READMYSQL_STRING(PLACE,place);
		  READMYSQL_STRING(COMPANY,company);
		  READMYSQL_STRING(SHIP_NA,ship_na);
		  READMYSQL_STRING(SHIP_AREA,ship_area);
		  READMYSQL_STRING(SHIP_KIND,ship_kind);
		  READMYSQL_DOUBLE(SHIP_LENGTH,ship_length,0.0);
		  READMYSQL_DOUBLE(TOTAL_WEIGH,total_weigh,0.0);
		  READMYSQL_DOUBLE(SHIP_POWER,ship_power,0.0);
		  READMYSQL_DOUBLE(VELOCITY,velocity,0.0);
		  READMYSQL_INT(RESISTANCE,resistance,0);
		  READMYSQL_STRING(WIND_RATE,wind_rate);
		  READMYSQL_STRING(FIRE_EQUIP,fire_equip);
		  READMYSQL_STRING(COMMUNICATE_EQUIP,communicate_equip);
		  READMYSQL_STRING(LOCATION,location);
		  READMYSQL_STRING(REMARK,remark);
		  READMYSQL_STRING(TEL,tel);

		  out<<"{id:\""<<shipid<<"\",mmsi:\""<<mmsi<<"\",place:\""<<place<<"\",comp:\""<<company<<"\",na:\""<<ship_na<<"\",area:\""<<ship_area<<"\",kind:\""<<ship_kind<<"\",ship_length:"<<ship_length<<",total_weigh:"<<total_weigh<<",ship_power:"<<ship_power<<",velocity:"<<velocity<<",resistance:"<<resistance<<",wind_rate:\""<<wind_rate<<"\",fire_equip:\""<<fire_equip<<"\",communicate_equip:\""<<communicate_equip<<"\",location:\""<<location<<"\",remark:\""<<remark<<"\",tel:\""<<tel<<"\"}";

	}
	
	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//2291
int SARSvc::GetRescuePlaneStatistic(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::GetRescuePlaneStatistic]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strCompany=root.getv("place","");
	string strPlaneModel=root.getv("planeModel","");
	string strVid=root.getv("vid","");


	MySql* psql = CREATE_MYSQL;

	char sql[1024]="";
	std::stringstream shipSql;

	shipSql<<"WHERE PLACE IN (";
	if(strCompany.empty())
	{
		sprintf(sql,"SELECT PLACE FROM boloomodb.T41_RESCUE_PLANE_STATISTIC");
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
		int idx=0;
		char company_name[30];
		while(psql->NextRow())
		{
			if(idx>0)
				shipSql<<",";
			idx++;
			READMYSQL_STR(PLACE,company_name);
			shipSql<<"'"<<company_name<<"'";
		}
	}
	else
	{

		shipSql<<"'"<<strCompany<<"'";
	}

	shipSql<<")";

	std::stringstream shipKindSql;
	shipKindSql<<" AND PLANE_MODEL IN (";
	if(strPlaneModel.empty())
	{
		sprintf(sql,"SELECT PLANE_MODEL FROM boloomodb.T41_RESCUE_PLANE_STATISTIC");
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);
		int idx = 0;
		char ship_kind[30];
		while (psql->NextRow())
		{
			if(idx>0)
				shipKindSql<<",";
			idx++;
			READMYSQL_STR(PLANE_MODEL,ship_kind);
			shipKindSql<<"'"<<ship_kind<<"'";
		}
	}
	else
	{
		shipKindSql<<"'"<<strPlaneModel<<"'";
	}
	shipKindSql<<") ";

	sprintf(sql,"SELECT PLANEID,PLACE,COMPANY,PLANE_NO,PLANE_MODEL,MAX_RANGE,CREW, \
				MAX_SPEED,CRUISE_SPEED,COMMUNICATION_EQUIP,LOCATION,RESCUE_EQUIP, \
				REMARK \
				FROM boloomodb.T41_RESCUE_PLANE_STATISTIC \
				%s %s ADN VERSION_ID='%s'",shipSql.str().c_str(),shipKindSql.str().c_str(),strVid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	string planeid="";
	string place="";
	string company="";
	string plane_no="";
	string plane_model="";
	int max_range=0;
	string crew="";
	int max_speed=0;
	double cruise_speed=0.0;
	string communication_equip="";
	string location="";
	string rescue_equip="";
	string remark="";
	string tel="";

	out<<"{seq:\""<<strSeq<<"\",info:[";
	int idx=0;

	while(psql->NextRow())
	{
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(PLANEID,planeid);
		READMYSQL_STRING(PLACE,place);
		READMYSQL_STRING(COMPANY,company);
		READMYSQL_STRING(PLANE_NO,plane_no);
		READMYSQL_STRING(PLANE_MODEL,plane_model);
		READMYSQL_INT(MAX_RANGE,max_range,0);
		READMYSQL_STRING(CREW,crew);
		READMYSQL_INT(MAX_SPEED,max_speed,0);
		READMYSQL_DOUBLE(CRUISE_SPEED,cruise_speed,0.0);
		READMYSQL_STRING(COMMUNICATION_EQUIP,communication_equip);
		READMYSQL_STRING(LOCATION,location);
		READMYSQL_STRING(RESCUE_EQUIP,rescue_equip);
		READMYSQL_STRING(REMARK,remark);
		READMYSQL_STRING(TEL,tel);

		out<<"{id:\""<<planeid<<"\",place:\""<<place<<"\",comp:\""<<company<<"\",plane_no:\""<<plane_no<<"\",plane_model:\""<<plane_model<<"\",max_range:"<<max_range<<",crew:\""<<crew<<"\",max_speed:"<<max_speed<<",cruise_speed:"<<cruise_speed<<",communication_equip:\""<<communication_equip<<"\",location:\""<<location<<"\",rescue_equip:\""<<rescue_equip<<"\",remark:\""<<remark<<"\",tel:\""<<tel<<"\"}";

	}

	out<<"]}";
	RELEASE_MYSQL_RETURN(psql, 0);
}

//2213
int SARSvc::GetSRUListInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::GetSRUListInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strPlanid=root.getv("planid","");

	MySql* psql = CREATE_MYSQL;

	char sql[1024]="";

	sprintf(sql,"SELECT DEPLOY_ID,BEGIN_TIME,REAL_BE_TIME,ETA,ATA,ETD,ATD,PEOPLE_NUM, \
				STATE FROM boloomodb.T41_SAR_STRENGTH_DEPLOY WHERE \
				PLAN_ID='%s'",strPlanid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\",data:[";

	int idx = 0;
	string deploy_id="";
	int begin_time=0;
	int real_be_time=0;
	int eta=0;
	int ata=0;
	int etd=0;
	int atd=0;
	int people_num=0;
	int state=0;

	while(psql->NextRow())
	{
		if(idx>0)
			out<<",";
		idx++;
		READMYSQL_STRING(DEPLOY_ID,deploy_id);
		READMYSQL_INT(BEGIN_TIME,begin_time,0);
		READMYSQL_INT(REAL_BE_TIME,real_be_time,0);
		READMYSQL_INT(ETA,eta,0);
		READMYSQL_INT(ATA,ata,0);
		READMYSQL_INT(ETD,etd,0);
		READMYSQL_INT(ATD,atd,0);
		READMYSQL_INT(PEOPLE_NUM,people_num,0);
		READMYSQL_INT(STATE,state,0);

		int final_begin_time=0;
		if(real_be_time)
			final_begin_time=real_be_time;
		else
			final_begin_time=begin_time;

		int final_ata=0;
		if(ata)
			final_ata=ata;
		else
			final_ata=eta;

		int final_atd=0;
		if(atd)
			final_atd=atd;
		else
			final_atd=etd;

		sprintf(sql,"SELECT t1.SHIPID,t1.SHIP_NA,t1.TEL,t1.COMPANY \
					FROM boloomodb.T41_RESCUE_BOAT_STATISTIC t1 \
					JOIN boloomodb.T41_SAR_STRENGTH_BOAT t2 \
					ON t1.SHIPID=t2.SHIPID \
					JOIN boloomodb.T41_SAR_STRENGTH_DEPLOY t3 \
					ON t3.DEPLOY_ID=t2.DEPLOY_ID \
					WHERE t3.DEPLOY_ID='%s'",deploy_id.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

		string shipid="";
		string ship_na="";
		string tel="";
		string company="";

		if(psql->NextRow())
		{
			READMYSQL_STRING(SHIPID,shipid);
			READMYSQL_STRING(SHIP_NA,ship_na);
			READMYSQL_STRING(TEL,tel);
			READMYSQL_STRING(COMPANY,company);

			out<<"{deployid:\""<<deploy_id<<"\",sruid:\""<<shipid<<"\",tp:0,na:\""<<ship_na<<"\",tel:\""<<tel<<"\",departm:"<<final_begin_time<<",starttm:"<<final_ata<<",endtm:"<<final_atd<<",num:"<<people_num<<",cmpid:\"\",cmpna:\""<<company<<"\",st:"<<state<<"}";
		}
		else
		{
			sprintf(sql,"SELECT t1.PLANEID,t1.PLANE_NO,t1.TEL,t1.COMPANY \
						FROM boloomodb.T41_SAR_STRENGTH_PLANE t2 \
						LEFT JOIN boloomodb.T41_RESCUE_PLANE_STATISTIC t1 \
						ON t1.PLANEID=t2.PLANEID \
						JOIN boloomodb.T41_SAR_STRENGTH_DEPLOY t3 \
						ON t3.DEPLOY_ID=t2.DEPLOY_ID \
						WHERE t3.DEPLOY_ID='%s'",deploy_id.c_str());
			CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

			string planeid="";
			string plane_no="";
			string tel="";
			string company="";

			if(psql->NextRow())
			{	
				READMYSQL_STRING(PLANEID,planeid);
				READMYSQL_STRING(PLANE_NO,plane_no);
				READMYSQL_STRING(TEL,tel);
				READMYSQL_STRING(COMPANY,company);
			}

			out<<"{deployid:\""<<deploy_id<<"\",sruid:\""<<planeid<<"\",tp:1,na:\""<<plane_no<<"\",tel:\""<<tel<<"\",departm:"<<final_begin_time<<",starttm:"<<final_ata<<",endtm:"<<final_atd<<",num:"<<people_num<<",cmpid:\"\",cmpna:\""<<company<<"\",st:"<<state<<"}";
		}
	}

	out<<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//2214
int SARSvc::GetONSRUInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::GetONSRUInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strDeployid=root.getv("deployid","");

	MySql* psql = CREATE_MYSQL;

	char sql[1024]="";

	sprintf(sql,"SELECT PLAN_ID,DEPLOY_ID,SEARCH_SPEED,BEGIN_PLACE,BEGIN_TIME,REAL_BE_TIME, \
				CSP,ETA,ATA,TSP,ETD,ATD,FLY_DISTANCE,PEOPLE_NUM,SEARCH_DISTANCE,SEARCH_AREA \
			    FROM boloomodb.T41_SAR_STRENGTH_DEPLOY WHERE \
				DEPLOY_ID='%s'",strDeployid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

	out<<"{seq:\""<<strSeq<<"\"";

	string deploy_id="";
	string plan_id="";
	double search_speed=0.0;
	string begin_place="";
	int begin_time=0;
	int real_be_time=0;
	string csp="";
	int eta=0;
	int ata=0;
	string tsp="";
	int etd=0;
	int atd=0;
	double fly_distance=0.0;
	double search_distance=0.0;
	double search_area=0.0;
	int people_num=0;

	if(psql->NextRow())
	{
		READMYSQL_STRING(PLAN_ID,plan_id);
		READMYSQL_STRING(DEPLOY_ID,deploy_id);
		READMYSQL_DOUBLE(SEARCH_SPEED,search_speed,0.0);
		READMYSQL_STRING(BEGIN_PLACE,begin_place);
		READMYSQL_INT(BEGIN_TIME,begin_time,0);
		READMYSQL_INT(REAL_BE_TIME,real_be_time,0);
		READMYSQL_STRING(CSP,csp);
		READMYSQL_INT(ETA,eta,0);
		READMYSQL_INT(ATA,ata,0);
		READMYSQL_STRING(TSP,tsp);
		READMYSQL_INT(ETD,etd,0);
		READMYSQL_INT(ATD,atd,0);
		READMYSQL_DOUBLE(FLY_DISTANCE,fly_distance,0.0);
		READMYSQL_DOUBLE(SEARCH_DISTANCE,search_distance,0.0);
		READMYSQL_DOUBLE(SEARCH_AREA,search_area,0.0);
		READMYSQL_INT(PEOPLE_NUM,people_num,0);

		sprintf(sql,"SELECT t1.SHIPID,t1.SHIP_NA,t1.TEL,t1.COMPANY,t1.VELOCITY,t2.FLAG \
					FROM boloomodb.T41_SAR_STRENGTH_BOAT t2 \
					LEFT JOIN boloomodb.T41_RESCUE_BOAT_STATISTIC t1 \
					ON t1.SHIPID=t2.SHIPID \
					JOIN boloomodb.T41_SAR_STRENGTH_DEPLOY t3 \
					ON t3.DEPLOY_ID=t2.DEPLOY_ID \
					WHERE t3.DEPLOY_ID='%s'",deploy_id.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

		string shipid="";
		string ship_na="";
		string tel="";
		string company="";
		int flag=0;
		double velocity=0;

		if(psql->NextRow())
		{
			READMYSQL_STRING(SHIPID,shipid);
			READMYSQL_STRING(SHIP_NA,ship_na);
			READMYSQL_STRING(TEL,tel);
			READMYSQL_STRING(COMPANY,company);
			READMYSQL_INT(FLAG,flag,0);
			READMYSQL_DOUBLE(VELOCITY,velocity,0.0);

			out<<",planid:\""<<plan_id<<"\",deployid:\""<<deploy_id<<"\",sruid:\""<<shipid<<"\",sruna:\""<<ship_na<<"\",srusrc:"<<flag<<",spd:"<<velocity<<",sspd:"<<search_speed<<",num:"<<people_num<<",comid:\"\",compna:\""<<company<<"\",tel:\""<<tel<<"\",dsp:\""<<begin_place<<"\",dsp_etd:"<<begin_time<<",dsp_atd:"<<real_be_time<<",csp:\""<<csp<<"\",csp_eta:"<<eta<<",csp_ata::"<<ata<<",tsp:\""<<tsp<<"\",tsp_etd:"<<etd<<",tsp_atd:"<<atd<<",dist:"<<fly_distance<<",sdist:"<<search_distance<<",sarea:"<<search_area;
		}
		else
		{
			sprintf(sql,"SELECT t1.PLANEID,t1.PLANE_NO,t1.TEL,t1.COMPANY,t1.CRUISE_SPEED \
						FROM boloomodb.T41_RESCUE_PLANE_STATISTIC t1 \
						JOIN boloomodb.T41_SAR_STRENGTH_PLANE t2 \
						ON t1.PLANEID=t2.PLANEID \
						JOIN boloomodb.T41_SAR_STRENGTH_DEPLOY t3 \
						ON t3.DEPLOY_ID=t2.DEPLOY_ID \
						WHERE t3.DEPLOY_ID='%s'",deploy_id.c_str());
			CHECK_MYSQL_STATUS(psql->Query(sql)>=0,3);

			string planeid="";
			string plane_no="";
			string tel="";
			string company="";
			double cruise_speed=0.0;

			if(psql->NextRow())
			{	
				READMYSQL_STRING(PLANEID,planeid);
				READMYSQL_STRING(PLANE_NO,plane_no);
				READMYSQL_STRING(TEL,tel);
				READMYSQL_STRING(COMPANY,company);
				READMYSQL_DOUBLE(CRUISE_SPEED,cruise_speed,0.0);
			}

			out<<",planid:\""<<plan_id<<"\",deployid:\""<<deploy_id<<"\",sruid:\""<<planeid<<"\",sruna:\""<<plane_no<<"\",srusrc:0,spd:"<<cruise_speed<<",sspd:"<<search_speed<<",num:"<<people_num<<",comid:\"\",compna:\""<<company<<"\",tel:\""<<tel<<"\",dsp:\""<<begin_place<<"\",dsp_etd:"<<begin_time<<",dsp_atd:"<<real_be_time<<",csp:\""<<csp<<"\",csp_eta:"<<eta<<",csp_ata::"<<ata<<",tsp:\""<<tsp<<"\",tsp_etd:"<<etd<<",tsp_atd:"<<atd<<",dist:"<<fly_distance<<",sdist:"<<search_distance<<",sarea:"<<search_area;
		}
	}

	out<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//2215
int SARSvc::AddOrEditSRUInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::AddOrEditSRUInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strPlanid=root.getv("planid","");
	int strBflag=root.getv("bflag",0);
	string strDeployid=root.getv("deployid","");
	string strSruid=root.getv("sruid","");
	int strSrutp=root.getv("srutp",0);
	int strSrusrc=root.getv("srusrc",0);
	double strSpd=root.getv("spd",0.0);
	double strSspd=root.getv("sspd",0.0);
	int strNum=root.getv("num",0);
	string strDsp=root.getv("dsp","");
	int strDsp_etd=root.getv("dsp_etd",0);
	int strDsp_atd=root.getv("dsp_atd",0);
	string strCsp=root.getv("csp","");
	int strCsp_eta=root.getv("csp_eta",0);
	int strCsp_ata=root.getv("csp_ata",0);
	string strTsp=root.getv("tsp","");
	int strTsp_etd=root.getv("tsp_etd",0);
	int strTsp_atd=root.getv("tsp_atd",0);
	double strDist=root.getv("dist",0.0);
	double strSdist=root.getv("sdist",0.0);
	double strSarea=root.getv("sarea",0.0);

	string strTemp = "SRU";
	long ltime=0;
	ltime=time(0);
	char szTmp[32];
	sprintf(szTmp,"%d",ltime);
	strTemp+=szTmp;

	MySql* psql = CREATE_MYSQL;

	char sql[1024]="";

	if(strBflag)
	{
		sprintf(sql,"UPDATE boloomodb.T41_SAR_STRENGTH_DEPLOY SET \
					PLAN_ID='%s',SEARCH_SPEED=%f,PEOPLE_NUM=%d, \
					BEGIN_PLACE='%s',BEGIN_TIME=%d,REAL_BE_TIME=%d, \
					CSP='%s',ETA=%d,ATA=%d,TSP='%s',ETD=%d,ATD=%d, \
					FLY_DISTANCE=%f,SEARCH_DISTANCE=%f,SEARCH_AREA=%f \
					WHERE DEPLOY_ID='%s'",strPlanid.c_str(),strSspd,strNum,strDsp.c_str(),strDsp_etd,strDsp_atd,strCsp.c_str(),strCsp_eta,strCsp_ata,strTsp.c_str(),strTsp_etd,strTsp_atd,strDist,strSdist,strSarea,strDeployid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		if(strSrutp)
		{
			sprintf(sql,"DELETE FROM boloomodb.T41_SAR_STRENGTH_PLANE \
						WHERE DEPLOY_ID='%s'",strDeployid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			sprintf(sql,"DELETE FROM boloomodb.T41_SAR_STRENGTH_BOAT \
						WHERE DEPLOY_ID='%s'",strDeployid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			sprintf(sql,"INSERT INTO boloomodb.T41_SAR_STRENGTH_PLANE \
						VALUES('%s','%s')",strDeployid.c_str(),strSruid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			sprintf(sql,"UPDATE boloomodb.T41_RESCUE_PLANE_STATISTIC \
						SET CRUISE_SPEED=%f WHERE PLANEID='%s'",strSpd,strSruid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		}
		else
		{
			sprintf(sql,"DELETE FROM boloomodb.T41_SAR_STRENGTH_PLANE \
						WHERE DEPLOY_ID='%s'",strDeployid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			sprintf(sql,"DELETE FROM boloomodb.T41_SAR_STRENGTH_BOAT \
						WHERE DEPLOY_ID='%s'",strDeployid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			sprintf(sql,"INSERT INTO boloomodb.T41_SAR_STRENGTH_BOAT \
						VALUES('%s','%s',%d)",strDeployid.c_str(),strSruid.c_str(),strSrusrc);
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
			
			if(strSrusrc == 0)
			{
				sprintf(sql,"UPDATE boloomodb.T41_RESCUE_BOAT_STATISTIC \
						SET VELOCITY=%f WHERE SHIPID='%s'",strSpd,strSruid.c_str());
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
			}
		}
	}
	else
	{
		sprintf(sql,"INSERT INTO boloomodb.T41_SAR_STRENGTH_DEPLOY VALUES('%s','%s',%f, \
					%d,%d,'%s','%s',%d,%d,'%s',%d,%d,%f,%f,%f,1,%d)",strTemp.c_str(),strPlanid.c_str(),strSspd,strDsp_etd,strDsp_atd,strDsp.c_str(),strCsp.c_str(),strCsp_eta,strCsp_ata,strTsp.c_str(),strTsp_etd,strTsp_atd,strDist,strSdist,strSarea,strNum);

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		if(strSrutp)
		{
			sprintf(sql,"INSERT INTO boloomodb.T41_SAR_STRENGTH_PLANE \
						VALUES('%s','%s')",strDeployid.c_str(),strSruid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			sprintf(sql,"UPDATE boloomodb.T41_RESCUE_PLANE_STATISTIC \
						SET CRUISE_SPEED=%f WHERE PLANEID='%s'",strSpd,strSruid.c_str());
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		}
		else
		{
			sprintf(sql,"INSERT INTO boloomodb.T41_SAR_STRENGTH_BOAT \
						VALUES('%s','%s',%d)",strDeployid.c_str(),strSruid.c_str(),strSrusrc);
			CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

			if(strSrusrc == 0)
			{
				sprintf(sql,"UPDATE boloomodb.T41_RESCUE_BOAT_STATISTIC \
							SET VELOCITY=%f WHERE SHIPID='%s'",strSpd,strSruid.c_str());
				CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
			}
		}
	}

	if(strBflag)
		out<<"{seq:\""<<strSeq<<"\",eid:0,deployid:\""<<strDeployid<<"\"}";
	else
		out<<"{seq:\""<<strSeq<<"\",eid:0,deployid:\""<<strTemp<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//2216
int SARSvc::DeleteONSRUInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::DeleteONSRUInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strDeployids=root.getv("deployids","");

	MySql* psql = CREATE_MYSQL;

	char sql[1024]="";

	Tokens deployids = StrSplit(strDeployids,"|");
	for(int i = 0;i < deployids.size();i++)
	{
		sprintf(sql,"DELETE FROM boloomodb.T41_SAR_STRENGTH_PLANE \
					WHERE DEPLOY_ID='%s'",deployids[i].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		sprintf(sql,"DELETE FROM boloomodb.T41_SAR_STRENGTH_BOAT \
					WHERE DEPLOY_ID='%s'",deployids[i].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		sprintf(sql,"DELETE FROM boloomodb.T41_SAR_STRENGTH_DEPLOY \
					WHERE DEPLOY_ID='%s'",deployids[i].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}

//2217
int SARSvc::StopONSRUInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[SARSvc::StopONSRUInfo]bad format:", jsonString, 1);

	string strSeq=root.getv("seq","");
	string strDeployids=root.getv("deployids","");

	MySql* psql = CREATE_MYSQL;

	char sql[1024]="";

	Tokens deployids = StrSplit(strDeployids,"|");
	for(int i = 0;i < deployids.size();i++)
	{
		sprintf(sql,"DELETE FROM boloomodb.T41_SAR_STRENGTH_PLANE \
					WHERE DEPLOY_ID='%s'",deployids[i].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		sprintf(sql,"DELETE FROM boloomodb.T41_SAR_STRENGTH_BOAT \
					WHERE DEPLOY_ID='%s'",deployids[i].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);

		sprintf(sql,"DELETE FROM boloomodb.T41_SAR_STRENGTH_DEPLOY \
					WHERE DEPLOY_ID='%s'",deployids[i].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0,3);
	}

	out<<"{seq:\""<<strSeq<<"\",eid:0}";

	RELEASE_MYSQL_RETURN(psql, 0);
}