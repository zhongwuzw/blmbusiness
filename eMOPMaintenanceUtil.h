#pragma once

#include <string> 
#include <define.h>
#include "Util.h"
															
using namespace std;

class eMOPMaintenanceUtil
{
public:
	eMOPMaintenanceUtil(void);
	~eMOPMaintenanceUtil(void);

	//工作计划 ***************************
	//船舶查询条件语句
	static int getShipSQLs(string &inShipids, string &out);
	//部门查询语句
	static int getDepartSQLs(string &strDepartid, string &out); 
	//CWBT查询语句
	static int getCWBT_SQLs(string &strDepartid, string &out);
	//日期查询语句
	static int getPlanDateSQLs(string &strDepartid, string &out);
	//类型查询语句
	static int getPlanTypeSQLs(string &strPtype, string &out);

};
