#ifndef __EMOPSMS_SVC_H__
#define __EMOPSMS_SVC_H__

#include <string>
#include "IBusinessService.h"
#include "kSQL.h"
#include "define.h"

using namespace std;

class eMOPSMSSvc:IBusinessService
{
public:
	eMOPSMSSvc();
	~eMOPSMSSvc();

	virtual bool Start(); 
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	DECLARE_SERVICE_MAP(eMOPSMSSvc)

private:



//sms学习*************************************************************
 //请求学习列表
int GetSMSList(const char* pUid, const char* jsonString, std::stringstream& out);
//请求学习详细信息
int GetSMSDetail(const char* pUid, const char* jsonString, std::stringstream& out);
//1.3	新建学习列表
int NewSMSStudy(const char* pUid, const char* jsonString, std::stringstream& out);
//1.4	请求可添加的文件或者部门或者船舶
int GetAddableFileDeparShip(const char* pUid, const char* jsonString, std::stringstream& out);
//1.5	删除或者增加文件或者部门或者船舶
int AddDelFileDeparShip(const char* pUid, const char* jsonString, std::stringstream& out);
//1.6	状态改变
int UpdateState(const char* pUid, const char* jsonString, std::stringstream& out);


//审计检查*******************************************************************
//2.1	请求审计检查列表
int GetAuditList(const char* pUid, const char* jsonString, std::stringstream& out);
//2.2	请求详细审计信息
int GetAuditDetail(const char* pUid, const char* jsonString, std::stringstream& out);
//2.3	新建审计信息
int NewAudit(const char* pUid, const char* jsonString, std::stringstream& out);
//2.4	新增缺陷记录
int AddFlaw(const char* pUid, const char* jsonString, std::stringstream& out);

//SMS文件管理******************************************************************
//获取文件列表
int GetFile(const char* pUid, const char* jsonString, std::stringstream& out);
//删除文件
int DeleteFile(const char* pUid, const char* jsonString, std::stringstream& out);
//获取文件所有建议
int GetSuggest(const char* pUid, const char* jsonString, std::stringstream& out);
//删除建议
int DeleteSuggest(const char* pUid, const char* jsonString, std::stringstream& out);
//文件或表格版本改变
int ChangeVersions(const char* pUid, const char* jsonString, std::stringstream& out);


private:
	string _GetNewObj(string prfex,string uid="")
	{
		time_t t = time(NULL);
		char time[30]="";
		sprintf(time,"%d",t);
		string id=prfex+uid+string(time);
		return id;
	} 


};

typedef ACE_Singleton<eMOPSMSSvc, ACE_Null_Mutex> g_eMOPSMSSvc;
#endif
