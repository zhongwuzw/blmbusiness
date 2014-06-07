#include "OilSpillSvc.h"
#include "blmcom_head.h"
#include "MessageService.h"
#include "LogMgr.h"
#include "MainConfig.h"
#include "IBusinessService.h"
#include "kSQL.h"
#include "Util.h"
#include "ObjectPool.h"
#include "json.h"

OilSpillSvc::OilSpillSvc(void)
{
}

OilSpillSvc::~OilSpillSvc(void)
{
}

IMPLEMENT_SERVICE_MAP(OilSpillSvc)

bool OilSpillSvc::Start()
{
	if(!g_MessageService::instance()->RegisterCmd(MID_OILSPILL, this))
		return false;

	SERVICE_MAP(SID_OILSPILL_GET_TASKLIST,OilSpillSvc,GetOilSpillTaskList);
	SERVICE_MAP(SID_OILSPILL_GET_SUBTASKLIST,OilSpillSvc,GetOilSpillSubTaskList);
	SERVICE_MAP(SID_OILSPILL_ADDUPD_TASKPROP,OilSpillSvc,AddUpdOilSpillTask);
	SERVICE_MAP(SID_OILSPILL_GET_TASKPROP,OilSpillSvc,GetOilSpillTaskDetail);
	SERVICE_MAP(SID_OILSPILL_DEL_TASK,OilSpillSvc,DelOilSpillTask);

	SERVICE_MAP(SID_OILSPILL_GET_OILDETAIL,OilSpillSvc,GetOilDetail);
	SERVICE_MAP(SID_OILSPILL_GET_OILFROCAST,OilSpillSvc,GetOilSpillForcast);
	SERVICE_MAP(SID_OILSPILL_GET_OILANALYSIS,OilSpillSvc,GetOilSpillAnalysis);
	SERVICE_MAP(0x13,OilSpillSvc,GetOilSpillTaskTrace);
	
	SERVICE_MAP(SID_OILSPILL_ADDUPD_EMERGENCE,OilSpillSvc,AddUpdSubTaskEmergency);
	SERVICE_MAP(SID_OILSPILL_GET_EMERGENCE,OilSpillSvc,GetSubTaskEmergencyDetail);
	SERVICE_MAP(SID_OILSPILL_ADDUPD_OBSERVATION,OilSpillSvc,AddUpdSubTaskObervation);
	SERVICE_MAP(SID_OILSPILL_GET_OBSERVATION,OilSpillSvc,GetSubTaskObervationDetail);

	SERVICE_MAP(SID_OILSPILL_ADDUPD_VIDEO,OilSpillSvc,AddUpdSubTaskVideo);
	SERVICE_MAP(SID_OILSPILL_GET_VIDEO,OilSpillSvc,GetSubTaskVideoDetail);
	SERVICE_MAP(SID_OILSPILL_DEL_SUBTASK,OilSpillSvc,DelOilSpillSubTask);
	SERVICE_MAP(SID_OILSPILL_GET_TASKSTATUS,OilSpillSvc,GetOilSpillTaskStatus);
    SERVICE_MAP(0x15,OilSpillSvc,GetOilSpillTaskCalStatus);

	


	// SERVICE_MAP(SID_OILSPILL_WTI_OIL_TRACE,OilSpillSvc,WtiGetOilTrace);
	// SERVICE_MAP(SID_OILSPILL_WTI_OIL_DISPOINTS,OilSpillSvc,WtiGetDispPoints);
	// SERVICE_MAP(SID_OILSPILL_WTI_OIL_DETAIL,OilSpillSvc,WtiGetPointDetail);

	SERVICE_MAP(0x41,OilSpillSvc,GetOilShiplist);
	SERVICE_MAP(0x22,OilSpillSvc,GetOilShipRelations);
	SERVICE_MAP(0x42,OilSpillSvc,GetOilGoodsDocklist);
	SERVICE_MAP(0x43,OilSpillSvc,ModifyShipInfo);

	SERVICE_MAP(0x30,OilSpillSvc,GetOilPlanlist);
	SERVICE_MAP(0x31,OilSpillSvc,GetOilPlanDetail);
	SERVICE_MAP(0x32,OilSpillSvc,NewModifyOilPlan);
	SERVICE_MAP(0x33,OilSpillSvc,OperatOilPlan);
	SERVICE_MAP(0x34,OilSpillSvc,NewModifySuppPlanApply);
	SERVICE_MAP(0x35,OilSpillSvc,DeleteSuppPlanApply);
	SERVICE_MAP(0x36,OilSpillSvc,NewModifyTransferPlanApply);
	SERVICE_MAP(0x37,OilSpillSvc,DeleteTransferPlanApply);
	SERVICE_MAP(0x38,OilSpillSvc,GetAllSuppPlanApply);
	SERVICE_MAP(0x39,OilSpillSvc,NewModifySuppPlanItem);

	SERVICE_MAP(0x3A,OilSpillSvc,GetOilSuppPlanDetail);
	SERVICE_MAP(0x3B,OilSpillSvc,DeleteSuppPlanItem);
	SERVICE_MAP(0x3C,OilSpillSvc,GetAllTransferPlanApply);
	SERVICE_MAP(0x3D,OilSpillSvc,NewModifyTransferPlanItem);
	SERVICE_MAP(0x3E,OilSpillSvc,GetOilTransferPlanDetail);
	SERVICE_MAP(0x3F,OilSpillSvc,DeleteTransferPlanItem);

	SERVICE_MAP(0x40,OilSpillSvc,SetOilSpillAlertArea);


	DEBUG_LOG("[MessageSvc::Start] OK......................................");

	return true;
}

int OilSpillSvc::GetOilSpillTaskList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilSpillTaskList]bad format:", jsonString, 1);

	string versionid=root.getv("bid", "");
	//string bid = root.getv("bid", "");
	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";
	sprintf(sql,"select TASK_ID,TASK_NAME,SPILL_DT,TASK_STATUS,SIMULATION_TIME,OIL_ID,API,SEA_AREA,SPILL_TOTAL from T41_oilspill_task WHERE VERSION_ID='%s'",versionid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;
	char taskid[128]="";
	char tasknm[256]="";
	char spilldate[64]="";
	char status[2]="";
	char seaarea[16]="";
	char oilid[64]="";
	float api=0.0;
	float forcast=0.0;
	float total=0.0;
	out << "[";
	while (psql->NextRow())
	{	
		READMYSQL_STR(TASK_ID, taskid);
		READMYSQL_STR(TASK_NAME, tasknm);
		READMYSQL_STR(SPILL_DT, spilldate);
		READMYSQL_STR(TASK_STATUS, status);
		READMYSQL_FLOAT(SIMULATION_TIME,forcast,0.0)
        READMYSQL_STR(OIL_ID, oilid)
		READMYSQL_FLOAT(API, api,0.0)
		READMYSQL_STR(SEA_AREA, seaarea)
		READMYSQL_FLOAT(SPILL_TOTAL,total,0.0)


		if (cnt>0)
			out << ",";
		out << "{osid:\"" << taskid << "\",osnm:\"" << tasknm<< "\",ostime:\"" << spilldate<< "\",osstatus:" << status << ",fcast:\""<<forcast<<"\",s_area:\"";
		out	<< seaarea<<"\",oilid:\""<<oilid<<"\",api:\""<<api<<"\",total:\""<<total<<"\"}";
		cnt++;
	}
	out << "]";

	
	RELEASE_MYSQL_RETURN(psql, 0);

}
int OilSpillSvc::GetOilSpillSubTaskList(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilSpillSubTaskList]bad format:", jsonString, 1);

	string taskid = root.getv("osid", "");


	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	sprintf (sql, "select SUBTASK_ID, SUBTASK_NAME,START_TM,STATUS from T41_oilspill_subtask_watch where TASK_ID='%s'", taskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;
	char subtaskid[128]="";
	char subtasknm[256]="";
	char subdate[64]="";
	char status[2]="";
	char mtype[2]="0";


	out << "{osid:\""<<taskid<<"\",";

	out << "subs:[";
	while (psql->NextRow())
	{	
		char ck[100] = "";
		char cnm[256] = "";
		READMYSQL_STR(SUBTASK_ID, subtaskid);
		READMYSQL_STR(SUBTASK_NAME, subtasknm);
		READMYSQL_STR(START_TM, subdate);
		READMYSQL_STR(STATUS, status);
		

		if (cnt>0)
			out << ",";
		out << "{subid:\"" << subtaskid << "\",subnm:\"" << subtasknm<< "\",subtime:\"" << subdate<< "\",subtype:1,mtype:0,status:\""<<status<<"\"}";
		cnt++;
	}

   
	
	char sql1[1024] = "";
	sprintf (sql1, "select SUBTASK_ID, SUBTASK_NAME,START_TM,TASK_TYPE,STATUS from T41_oilspill_subtask_emergence where TASK_ID='%s'", taskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql1), 3);

	while (psql->NextRow())
	{	
	
		READMYSQL_STR(SUBTASK_ID, subtaskid);
		READMYSQL_STR(SUBTASK_NAME, subtasknm);
		READMYSQL_STR(START_TM, subdate);
		READMYSQL_STR(STATUS, status);
		READMYSQL_STR(TASK_TYPE, mtype);

		if (cnt>0)
			out << ",";
		out << "{subid:\"" << subtaskid << "\",subnm:\"" << subtasknm<< "\",subtime:\"" << subdate<<"\",mtype:"<<mtype<<",subtype:0,status:\""<<status<<"\"}";
		cnt++;
	}


	char sql2[1024] = "";
	sprintf (sql2, "select SUBTASK_ID, SUBTASK_NAME,UPLOAD_TM from T41_oilspill_subtask_video where TASK_ID='%s'", taskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql2), 3);

	while (psql->NextRow())
	{	
		char ck[100] = "";
		char cnm[256] = "";
		READMYSQL_STR(SUBTASK_ID, subtaskid);
		READMYSQL_STR(SUBTASK_NAME, subtasknm);
		READMYSQL_STR(UPLOAD_TM, subdate);

		if (cnt>0)
			out << ",";
		out << "{subid:\"" << subtaskid << "\",subnm:\"" << subtasknm<< "\",subtime:\"" << subdate<< "\",subtype:2,mtype:0,status:\""<< "0\"}";
		cnt++;
	}

	out << "]}";


	RELEASE_MYSQL_RETURN(psql, 0);

}
int OilSpillSvc::AddUpdOilSpillTask(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilSpillSubTaskList]bad format:", jsonString, 1);

    MySql* psql = CREATE_MYSQL;

    int flag = root.getv("bflag", -1);
	string taskid=root.getv("osid", "");
	string taskname=root.getv("osnm", "");
	string versionid=root.getv("vid", "");
	string sea_area=root.getv("area", "");

	string gridsize=root.getv("gridsize", "150");
	//string gridsize="150";
	string forecast=root.getv("forecast", "");
	string oiltype=root.getv("oiltype", "");
	string API=root.getv("API", "");
	long leaktime=root.getv("leaktm", 0);
	string osage=root.getv("osage", "");
	int b_once=root.getv("bonce", -1);
	string leakduration=root.getv("leakdur", "");
	string leakspeed =root.getv("leakspeed", "");
    string leakcap=root.getv("leakcap", "");

	if (b_once==1)//非一次性泄露
	{
		double leakoil=atof(leakduration.c_str())*atof(leakspeed.c_str());
		char leakoilStr[32];
		sprintf (leakoilStr, "%.2f", leakoil);
		leakcap=string(leakoilStr);
	}
	//string leakpos=root.getv("leakpos", "");


	int areacount=root.getv("areacount", 1);
	int sdc=root.getv("sdc", 0); //斯托克斯漂移校正
	int ewinddec=root.getv("ewinddec", 0); //有效风速衰减
	string ewdvalue=root.getv("ewdvalue", "");//有效风速衰减值 0-1之间

	int smag=root.getv("smag", 0); //Smagorinsky方案
	string driftf=root.getv("driftf", "");//漂移因子 默认值为0
	string drifta=root.getv("drifta", "");//漂移角度

	int vadrifta=root.getv("vadrifta", 0);//可变漂移角度
	string therltop=root.getv("therltop", "0.01");//温跃层上
	string therlbtm=root.getv("therlbtm", "0.001");//温跃层下
	string therldeep=root.getv("therldeep", "30");//温跃层深度
	string hordif=root.getv("hordif", "");//水平扩散
	string nofp=root.getv("nofp", "");//包数量
	string deptof=root.getv("deptof", "");//预测的深度

	if (deptof.empty())
	{
       deptof="10,30,120";
	}
	string ccslick=root.getv("ccslick", "");//浮油平流洋流
	if (ccslick.empty())
	{
		ccslick="00";
	}


	//乳化参数
    string cewf=root.getv("cewf", "0.65");//
    string criwf=root.getv("criwf", "0.0000016");//
	string rmaxwf=root.getv("rmaxwf", "1.333");//
	int bmaxwf=root.getv("bmaxwf", 1);//

	//分散参数
	string ddvsd=root.getv("ddvsd", "0.001");//
	string crddbyw=root.getv("crddbyw", "0.000008");//
	string cfdcsize=root.getv("cfdcsize", "50.0");//
	string cdslick=root.getv("cdslick", "2000");//
	string rvlarged=root.getv("rvlarged", "0.08");//
	string rvsmalld=root.getv("rvsmalld", "0.0003");//
	string dwslayer=root.getv("dwslayer", "0.5");//
	string itbow=root.getv("itbow", "24");//
	string mfdispered=root.getv("mfdispered", "1.0");//

    //扩散参数
    string thinslick=root.getv("thinslick", "1.0");//
	string thickslick=root.getv("thickslick", "150.0");//
	string depstst=root.getv("depstst", "0.0015");//

    //海岸影响参数参数
	string eeloadpr=root.getv("eeloadpr", "1000");//
	string eoapir=root.getv("eoapir", "0.0");//
	//计算参数
	string ntperhs=root.getv("ntperhs", "2");//
	string darraysr=root.getv("darraysr", "2000");//





	vector<TaskAreaItem> vecArea;

	Json *JsonlikPos=root.get("leakpos");
	if (JsonlikPos)
	{
		for (int i = 0; i < JsonlikPos->size(); i++)
		{ 
			TaskAreaItem areaItem;
			Json *jsona_area=JsonlikPos->get(i);
			areaItem.areaid=jsona_area->getv("areaid","");
			areaItem.areaname=jsona_area->getv("areanm","");
			areaItem.areatype=jsona_area->getv("areatp","");
			areaItem.points=jsona_area->getv("points","");
			areaItem.surplus=jsona_area->getv("oilton","");
			vecArea.push_back(areaItem);
		}
	}
	


	char gmt0now[20];
	GmtNow(gmt0now);

	if (flag==0) //新增
	{
		uint64 timeusec = GmtNowUsec();
		char task_id[32] = "";
		sprintf (task_id, "T%lld", timeusec);

		char sql[1024] = "";
	    sprintf (sql, "INSERT INTO T41_oilspill_task (TASK_ID,TASK_NAME,CREATOR,CREATE_DT,TASK_STATUS,SIMULATION_TIME,OIL_ID,API,SEA_AREA,VERSION_ID,\
		               SPILL_DT,OIL_AGE,ONETIME_FLAG,SPILL_Duration,SPILL_RATE,SPILL_TOTAL,GRID_SIZE,SLICKS_NO,\
					   STOKES,WC_DRIFT_FACTOR,WC_DRIFT_ANGLE,WC_VAR_DRIFT_ANGLE,WC_REDUCTION,REDUCTION_FACTOR,SMAGORINSKY,HORI_DIFFUSIVITY,VERT_DIFFUSIVITY_MAX,VERT_DIFFUSIVITY_MIN,THERMOCLINE_DEPTH,PARCEL_NO,SLICK_CURRENTS,\
					   EMP1,EMP2,EMP3,EMP4,DISP1,DISP2,DISP3,DISP4,DISP5,DISP6,DISP7,DISP8,DISP9,SPP1,SPP2,SPP3,CIP1,CIP2,COP1,COP2) \
					   VALUES ('%s','%s','%s','%s','1', '00%s', '%s', '%s','%s','%s',\
					   '%d','%s','%d','%s','%s','%s','%s','%d',\
					   '0%d','%s','%s','0%d','0%d','%s','0%d','%s','%s','%s','%s','%s','%s',\
					   '%s','%s','%s','%d','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')", 
					  task_id, taskname.c_str(),pUid,gmt0now, forecast.c_str(), oiltype.c_str(), API.c_str(),sea_area.c_str(),versionid.c_str(),
					  leaktime,osage.c_str(),b_once,leakduration.c_str(),leakspeed.c_str(),leakcap.c_str(),gridsize.c_str(),areacount,
					  sdc,driftf.c_str(),drifta.c_str(),vadrifta,ewinddec,ewdvalue.c_str(),smag,hordif.c_str(),therltop.c_str(),therlbtm.c_str(),therldeep.c_str(),nofp.c_str(),ccslick.c_str(),
                      cewf.c_str(),criwf.c_str(),rmaxwf.c_str(),bmaxwf,ddvsd.c_str(),crddbyw.c_str(),cfdcsize.c_str(),cdslick.c_str(),rvlarged.c_str(),rvsmalld.c_str(),dwslayer.c_str(),itbow.c_str(),mfdispered.c_str(),
                      thinslick.c_str(),thickslick.c_str(),depstst.c_str(),eeloadpr.c_str(),eoapir.c_str(),ntperhs.c_str(),darraysr.c_str());
		

		

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		taskid=string(task_id);

		if (!vecArea.empty())
		{
			TaskAreaItem FItem=vecArea[0];
			stringstream ss;
			ss<<"INSERT INTO T41_oilspill_task_area(TASK_ID,AREA_ID,AREA_NAME,AREA_TYPE,AREA_POINTS,OIL_TON,UPDATE_DT,IS_VALID)";
			ss<<"VALUES('"<<taskid<<"','"<<FItem.areaid<<"','"<<FItem.areaname<<"','"<<FItem.areatype<<"','"<<FItem.points<<"','"<<FItem.surplus<<"','"<<gmt0now<<"','1')";


			for (int j=1;j<vecArea.size();j++)
			{
				ss<<",";
				ss<<"('"<<taskid<<"','"<<vecArea[j].areaid<<"','"<<vecArea[j].areaname<<"','"<<vecArea[j].areatype<<"','"<<vecArea[j].points<<"','"<<vecArea[j].surplus<<"','"<<gmt0now<<"','1')";
			}

			string cursql=ss.str();
			CHECK_MYSQL_STATUS(psql->Execute(cursql.c_str())>=0, 3);

		}



	}
	else if (flag==1&&!taskid.empty())
	{
      
		

       
			char sql[1024] = "";
		sprintf (sql, "update T41_oilspill_task set TASK_NAME='%s',SIMULATION_TIME='00%s',OIL_ID='%s',API='%s',SPILL_DT='%ld',OIL_AGE='%s',SEA_AREA='%s',\
					   ONETIME_FLAG='%d',SPILL_Duration='%s',SPILL_RATE='%s',SPILL_TOTAL='%s',GRID_SIZE='%s',SLICKS_NO='%d',\
		               STOKES='0%d',WC_DRIFT_FACTOR='%s',WC_DRIFT_ANGLE='%s',WC_VAR_DRIFT_ANGLE='0%d',WC_REDUCTION='0%d',REDUCTION_FACTOR='%s',SMAGORINSKY='0%d',\
					   HORI_DIFFUSIVITY='%s',VERT_DIFFUSIVITY_MAX='%s',VERT_DIFFUSIVITY_MIN='%s',THERMOCLINE_DEPTH='%s',PARCEL_NO='%s',SLICK_CURRENTS='%s',\
		               EMP1='%s',EMP2='%s',EMP3='%s',EMP4='%d',DISP1='%s',DISP2='%s',DISP3='%s',DISP4='%s',DISP5='%s',DISP6='%s',DISP7='%s',DISP8='%s',DISP9='%s',\
                       SPP1='%s',SPP2='%s',SPP3='%s',CIP1='%s',CIP2='%s',COP1='%s',COP2='%s' \
		               where TASK_ID='%s'",
					   taskname.c_str(),forecast.c_str(), oiltype.c_str(), API.c_str(),leaktime,osage.c_str(),sea_area.c_str(), b_once,leakduration.c_str(),leakspeed.c_str(),leakcap.c_str(),gridsize.c_str(),areacount,
					   sdc,driftf.c_str(),drifta.c_str(),vadrifta,ewinddec,ewdvalue.c_str(),smag,hordif.c_str(),therltop.c_str(),therlbtm.c_str(),therldeep.c_str(),nofp.c_str(),ccslick.c_str(),
					   cewf.c_str(),criwf.c_str(),rmaxwf.c_str(),bmaxwf,ddvsd.c_str(),crddbyw.c_str(),cfdcsize.c_str(),cdslick.c_str(),rvlarged.c_str(),rvsmalld.c_str(),dwslayer.c_str(),itbow.c_str(),mfdispered.c_str(),
					   thinslick.c_str(),thickslick.c_str(),depstst.c_str(),eeloadpr.c_str(),eoapir.c_str(),ntperhs.c_str(),darraysr.c_str(),taskid.c_str()
			    );
		
       CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

		char sql1[512]="";

		sprintf (sql1, "delete from T41_oilspill_task_area where TASK_ID='%s'",taskid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);

		if (!vecArea.empty())
		{
			TaskAreaItem FItem=vecArea[0];
			stringstream ss;
			ss<<"INSERT INTO T41_oilspill_task_area(TASK_ID,AREA_ID,AREA_NAME,AREA_TYPE,AREA_POINTS,OIL_TON,UPDATE_DT,IS_VALID)";
			ss<<"VALUES('"<<taskid<<"','"<<FItem.areaid<<"','"<<FItem.areaname<<"','"<<FItem.areatype<<"','"<<FItem.points<<"','"<<FItem.surplus<<"','"<<gmt0now<<"','1')";


			for (int j=1;j<vecArea.size();j++)
			{
				ss<<",";
				ss<<"('"<<taskid<<"','"<<vecArea[j].areaid<<"','"<<vecArea[j].areaname<<"','"<<vecArea[j].areatype<<"','"<<vecArea[j].points<<"','"<<vecArea[j].surplus<<"','"<<gmt0now<<"','1')";
			}

			string cursql=ss.str();
			CHECK_MYSQL_STATUS(psql->Execute(cursql.c_str())>=0, 3);
		}
		
	}



	out << "{taskid:\""<<taskid<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
	

}
int OilSpillSvc::GetOilSpillTaskDetail(const char* pUid, const char* jsonString, std::stringstream& out) 
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilSpillTaskDetail]bad format:", jsonString, 1);

	string taskid = root.getv("osid", "");
	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	sprintf (sql, "SELECT TASK_NAME,CREATOR,CREATE_DT,TASK_STATUS,SIMULATION_TIME,OIL_ID,API,SEA_AREA,\
				  SPILL_DT,OIL_AGE,ONETIME_FLAG,SPILL_Duration,SPILL_RATE,SPILL_TOTAL,GRID_SIZE,POSITION,SLICKS_NO,\
				  STOKES,WC_DRIFT_FACTOR,WC_DRIFT_ANGLE,WC_VAR_DRIFT_ANGLE,WC_REDUCTION,REDUCTION_FACTOR,SMAGORINSKY,HORI_DIFFUSIVITY,VERT_DIFFUSIVITY_MAX,VERT_DIFFUSIVITY_MIN,THERMOCLINE_DEPTH,PARCEL_NO,FORECAST_DEPTH,SLICK_CURRENTS,\
				  EMP1,EMP2,EMP3,EMP4,DISP1,DISP2,DISP3,DISP4,DISP5,DISP6,DISP7,DISP8,DISP9,SPP1,SPP2,SPP3,CIP1,CIP2,COP1,COP2 \
				  FROM T41_oilspill_task WHERE TASK_ID='%s'",taskid.c_str());
	
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char taskname[512]="";
	char gridsize[12]="";
	char forecast[10]="";
	char oiltype[64]="";
	char API[32]="";
	long leaktime=0;
	char osage[24]="";
	int b_once=0;
	char leakduration[32]="";
	char leakspeed[32] ="";
	char leakcap[32]="";
	char leakpos[1024]="";
	int areacount=1;
	int sdc=0; //斯托克斯漂移校正
	int ewinddec=0; //有效风速衰减
	char ewdvalue[12]="";//有效风速衰减值 0-1之间
	char sea_area[64]="";

	int smag=0; //Smagorinsky方案
	char driftf[12]="";//漂移因子 默认值为0
	char drifta[12]="";//漂移角度

	int vadrifta=0;//可变漂移角度
	char therltop[12]="0.01";//温跃层上
	char therlbtm[12]="0.001";//温跃层下
	char therldeep[12]="30";//温跃层深度
	char hordif[32]="";//水平扩散
	char nofp[12]="";//包数量
	char deptof[12]="";//预测的深度
	char ccslick[12]="";//浮油平流洋流


	//乳化参数
	char cewf[12]="0.65";//
	char criwf[12]="0.0000016";//
	char rmaxwf[12]="1.333";//
	int bmaxwf=1;//

	//分散参数
	char ddvsd[12]="0.001";//
	char crddbyw[12]="0.000008";//
	char cfdcsize[12]="50.0";//
	char cdslick[12]="2000";//
	char rvlarged[12]="0.08";//
	char rvsmalld[12]="0.0003";//
	char dwslayer[12]="0.5";//
	char itbow[12]="24";//
	char mfdispered[12]="1.0";//

	//扩散参数
	char thinslick[12]="1.0";//
	char thickslick[12]="150.0";//
	char depstst[12]="0.0015";//

	//海岸影响参数参数
	char eeloadpr[12]="1000";//
	char eoapir[12]="0.0";//
	//计算参数
	char ntperhs[12]="2";//
	char darrays[12]="2000";//


	if (psql->NextRow())
	{
		READMYSQL_STR(TASK_NAME, taskname);
		READMYSQL_INT(SPILL_DT, leaktime,0);
		READMYSQL_STR(OIL_ID, oiltype);
		READMYSQL_STR(API, API);
		READMYSQL_STR(SIMULATION_TIME, forecast);

		READMYSQL_STR(OIL_AGE, osage);

		READMYSQL_INT(ONETIME_FLAG, b_once,0);
		READMYSQL_STR(SPILL_Duration, leakduration);
		READMYSQL_STR(SPILL_RATE, leakspeed);
		READMYSQL_STR(SPILL_TOTAL, leakcap);
		READMYSQL_STR(GRID_SIZE, gridsize);
		READMYSQL_STR(POSITION, leakpos);
		READMYSQL_STR(SEA_AREA, sea_area)

		READMYSQL_INT(STOKES, sdc,0);
		READMYSQL_STR(WC_DRIFT_FACTOR, driftf);
		READMYSQL_STR(WC_DRIFT_ANGLE, drifta);
		READMYSQL_INT(WC_VAR_DRIFT_ANGLE, vadrifta,0);
		READMYSQL_INT(WC_REDUCTION, ewinddec, 0);
		READMYSQL_STR(REDUCTION_FACTOR, ewdvalue);
		READMYSQL_INT(SMAGORINSKY, smag, 0);

		READMYSQL_STR(HORI_DIFFUSIVITY, hordif);
		READMYSQL_STR(VERT_DIFFUSIVITY_MAX, therltop);
		READMYSQL_STR(VERT_DIFFUSIVITY_MIN, therlbtm);
		READMYSQL_STR(THERMOCLINE_DEPTH, therldeep);
		READMYSQL_STR(PARCEL_NO, nofp);
		READMYSQL_STR(FORECAST_DEPTH, deptof);
		READMYSQL_STR(SLICK_CURRENTS, ccslick);
		
		
		READMYSQL_STR(EMP1, cewf);
		READMYSQL_STR(EMP2, criwf);
		READMYSQL_STR(EMP3, rmaxwf);
		READMYSQL_INT(EMP4, bmaxwf, 0);


		READMYSQL_STR(DISP1, ddvsd);
		READMYSQL_STR(DISP2, crddbyw);
		READMYSQL_STR(DISP3, cfdcsize);
		READMYSQL_STR(DISP4, cdslick);
		READMYSQL_STR(DISP5, rvlarged);
		READMYSQL_STR(DISP6, rvsmalld);
		READMYSQL_STR(DISP7, dwslayer);
		READMYSQL_STR(DISP8, itbow);
		READMYSQL_STR(DISP9, mfdispered);

		READMYSQL_STR(SPP1, thinslick);
		READMYSQL_STR(SPP2, thickslick);
		READMYSQL_STR(SPP3, depstst);

		READMYSQL_STR(CIP1, eeloadpr);
		READMYSQL_STR(CIP2, eoapir);
		READMYSQL_STR(COP1, ntperhs);
        READMYSQL_STR(COP2, darrays);


	}

	out << "{osnm:\"" << taskname << "\",gridsize:\"" << gridsize << "\",forecast:\"" << forecast << "\",oiltype:\"" << oiltype << "\",API:\"" << API << "\",area:\"" << sea_area << "\",leaktm:" << leaktime; 
	out	<< ",osage:\"" <<  osage << "\",bonce:" << b_once << ",leakdur:\"" << leakduration << "\",leakspeed :\"" << leakspeed << "\",leakcap:\""<<leakcap;
	out << "\",sdc:"<<sdc<<",ewinddec:"<<ewinddec<<",ewdvalue:\""<< ewdvalue << "\",smag :\"" << smag << "\",driftf:\""<<driftf << "\",drifta:\"" << drifta<<"\",vadrifta:"<<vadrifta;
    out <<",therltop:\"" <<  therltop << "\",therlbtm:\"" << therlbtm << "\",therldeep:\"" << therldeep << "\",hordif :\"" << hordif << "\",nofp:\""<<nofp << "\",deptof:\"" << deptof;
	out <<"\",ccslick:\"" <<  ccslick << "\",cewf:\"" << cewf <<"\",criwf:\"" << criwf << "\",rmaxwf :\"" << rmaxwf << "\",bmaxwf:"<<bmaxwf << ",ddvsd:\"" << ddvsd;
	out <<"\",crddbyw:\"" <<  crddbyw << "\",cfdcsize:\"" << cfdcsize <<"\",cdslick:\"" << cdslick << "\",rvlarged :\"" << rvlarged << "\",rvsmalld:\""<<rvsmalld << "\",dwslayer:\"" << dwslayer;
	out <<"\",itbow:\"" <<  itbow << "\",mfdispered:\"" << mfdispered <<"\",thinslick:\"" << thinslick << "\",thickslick :\"" << thickslick << "\",depstst:\""<<depstst << "\",eeloadpr:\"" << eeloadpr;
    out <<"\",eoapir:\"" <<  eoapir << "\",ntperhs:\"" << ntperhs <<"\",darraysr:\"" << darrays <<"\",leakpos:[";
	

	char sql1[1024] = "";
	sprintf (sql1, "select AREA_ID,AREA_NAME,AREA_TYPE,AREA_POINTS,OIL_TON from T41_oilspill_task_area where TASK_ID='%s'",taskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql1),3);

	char areaid[64]="";
	char areaname[128]="";
	char areatype[16]="";
	char points[512]="";
	char remainoil[16]="";
	int index=0;
	while (psql->NextRow())
	{
		READMYSQL_STR(AREA_ID, areaid)
			READMYSQL_STR(AREA_NAME, areaname)
			READMYSQL_STR(AREA_TYPE, areatype)
			READMYSQL_STR(AREA_POINTS, points)
			READMYSQL_STR(OIL_TON, remainoil)

			if (index>0)
			{
				out <<","; 
			}

			out << "{areaid:\"" << areaid<< "\",areanm:\"" << areaname << "\",areatp:\"" << areatype<<"\",points:\"" << points<<"\",oilton:\"" << remainoil<<"\"}";

			index++;
	}

	out<<"]}";


	RELEASE_MYSQL_RETURN(psql, 0);

}
int OilSpillSvc::DelOilSpillTask(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::DelOilSpillTask(]bad format:", jsonString, 1);

	string taskid = root.getv("osid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[1024] = "";
	sprintf (sql, "Delete from T41_oilspill_task WHERE TASK_ID = '%s'",taskid.c_str());
	
	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);
	return 0;

}

int OilSpillSvc::GetOilDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilDetail]bad format:", jsonString, 1);

	string oiltypeid = root.getv("oiltypeid", "");
	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	sprintf (sql, "select OIL_NAME,LOCATION,API,OIL_DENSITY,OIL_RES_DENSITY,OIL_RES_PERCENT,OIL_VISCOSITY,OIL_VISC_TEMP,\
		              OIL_VAP_PRESSURE from T91_oil_type where OIL_ID='%s'",oiltypeid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char oilnm[64]="";
	char api[32]="";
	char location[256]="";
	char fieldnm[64]="";
	char reference[32]="";
	char density[32]="";
	char resdensity[32]="";
	char respercent[32] ="";
	char viscosity[32]="";
	char temviscosity[64]="";
	char vapressure[64]="";

	if (psql->NextRow())
	{
		READMYSQL_STR(OIL_NAME, oilnm);
		READMYSQL_STR(LOCATION, location);
		READMYSQL_STR(API, api);
		READMYSQL_STR(OIL_DENSITY, density);
		READMYSQL_STR(OIL_RES_DENSITY, resdensity);
		READMYSQL_STR(OIL_RES_PERCENT, respercent);
		READMYSQL_STR(OIL_VISCOSITY, viscosity);

		READMYSQL_STR(OIL_VISC_TEMP, temviscosity);
		READMYSQL_STR(OIL_VAP_PRESSURE, vapressure);
	}

	out << "{oilnm:\"" << oilnm << "\",api:\"" << api << "\",location:\"" << location << "\",fieldnm:\"" << fieldnm << "\",reference:\"" << reference; 
	out << "\",density:\""<<density<<"\",resdensity:\""<<resdensity<<"\",respercent:\""<< respercent << "\",viscosity :\"" << viscosity << "\",temviscosity:\""<<temviscosity << "\",vapressure:\"" << vapressure<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;

}
int OilSpillSvc::GetOilSpillForcast(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilSpillForcast]bad format:", jsonString, 1);


	string taskid = root.getv("osid", "");
	long time=root.getv("tm", 0);

	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";
	sprintf (sql, "select TASK_ID,EVAP,DISP,SRF,CSTFXD,SRF_VOL,TOT_SRF_VOL from T41_oilspill_task_result_statistic \
	               where TASK_ID='%s' AND ANALYSES_TM=%d",taskid.c_str(),time);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
   
	double totaloil=0.0;
	double evap=0.0;
	double disp=0.0;
	double srf=0.0;
	double cstfxd=0.0;
	double srf_vol=0.0;
	double tot_srf_vol=0.0;

	if (psql->NextRow())
	{
		READMYSQL_DOUBLE(EVAP, evap,0.0)
		READMYSQL_DOUBLE(DISP, disp,0.0)
		READMYSQL_DOUBLE(SRF, srf,0.0)
		READMYSQL_DOUBLE(CSTFXD, cstfxd,0.0)
		READMYSQL_DOUBLE(SRF_VOL, srf_vol,0.0)
		READMYSQL_DOUBLE(TOT_SRF_VOL, tot_srf_vol,0.0)
	}

	totaloil=srf_vol*100/srf;


	vector<TaskPointThick> vecThickPt;
	char sql1[256] = "";
	sprintf (sql1, "select TASK_ID,LATITUDE,LONGITUDE,OIL_AMOUNT from T41_oilspill_task_result_srf_detail where TASK_ID='%s' AND ANALYSES_TM=%d" ,
				   taskid.c_str(),time);

	CHECK_MYSQL_STATUS(psql->Query(sql1), 3);
    
	while (psql->NextRow())
	{
		TaskPointThick ptItem;
		READMYSQL_FLOAT(LATITUDE, ptItem.ypos,361)
		READMYSQL_FLOAT(LONGITUDE, ptItem.xpos,361)
		READMYSQL_FLOAT(OIL_AMOUNT, ptItem.thickval,0.0)
		vecThickPt.push_back(ptItem);
	}


	char sql2[256] = "";
	sprintf (sql2, "select TASK_ID,LATITUDE,LONGITUDE,LATITUDE1,LONGITUDE1,OIL_AMOUNT from t41_oilspill_task_result_cst_detail where TASK_ID='%s' AND ANALYSES_TM=%d" ,
		taskid.c_str(),time);

	CHECK_MYSQL_STATUS(psql->Query(sql1), 3);

	while (psql->NextRow())
	{
		TaskPointThick ptItem;
		READMYSQL_FLOAT(LATITUDE, ptItem.ypos,361)
		READMYSQL_FLOAT(LONGITUDE, ptItem.xpos,361)
		READMYSQL_FLOAT(OIL_AMOUNT, ptItem.thickval,0.0)
		vecThickPt.push_back(ptItem);


		TaskPointThick ptItem2;
		ptItem2.thickval=ptItem.thickval;
		READMYSQL_FLOAT(LATITUDE1, ptItem2.ypos,361)
		READMYSQL_FLOAT(LONGITUDE1, ptItem2.xpos,361)
		vecThickPt.push_back(ptItem2);
	}


	char simvalstr[512];
	sprintf (simvalstr, "simval:{evapor:\"%.1f\",evapor_p:\"%.4f\",disperse:\"%.1f\",disperse_p:\"%.4f\",surface:\"%.1f\",surface_p:\"%.4f\",\
                         coast:\"%.1f\",coast_p:\"%.4f\",total_remain:\"%.1f\"},",evap*totaloil/100,evap,disp*totaloil/100,disp,srf_vol,srf,
						 cstfxd*totaloil/100,cstfxd,tot_srf_vol);

	out << "{osid:\""<<taskid<<"\",tm:"<<time<<","<<string(simvalstr)<<"details:[";

	for (int i=0;i<vecThickPt.size();i++)
	{
		
		if (i>0)
		{
			out<<",";
		}
		out<<"{pos:\""<<vecThickPt[i].xpos<<","<<vecThickPt[i].ypos<<"\",thick:\""<<vecThickPt[i].thickval<<"\"}";

	}


	//vector<EqualLevelItem> EqualList;

	//char sql1[1024] = "";
	//sprintf (sql1, "SELECT CONTOUR_ID,OIL_LEVEL,LOCATION FROM t41_oilspill_task_srf_contour WHERE TASK_ID='%s' and analysis_tm=%d",taskid.c_str(),time);

	//CHECK_MYSQL_STATUS(psql->Query(sql1), 3);

	//while (psql->NextRow())
	//{
	//	EqualLevelItem ptItem;
	//	READMYSQL_STR(CONTOUR_ID, ptItem.id)
	//	READMYSQL_STRING(LOCATION, ptItem.location)
	//	READMYSQL_INT(OIL_LEVEL, ptItem.level,1)
	//	EqualList.push_back(ptItem);
	//}



	//for (int i=0;i<EqualList.size();i++)
	//{

	//	if (i>0)
	//	{
	//		out<<",";
	//	}
	//	out<<"{id:\""<<EqualList[i].id<<"\",loc:\""<<EqualList[i].location<<"\",level:\""<<EqualList[i].level<<"\"}";

	//}


	out <<"]}";

	RELEASE_MYSQL_RETURN(psql, 0);
	
}
int OilSpillSvc::GetOilSpillAnalysis(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilSpillAnalysis]bad format:", jsonString, 1);

	string taskid = root.getv("osid", "");
	long startm=root.getv("starttm", 0);
	long endtm=root.getv("endtm", 0);

	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";
	sprintf (sql, "select TASK_ID,ANALYSES_TM,EVAP,DISP,SRF,CSTFXD,SRF_VOL,TOT_SRF_VOL,VISEM1,VISEM2,VISOIL1,VISOIL2,DENOIL1,DENOIL2,WFRAC1,WFRAC2 \
		     from T41_oilspill_task_result_statistic where TASK_ID='%s' AND ANALYSES_TM>=%d AND ANALYSES_TM<=%d",taskid.c_str(),startm,endtm);

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	double totaloil=0.0;
	double evap=0.0;
	double disp=0.0;
	double srf=0.0;
	double cstfxd=0.0;
	double srf_vol=0.0;
	double tot_srf_vol=0.0;
	double visem1=0.0;
	double visem2=0.0;
	double visoil1=0.0;
	double visoil2=0.0;
	double denoil1=0.0;
	double denoil2=0.0;
	double wrfac1=0.0;
	double wrfac2=0.0;
	long dtime=0;

    out <<"{osid:\""<<taskid<<"\",statistic:[";
	double total=0.0;

	int index=0;
	while (psql->NextRow())
	{
		    READMYSQL_DOUBLE(EVAP, evap,0.0)
			READMYSQL_DOUBLE(DISP, disp,0.0)
			READMYSQL_DOUBLE(SRF, srf,0.0)
			READMYSQL_DOUBLE(CSTFXD, cstfxd,0.0)
			READMYSQL_DOUBLE(SRF_VOL, srf_vol,0.0)
			READMYSQL_DOUBLE(TOT_SRF_VOL, tot_srf_vol,0.0)
			READMYSQL_DOUBLE(VISEM1, visem1,0.0)
			READMYSQL_DOUBLE(VISEM2, visem2,0.0)
			READMYSQL_DOUBLE(VISOIL1, visoil1,0.0)
			READMYSQL_DOUBLE(VISOIL2, visoil2,0.0)
			READMYSQL_DOUBLE(DENOIL1, denoil1,0.0)
			READMYSQL_DOUBLE(DENOIL2, denoil2,0.0)
			READMYSQL_DOUBLE(WFRAC1, wrfac1,0.0)
			READMYSQL_DOUBLE(WFRAC2, wrfac2,0.0)
			READMYSQL_INT(ANALYSES_TM, dtime,0)


			if (index>0)
			{
				out<<",";
			}
			

			total=srf_vol*100/srf;

			evap=total*evap/100;
            disp=total*disp/100;
			cstfxd=total*cstfxd/100;


			char simvalstr[512];
			sprintf (simvalstr, "{datatm:%d,evapor:\"%.1f\",disperse:\"%.1f\",surface:\"%.1f\",coast:\"%.1f\",total_remain:\"%.1f\",\
								  wfrac1:\"%.1f\", wfrac2:\"%.1f\", visem1:\"%.1f\", visem2:\"%.1f\", visoil1:\"%.1f\", visoil2:\"%.1f\", denoil1:\"%.1f\", denoil2:\"%.1f\"}"
								  ,dtime,evap,disp,srf_vol,cstfxd,tot_srf_vol,wrfac1,wrfac2,visem1,visem2,visoil1,visoil2,denoil1,denoil2);

			out<<string(simvalstr);
			index++;
	}

	out <<"],measures:[";


	index=0;

	char sql1[512] = "";
	sprintf (sql1, "select SUBTASK_ID,TASK_TYPE,START_TM,SPLASH_AREA,DURATION,SPLASH_EFFICIENCY,SKIM_EFFICIENCY,\
		            BURN_AREA,BURN_THICK,BURN_RETURNRATE FROM t41_oilspill_subtask_emergence WHERE TASK_ID='%s'",taskid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql1), 3);


	char tasktype[2]="";
	long starttm=0;
	double duration=0.0;
	double splash_effi=0.0;
	double splash_area=0.0;

	double skim_effi=0.0;
	double burn_area=0.0;
	double burn_thick=0.0;
	double burn_returnrate=0.0;

	while (psql->NextRow())
	{
		READMYSQL_INT(START_TM, starttm,0)
        READMYSQL_STR(TASK_TYPE,tasktype)
        
		READMYSQL_DOUBLE(DURATION, duration,0.0)
		READMYSQL_DOUBLE(SPLASH_EFFICIENCY, splash_effi,0.0)
		READMYSQL_DOUBLE(SPLASH_AREA, splash_area,0.0)
		READMYSQL_DOUBLE(SKIM_EFFICIENCY, skim_effi,0.0)
		READMYSQL_DOUBLE(BURN_AREA, burn_area,0.0)
		READMYSQL_DOUBLE(BURN_THICK, burn_thick,0.0)
		READMYSQL_DOUBLE(BURN_RETURNRATE, burn_returnrate,0.0)
				
		double totalval=0.0;

	
		if (string(tasktype)=="0")//分散剂
		{
            totalval=splash_effi*splash_area;//需要修改
		}
		else if (string(tasktype)=="1")//撇去
		{
            totalval=skim_effi*duration;
		}
		else if (string(tasktype)=="2")//燃烧
		{
            totalval=burn_thick*(1-burn_returnrate)*burn_area;//需要修改
		}

		if (index>0)
		{
			out<<",";
		}

		char simvalstr[512];
		sprintf (simvalstr, "{datatm:%d,mtype:%s,val:\"%.1f\"}",starttm,tasktype,totalval);

		out<<string(simvalstr);
		index++;
	}


	out <<"],total:\""<<total<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}


int OilSpillSvc::AddUpdSubTaskEmergency(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::AddUpdSubTaskEmergency]bad format:", jsonString, 1);


	MySql* psql = CREATE_MYSQL;

	int flag = root.getv("bflag", -1);
	int subtype=root.getv("mtype", -1);//0分散剂1 撇去2燃烧

	string taskid=root.getv("osid", "");
	string subtaskid=root.getv("subid", "");
	string subtaskname=root.getv("subnm", "");
	string opternm=root.getv("opternm", "");
	long starttm=root.getv("starttm", 0);
	string duration=root.getv("duration", "");

	string splasharea="";
	string splasheffe="";

	Json *Jsonsplash=root.get("splash");
	if (Jsonsplash)
	{
		 splasharea=Jsonsplash->getv("splasharea", "");
		 splasheffe=Jsonsplash->getv("splasheffe", "");
         
	}

    string skimeff="";
	Json *Jsonskim=root.get("skim");
	if (Jsonskim)
	{
		 skimeff=Jsonskim->getv("skimeff", "");
	}


	string burnarea="";
	string burnthick="";
	string burnreturnrate="";
	string burncap="";

	Json *Jsonburn=root.get("burn");

	if (Jsonburn)
	{
		 burnarea=Jsonburn->getv("burnarea", "");
		 burnthick=Jsonburn->getv("burnthick", "");
		 burnreturnrate=Jsonburn->getv("burnreturnrate", "");
		 burncap=Jsonburn->getv("burncap", "");
	}

	char gmt0now[20];
	GmtNow(gmt0now);

	if (flag==0&&!taskid.empty()) //新增
	{
		uint64 timeusec = GmtNowUsec();
		char subtask_id[32] = "";
		sprintf (subtask_id, "SE%lld", timeusec);

		char sql[1024] = "";
		sprintf (sql, "INSERT INTO T41_oilspill_subtask_emergence (TASK_ID,SUBTASK_ID,TASK_TYPE,SUBTASK_NAME,OPERATOR,START_TM,DURATION,SPLASH_AREA,\
					  SPLASH_EFFICIENCY,SKIM_EFFICIENCY,BURN_AREA,BURN_THICK,BURN_RETURNRATE,STATUS,UPDATE_DT,IS_VALID) \
					   VALUES ('%s','%s','%d','%s','%s', '%d', '%s', '%s','%s','%s','%s','%s','%s','1','%s','1')", 
					   taskid.c_str(),subtask_id, subtype,subtaskname.c_str(),opternm.c_str(), starttm, duration.c_str(), splasharea.c_str(),
					   splasheffe.c_str(),skimeff.c_str(),burnarea.c_str(),burnthick.c_str(),burnreturnrate.c_str(),gmt0now
					  );

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		subtaskid=string(subtask_id);
	}
	else if (flag==1&&!subtaskid.empty())
	{

		char sql[1024] = "";
		sprintf (sql, "update T41_oilspill_subtask_emergence set SUBTASK_NAME='%s',OPERATOR='%s',START_TM='%d',DURATION='%s',SPLASH_AREA='%ld',SPLASH_EFFICIENCY='%s',\
					  SKIM_EFFICIENCY='%d',BURN_AREA='%s',BURN_THICK='%s',BURN_RETURNRATE='%s' where SUBTASK_ID='%s'",
					  subtaskname.c_str(),opternm.c_str(), starttm, duration.c_str(),splasharea.c_str(),splasheffe.c_str(),
					  skimeff.c_str(),burnarea.c_str(),burnthick.c_str(),burnreturnrate.c_str(),subtaskid.c_str()
					  );

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}



	out << "{subid:\""<<subtaskid<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;

}
int OilSpillSvc::GetSubTaskEmergencyDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetSubTaskEmergencyDetail]bad format:", jsonString, 1);

	string subtaskid=root.getv("subid", "");
	//int subtype=root.getv("subtype", -1);
	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	sprintf (sql, "select TASK_ID,SUBTASK_ID,TASK_TYPE,SUBTASK_NAME,OPERATOR,START_TM,DURATION,SPLASH_AREA,\
				   SPLASH_EFFICIENCY,SKIM_EFFICIENCY,BURN_AREA,BURN_THICK,BURN_RETURNRATE  \
	               from T41_oilspill_subtask_emergence where SUBTASK_ID='%s'",subtaskid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char taskid[32]="";
	int  subtype=-1;
	char subnm[256]="";
	char opternm[64]="";
	long starttm=0;
	char duration[32]="";

	char splasharea[32]="";
	char splasheffe[32] ="";
	char skimeff[32]="";
	char burnarea[64]="";
	char burnthick[64]="";
	char burnreturnrate[64]="";
	char burncap[64]="";

	if (psql->NextRow())
	{
		READMYSQL_STR(TASK_ID, taskid)
		READMYSQL_STR(SUBTASK_NAME, subnm)
		READMYSQL_STR(OPERATOR, opternm)
		
		READMYSQL_INT(TASK_TYPE, subtype,-1)
		READMYSQL_INT(START_TM, starttm,0)
		READMYSQL_STR(DURATION, duration)
		READMYSQL_STR(SPLASH_AREA, splasharea)
		READMYSQL_STR(SPLASH_EFFICIENCY, splasheffe)
		READMYSQL_STR(SKIM_EFFICIENCY, skimeff)
		READMYSQL_STR(BURN_AREA, burnarea)
		READMYSQL_STR(BURN_THICK, burnthick)
		READMYSQL_STR(BURN_RETURNRATE, burnreturnrate)
	}

	out << "{subid:\"" << subtaskid << "\",osid:\"" << taskid << "\",mtype:" << subtype << ",subnm:\"" << subnm << "\",opternm:\"" << opternm; 
	out << "\",starttm:"<<starttm<<",duration:\""<<duration<<"\",splash:{splasharea:\""<<splasharea << "\",splasheffe :\"" <<splasheffe ;
	out << "\"},skim:{skimeff:\""<<skimeff << "\"},burn:{burnarea:\"" << burnarea<< "\",burnthick:\"" << burnthick;
	out	<< "\",burnreturnrate:\""<<burnreturnrate<<"\",burncap:\"\"}}";

	RELEASE_MYSQL_RETURN(psql, 0);



	return 0;
}

int OilSpillSvc::AddUpdSubTaskObervation(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::AddUpdSubTaskObervation]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	int flag = root.getv("bflag", -1);
	string taskid=root.getv("osid", "");
	string subtaskid=root.getv("subid", "");
	string subtaskname=root.getv("obnm", "");
	string obperson=root.getv("obperson", "");
	long obsertime=root.getv("obsertime", 0);
	string forecast=root.getv("forecast", "");

	vector<TaskAreaItem> vecArea;

	Json* Jsonareas = root.get("areas");
	for (int i = 0; i < Jsonareas->size(); i++)
	{ 
		TaskAreaItem areaItem;

		Json *jsona_area=Jsonareas->get(i);
		areaItem.areaid=jsona_area->getv("areaid","");
		areaItem.areaname=jsona_area->getv("areanm","");
		areaItem.areatype=jsona_area->getv("areatp","");
		areaItem.points=jsona_area->getv("points","");
		areaItem.surplus=jsona_area->getv("surplus","");
		areaItem.zarea=jsona_area->getv("zarea","");
		vecArea.push_back(areaItem);
	}

	char gmt0now[20];
	GmtNow(gmt0now);

	if (flag==0&&!taskid.empty()) //新增
	{
		uint64 timeusec = GmtNowUsec();
		char subtask_id[32] = "";
		sprintf (subtask_id, "SO%lld", timeusec);

		char sql[1024] = "";
		sprintf (sql, "INSERT INTO T41_oilspill_subtask_watch (TASK_ID,SUBTASK_ID,SUBTASK_NAME,OPERATOR,START_TM,AREA_IDS,PRIDICT_DURATION,STATUS,UPDATE_DT,IS_VALID) \
					   VALUES ('%s','%s','%s','%s', '%d', '%s','%s','1','%s','1')", 
					   taskid.c_str(),subtask_id,subtaskname.c_str(),obperson.c_str(), obsertime,"", forecast.c_str(),gmt0now
					  );

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		subtaskid=string(subtask_id);


		if (!vecArea.empty())
		{
            TaskAreaItem FItem=vecArea[0];
			stringstream ss;
			ss<<"INSERT INTO T41_oilspill_subtask_watch_area(SUBTASK_ID,AREA_ID,AREA_NAME,AREA_TYPE,AREA_POINTS,ZONE_AREA,REMAIN_OIL,UPDATE_DT,IS_VALID)";
			ss<<"VALUES('"<<subtask_id<<"','"<<FItem.areaid<<"','"<<FItem.areaname<<"','"<<FItem.areatype<<"','"<<FItem.points<<"','"<<FItem.zarea<<"','"<<FItem.surplus<<"','"<<gmt0now<<"','1')";
		

			for (int j=1;j<vecArea.size();j++)
			{
				ss<<",";
				ss<<"('"<<subtask_id<<"','"<<vecArea[j].areaid<<"','"<<vecArea[j].areaname<<"','"<<vecArea[j].areatype<<"','"<<vecArea[j].points<<"','"<<vecArea[j].zarea<<"','"<<vecArea[j].surplus<<"','"<<gmt0now<<"','1')";
			}
		
			string cursql=ss.str();
		    CHECK_MYSQL_STATUS(psql->Execute(cursql.c_str())>=0, 3);
		
		}

	
	}
	else if (flag==1&&!subtaskid.empty())
	{

		char sql[1024] = "";
		sprintf (sql, "update T41_oilspill_subtask_watch set SUBTASK_NAME='%s',OPERATOR='%s',START_TM='%d',PRIDICT_DURATION='%s' where SUBTASK_ID='%s'",
					   subtaskname.c_str(),obperson.c_str(), obsertime, forecast.c_str(),subtaskid.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);


		char sql1[512]="";

		sprintf (sql1, "delete from T41_oilspill_subtask_watch_area where SUBTASK_ID='%s'",subtaskid.c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);

		if (!vecArea.empty())
		{
			TaskAreaItem FItem=vecArea[0];
			stringstream ss;
			ss<<"INSERT INTO T41_oilspill_subtask_watch_area(SUBTASK_ID,AREA_ID,AREA_NAME,AREA_TYPE,AREA_POINTS,ZONE_AREA,REMAIN_OIL,UPDATE_DT,IS_VALID)";
			ss<<"VALUES('"<<subtaskid<<"','"<<FItem.areaid<<"','"<<FItem.areaname<<"','"<<FItem.areatype<<"','"<<FItem.points<<"','"<<FItem.zarea<<"','"<<FItem.surplus<<"','"<<gmt0now<<"','1')";


			for (int j=1;j<vecArea.size();j++)
			{
				ss<<",";
				ss<<"('"<<subtaskid<<"','"<<vecArea[j].areaid<<"','"<<vecArea[j].areaname<<"','"<<vecArea[j].areatype<<"','"<<vecArea[j].points<<"','"<<vecArea[j].zarea<<"','"<<vecArea[j].surplus<<"','"<<gmt0now<<"','1')";
			}

			string cursql=ss.str();
			CHECK_MYSQL_STATUS(psql->Execute(cursql.c_str())>=0, 3);
		}
	}



	out << "{subid:\""<<subtaskid<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);


	return 0;

}

int OilSpillSvc::GetSubTaskObervationDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[OilSpillSvc::GetSubTaskObervationDetail]bad format:", jsonString, 1);

	
	string subtaskid=root.getv("subid", "");
	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	sprintf (sql, "select TASK_ID,SUBTASK_ID,SUBTASK_NAME,OPERATOR,START_TM,PRIDICT_DURATION \
				  from T41_oilspill_subtask_watch where SUBTASK_ID='%s'",subtaskid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char taskid[32]="";
	char subnm[256]="";
	char opternm[64]="";
	long starttm=0;
	char duration[32]="";

	if (psql->NextRow())
	{
		READMYSQL_STR(TASK_ID, taskid)
		READMYSQL_STR(SUBTASK_NAME, subnm)
		READMYSQL_STR(OPERATOR, opternm)
		READMYSQL_INT(START_TM, starttm,0)
		READMYSQL_STR(PRIDICT_DURATION, duration)
	}

	out << "{subid:\"" << subtaskid<< "\",obnm:\"" << subnm << "\",obperson:\"" << opternm; 
	out << "\",obsertime:"<<starttm<<",forecast:\""<<duration<<"\",areas:[";


	char sql1[1024] = "";
	sprintf (sql1, "select AREA_ID,AREA_NAME,AREA_TYPE,AREA_POINTS,ZONE_AREA,REMAIN_OIL from T41_oilspill_subtask_watch_area where SUBTASK_ID='%s'",subtaskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql1),3);

	char areaid[64]="";
    char areaname[128]="";
	char areatype[16]="";
	char points[512]="";
	char remainoil[16]="";
	char zonearea[16]="";
	int index=0;
	while (psql->NextRow())
	{
		    READMYSQL_STR(AREA_ID, areaid)
			READMYSQL_STR(AREA_NAME, areaname)
			READMYSQL_STR(AREA_TYPE, areatype)
			READMYSQL_STR(AREA_POINTS, points)
			READMYSQL_STR(REMAIN_OIL, remainoil)
			READMYSQL_STR(ZONE_AREA, zonearea)
			
			if (index>0)
			{
				out <<","; 
			}
			
			out << "{areaid:\"" << areaid<< "\",areanm:\"" << areaname << "\",areatp:\"" << areatype<<"\",points:\"" << points<<"\",zarea:\"" << zonearea<<"\",surplus:\"" << remainoil<<"\"}";
			
			index++;
	}
    

	out <<"]}"; 
	
	RELEASE_MYSQL_RETURN(psql, 0);


	
	return 0;
}


int OilSpillSvc::AddUpdSubTaskVideo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::GetOilSpillSubTaskList]bad format:", jsonString, 1);


	MySql* psql = CREATE_MYSQL;

	int flag = root.getv("bflag", -1);
	string taskid=root.getv("osid", "");
	string subtaskid=root.getv("subid", "");

	string subtaskname=root.getv("videonm", "");
	long videolen=root.getv("videolen", 0);
	string des=root.getv("des", "");
	string addr=root.getv("addr", "");


	char gmt0now[20];
	GmtNow(gmt0now);

	if (flag==0&&!taskid.empty()) //新增
	{
		uint64 timeusec = GmtNowUsec();
		char subtask_id[32] = "";
		sprintf (subtask_id, "SV%lld", timeusec);

		char sql[1024] = "";
		sprintf (sql, "INSERT INTO T41_oilspill_subtask_video (TASK_ID,SUBTASK_ID,SUBTASK_NAME,UPLOAD_TM,`DESC`,IS_VALID) \
					  VALUES ('%s','%s','%s','%d','%s','1')", 
					  taskid.c_str(),subtask_id,subtaskname.c_str(),gmt0now,des.c_str()
				);

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
		subtaskid=string(subtask_id);
	}
	else if (flag==1&&!subtaskid.empty())
	{

		char sql[1024] = "";
		sprintf (sql, "update T41_oilspill_subtask_video set SUBTASK_NAME='%s',`DESC`='%s' where SUBTASK_ID='%s'",
					  subtaskname.c_str(),des.c_str(), subtaskid.c_str());

		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
	}


	out << "{subid:\""<<subtaskid<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);


	return 0;

}
int OilSpillSvc::GetSubTaskVideoDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::GetOilSpillSubTaskList]bad format:", jsonString, 1);

	string subtaskid=root.getv("subid", "");
	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";
	sprintf (sql, "select TASK_ID,SUBTASK_ID,SUBTASK_NAME,UPLOAD_TM,`DESC` \
				   from T41_oilspill_subtask_video where SUBTASK_ID='%s'",subtaskid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char taskid[32]="";
	char subnm[256]="";
	char desc[1024]="";
	char addr[512]="";
	

	if (psql->NextRow())
	{
		READMYSQL_STR(TASK_ID, taskid)
		READMYSQL_STR(SUBTASK_NAME, subnm)
		READMYSQL_STR(DESC, desc)
	}


	char sql1[1024] = "select VIDEO_KEY,VIDEO_LEN,BIT_RATE from t41_video_index WHERE OBJ_ID=''";

	CHECK_MYSQL_STATUS(psql->Query(sql1), 3);
	char videokey[256]="";
	int videolen=0;
	int bitrate=0;

	if (psql->NextRow())
	{
		READMYSQL_STR(VIDEO_KEY, videokey)
		READMYSQL_INT(VIDEO_LEN, videolen,0)
		READMYSQL_INT(BIT_RATE, bitrate,0)
	}


	out << "{subid:\"" << subtaskid <<  "\",videoid:\"" << videokey <<"\",videonm:\"" << subnm << "\",desc:\"" << desc << "\",videolen:\"" << videolen<< "\",bitrate:\""<<bitrate<<"\"}";


	RELEASE_MYSQL_RETURN(psql, 0);


}

int OilSpillSvc::DelOilSpillSubTask(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::GetOilSpillSubTaskList]bad format:", jsonString, 1);

	string subtaskid=root.getv("subid", "");
	string subtype=root.getv("subtype","-1");
	
	MySql* psql = CREATE_MYSQL;

	char sql[1024] = "";

	if (subtype=="0")//0：应急措施 
	{
		sprintf (sql, "delete from T41_oilspill_subtask_emergence where SUBTASK_ID='%s'",subtaskid.c_str());
	}
	else if (subtype=="1")//1：观测值
	{
		sprintf (sql, "delete from T41_oilspill_subtask_watch where SUBTASK_ID='%s'",subtaskid.c_str());
	}
	else if (subtype=="2")//2：视频
	{
		sprintf (sql, "delete from T41_oilspill_subtask_video where SUBTASK_ID='%s'",subtaskid.c_str());
	}


	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

	out << "{\"eid\":\"0\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

	return 0;

}

int OilSpillSvc::GetOilSpillTaskCalStatus(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::GetOilSpillTaskCalStatus]bad format:", jsonString, 1);
	string taskid = root.getv("osid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[256] = "";
	sprintf (sql, "select TASK_ID,TASK_NAME,SPILL_TOTAL,TASK_STATUS,ERROR_CODE from T41_oilspill_task where TASK_ID='%s'",taskid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char tasknm[256]="";
	char status[2]="";
	char errcode[16]="";

	if (psql->NextRow())
	{	
		READMYSQL_STR(TASK_NAME, tasknm);
		READMYSQL_STR(TASK_STATUS, status);
		READMYSQL_STR(ERROR_CODE, errcode);

	}

	out << "{osid:\"" << taskid <<"\",osstatus:" << status<<",errcode:\""<<errcode<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);


}

int OilSpillSvc::GetOilSpillTaskStatus(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[BerthSvc::GetOilSpillSubTaskList]bad format:", jsonString, 1);
	string taskid = root.getv("osid", "");

	MySql* psql = CREATE_MYSQL;
	
	char sql[256] = "";
	sprintf (sql, "select TASK_ID,TASK_NAME,SPILL_TOTAL,TASK_STATUS from T41_oilspill_task where TASK_ID='%s'",taskid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char tasknm[256]="";
	char spilltotal[16]="";
	char status[2]="";

	if (psql->NextRow())
	{	
		READMYSQL_STR(TASK_NAME, tasknm);
		READMYSQL_STR(SPILL_TOTAL, spilltotal);
		READMYSQL_STR(TASK_STATUS, status);
		
	}



	out << "{osid:\"" << taskid <<"\",osstatus:" << status<<",total:\""<<spilltotal<<"\",areas:[";


	
		char sql1[1024] = "";
		sprintf (sql1, "select AREA_ID,AREA_NAME,AREA_TYPE,AREA_POINTS,OIL_TON from T41_oilspill_task_area where TASK_ID='%s'",taskid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql1),3);

		char areaid[64]="";
		char areaname[128]="";
		char areatype[16]="";
		char points[512]="";
		char remainoil[16]="";
		int index=0;
		while (psql->NextRow())
		{
			READMYSQL_STR(AREA_ID, areaid)
			READMYSQL_STR(AREA_NAME, areaname)
			READMYSQL_STR(AREA_TYPE, areatype)
			READMYSQL_STR(AREA_POINTS, points)
			READMYSQL_STR(OIL_TON, remainoil)

				if (index>0)
				{
					out <<","; 
				}

				out << "{areaid:\"" << areaid<< "\",areanm:\"" << areaname << "\",areatp:\"" << areatype<<"\",points:\"" << points<<"\",oilton:\"" << remainoil<<"\"}";

				index++;
		}
        

	out <<"]}";
	


	RELEASE_MYSQL_RETURN(psql, 0);

}



int OilSpillSvc::GetOilSpillTaskTrace(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::WtiGetOilTrace]bad format:", jsonString, 1);

	string taskid = root.getv("osid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";
	sprintf (sql, "select ANALYSES_TM,LATITUDE,LONGITUDE from T41_oilspill_track where TASK_ID='%s' and IS_VALID='1'",taskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	long starttm=0;
	long endtm=0;
	long datatime=0;
	char x[32]="";
	char y[32]="";
	int cnt=0;

	out << "{points:[";
	while (psql->NextRow())
	{	
		READMYSQL_INT(ANALYSES_TM,datatime,0)
			READMYSQL_STR(LONGITUDE,x)
			READMYSQL_STR(LATITUDE,y)

			if (cnt==0)
			{
				starttm=datatime;
				endtm=datatime;
			}
			else
			{
				if (datatime<starttm)
				{
					starttm=datatime;
				}
				if (datatime>endtm)
				{
					endtm=datatime;
				}

				out << ",";
			}

			out << "{tm:" << datatime << ",x:\"" << x<< "\",y:\"" << y<<"\"}";
			cnt++;
	}
	out << "],starttm:"<<starttm<<",endtm:"<<endtm<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}

int OilSpillSvc::WtiGetOilTrace(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[OilSpillSvc::WtiGetOilTrace]bad format:", jsonString, 1);

	//string bid = root.getv("bid", "");
	string taskid = root.getv("osid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";
	sprintf (sql, "select ANALYSES_TM,LATITUDE,LONGITUDE from T41_oilspill_wti_track where TASK_ID='%s' and IS_VALID='1'",taskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	long starttm=0;
	long endtm=0;
	long datatime=0;
	char x[32]="";
	char y[32]="";
	int cnt=0;

	out << "{points:[";
	while (psql->NextRow())
	{	
	    READMYSQL_INT(ANALYSES_TM,datatime,0)
		READMYSQL_STR(LONGITUDE,x)
		READMYSQL_STR(LATITUDE,y)

		if (cnt==0)
		{
			starttm=datatime;
			endtm=datatime;
		}
		else
		{
			if (datatime<starttm)
			{
                 starttm=datatime;
			}
			if (datatime>endtm)
			{
				endtm=datatime;
			}

			out << ",";
		}
				
		out << "{tm:" << datatime << ",x:\"" << x<< "\",y:\"" << y<<"\"}";
		cnt++;
	}
	out << "],starttm:"<<starttm<<",endtm:"<<endtm<<"}";


	RELEASE_MYSQL_RETURN(psql, 0);

}
int OilSpillSvc::WtiGetDispPoints(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilSpillTaskList]bad format:", jsonString, 1);

	string taskid = root.getv("osid", "");
    long datatime = root.getv("tm", 0);

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";
	sprintf (sql, "select OIL_AREA,OIL_THICK from T41_oilspill_wti_result where TASK_ID='%s' and ANALYSES_TM='%d'",taskid.c_str(),datatime);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;
	string oilarea="";
	float thick=0.0;
	
	out << "{details:[";
	
	while (psql->NextRow())
	{	

		READMYSQL_STRING(OIL_AREA, oilarea);
		READMYSQL_FLOAT(OIL_THICK,thick,0.0)

			if (cnt>0)
				out << ",";
		out << "{tk:\"" << thick << "\",pts:\"" << oilarea<< "\"}";
		cnt++;
	}
	out << "],osid:\""<<taskid<<"\",tm:"<<datatime<<"}";


	RELEASE_MYSQL_RETURN(psql, 0);


}
int OilSpillSvc::WtiGetPointDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::WtiGetPointDetail]bad format:", jsonString, 1);

	string taskid = root.getv("osid", "");
	long datatime = root.getv("tm", 0);

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";
	sprintf (sql, "select OIL_AREA,THICKNESS,MASS,VISCOSITY,WATERCONTE,STATUS from T41_oilspill_wti_point_detail where TASK_ID='%s' and ANALYSES_TM='%d'",taskid.c_str(),datatime);
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;
	char area[128]="";
	float thick=0.0;
	float mass=0.0;
	float viscosity=0.0;
	float watercont=0.0;
	int stats=0;
	out << "{details:[";
	while (psql->NextRow())
	{	
		READMYSQL_STR(OIL_AREA, area);
		READMYSQL_FLOAT(THICKNESS,thick,0.0)
			READMYSQL_FLOAT(MASS,mass,0.0)
			READMYSQL_FLOAT(VISCOSITY,viscosity,0.0)
			READMYSQL_FLOAT(WATERCONTE,watercont,0.0)
			READMYSQL_INT(STATUS,stats,0)

			if (cnt>0)
				out << ",";
		out << "{pos:\"" << area << "\",oil:\"" << mass<< "\",thick:\"" << thick<< "\",visco:\"" << viscosity << "\",wcon:\""<<watercont<< "\",status:\""<<stats<<"\"}";
		cnt++;
	}
	out << "],osid:\""<<taskid<<"\",tm:"<<datatime<<"}";


	RELEASE_MYSQL_RETURN(psql, 0);

}


//请求所有溢油作业船舶列表0x21
int OilSpillSvc::GetOilShiplist(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

    string strVid=root.getv("vid", "");//版本号ID


    char sql[512] = "";
	sprintf (sql, "select * from boloomodb.T41_oilspill_ship T1 where T1.IS_VALID='1' ");
    MySql* psql = CREATE_MYSQL;
   	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;

	char shipid[128]="";
	char nm_cn[128]="";
	char nm_en[128]="";
	char mmsi[128]="";
	char type[128]="";
	char imo[128]="";
	char callsign[128]="";

	out<<"[";
	while (psql->NextRow())
	{
		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(NAME_CN, nm_cn);
		READMYSQL_STR(NAME_EN, nm_en);
		READMYSQL_STR(MMSI, mmsi);
		READMYSQL_STR(SHIPTYPE,type);
		READMYSQL_STR(IMO,imo);
		READMYSQL_STR(CALLSIGN,callsign);
		if (cnt>0)
			out << ",";
		out<<"{shipid:\"" << shipid << "\",nm_cn:\"" << nm_cn << "\",nm_en:\"" << nm_en << "\",mmsi:\"" << mmsi << "\",imo:\"" << imo << "\",callsign:\"" << callsign << "\",type:\"" << type << "\"}";
		cnt++;
	}

	out << "]";


	RELEASE_MYSQL_RETURN(psql, 0);
}

//请求溢油作业船舶联系信息0x22
int OilSpillSvc::GetOilShipRelations(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strShipid=root.getv("shipid", "");
	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";

	sprintf (sql, "select * from boloomodb.T41_oilspill_ship T1 where T1.SHIPID='%s' AND T1.IS_VALID='1' ",strShipid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char shipid[128]="";
	char nm_cn[128]="";
	char nm_en[128]="";
	char mmsi[128]="";
	char type[128]="";
	char imo[128]="";
	char callsign[128]="";
	char department[128]="";
	char seperateflag[128]="";

	float spd=0.0;
	float loa=0.0;
	float lbp=0.0;
	float depth=0.0;
	float dwt=0.0;
	float gton=0.0;
	float capacity=0.0;
	float oilrate=0.0;
	float seppercent=0.0;
	float transrate=0.0;

	out<<"{";
	if (psql->NextRow())
	{
		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(NAME_CN, nm_cn);
		READMYSQL_STR(NAME_EN, nm_en);
		READMYSQL_STR(MMSI, mmsi);
		READMYSQL_STR(SHIPTYPE,type);
		READMYSQL_STR(IMO,imo);
		READMYSQL_STR(CALLSIGN,callsign);

		READMYSQL_FLOAT(SPEED,spd,0.0);
		READMYSQL_FLOAT(LOA,loa,0.0);
		READMYSQL_FLOAT(LBP,lbp,0.0);
		READMYSQL_FLOAT(DEPTH,depth,0.0);
		READMYSQL_FLOAT(DWT,dwt,0.0);
		READMYSQL_FLOAT(GROSS_TON,gton,0.0);
		READMYSQL_STR(DEPARTMENT,department);
		READMYSQL_FLOAT(CAPACITY,capacity,0.0);
		READMYSQL_FLOAT(OIL_RATE,oilrate,0.0);
		READMYSQL_STR(SEPARATE_FLAG,seperateflag);
		READMYSQL_FLOAT(SEPARATE_PERCENT,seppercent,0.0);
		READMYSQL_FLOAT(TRANSFER_RATE,transrate,0.0);

		out<<"shipid:\"" << shipid << "\",nm_cn:\"" << nm_cn << "\",nm_en:\"" << nm_en << "\",mmsi:\"" << mmsi << "\",imo:\"" << imo << "\",callsign:\"" << callsign << "\",tp:\"" << type << "\"";
		out<<",spd:\"" << spd << "\",loa:\"" << loa << "\",lbp:\"" << lbp << "\",depth:\"" << depth << "\",dwt:\"" << dwt << "\",gton:\"" << gton << "\",department:\"" << department << "\"";
		out<<",capacity:\"" << capacity << "\",oilrate:\"" << oilrate << "\",seperateflag:\"" << seperateflag << "\",seppercent:\"" << seppercent << "\",transrate:\"" << transrate << "\"";

	}

	sprintf (sql, "select * from boloomodb.T41_oilspill_ship_relations T1 where T1.SHIPID='%s' AND T1.IS_VALID='1' ",strShipid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;

	char usrname[128]="";
	char position[128]="";
	char tel[128]="";
	char addr[128]="";

	out<<",relations:[";
	while (psql->NextRow())
	{
		READMYSQL_STR(USRNAME, usrname);
		READMYSQL_STR(POSITION, position);
		READMYSQL_STR(TELEPHONE, tel);
		READMYSQL_STR(ADDRESS, addr);
		if (cnt>0)
			out << ",";
		out<<"{usrname:\"" << usrname << "\",position:\"" << position << "\",tel:\"" << tel << "\",addr:\"" << addr << "\"}";
		cnt++;
	}
	out << "]";
	out << "}";

	RELEASE_MYSQL_RETURN(psql, 0);
}


int OilSpillSvc::ModifyShipInfo(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::ModifyShipInfo]bad format:", jsonString, 1);

	string strShipid=root.getv("shipid","");
	string strCapacity=root.getv("capacity","");
	string strOilrate=root.getv("oilrate","");
	string strFlag=root.getv("sepflag","");
	string strSeperate=root.getv("seprate","");
	string strTransferrate=root.getv("transferrate","");

	char sql[1024] = "";

	sprintf (sql,"update boloomodb.t41_oilspill_ship \
	    	set CAPACITY='%s',OIL_RATE='%s',SEPARATE_FLAG='%s',SEPARATE_PERCENT='%s',TRANSFER_RATE='%s' \
	        where shipid='%s' ",strCapacity.c_str(),strOilrate.c_str(),strFlag.c_str(),
		    strSeperate.c_str(),strTransferrate.c_str(),strShipid.c_str());


	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<<"{eid:0, shipid:\""<<strShipid<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}

//请求物资码头列表0x23
//input
//{portid:” 56646”}
//output
//{
//docks:
//	[
//	{dockid:””,docknm:””,xpos:””,ypos:””,addr:””,remark:””,harbor:””,landar:””,waterar:””,workar:””,slen:””}, {dockid:””,docknm:””,xpos:””,ypos:””,addr:””,remark:””,harbor:””,landar:””,waterar:””,workar:””,slen:””}
//	]
//} 
int OilSpillSvc::GetOilGoodsDocklist(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strPortid=root.getv("portid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";
	sprintf (sql, "select * from boloomodb.t41_port_terminal T1 where T1.Portid='%s' AND T1.IS_VALID='1' AND T1.Location!='' AND T1.Location is not null ",strPortid.c_str());
	
    	/*FILE *pf;
	string strTmp = sql;
	pf=fopen("/tmp/json1.log","wb");
	fwrite(strTmp.c_str(),sizeof(char),strTmp.length(),pf);
	fclose(pf);*/

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;

	char dockid[128]="";
	char docknm[128]="";
	char location[128]="";
	char addr[128]="";
	char remark[128]="";
	char harbor[128]="";
	char landar[128]="";
	char waterar[128]="";
	char workar[128]="";
	double slen=0.0;


	out<<"{docks:[";
	while (psql->NextRow())
	{
		READMYSQL_STR(Terminal_id, dockid);
		READMYSQL_STR(Name, docknm);
		READMYSQL_STR(Location, location);
		//READMYSQL_STR(, ypos);
		READMYSQL_STR(LOCATION_DES, addr);
		READMYSQL_STR(Remark, remark);
		READMYSQL_STR(AreaName, harbor);
		READMYSQL_STR(LAND_AREA, landar);
		READMYSQL_STR(Area, waterar);
		READMYSQL_STR(WORK_AREA, workar);
		READMYSQL_DOUBLE(COASTLINE,slen,0.0);
		if (cnt>0)
			out << ",";
		out<<"{dockid:\"" << dockid << "\",docknm:\"" << docknm << "\",addr:\"" << addr << "\",remark:\"" << remark << "\",location:\"" << location<< "\"";
		out<<",harbor:\"" << harbor << "\",landar:\"" << landar << "\",waterar:\"" << waterar << "\",workar:\"" << workar << "\",slen:\"" << slen << "\"}";
		cnt++;
	}
	out << "]}";
	RELEASE_MYSQL_RETURN(psql, 0);

}

//请求溢油处置计划列表 0x30
//input
//{ oiltaskid:”” }
//output
//{
//oiltaskid:””,
//plans:
//	[
//	{ planid:”P0001”,shipid:”s1”,shipnm:””,type:3,subtype:2,status:1, 
//stm:”2013-12-13 10:00:00”,rtm:””,swtm:””,ewtm:””},
//	{ planid:”P0002”,shipid:”s2”,shipnm:””,type:2,subtype:1,status:1, 
//stm:””,rtm:””,swtm:””,ewtm:””}
//	……
//	]
//}

int OilSpillSvc::GetOilPlanlist(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strOiltaskid=root.getv("oiltaskid", "");

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";
	sprintf (sql, "select * from boloomodb.T41_oilspill_handleplan T1 where T1.OIL_TASK_ID='%s' AND T1.IS_VALID='1' ",strOiltaskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;

	char planid[128]="";
	char shipid[128]="";
	char shipnm[128]="";
	char type[128]="";
	char subtype[128]="";
	char stm[128]="";
	char rtm[128]="";
	char swtm[128]="";
	char ewtm[128]="";
	char status[128]="";
	char xpos[32]="";
	char ypos[32]="";


	out<<"{oiltaskid:\""<<strOiltaskid<<"\",plans:[";
	while (psql->NextRow())
	{
		READMYSQL_STR(PLAN_ID, planid);
		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(SHIPNAME, shipnm);
		READMYSQL_STR(TYPE, type);
		READMYSQL_STR(SUBTYPE, subtype);
		READMYSQL_STR(START_TIME, stm);
		READMYSQL_STR(RETURN_TIME, rtm);
		READMYSQL_STR(BEGIN_WORK_TIME, swtm);
		READMYSQL_STR(END_WORK_TIME, ewtm);
		READMYSQL_STR(STATUS,status);
		READMYSQL_STR(WORK_XPOS,xpos);
		READMYSQL_STR(WORK_YPOS,ypos);
	
		if (cnt>0)
			out << ",";
		out<<"{planid:\"" << planid << "\",shipid:\"" << shipid << "\",shipnm:\"" << shipnm << "\",type:\"" << type << "\"";
		out<<",subtype:\"" << subtype << "\",stm:\"" << stm << "\",rtm:\"" << rtm << "\",swtm:\"" << swtm << "\",ewtm:\""; 
		out<< ewtm << "\",status:\"" << status <<"\",xpos:\"" << xpos <<"\",ypos:\"" << ypos << "\"}";
		cnt++;
	}
	out << "]}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//溢油处置计划详细信息 0x31
//input
//{planid:””, oiltaskid:”’}
//output
//{
//planid:””,oiltaskid:””,shipid:””,shipnm:””,type:1,xpos:””,ypos:””,workdura:””,dockid:””,loaddura:””, goodstype :””, amount :””,
//voyage:
//[
//{seq:1,xpos:””,ypos:””,posdes:””,distance:””,spd:””,saildura:””,workdura:””,delay:””,eta:””,etd:””,ata:””,atd:””,status:1},
//…….],
//supplyapply:
//[
//{applyid:””,state:0,plantm:””,plandura:””,desc:””,xpos:””,ypos:””,posdes:””,shipid:””,shipnm:””,starttm:””,finishtm:””
//}
//…
//]
//}

int OilSpillSvc::GetOilPlanDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strOiltaskid=root.getv("oiltaskid", "");
	string strPlanid=root.getv("planid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";

	sprintf(sql,"SELECT * FROM boloomodb.t41_oilspill_handleplan T1 WHERE T1.PLAN_ID='%s' AND T1.OIL_TASK_ID='%s'",strPlanid.c_str(),strOiltaskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char planid[128]="";
	char oiltaskid[128]="";
	char shipid[128]="";
	char shipnm[128]="";
	char type[128]="";
	float xpos=0.0;
	float ypos=0.0;
	float workdura=0.0;
	char dockid[128]="";
	float loaddura=0.0;
	char goodstype[128]="";
	float amount=0.0;

	out<<"{";
	if (psql->NextRow())
	{
		READMYSQL_STR(PLAN_ID, planid);
		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(SHIPNAME, shipnm);
		READMYSQL_STR(SUBTYPE, type);
		READMYSQL_STR(OIL_TASK_ID, oiltaskid);
		READMYSQL_FLOAT(WORK_XPOS, xpos,0.0);
		READMYSQL_FLOAT(WORK_YPOS, ypos,0.0);
		READMYSQL_FLOAT(WORK_DURATION, workdura,0.0);
		READMYSQL_STR(DOCK_ID, dockid);
		READMYSQL_FLOAT(LOAD_DURATION,loaddura,0.0);
		READMYSQL_STR(GOODS_TYPE,goodstype);
		READMYSQL_FLOAT(LOAD_DURATION,amount,0.0);
	}

	out<<"planid:\"" << planid << "\",oiltaskid:\"" << oiltaskid << "\",shipid:\"" << shipid << "\",shipnm:\"" << shipnm << "\"";
	out<<",type:\"" << type << "\",xpos:\"" << xpos << "\",ypos:\"" << ypos << "\",workdura:\"" << workdura << "\",dockid:\"" << dockid << "\"";
	out<<",loaddura:\"" << loaddura << "\",goodstype:\"" << goodstype << "\",amount:\"" << amount << "\"";


	sprintf(sql,"SELECT * FROM boloomodb.t41_oilspill_plan_voyage T1 WHERE T1.PLAN_ID='%s'",strPlanid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int seq=0;
	char posdes[128]="";
	float xpos2=0.0;
	float ypos2=0.0;
	float distance=0.0;
	float spd=0.0;
	float saildura=0.0;
	float workdura2=0.0;
	float delay=0.0;
	unsigned int eta=0;
	unsigned int etd=0;
	unsigned int ata=0;
	unsigned int atd=0;
	char status[128]="";

	int cnt=0;
	out<<",voyage:[";
	while (psql->NextRow())
	{
		READMYSQL_INT(SEQ, seq,0);
		READMYSQL_STR(POSITION_DES, posdes);
		READMYSQL_FLOAT(XPOS, xpos2,0.0);
		READMYSQL_FLOAT(YPOS, ypos2,0.0);
		READMYSQL_FLOAT(DISTANCE, distance,0.0);
		READMYSQL_FLOAT(SPEED, spd,0.0);
		READMYSQL_FLOAT(VOY_DURATION, saildura,0.0);
		READMYSQL_FLOAT(WORK_DURATION, workdura2,0.0);
		READMYSQL_FLOAT(DELAY, delay,0.0);
		READMYSQL_INT(ETA, eta,0);
		READMYSQL_INT(ETD, etd,0);
		READMYSQL_INT(ATA, ata,0);
		READMYSQL_INT(ATD, atd,0);
		READMYSQL_STR(VOY_STATUS,status);
		if (cnt>0)
			out << ",";
		out<<"{seq:\"" << seq << "\",posdes:\"" << posdes << "\",xpos:\"" << xpos2 << "\",ypos:\"" << ypos2 << "\"";
		out<<",distance:\"" << distance << "\",spd:\"" << spd << "\",saildura:\"" << saildura << "\",workdura:\"" << workdura2 << "\",delay:\"" << delay << "\"";
		out<<",eta:\"" << eta << "\",etd:\"" << etd << "\",ata:\"" << ata << "\",atd:\"" << atd << "\",status:\"" << status << "\"}";
		cnt++;
	}

	out<<"]";



	sprintf(sql,"SELECT * FROM boloomodb.t41_oilspill_ensuresupplyapply T1 WHERE T1.PLAN_ID='%s' AND T1.OIL_TASK_ID='%s' AND T1.IS_VALID='1'",strPlanid.c_str(),strOiltaskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char applyid[128]="";
	char plantm[128]="";
	float plandura=0.0;
	char desc[1280]="";
	float xpos3=0.0;
	float ypos3=0.0;
	char posdes2[128]="";
	char state[128]="";
	char shipid2[128]="";
	char shipnm2[128]="";
	char starttm[128]="";
	char finishtm[128]="";


	cnt=0;
	out<<",supplyapply:[";
	while (psql->NextRow())
	{
		READMYSQL_STR(APPLY_ID, applyid);
		READMYSQL_STR(PLANTIME, plantm);
		READMYSQL_FLOAT(WORK_DURATION, plandura,0.0);
		READMYSQL_STR(GOODS_DESC, desc);
		READMYSQL_STR(POSITION_DES, posdes2);
		READMYSQL_FLOAT(WORK_XPOS, xpos3,0.0);
		READMYSQL_FLOAT(WORK_YPOS, ypos3,0.0);
		READMYSQL_STR(STATE,state);
		READMYSQL_STR(SHIPID, shipid2);
		READMYSQL_STR(SHIPNAME, shipnm2);
		READMYSQL_STR(STARTTIME, starttm);
		READMYSQL_STR(FINISHTIME, finishtm);
		if (cnt>0)
			out << ",";
		out<<"{applyid:\"" << applyid << "\",plantm:\"" << plantm << "\",plandura:\"" << plandura << "\",desc:\"" << desc << "\"";
		out<<",posdes:\"" << posdes2 << "\",xpos:\"" << xpos3 << "\",ypos:\"" << ypos3 << "\",state:\"" << state << "\",shipid:\"" << shipid2 << "\"";
		out<<",shipnm:\"" << shipnm2 << "\",starttm:\"" << starttm << "\",finishtm:\"" << finishtm << "\"}";
		cnt++;
	}

	out<<"]";



	sprintf(sql,"SELECT * FROM boloomodb.t41_oilspill_transferapply T1 WHERE T1.PLAN_ID='%s' AND T1.OIL_TASK_ID='%s' AND T1.IS_VALID='1'",strPlanid.c_str(),strOiltaskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char applyid2[128]="";
	char shipid3[128]="";
	char shipnm3[128]="";
	float xpos4=0.0;
	float ypos4=0.0;
	float totaloil=0.0;
	float transferoil=0.0;
	float tfrrate=0.0;
	float tfrduration=0.0;

	char posdes3[128]="";
	char state2[128]="";

	char starttm2[128]="";
	char finishtm2[128]="";
	char plantm2[128]="";


	cnt=0;
	out<<",transferapply:[";
	while (psql->NextRow())
	{
		READMYSQL_STR(APPLY_ID, applyid2);
		READMYSQL_STR(STATE, state2);
		READMYSQL_STR(PLANTIME, plantm2);
		READMYSQL_STR(POSITION_DES, posdes3);
		READMYSQL_FLOAT(WORK_XPOS, xpos4,0.0);
		READMYSQL_FLOAT(WORK_YPOS, ypos4,0.0);

		READMYSQL_FLOAT(OIL_TOTAL, totaloil,0.0);
		READMYSQL_FLOAT(OIL_TRANSFER, transferoil,0.0);
		READMYSQL_FLOAT(TRANSFER_EFFICIENCY, tfrrate,0.0);
		READMYSQL_FLOAT(TRANSFER_DURATION, tfrduration,0.0);

		READMYSQL_STR(SHIPID, shipid3);
		READMYSQL_STR(SHIPNAME, shipnm3);
		READMYSQL_STR(STARTTIME, starttm2);
		READMYSQL_STR(FINISHTIME, finishtm2);
		if (cnt>0)
			out << ",";
		out<<"{applyid:\"" << applyid2 << "\",plantm:\"" << plantm2 << "\",totaloil:\"" << totaloil << "\",tfrrate:\"" << tfrrate << "\",tfrduration:\"" << tfrduration << "\",transferoil:\"" << transferoil << "\"";
		out<<",posdes:\"" << posdes3 << "\",xpos:\"" << xpos4 << "\",ypos:\"" << ypos4 << "\",state:\"" << state2 << "\",shipid:\"" << shipid3 << "\"";
		out<<",shipnm:\"" << shipnm3 << "\",starttm:\"" << starttm2 << "\",finishtm:\"" << finishtm2 << "\"}";
		cnt++;
	}

	out<<"]";

	out<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//新建(修改)溢油处置计划 0x32
//input
//{bflag:1,planid:””,oiltaskid:””,shipid:””,shipnm:””,type:1,xpos:””,ypos:””,workdura:””,dockid:””,docknm:””,loaddura:””, goodstype :””, amount :””,voyage:
//[
//{seq:1,xpos:””,ypos:””,posdes:””,distance:””,spd:””,saildura:””,workdura:””,delay:””,eta:””,etd:””,ata:””,atd:””,status:1},
//…….
//],
//supplyapply:
//[
//{
//applyid:”1” ,xpos:””,ypos:””,posdes:””,planworktime:””,planworkdura:””, desc :””
//}
//…
//]
//}
//output
//{eid:0,planid:””，apply:”1|2233;2|323233”}

int OilSpillSvc::NewModifyOilPlan(const char* pUid, const char* jsonString, std::stringstream& out)
{

	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string currTime = OilSpillGetCurrentTmStr();

	int flag = root.getv("bflag", -1);
	string strOiltaskid=root.getv("oiltaskid", "");
	string strPlanid=root.getv("planid", "");
	string shipid=root.getv("shipid", "");
    int subtype=root.getv("type", -1);
	string xpos=root.getv("xpos", "");
	string ypos=root.getv("ypos", "");
	string workdura=root.getv("workdura", "");
	string dockid=root.getv("dockid", "");
	string shipnm=root.getv("shipnm", "");
	string loaddura=root.getv("loaddura", "");
	string goodstype=root.getv("goodstype", "");
	string amount=root.getv("amount", "");

	string startdt=root.getv("starttm", "");
	string enddt=root.getv("endtm", "");
	string workstartdt=root.getv("startworktm", "");
	string workenddt=root.getv("endworktm", "");


	Json* voyages = root.get("voyage");
	Json* supplyapplys = root.get("supplyapply");
	Json* transfers = root.get("transfer");

	time_t t = time(NULL);
	char time[30]="";
	sprintf(time,"%d",t);

	char sql[512]="";
	if (flag==0)//新增
	{	
		strPlanid = string("JH")+string(pUid)+string(time);
		sprintf (sql,"INSERT INTO boloomodb.t41_oilspill_handleplan (PLAN_ID,OIL_TASK_ID,SHIPID,SHIPNAME,SUBTYPE,WORK_XPOS,WORK_YPOS,\
					 DOCK_ID,WORK_DURATION,LOAD_DURATION,GOODS_TYPE,GOODS_NUM,START_TIME,RETURN_TIME,BEGIN_WORK_TIME,END_WORK_TIME,IS_VALID,STATUS,TYPE) \
					 VALUES ('%s','%s','%s','%s','%d','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','1','1','1')",\
					 strPlanid.c_str(), strOiltaskid.c_str(),shipid.c_str(),shipnm.c_str(),subtype,xpos.c_str(),ypos.c_str(),dockid.c_str(),workdura.c_str()
					 ,loaddura.c_str(),goodstype.c_str(),amount.c_str(),startdt.c_str(),enddt.c_str(),workstartdt.c_str(),workenddt.c_str());
	}
	else if (flag==1)//修改
	{
		sprintf (sql,"UPDATE boloomodb.t41_oilspill_handleplan T1 SET PLAN_ID='%s',OIL_TASK_ID='%s',SHIPID='%s',SUBTYPE='%d',WORK_XPOS='%s',WORK_YPOS='%s',\
					 DOCK_ID='%s',WORK_DURATION='%s',LOAD_DURATION='%s',GOODS_TYPE='%s',GOODS_NUM='%s',START_TIME='%s',RETURN_TIME='%s',BEGIN_WORK_TIME='%s',\
					 END_WORK_TIME='%s' WHERE T1.OIL_TASK_ID='%s' AND T1.PLAN_ID='%s'",\
					 strPlanid.c_str(), strOiltaskid.c_str(),shipid.c_str(),subtype,xpos.c_str(),ypos.c_str(),dockid.c_str(),workdura.c_str(),
					 loaddura.c_str(),goodstype.c_str(),amount.c_str(),startdt.c_str(),enddt.c_str(),workstartdt.c_str(),workenddt.c_str(),strOiltaskid.c_str(),strPlanid.c_str());
	}

	CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);


	if (voyages)
	{
		for (int i=0;i<voyages->size();i++)
		{
			Json* voyageItem=voyages->get(i);
			if (!voyageItem)
			{
				continue;
			}


			string xpos2=voyageItem->getv("xpos","");
			string ypos2=voyageItem->getv("ypos","");
			string posdes=voyageItem->getv("posdes","");
			int seq=voyageItem->getv("seq",0);
			string distance=voyageItem->getv("distance","");
			string spd=voyageItem->getv("spd","");
			string saildura=voyageItem->getv("saildura","");
			string workdura=voyageItem->getv("workdura","");
			string delay=voyageItem->getv("delay","");
			string eta=voyageItem->getv("eta","");
			string etd=voyageItem->getv("etd","");
			string ata=voyageItem->getv("ata","");
			string atd=voyageItem->getv("atd","");
			string status=voyageItem->getv("status","");

			char sql1[512]="";
			if (flag==0)//新增
			{
				sprintf (sql1,"INSERT INTO boloomodb.t41_oilspill_plan_voyage (PLAN_ID,SEQ,XPOS,YPOS,POSITION_DES,DISTANCE,SPEED,VOY_DURATION,WORK_DURATION,DELAY,ETA,ETD,ATA,ATD,VOY_STATUS,IS_VALID) \
							  VALUES ('%s',%d,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','1')",strPlanid.c_str(),seq,xpos2.c_str(),ypos2.c_str(),posdes.c_str(),distance.c_str(),\
							  spd.c_str(),saildura.c_str(),workdura.c_str(),delay.c_str(),eta.c_str(),etd.c_str(),ata.c_str(),atd.c_str(),status.c_str());
			}
			else if (flag==1)//修改
			{
				sprintf (sql1,"UPDATE boloomodb.t41_oilspill_plan_voyage T1 SET XPOS='%s',YPOS='%s',POSITION_DES='%s',DISTANCE='%s',SPEED='%s',VOY_DURATION='%s',WORK_DURATION='%s',DELAY='%s',ETA='%s',ETD='%s',ATA='%s',ATD='%s',VOY_STATUS='%s' \
							   WHERE T1.PLAN_ID='%s' AND T1.SEQ=%d",xpos2.c_str(),ypos2.c_str(),posdes.c_str(),distance.c_str(),\
							  spd.c_str(),saildura.c_str(),workdura.c_str(),delay.c_str(),eta.c_str(),etd.c_str(),ata.c_str(),atd.c_str(),status.c_str(),strPlanid.c_str(),seq);
			}
			
			CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);

		}
	}

	char idx[10]="";
	if (supplyapplys)
	{
		for (int i=0;i<supplyapplys->size();i++)
		{
			Json* supplyapplyItem=supplyapplys->get(i);
			if (!supplyapplyItem)
			{
				continue;
			}


			string applyid=supplyapplyItem->getv("applyid","");
			string xpos3=supplyapplyItem->getv("xpos","");
			string ypos3=supplyapplyItem->getv("ypos","");
			string posdes2=supplyapplyItem->getv("posdes","");
			string planworktime=supplyapplyItem->getv("planworktime","");
			string planworkdura=supplyapplyItem->getv("planworkdura","");
			string desc=supplyapplyItem->getv("desc","");


			char sql2[512]="";
			if (flag==0)//新增
			{
				sprintf(idx,"_%d",i);
				applyid = string("ES")+string(pUid)+string(time)+string(idx);

				sprintf (sql2,"INSERT INTO boloomodb.t41_oilspill_ensuresupplyapply (APPLY_ID,PLAN_ID,OIL_TASK_ID,WORK_XPOS,WORK_YPOS,WORK_POSITION,PLANTIME,WORK_DURATION,GOODS_DESC,IS_VALID,LAST_UPD_DT,STATE) \
							  VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','%s','1','%s','0')",applyid.c_str(),strPlanid.c_str(),strOiltaskid.c_str(),xpos3.c_str(),ypos3.c_str(),posdes2.c_str(),planworktime.c_str(),\
							  planworkdura.c_str(),desc.c_str(),currTime.c_str());
			}
			else if (flag==1)//修改
			{
				sprintf (sql2,"UPDATE boloomodb.t41_oilspill_ensuresupplyapply T1 SET WORK_XPOS='%s',WORK_YPOS='%s',WORK_POSITION='%s',PLANTIME='%s',WORK_DURATION='%s',GOODS_DESC='%s',LAST_UPD_DT='%s' \
							  WHERE T1.APPLY_ID='%s' ",xpos3.c_str(),ypos3.c_str(),posdes2.c_str(),planworktime.c_str(),
							  planworkdura.c_str(),desc.c_str(),currTime.c_str(),applyid.c_str());
			}

			CHECK_MYSQL_STATUS(psql->Execute(sql2)>=0, 3);

		}
	}


	if (transfers)
	{
		for (int i=0;i<transfers->size();i++)
		{
			Json* transferItem=transfers->get(i);
			if (!transferItem)
			{
				continue;
			}

			string xpos2=transferItem->getv("xpos","");
			string ypos2=transferItem->getv("ypos","");
			string posdes=transferItem->getv("posdes","");

			string applyid=transferItem->getv("applyid","");
			string planworktime=transferItem->getv("planworktime","");
			string totaloil=transferItem->getv("totaloil","");
			string transferoil=transferItem->getv("transferoil","");

			char sql3[512]="";
			if (flag==0)//新增
			{
				sprintf(idx,"_%d",i);
				applyid = string("TR")+string(pUid)+string(time)+string(idx);

				sprintf (sql3,"INSERT INTO boloomodb.t41_oilspill_transferapply (APPLY_ID,PLAN_ID,OIL_TASK_ID,WORK_XPOS,WORK_YPOS,WORK_POSITION,PLANTIME,OIL_TOTAL,OIL_TRANSFER,IS_VALID,LAST_UPD_DT,STATE) \
							  VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','%s','1','%s','0')",applyid.c_str(),strPlanid.c_str(),strOiltaskid.c_str(),xpos2.c_str(),ypos2.c_str(),posdes.c_str(),planworktime.c_str(),totaloil.c_str(),transferoil.c_str(),currTime.c_str());
			}
			else if (flag==1)//修改
			{
				sprintf (sql3,"UPDATE boloomodb.t41_oilspill_transferapply T1 SET WORK_XPOS='%s',WORK_YPOS='%s',WORK_POSITION='%s',PLANTIME='%s',OIL_TOTAL='%s',OIL_TRANSFER='%s' \
							   WHERE T1.APPLY_ID='%s' ",xpos2.c_str(),ypos2.c_str(),posdes.c_str(),planworktime.c_str(),\
							  totaloil.c_str(),transferoil.c_str(),applyid.c_str());
			}

			
			CHECK_MYSQL_STATUS(psql->Execute(sql3)>=0, 3);

		}
	}

	out<<"{eid:0, planid:\""<<strPlanid<<"\",apply:\""<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);
}
//溢油处置计划操作（出发、开始作业、结束作业、删除计划） 0x33
//input
//{planid:””,oiltaskid:””,operation:1}
//output
//  {eid:0, planid:””}
int OilSpillSvc::OperatOilPlan(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strOiltaskid=root.getv("oiltaskid", "");
	string strPlanid=root.getv("planid", "");
	int operation=root.getv("operation", 0);

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";

	// 0删除 1出发 2开始作业 3结束作业 
	if (operation==0)
	{
		sprintf (sql, "UPDATE boloomodb.T41_oilspill_handleplan T1 SET T1.IS_VALID='0' where (T1.OIL_TASK_ID='%s' AND T1.PLAN_ID='%s') ",strOiltaskid.c_str(),strPlanid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	}
	else if (operation==1)
	{
		sprintf (sql, "UPDATE boloomodb.T41_oilspill_handleplan T1 SET T1.STATUS='2' where (T1.OIL_TASK_ID='%s' AND T1.PLAN_ID='%s') AND T1.IS_VALID='1' ",strOiltaskid.c_str(),strPlanid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	}
	else if (operation==2)
	{
		sprintf (sql, "UPDATE boloomodb.T41_oilspill_handleplan T1 SET T1.STATUS='3' where (T1.OIL_TASK_ID='%s' AND T1.PLAN_ID='%s') AND  T1.IS_VALID='1' ",strOiltaskid.c_str(),strPlanid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	}
	else if (operation==3)
	{
		sprintf (sql, "UPDATE boloomodb.T41_oilspill_handleplan T1 SET T1.STATUS='4' where (T1.OIL_TASK_ID='%s' AND T1.PLAN_ID='%s') AND  T1.IS_VALID='1' ",strOiltaskid.c_str(),strPlanid.c_str());
		CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	}
	
	out<<"{eid:0, planid:\""<<strPlanid<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//新建（修改）保障计划申请 0x34
//input
//{bflag:1,applyid:””, planid:””,oiltaskid:””,xpos:””,ypos:””,posdes:””,planworktime:””,planworkdura:””, desc :””}
//output
// {eid:0, applyid:””}

int OilSpillSvc::NewModifySuppPlanApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	int bflag=root.getv("bflag", 0);
	string strApplyid=root.getv("applyid","");
	string strPlanid=root.getv("planid","");
	string strOiltaskid=root.getv("oiltaskid","");
	string strxpos=root.getv("xpos","");
	string strypos=root.getv("ypos","");
	string strposdes=root.getv("posdes","");
	string strPlanworktime=root.getv("planworktime","");
	string strPlanworkdura=root.getv("planworkdura","");
	string strDesc=root.getv("desc","");

	string currTime = OilSpillGetCurrentTmStr();
	char sql[512] = "";
	if (bflag==0)//新建
	{
		time_t t = time(NULL);
		char time[30]="";
		// itoa(t, time, 10);
		sprintf(time,"%d",t);
		strApplyid = string("E")+string(pUid)+string(time);
		sprintf (sql,"INSERT INTO boloomodb.t41_oilspill_ensuresupplyapply (APPLY_ID,PLAN_ID,OIL_TASK_ID,WORK_XPOS,WORK_YPOS,WORK_POSITION,PLANTIME,WORK_DURATION,GOODS_DESC,IS_VALID,LAST_UPD_DT,STATE) \
					 VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%s','1','%s','0') ",strApplyid.c_str(),strPlanid.c_str(),strOiltaskid.c_str(),strxpos.c_str(),strypos.c_str(),strposdes.c_str(),\
					 strPlanworktime.c_str(),strPlanworkdura.c_str(),strDesc.c_str(),currTime.c_str());
	}
	else{//修改
		sprintf (sql,"UPDATE boloomodb.t41_oilspill_ensuresupplyapply T1 SET WORK_XPOS='%s',WORK_YPOS='%s',WORK_POSITION='%s',PLANTIME='%s',WORK_DURATION='%s',GOODS_DESC='%s',LAST_UPD_DT='%s') \
					 WHERE T1.APPLY_ID='%s' ",strxpos.c_str(),strypos.c_str(),strposdes.c_str(),
					 strPlanworktime.c_str(),strPlanworkdura.c_str(),strDesc.c_str(),currTime.c_str(),strApplyid.c_str());
	}

	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<<"{eid:0, applyid:\""<<strApplyid<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//删除保障计划申请 0x35
//input
//{applyid:””, planid:””,oiltaskid:””}
//output
//{eid:0, applyid:””}

int OilSpillSvc::DeleteSuppPlanApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strApplyid=root.getv("applyid","");
	string strPlanid=root.getv("planid","");
	string strOiltaskid=root.getv("oiltaskid","");

	string currTime = OilSpillGetCurrentTmStr();
	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";
	sprintf (sql,"UPDATE boloomodb.t41_oilspill_ensuresupplyapply T1 SET T1.IS_VALID='0',T1.LAST_UPD_DT='%s' WHERE T1.APPLY_ID='%s' AND T1.PLAN_ID='%s' AND T1.OIL_TASK_ID='%s'",currTime.c_str(),strApplyid.c_str(),strPlanid.c_str(),strOiltaskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<<"{eid:0, applyid:\""<<strApplyid<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}

//新建（修改）转驳计划申请 0x36
//input
//{bflag:1,applyid:””,planid:””,oiltaskid:””,xpos:””,ypos:””,posdes:””,planworktime:””,totaloil:””,transferoil:”” ,tfrrate:””,tfrduration:””}
//output
// {eid:0, applyid:””}
int OilSpillSvc::NewModifyTransferPlanApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	int bflag=root.getv("bflag", 0);
	string strApplyid=root.getv("applyid","");
	string strPlanid=root.getv("planid","");
	string strOiltaskid=root.getv("oiltaskid","");
	string strxpos=root.getv("xpos","");
	string strypos=root.getv("ypos","");
	string strposdes=root.getv("posdes","");
	string strPlanworktime=root.getv("planworktime","");
	string strTotaloil=root.getv("totaloil","");
	string strTransferoil=root.getv("transferoil","");
	string strTfrrate=root.getv("tfrrate","");
	string strTfrduration=root.getv("tfrduration","");

	string currTime = OilSpillGetCurrentTmStr();
	char sql[512] = "";
	if (bflag==0)//新建
	{
		time_t t = time(NULL);
		char time[30]="";
		// itoa(t, time, 10);
		sprintf(time,"%d",t);
		strApplyid = string("T")+string(pUid)+string(time);

		sprintf (sql,"INSERT INTO boloomodb.t41_oilspill_transferapply (APPLY_ID,PLAN_ID,OIL_TASK_ID,WORK_XPOS,WORK_YPOS,WORK_POSITION,PLANTIME,OIL_TOTAL,OIL_TRANSFER,TRANSFER_EFFICIENCY,TRANSFER_DURATION,LAST_UPD_DT,IS_VALID,STATE) \
					 VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','1','0') ",strApplyid.c_str(),strPlanid.c_str(),strOiltaskid.c_str(),strxpos.c_str(),strypos.c_str(),strposdes.c_str(),\
					 strPlanworktime.c_str(),strTotaloil.c_str(),strTransferoil.c_str(),strTfrrate.c_str(),strTfrduration.c_str(),currTime.c_str());
	}
	else{//修改
		sprintf (sql,"UPDATE boloomodb.t41_oilspill_transferapply T1 SET PLAN_ID='%s',OIL_TASK_ID='%s',WORK_XPOS='%s',WORK_YPOS='%s',WORK_POSITION='%s',PLANTIME,OIL_TOTAL='%s',OIL_TRANSFER='%s',TRANSFER_EFFICIENCY='%s',TRANSFER_DURATION='%s',LAST_UPD_DT='%s') \
					 WHERE T1.APPLY_ID='%s' ",strPlanid.c_str(),strOiltaskid.c_str(),strxpos.c_str(),strypos.c_str(),strposdes.c_str(),\
					 strPlanworktime.c_str(),strTotaloil.c_str(),strTransferoil.c_str(),strTfrrate.c_str(),strTfrduration.c_str(),currTime.c_str(),strApplyid.c_str());
	}


	MySql* psql = CREATE_MYSQL;
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	out<<"{eid:0, applyid:\""<<strApplyid<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//删除转驳计划申请 0x37
//input
//{applyid:””, planid:””,oiltaskid:””}
//output
//{eid:0, applyid:””}
int OilSpillSvc::DeleteTransferPlanApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strApplyid=root.getv("applyid","");
	string strPlanid=root.getv("planid","");
	string strOiltaskid=root.getv("oiltaskid","");
	string currTime = OilSpillGetCurrentTmStr();
	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";
	sprintf (sql,"UPDATE boloomodb.t41_oilspill_transferapply T1 SET T1.IS_VALID='0',T1.LAST_UPD_DT='%s' WHERE T1.APPLY_ID='%s' AND T1.PLAN_ID='%s' AND T1.OIL_TASK_ID='%s'",currTime.c_str(),strApplyid.c_str(),strPlanid.c_str(),strOiltaskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	out<<"{eid:0, applyid:\""<<strApplyid<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}
//请求所有待处理保障计划 0x38
//input
//{oiltaskid:””}
//output
// [
//{applyid:”1”,planid:””,xpos:””,ypos:””,posdes:””,planworktime:””,planworkdura:””, desc :””},
//{applyid:”2”,planid:””,xpos:””,ypos:””,posdes:””,planworktime:””,planworkdura:””, desc :””}
//…
//]

int OilSpillSvc::GetAllSuppPlanApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strOiltaskid=root.getv("oiltaskid","");

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";

	sprintf (sql, "select *,UNIX_TIMESTAMP(PLANTIME) as PLANTIMESTAMP from boloomodb.t41_oilspill_ensuresupplyapply T1 where T1.OIL_TASK_ID='%s' AND T1.state='0' AND T1.IS_VALID='1' ",strOiltaskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;

	char planid[128]="";
	char applyid[128]="";
	float xpos=0.0;
	float ypos=0.0;
	char posdes[128]="";
	char planworktime[128]="";
	float planworkdura=0.0;
	char desc[128]="";

	out<<"[";
	while (psql->NextRow())
	{
		READMYSQL_STR(PLAN_ID, planid);
		READMYSQL_STR(APPLY_ID, applyid);
		READMYSQL_FLOAT(WORK_XPOS, xpos,0.0);
		READMYSQL_FLOAT(WORK_YPOS, ypos,0.0);
		READMYSQL_STR(WORK_POSITION, posdes);
		READMYSQL_STR(PLANTIMESTAMP, planworktime);
		READMYSQL_FLOAT(WORK_DURATION, planworkdura,0.0);
		READMYSQL_STR(GOODS_DESC, desc);

		if (cnt>0)
			out << ",";
		out<<"{planid:\"" << planid << "\",applyid:\"" << applyid << "\",xpos:\"" << xpos << "\",ypos:\"" << ypos << "\"";
		out<<",posdes:\"" << posdes << "\",planworktime:\"" << planworktime << "\",planworkdura:\"" << planworkdura << "\",desc:\"" << desc << "\"}";
		cnt++;
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//新建（修改）溢油保障计划 0x39
//input
//{bflag:1,planid:””,oiltaskid:””,shipid:””,shipnm:””,dockid:””,fromdock:1,xpos:””,ypos:””,
//voyage:
//[
//{seq:1,applyid:””,xpos:””,ypos:””,posdes:””,distance:””,spd:””,saildura:””,workdura:””,delay:””,eta:””,etd:””,ata:””,atd:””,status:1},
//…….
//]
//}
//output
// {eid:0,planid:”” }
int OilSpillSvc::NewModifySuppPlanItem(const char* pUid, const char* jsonString, std::stringstream& out)
{

   JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

   MySql* psql = CREATE_MYSQL;

   string currTime = OilSpillGetCurrentTmStr();

   int flag = root.getv("bflag", -1);
   string strOiltaskid=root.getv("oiltaskid", "");
   string strPlanid=root.getv("planid", "");
   string shipid=root.getv("shipid", "");
   string shipnm=root.getv("shipnm", "");
   string xpos=root.getv("xpos", "");
   string ypos=root.getv("ypos", "");
   string fromdock=root.getv("fromdock", "");
   string dockid=root.getv("dockid", "");

   Json* voyages = root.get("voyage");

   char sql[512]="";

   if (flag==0)//新建
   {
	   time_t t = time(NULL);
	   char time[30]="";
		// itoa(t, time, 10);
		sprintf(time,"%d",t);
	   strPlanid = string("JH")+string(pUid)+string(time);
		sprintf (sql,"INSERT INTO boloomodb.t41_oilspill_handleplan (PLAN_ID,OIL_TASK_ID,SHIPID,SHIPNAME,WORK_XPOS,WORK_YPOS,DOCK_ID,FROMDOCK_FLAG,STATUS,IS_VALID,TYPE) VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','1','1','2')",\
			strPlanid.c_str(),strOiltaskid.c_str(),shipid.c_str(),shipnm.c_str(),xpos.c_str(),ypos.c_str(),dockid.c_str(),fromdock.c_str());
   }
   else if (flag==1)
   {
	   sprintf (sql,"UPDATE boloomodb.t41_oilspill_handleplan T1 SET OIL_TASK_ID='%s',SHIPID='%s',SHIPNAME='%s',WORK_XPOS='%s',WORK_YPOS='%s',DOCK_ID='%s',FROMDOCK_FLAG='%s' WHERE T1.PLAN_ID='%s'",\
		   strOiltaskid.c_str(),shipid.c_str(),shipnm.c_str(),xpos.c_str(),ypos.c_str(),dockid.c_str(),fromdock.c_str(),strPlanid.c_str());

	
   }
   CHECK_MYSQL_STATUS(psql->Query(sql), 3);


   if (voyages)
   {
	   for (int i=0;i<voyages->size();i++)
	   {
		   Json* voyageItem=voyages->get(i);
		   if (!voyageItem)
		   {
			   continue;
		   }

		   string strApplyid = voyageItem->getv("applyid","");

		   string xpos2=voyageItem->getv("xpos","");
		   string ypos2=voyageItem->getv("ypos","");
		   string posdes=voyageItem->getv("posdes","");
		   int seq=voyageItem->getv("seq",0);
		   string distance=voyageItem->getv("distance","");
		   string spd=voyageItem->getv("spd","");
		   string saildura=voyageItem->getv("saildura","");
		   string workdura=voyageItem->getv("workdura","");
		   string delay=voyageItem->getv("delay","");
		   string eta=voyageItem->getv("eta","");
		   string etd=voyageItem->getv("etd","");
		   string ata=voyageItem->getv("ata","");
		   string atd=voyageItem->getv("atd","");
		   int status=voyageItem->getv("status",-1);
		

		   char sql1[512]="";
		   if (flag==0)
		   {
			   sprintf (sql1,"INSERT INTO boloomodb.t41_oilspill_plan_voyage (PLAN_ID,SEQ,XPOS,YPOS,POSITION_DES,DISTANCE,SPEED,VOY_DURATION,WORK_DURATION,DELAY,ETA,ETD,ATA,ATD,VOY_STATUS,APPLYID,IS_VALID) \
							 VALUES ('%s',%d,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%d','%s','1')",strPlanid.c_str(),seq,xpos2.c_str(),ypos2.c_str(),posdes.c_str(),distance.c_str(),\
							 spd.c_str(),saildura.c_str(),workdura.c_str(),delay.c_str(),eta.c_str(),etd.c_str(),ata.c_str(),atd.c_str(),status,strApplyid.c_str());
		   }
		   else if (flag==1)
		   {
			   sprintf (sql1,"UPDATE boloomodb.t41_oilspill_plan_voyage T1 SET XPOS='%s',YPOS='%s',POSITION_DES='%s',DISTANCE='%s',SPEED='%s',VOY_DURATION='%s',WORK_DURATION='%s',DELAY,ETA='%s',ETD='%s',ATA='%s',ATD='%s',VOY_STATUS='%d',APPLYID='%s' \
							 WHERE T1.PLAN_ID='%s' AND T1.SEQ=%d",xpos2.c_str(),ypos2.c_str(),posdes.c_str(),distance.c_str(),\
							 spd.c_str(),saildura.c_str(),workdura.c_str(),delay.c_str(),eta.c_str(),etd.c_str(),ata.c_str(),atd.c_str(),status,strApplyid.c_str(),strPlanid.c_str(),seq);
		   }

		   CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);


		   if (strApplyid!="")
		   {

			   time_t timeETA = atol(eta.c_str());
			   time_t timeETD = atol(etd.c_str());
			   string strETA = TimeToTimestampStr(timeETA);
			   string strETD = TimeToTimestampStr(timeETD);
			   char sql2[512]="";

			   sprintf (sql2,"UPDATE boloomodb.t41_oilspill_ensuresupplyapply T1 SET T1.SHIPID='%s',T1.SHIPNAME='%s',T1.STATE='1',T1.STARTTIME='%s',T1.FINISHTIME='%s',T1.LAST_UPD_DT='%s' WHERE T1.APPLY_ID='%s'",\
				   shipid.c_str(),shipnm.c_str(),strETA.c_str(),strETD.c_str(),currTime.c_str(),strApplyid.c_str());
			   CHECK_MYSQL_STATUS(psql->Execute(sql2)>=0, 3);
		   }

	   }
   }

   out<<"{eid:0, planid:\""<<strPlanid<<"\"}";

   RELEASE_MYSQL_RETURN(psql, 0);
}

//获取溢油保障计划详细信息0x3A
//input
//{planid:””, oiltaskid:”’}
//output
//{
//planid:””,oiltaskid:””,shipid:””,shipnm:””,dockid:””,docknm:””,fromdock:1,xpos:””,ypos:””,
//voyage:
//[
//{seq:1,applyid:””,xpos:””,ypos:””,posdes:””,distance:””,spd:””,saildura:””,workdura:””,delay:””,eta:””,etd:””,ata:””,atd:””,status:1},
//…….]
//}

int OilSpillSvc::GetOilSuppPlanDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strOiltaskid=root.getv("oiltaskid", "");
	string strPlanid=root.getv("planid", "");

	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";

	sprintf(sql,"SELECT * FROM boloomodb.t41_oilspill_handleplan T1 WHERE T1.PLAN_ID='%s' AND T1.OIL_TASK_ID='%s'",strPlanid.c_str(),strOiltaskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	char planid[128]="";
	char oiltaskid[128]="";
	char shipid[128]="";
	char shipnm[128]="";
	float xpos=0.0;
	float ypos=0.0;
	char dockid[128]="";
	char docknm[128]="";
	char fromdock[128]="";

	out<<"{";
	if (psql->NextRow())
	{
		READMYSQL_STR(PLAN_ID, planid);
		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(SHIPNAME, shipnm);
		READMYSQL_STR(OIL_TASK_ID, oiltaskid);
		READMYSQL_FLOAT(WORK_XPOS, xpos,0.0);
		READMYSQL_FLOAT(WORK_YPOS, ypos,0.0);
		READMYSQL_STR(DOCK_ID, dockid);
		READMYSQL_STR(FROMDOCK_FLAG,fromdock);
	}

	out<<"planid:\"" << planid << "\",oiltaskid:\"" << oiltaskid << "\",shipid:\"" << shipid << "\",shipnm:\"" << shipnm << "\"";
	out<<",xpos:\"" << xpos << "\",ypos:\"" << ypos << "\",fromdock:\"" << fromdock << "\",dockid:\"" << dockid << "\"";



	sprintf(sql,"SELECT T1.* FROM boloomodb.t41_oilspill_plan_voyage T1 WHERE T1.PLAN_ID='%s'",strPlanid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int seq=0;
	char posdes[128]="";
	float xpos2=0.0;
	float ypos2=0.0;
	float distance=0.0;
	float spd=0.0;
	float saildura=0.0;
	float workdura2=0.0;
	float delay=0.0;
	unsigned int eta=0;
	unsigned int etd=0;
	unsigned int ata=0;
	unsigned int atd=0;
	char status[128]="";

	char applyId[128]="";

	int cnt=0;
	out<<",voyage:[";
	while (psql->NextRow())
	{
		READMYSQL_INT(SEQ, seq,0);
		READMYSQL_STR(APPLYID, applyId);
		READMYSQL_STR(POSITION_DES, posdes);
		READMYSQL_FLOAT(XPOS, xpos2,0.0);
		READMYSQL_FLOAT(YPOS, ypos2,0.0);
		READMYSQL_FLOAT(DISTANCE, distance,0.0);
		READMYSQL_FLOAT(SPEED, spd,0.0);
		READMYSQL_FLOAT(VOY_DURATION, saildura,0.0);
		READMYSQL_FLOAT(WORK_DURATION, workdura2,0.0);
		READMYSQL_FLOAT(DELAY, delay,0.0);
		READMYSQL_INT(ETA, eta,0);
		READMYSQL_INT(ETD, etd,0);
		READMYSQL_INT(ATA, ata,0);
		READMYSQL_INT(ATD, atd,0);
		READMYSQL_STR(VOY_STATUS,status);
		if (cnt>0)
			out << ",";
		out<<"{seq:\"" << seq << "\",applyid:\"" << applyId << "\",posdes:\"" << posdes << "\",xpos:\"" << xpos2 << "\",ypos:\"" << ypos2 << "\"";
		out<<",distance:\"" << distance << "\",spd:\"" << spd << "\",saildura:\"" << saildura << "\",workdura:\"" << workdura2 << "\",delay:\"" << delay << "\"";
		out<<",eta:\"" << eta << "\",etd:\"" << etd << "\",ata:\"" << ata << "\",atd:\"" << atd << "\",status:\"" << status << "\"}";
		cnt++;
	}

	out<<"]";

	out<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//删除保障计划0x3B
//input
//{planid:””,oiltaskid:””}
//output
//{eid:0, planid:””}
int OilSpillSvc::DeleteSuppPlanItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strPlanid=root.getv("planid","");
	string strOiltaskid=root.getv("oiltaskid","");

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";

	sprintf (sql,"UPDATE boloomodb.t41_oilspill_handleplan T1 SET T1.IS_VALID='0' WHERE T1.PLAN_ID='%s' AND T1.OIL_TASK_ID='%s'",strPlanid.c_str(),strOiltaskid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql), 3);

	//更新申请表的状态、删除保障航次
	sprintf(sql,"select APPLYID from boloomodb.T41_oilspill_plan_voyage where plan_id='%s'",strPlanid.c_str());
    CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	vector<string> vecApplyids;
	while (psql->NextRow())
	{
		char applyid[256]="";
		READMYSQL_STR(APPLYID, applyid);
        vecApplyids.push_back(applyid);
	}
    
	//更新
    for (int i=0;i<vecApplyids.size();i++)
    {
		sprintf (sql,"UPDATE boloomodb.t41_oilspill_ensuresupplyapply T1 SET T1.SHIPID='',T1.SHIPNAME='',T1.STATE='0',\
			T1.STARTTIME='',T1.FINISHTIME='' WHERE T1.APPLY_ID='%s'",vecApplyids[i].c_str());
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);

    }
	//删除
	sprintf(sql,"delete from boloomodb.T41_oilspill_plan_voyage where plan_id='%s'",strPlanid.c_str());
	CHECK_MYSQL_STATUS(psql->Execute(sql), 3);

    

		

	out<<"{eid:0, planid:\""<<strPlanid<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}

//请求所有待处理转驳计划 0x3C
//input
//{oiltaskid:””}
//[
//{applyid:”1”,planid:””,xpos:””,ypos:””,posdes:””,planworktime:””,totaloil:””,transferoil:””,
//tfrrate:””,tfrduration:””
//},
//{applyid:”2”,planid:””,xpos:””,ypos:””,posdes:””,planworktime:””,totaloil:””,transferoil:””,
//tfrrate:””,tfrduration:””}
//]

int OilSpillSvc::GetAllTransferPlanApply(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strOiltaskid=root.getv("oiltaskid","");

	MySql* psql = CREATE_MYSQL;

	char sql[512] = "";

	sprintf (sql, "select T1.*,UNIX_TIMESTAMP(T1.PLANTIME) AS PLANTIME_UX from boloomodb.t41_oilspill_transferapply T1 where T1.OIL_TASK_ID='%s' AND T1.state='0' AND T1.IS_VALID='1' ",strOiltaskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int cnt = 0;

	char planid[128]="";
	char applyid[128]="";
	double xpos=0.0;
	double ypos=0.0;
	char posdes[128]="";
	char planworktime[128]="";

	float totaloil=0.0;
	float transferoil=0.0;
	float tfrrate=0.0;
	float tfrduration=0.0;


	out<<"[";
	while (psql->NextRow())
	{
		READMYSQL_STR(PLAN_ID, planid);
		READMYSQL_STR(APPLY_ID, applyid);
		READMYSQL_DOUBLE(WORK_XPOS, xpos,0.0);
		READMYSQL_DOUBLE(WORK_YPOS, ypos,0.0);
		READMYSQL_STR(WORK_POSITION, posdes);
		READMYSQL_STR(PLANTIME_UX, planworktime);
		READMYSQL_FLOAT(OIL_TOTAL, totaloil,0.0);
		READMYSQL_FLOAT(OIL_TRANSFER, transferoil,0.0);
		READMYSQL_FLOAT(TRANSFER_EFFICIENCY, tfrrate,0.0);
		READMYSQL_FLOAT(TRANSFER_DURATION, tfrduration,0.0);

		if (cnt>0)
			out << ",";
		out<<"{planid:\"" << planid << "\",applyid:\"" << applyid << "\",xpos:\"" << xpos << "\",ypos:\"" << ypos << "\"";
		out<<",posdes:\"" << posdes << "\",planworktime:\"" << planworktime << "\",totaloil:\"" << totaloil << "\"";
		out<<",transferoil:\"" << transferoil << "\",tfrrate:\"" << tfrrate << "\",tfrduration:\"" << tfrduration << "\"}";
		cnt++;
	}
	out << "]";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//新建（修改）溢油转驳计划 0x3D
//input
//{bflag:1,planid:””,oiltaskid:””,shipid:””,shipnm:””,dockid:””,fromdock:1,xpos:””,ypos:””,
//voyage:
//[
//{seq:1,applyid:””,xpos:””,ypos:””,posdes:””,distance:””,spd:””,saildura:””,workdura:””,delay:””,eta:””,etd:””,ata:””,atd:””,status:1},
//…….
//]
//}
//output
//{eid:0,planid:”” }
int OilSpillSvc::NewModifyTransferPlanItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	MySql* psql = CREATE_MYSQL;

	string currTime = OilSpillGetCurrentTmStr();

	int flag = root.getv("bflag", -1);
	string strOiltaskid=root.getv("oiltaskid", "");
	string strPlanid=root.getv("planid", "");
	string shipid=root.getv("shipid", "");
	string shipnm=root.getv("shipnm", "");
	string xpos=root.getv("xpos", "");
	string ypos=root.getv("ypos", "");
	int fromdock=root.getv("fromdock", -1);
	string dockid=root.getv("dockid", "");

	Json* voyages = root.get("voyage");

	char sql[512]="";

	if (flag==0)//新建
	{
		time_t t = time(NULL);
		char time[30]="";
		// itoa(t, time, 10);
		sprintf(time,"%d",t);
		strPlanid = string("JH")+string(pUid)+string(time);
		sprintf (sql,"INSERT INTO boloomodb.t41_oilspill_handleplan (PLAN_ID,OIL_TASK_ID,SHIPID,SHIPNAME,WORK_XPOS,WORK_YPOS,DOCK_ID,FROMDOCK_FLAG,STATUS,IS_VALID,TYPE) VALUES ('%s','%s','%s','%s','%s','%s','%s','%d','1','1','3')",\
			strPlanid.c_str(),strOiltaskid.c_str(),shipid.c_str(),shipnm.c_str(),xpos.c_str(),ypos.c_str(),dockid.c_str(),fromdock);
	}
	else if (flag==1)
	{
		sprintf (sql,"UPDATE boloomodb.t41_oilspill_handleplan T1 SET OIL_TASK_ID='%s',SHIPID='%s',SHIPNAME='%s',WORK_XPOS='%s',WORK_YPOS='%s',DOCK_ID='%s',FROMDOCK_FLAG='%d' WHERE T1.PLAN_ID='%s'",\
			strOiltaskid.c_str(),shipid.c_str(),shipnm.c_str(),xpos.c_str(),ypos.c_str(),dockid.c_str(),fromdock,strPlanid.c_str());

	}
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	if (voyages)
	{
		for (int i=0;i<voyages->size();i++)
		{
			Json* voyageItem=voyages->get(i);
			if (!voyageItem)
			{
				continue;
			}


			string strApplyid = voyageItem->getv("applyid","");

			string xpos2=voyageItem->getv("xpos","");
			string ypos2=voyageItem->getv("ypos","");
			string posdes=voyageItem->getv("posdes","");
			int seq=voyageItem->getv("seq",0);
			string distance=voyageItem->getv("distance","");
			string spd=voyageItem->getv("spd","");
			string saildura=voyageItem->getv("saildura","");
			string workdura=voyageItem->getv("workdura","");
			string delay=voyageItem->getv("delay","");
			string eta=voyageItem->getv("eta","");
			string etd=voyageItem->getv("etd","");
			string ata=voyageItem->getv("ata","");
			string atd=voyageItem->getv("atd","");
			int status=voyageItem->getv("status",0);

			char sql1[512]="";
			if (flag==0)
			{
				sprintf (sql1,"INSERT INTO boloomodb.t41_oilspill_plan_voyage (PLAN_ID,SEQ,XPOS,YPOS,POSITION_DES,DISTANCE,SPEED,VOY_DURATION,WORK_DURATION,DELAY,ETA,ETD,ATA,ATD,VOY_STATUS,APPLYID,IS_VALID) \
							  VALUES ('%s',%d,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%d','%s','1')",strPlanid.c_str(),seq,xpos2.c_str(),ypos2.c_str(),posdes.c_str(),distance.c_str(),\
							  spd.c_str(),saildura.c_str(),workdura.c_str(),delay.c_str(),eta.c_str(),etd.c_str(),ata.c_str(),atd.c_str(),status,strApplyid.c_str());
			}
			else if (flag==1)
			{
				sprintf (sql1,"UPDATE boloomodb.t41_oilspill_plan_voyage T1 SET XPOS='%s',YPOS='%s',POSITION_DES='%s',DISTANCE='%s',SPEED='%s',VOY_DURATION='%s',WORK_DURATION='%s',DELAY,ETA='%s',ETD='%s',ATA='%s',ATD='%s',VOY_STATUS='%s' \
							  WHERE T1.PLAN_ID='%s' AND T1.SEQ=%d",xpos2.c_str(),ypos2.c_str(),posdes.c_str(),distance.c_str(),\
							  spd.c_str(),saildura.c_str(),workdura.c_str(),delay.c_str(),eta.c_str(),etd.c_str(),ata.c_str(),atd.c_str(),status,strPlanid.c_str(),seq);
			}

			CHECK_MYSQL_STATUS(psql->Execute(sql1)>=0, 3);

			if (strApplyid!="")
			{

				time_t timeETA = atol(eta.c_str());
				time_t timeETD = atol(etd.c_str());
				string strETA = TimeToTimestampStr(timeETA);
				string strETD = TimeToTimestampStr(timeETD);
				char sql2[512]="";

				sprintf (sql2,"UPDATE boloomodb.t41_oilspill_transferapply T1 SET T1.SHIPID='%s',T1.SHIPNAME='%s',T1.STATE='1',T1.STARTTIME='%s',T1.FINISHTIME='%s' WHERE T1.APPLY_ID='%s'",\
					shipid.c_str(),shipnm.c_str(),strETA.c_str(),strETD.c_str(),strApplyid.c_str());
	
				CHECK_MYSQL_STATUS(psql->Execute(sql2)>=0, 3);
			}

		}
	}

	out<<"{eid:0, planid:\""<<strPlanid<<"\"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//溢油转驳计划详细信息3E
//input
//{planid:””, oiltaskid:”’}
//output
//{
//planid:””,oiltaskid:””,shipid:””,shipnm:””,dockid:””,docknm:””,fromdock:1,xpos:””,ypos:””,capacity:””,rate:””
//voyage:
//[
//{seq:1,applyid:””,xpos:””,ypos:””,posdes:””,distance:””,spd:””,saildura:””,workdura:””,delay:””,eta:””,etd:””,ata:””,atd:””,status:1},
//…….]
//}

int OilSpillSvc::GetOilTransferPlanDetail(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strOiltaskid=root.getv("oiltaskid", "");
	string strPlanid=root.getv("planid", "");

	char sql[512] = "";

	MySql* psql = CREATE_MYSQL;

	sprintf(sql,"SELECT * FROM boloomodb.t41_oilspill_handleplan T1 WHERE T1.PLAN_ID='%s' AND T1.OIL_TASK_ID='%s'",strPlanid.c_str(),strOiltaskid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	 string strTmp=string(sql);

	char planid[128]="";
	char oiltaskid[128]="";
	char shipid[128]="";
	char shipnm[128]="";
	float xpos=0.0;
	float ypos=0.0;
	char dockid[128]="";
	char docknm[128]="";
	char fromdock[128]="";

	double capacity=0.0;
	float rate=0.0;

	out<<"{";
	if (psql->NextRow())
	{
		READMYSQL_STR(PLAN_ID, planid);
		READMYSQL_STR(SHIPID, shipid);
		READMYSQL_STR(SHIPNAME, shipnm);
		READMYSQL_STR(OIL_TASK_ID, oiltaskid);
		READMYSQL_FLOAT(WORK_XPOS, xpos,0.0);
		READMYSQL_FLOAT(WORK_YPOS, ypos,0.0);
		READMYSQL_STR(DOCK_ID, dockid);
		READMYSQL_STR(FROMDOCK_FLAG,fromdock);

		READMYSQL_FLOAT(CAPACITY, capacity,0.0);
		READMYSQL_FLOAT(OIL_RATE, rate,0.0);
	}

	out<<"planid:\"" << planid << "\",oiltaskid:\"" << oiltaskid << "\",shipid:\"" << shipid << "\",shipnm:\"" << shipnm << "\"";
	out<<",xpos:\"" << xpos << "\",ypos:\"" << ypos << "\",fromdock:\"" << fromdock << "\",rate:\"" << rate << "\",capacity:\"" << capacity << "\",dockid:\"" << dockid << "\"";


	sprintf(sql,"SELECT T1.* FROM boloomodb.t41_oilspill_plan_voyage T1 WHERE T1.PLAN_ID='%s'",strPlanid.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);

	int seq=0;
	char posdes[128]="";
	float xpos2=0.0;
	float ypos2=0.0;
	float distance=0.0;
	float spd=0.0;
	float saildura=0.0;
	float workdura2=0.0;
	float delay=0.0;
	unsigned int eta=0;
	unsigned int etd=0;
	unsigned int ata=0;
	unsigned int atd=0;
	char status[128]="";

	char applyId[128]="";

	int cnt=0;
	out<<",voyage:[";
	while (psql->NextRow())
	{
		READMYSQL_INT(SEQ, seq,0);
		READMYSQL_STR(APPLYID, applyId);
		READMYSQL_STR(POSITION_DES, posdes);
		READMYSQL_FLOAT(XPOS, xpos2,0.0);
		READMYSQL_FLOAT(YPOS, ypos2,0.0);
		READMYSQL_FLOAT(DISTANCE, distance,0.0);
		READMYSQL_FLOAT(SPEED, spd,0.0);
		READMYSQL_FLOAT(VOY_DURATION, saildura,0.0);
		READMYSQL_FLOAT(WORK_DURATION, workdura2,0.0);
		READMYSQL_FLOAT(DELAY, delay,0.0);
		READMYSQL_INT(ETA, eta,0);
		READMYSQL_INT(ETD, etd,0);
		READMYSQL_INT(ATA, ata,0);
		READMYSQL_INT(ATD, atd,0);
		READMYSQL_STR(VOY_STATUS,status);
		if (cnt>0)
			out << ",";
		out<<"{seq:\"" << seq << "\",applyid:\"" << applyId << "\",posdes:\"" << posdes << "\",xpos:\"" << xpos2 << "\",ypos:\"" << ypos2 << "\"";
		out<<",distance:\"" << distance << "\",spd:\"" << spd << "\",saildura:\"" << saildura << "\",workdura:\"" << workdura2 << "\",delay:\"" << delay << "\"";
		out<<",eta:\"" << eta << "\",etd:\"" << etd << "\",ata:\"" << ata << "\",atd:\"" << atd << "\",status:\"" << status << "\"}";
		cnt++;
	}

	out<<"]";

	out<<"}";

	RELEASE_MYSQL_RETURN(psql, 0);

}
//删除转驳计划0x3F
//input
//{planid:””,oiltaskid:””}
//output
//{  eid:0, planid:”” }
int OilSpillSvc::DeleteTransferPlanItem(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strPlanid=root.getv("planid","");
	string strOiltaskid=root.getv("oiltaskid","");
	string currTime = OilSpillGetCurrentTmStr();
	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";

	sprintf (sql,"UPDATE boloomodb.t41_oilspill_transferapply T1 SET T1.IS_VALID='0',T1.LAST_UPD_DT='%s' WHERE T1.PLAN_ID='%s' AND T1.OIL_TASK_ID='%s'",currTime.c_str(),strPlanid.c_str(),strOiltaskid.c_str());

	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	out<<"{eid:0, planid:\""<<strPlanid<<"\"}";
	RELEASE_MYSQL_RETURN(psql, 0);

}
//设置溢油预警区域0x40
//input
//{oiltaskid:””,areaid:””,areanm:””,points:””}
//output
// {eid:0}
int OilSpillSvc::SetOilSpillAlertArea(const char* pUid, const char* jsonString, std::stringstream& out)
{
	JSON_PARSE_RETURN("[OilSpillSvc::GetOilShiplist]bad format:", jsonString, 1);

	string strAreaid=root.getv("areaid","");
	string strOiltaskid=root.getv("oiltaskid","");
	string strAreanm=root.getv("areanm","");
	string strPoints=root.getv("points","");

	string currTime = OilSpillGetCurrentTmStr();

	MySql* psql = CREATE_MYSQL;
	char sql[512] = "";

	sprintf (sql, "INSERT INTO boloomodb.t41_oilspill_warnarea VALUES('%s','%s','%s','%s','1','%s')",strOiltaskid.c_str(),strAreaid.c_str(),strAreanm.c_str(),strPoints.c_str(),currTime.c_str());
	CHECK_MYSQL_STATUS(psql->Query(sql), 3);
	out<<"{eid:0}";
	RELEASE_MYSQL_RETURN(psql, 0);

}
