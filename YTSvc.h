#ifndef __YT_SVC_H__
#define __YT_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"

using namespace std;
//部门信息

struct Depart
{
	string departId;
	string departName;
	int departType;
	list<Depart> childDepart;
	Depart():departId(""),departName(""),departType(0){}
};

 
//用户信息
struct User 
{
	string uid;
	string name;
	string tel;
	string fax;
	string mobile;
	string email;
	int stime;
	int etime;
	string job;  
	int state; 
	int cstype; // 0:岸端、1:船端

	Depart depart;//部门 


	User():uid(""),name(""),tel(""),fax(""),mobile(""),email(""),stime(0),etime(0),job(""),state(0),cstype(0){}
};


class YTSvc : IBusinessService, ACE_Event_Handler
{
public:
    YTSvc();
    ~YTSvc();

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(YTSvc)

private:
    int getLatestEvents(const char* pUid, const char* jsonString, std::stringstream& out);
	int getHistoryEvents(const char* pUid, const char* jsonString, std::stringstream& out);
	int updateEvent(const char* pUid, const char* jsonString, std::stringstream& out);

	int getNotices(const char* pUid, const char* jsonString, std::stringstream& out);
	int postNotice(const char* pUid, const char* jsonString, std::stringstream& out);
	int editNotice(const char* pUid, const char* jsonString, std::stringstream& out);
	int deleteNotice(const char* pUid, const char* jsonString, std::stringstream& out);

	int getSections(const char* pUid, const char* jsonString, std::stringstream& out);
	int addSection(const char* pUid, const char* jsonString, std::stringstream& out);
	int deleteSection(const char* pUid, const char* jsonString, std::stringstream& out);

	/////////////////// 用户权限管理 //////////////////////
	int GetCorpInfo(std::string &o_strCorpid,std::string &o_strCorpName,std::string &i_strUid);
	int GetCorpDpt(std::stringstream& out,std::string &i_strCorpId);
	int GeDptUser(std::stringstream& out,std::string &i_strDptId);
	int GetSubData(std::stringstream& out,short &o_nSt,std::string &i_strKey,std::string &i_strRoleid,MySql* psql );
	int GetFuncData(std::stringstream& out,short &o_nSt,std::string &i_strKey,std::string &i_strRoleid,MySql* psql);

	int GetAllUserGeneralInfo_C(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetGetAllRoles(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetRoleAuthorities(const char* pUid, const char* jsonString, std::stringstream& out);
	int SetUserRoleName(const char* pUid, const char* jsonString, std::stringstream& out);
	int NewUserRole(const char* pUid, const char* jsonString, std::stringstream& out);
	int DelUserRole(const char* pUid, const char* jsonString, std::stringstream& out);
	int ModifyRoleAuthorities(const char* pUid, const char* jsonString, std::stringstream& out);
	int NewDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	int UpdateDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	int DelDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	int NewUser(const char* pUid, const char* jsonString, std::stringstream& out);
	int DelUser(const char* pUid, const char* jsonString, std::stringstream& out);
	int ModifyUserDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int SearchUser(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetUserRoles(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetUserDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	int ResetPwd(const char* pUid, const char* jsonString, std::stringstream& out);
	int AddOrDelUserRole(const char* pUid, const char* jsonString, std::stringstream& out); 
	int EditUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetDepartArea(const char* pUid, const char* jsonString, std::stringstream& out);

	// 获取用户所在公司，公司部门
	int GetCompanyDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	int SearchShipDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetDptUsers(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetRoleUsers(const char* pUid, const char* jsonString, std::stringstream& out);

		//搜救船舶列表 
	int GetRescueShipStatistic(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//搜救船舶列表 
	int GetRescuePlaneStatistic(const char* pUid, const char* jsonString, std::stringstream& out);

};

typedef ACE_Singleton<YTSvc, ACE_Null_Mutex> g_YTSvc;

#endif
