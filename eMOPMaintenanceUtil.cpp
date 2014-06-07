#include "eMOPMaintenanceUtil.h"

eMOPMaintenanceUtil::eMOPMaintenanceUtil(void)
{
}

eMOPMaintenanceUtil::~eMOPMaintenanceUtil(void)
{
}

void GetYearMon(int &o_iYear,int &o_iMon,string &i_strData)
{// 2013-09
	int iPos=i_strData.find("-");
	o_iYear=atoi(i_strData.substr(0,4).c_str());
	o_iMon=atoi(i_strData.substr(5,i_strData.length()-5).c_str());
}

int eMOPMaintenanceUtil::getShipSQLs(string &strShipid, string &out){ 
	out = "";
	char szTmp[1024];
	memset(szTmp,0,sizeof(szTmp));
	Tokens svrList;	
	int cnt=0;
	if(strShipid.find(",") != string::npos)
	{					 
		svrList = StrSplit(strShipid, ",");
		out+="(";
		for(int i=0; i<(int)svrList.size(); i++)
		{
			if(cnt++)
				out+=" OR ";
			sprintf(szTmp,"T1.SHIPID='%s'",svrList[i].c_str());
			out+=szTmp;
		}
		out+=")";
	}
	else
	{
		sprintf(szTmp,"T1.SHIPID='%s'",strShipid.c_str());
		out=szTmp;
	}

	return 0;
}

//cwbt查询语句
int eMOPMaintenanceUtil::getCWBT_SQLs(string &strCwbt, string &out){
	out = "";
	char szTmp[1024];
	memset(szTmp,0,sizeof(szTmp));
	int cnt=0;
	Tokens svrList;	  

	// cwbt
	if(!strCwbt.empty())
	{
		memset(szTmp,0,sizeof(szTmp));
		cnt=0;
		if(strCwbt.find(",") != string::npos)
		{					 
			svrList = StrSplit(strCwbt, ",");
			out+=" AND (";
			for(int i=0; i<(int)svrList.size(); i++)
			{
				if(cnt++)
					out+=" OR ";
				sprintf(szTmp,"T1.CWBT='%s'",svrList[i].c_str());
				out+=szTmp;
			}
			out+=")";
		}
		else
		{
			sprintf(szTmp," AND T1.CWBT='%s'",strCwbt.c_str());
			out=szTmp;
		}
	}

	return 0;
}

//类型查询语句
int eMOPMaintenanceUtil::getPlanTypeSQLs(string &strPtype, string &out)
{
	out = "";
	char szTmp[1024];
	memset(szTmp,0,sizeof(szTmp));
	int cnt=0;
	Tokens svrList;	  
	// ptype
	if(!strPtype.empty())
	{
		memset(szTmp,0,sizeof(szTmp));
		cnt=0;
		if(strPtype.find(",") != string::npos)
		{					 
			svrList = StrSplit(strPtype, ",");
			for(int i=0; i<(int)svrList.size(); i++)
			{
				if(cnt++)
					out+=" OR ";
	//			sprintf(szTmp,"T1.SHIPID='%s'",svrList[i].c_str());
				out+=szTmp;
			}
		}
		else
		{
			sprintf(szTmp,"AND T1.DEPARTMENT_CODE='%s'",strPtype.c_str());
			out=szTmp;
			if(strPtype=="1")
			{// 待批复
				out="";
			}
			else if(strPtype=="2")
			{// 过期:和当前系统时间比
				time_t rawtime;
//				int iTime=time(&rawtime);
				struct tm * timeinfo;
				time ( &rawtime );
				timeinfo = localtime ( &rawtime );
				int iYear=timeinfo->tm_year+1900;
				int iMon =timeinfo->tm_mon+1;
				sprintf(szTmp," AND (T3.STATUS<>3 AND ((T3.SCHEDULED_YEAR<%d) OR (T3.SCHEDULED_YEAR=%d and T3.SCHEDULED_MON<%d)))",iYear,iYear,iMon);// 
				out=szTmp;
			}
			else if(strPtype=="3")
			{// 预警
				out=" AND (";
				// 过期
				time_t rawtime;
				struct tm * timeinfo;
				time ( &rawtime );
				timeinfo = localtime ( &rawtime );
				int iYear=timeinfo->tm_year+1900;
				int iMon =timeinfo->tm_mon+1;
				sprintf(szTmp," (T3.STATUS<>3 AND ((T3.SCHEDULED_YEAR<%d) OR (T3.SCHEDULED_YEAR=%d and T3.SCHEDULED_MON<=%d)))",iYear,iYear,iMon+2);// 过期和当月
				out+=szTmp;
//				sprintf(szTmp," OR (T3.STATUS<>3 AND (T3.SCHEDULED_YEAR=%d and T3.SCHEDULED_MON<=%d) and (t2.LEVEL='d' or t2.LEVEL='e')",iYear,iMon+1);// 
//				out+=szTmp;
//				sprintf(szTmp," OR (T3.STATUS<>3 AND (T3.SCHEDULED_YEAR=%d and T3.SCHEDULED_MON<=%d) and (t2.LEVEL='e' or t2.LEVEL='f' or t2.LEVEL='g' )",iYear,iMon+2);// 
//				out+=szTmp;
				out+=")";
			}
			else if(strPtype=="4")
			{// 未首排
				//sprintf(szTmp," AND (T3.SCHEDULED_YEAR IS NULL and T3.SCHEDULED_MON IS NULL) ");
				sprintf(szTmp," AND (T3.SCHEDULED_YEAR<=0 or T3.SCHEDULED_MON<=0)");
				out=szTmp;
			}
			else if(strPtype=="5")
			{// 提前:T3.STATUS=3 AND 
				sprintf(szTmp," AND ((YEAR(T3.FINISHED_DATE)<T3.SCHEDULED_YEAR) OR (YEAR(T3.FINISHED_DATE)=T3.SCHEDULED_YEAR and MONTH(T3.FINISHED_DATE)<T3.SCHEDULED_MON))");
				out=szTmp;
			}
			else if(strPtype=="6")
			{// 滞后T3.STATUS=3 AND 
				sprintf(szTmp," AND ((YEAR(T3.FINISHED_DATE)>T3.SCHEDULED_YEAR) OR (YEAR(T3.FINISHED_DATE)=T3.SCHEDULED_YEAR and MONTH(T3.FINISHED_DATE)>T3.SCHEDULED_MON))");
				out=szTmp;
			}
		}
	}

	return 0;
}
	//日期查询语句
int eMOPMaintenanceUtil::getPlanDateSQLs(string &strPdate, string &out){ 
	out = "";
	char szTmp[1024];
	memset(szTmp,0,sizeof(szTmp));
	int cnt=0;
	Tokens svrList;	  
	// pdate
	if(!strPdate.empty())
	{
		int iYear=0;int iMon=0;
		memset(szTmp,0,sizeof(szTmp));
		cnt=0;
		if(strPdate.find(",") != string::npos)
		{					 
			svrList = StrSplit(strPdate, ",");
			out+=" AND (";
			for(int i=0; i<(int)svrList.size(); i++)
			{
				if(cnt++)
					out+=" OR ";
				GetYearMon(iYear,iMon,svrList[i]);
				sprintf(szTmp,"(T3.SCHEDULED_YEAR=%d) AND T3.SCHEDULED_MON=%d)",iYear,iMon);
				out+=szTmp;
			}
			out+=")";
		}
		else
		{
			GetYearMon(iYear,iMon,strPdate);
			sprintf(szTmp," AND (T3.SCHEDULED_YEAR=%d and T3.SCHEDULED_MON=%d) ",iYear,iMon);
			out=szTmp;
		} 
	}
	return 0;
}

//部门查询语句
int eMOPMaintenanceUtil::getDepartSQLs(string &strDepartid, string &out)
{ 
	out = "";
	char szTmp[1024];
	memset(szTmp,0,sizeof(szTmp));
	Tokens svrList;	
	if(!strDepartid.empty())
	{
		memset(szTmp,0,sizeof(szTmp));
		int cnt=0;
		if(strDepartid.find(",") != string::npos)
		{					 
			svrList = StrSplit(strDepartid, ",");
			out+=" AND (";
			for(int i=0; i<(int)svrList.size(); i++)
			{
				if(cnt++)
					out+=" OR ";
				sprintf(szTmp,"T1.DEPARTMENT_CODE='%s'",svrList[i].c_str());
				out+=szTmp;
			}
			out+=")";
		}
		else
		{
			sprintf(szTmp," AND T1.DEPARTMENT_CODE='%s'",strDepartid.c_str());
			out=szTmp;
		}
	}
	return 0;
}
