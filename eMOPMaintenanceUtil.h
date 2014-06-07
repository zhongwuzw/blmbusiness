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

	//�����ƻ� ***************************
	//������ѯ�������
	static int getShipSQLs(string &inShipids, string &out);
	//���Ų�ѯ���
	static int getDepartSQLs(string &strDepartid, string &out); 
	//CWBT��ѯ���
	static int getCWBT_SQLs(string &strDepartid, string &out);
	//���ڲ�ѯ���
	static int getPlanDateSQLs(string &strDepartid, string &out);
	//���Ͳ�ѯ���
	static int getPlanTypeSQLs(string &strPtype, string &out);

};
