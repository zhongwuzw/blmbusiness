#ifndef __EMOP_USERMGR_SVC_H__
#define __EMOP_USERMGR_SVC_H__

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


//船舶信息
struct ShipDepart
{
	string shipId;
	string shipName; 
	list<Depart> departList;
	ShipDepart():shipId(""),shipName(""){}
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



typedef struct KeyValue
{
	string m_strKey;
	string m_strValue;
}_KEY_VALUE;
//T41_EMOP_SHIP_SAVE_BOAT
typedef struct SHIP_SAVEBOAT
{
	char left_boat[64];//	左艇 救生艇
	char lb_style[64];//	左艇形式 救生艇
	char right_boat[64];//	右艇 救生艇
	char rb_style[64];//	右艇形式 救生艇
	char left_raft[64];//	左舷 救生筏
	char lr_style[64];//	左舷形式 救生筏
	char right_raft[64];//	右舷 救生筏
	char rr_style[64];//	右舷形式 救生筏
}ST_SAVEBOAT;
class eMOPUserMgrSvc : IBusinessService, ACE_Event_Handler
{
public:
    eMOPUserMgrSvc();
    ~eMOPUserMgrSvc();

    virtual bool Start();
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(eMOPUserMgrSvc)
// data
private:
// method
private:
	//获取公司用户管理概要信息
    int GetAllUserGeneralInfo_C(const char* pUid, const char* jsonString, std::stringstream& out);
	//角色管理
	int GetGetAllRoles(const char* pUid, const char* jsonString, std::stringstream& out);
	//角色对应的权限
	int GetRoleAuthorities(const char* pUid, const char* jsonString, std::stringstream& out);
	// 修改角色名称
	int SetUserRoleName(const char* pUid, const char* jsonString, std::stringstream& out);
	//添加角色
	int NewUserRole(const char* pUid, const char* jsonString, std::stringstream& out);
	// 删除角色
	int DelUserRole(const char* pUid, const char* jsonString, std::stringstream& out);
	// 修改角色权限
	int ModifyRoleAuthorities(const char* pUid, const char* jsonString, std::stringstream& out);
	// 添加部门
	int NewDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	// 编辑部门
	int UpdateDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	//添加部门船队船舶
	int NewDptShip(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除部门船队船舶
	int DelDptShip(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除部门
	int DelDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	// 添加用户
	int NewUser(const char* pUid, const char* jsonString, std::stringstream& out);
	// 删除用户
	int DelUser(const char* pUid, const char* jsonString, std::stringstream& out);
	//更改用户部门
	int ModifyUserDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	//查看用户详情
	int GetUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	//用户添加船舶
	int AddUserShip(const char* pUid, const char* jsonString, std::stringstream& out);
	//用户删除船舶
	int DelUserShip(const char* pUid, const char* jsonString, std::stringstream& out);
	//用户拥有的船舶
	int GetAllUserShips(const char* pUid, const char* jsonString, std::stringstream& out);
	//查找用户
	int SearchUser(const char* pUid, const char* jsonString, std::stringstream& out);
	//根据船舶ID查找船舶对应的部门
	int SearchShipDpt(const char* pUid, const char* jsonString, std::stringstream& out);
	// 重置密码
	int ResetPwd(const char* pUid, const char* jsonString, std::stringstream& out);
	//禁用/恢复用户
	int EnableUser(const char* pUid, const char* jsonString, std::stringstream& out);
	//获取船舶用户管理概要信息
	int GetAllUserGeneralInfo_S(const char* pUid, const char* jsonString, std::stringstream& out);
	// 获取用户角色
	int GetUserRoles(const char* pUid, const char* jsonString, std::stringstream& out);
	// 获取用户所在部门
	int GetUserDpt(const char* pUid, const char* jsonString, std::stringstream& out);
    // 获取用户所在公司，公司部门，船舶信息
	int GetCompanyDpt(const char* pUid, const char* jsonString, std::stringstream& out);
    // 获取公司某部门下所有用户
	int GetDptUsers(const char* pUid, const char* jsonString, std::stringstream& out); 
	// 查询某角色下的所有用户
	int GetRoleUsers(const char* pUid, const char* jsonString, std::stringstream& out); 
	// 修改用户状态
	int UpdateUserState(const char* pUid, const char* jsonString, std::stringstream& out); 
	// 将已有角色添加给用户，或删除用户已经有的的角色
	int AddOrDelUserRole(const char* pUid, const char* jsonString, std::stringstream& out); 
	// 将已有船舶添加给用户，或删除用户已经有的船舶
	int AddOrDelUserShip(const char* pUid, const char* jsonString, std::stringstream& out); 

	//
	int GetSubData(std::stringstream& out,short &o_nSt,string &i_strKey,string &i_strRoleid ,MySql* psql);
	int GetFuncData(std::stringstream& out,short &o_nSt,string &i_strKey,string &i_strRoleid,MySql* psql);
	//
	int GetCorpInfo(string &o_strCorpid,string &o_strCorpName,string &i_strUid);
	int GetCorpDpt(std::stringstream& out,string &i_strCorpId,bool i_bShip=false); 
	int GeDptUser(std::stringstream& out,string &i_strDptId,bool i_bShip=false);
	int GetUserShips(std::stringstream& out,string &i_strUId);
	int GetDptShips(std::stringstream& out,string &i_strDptId);
	int GetCorpShips(std::stringstream& out,string &i_strCorpId);



	//获取船舶资料详细信息0xf1

	int GetShipDetailInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//获取船舶历史0xf2
    int GetShipHistoryList(const char* pUid, const char* jsonString, std::stringstream& out);
 
	//获取船舶通讯0xf3
    int GetShipCommunicationList(const char* pUid, const char* jsonString, std::stringstream& out);

	//获取船舶设备0xf4
	int GetShipEquipmentList(const char* pUid, const char* jsonString, std::stringstream& out);
	///
	int GetShipBaseData(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetShipDetail1(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetShipDetail2(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetShipDetail3(const char* pUid, const char* jsonString, std::stringstream& out);
	int GetShipEquipments(const char* pUid, const char* jsonString, std::stringstream& out);
// other method
	int GetShipComminfo(string &o_strC,string &o_strABM,string &i_strShipid);
	int GetSaveboatInfo(ST_SAVEBOAT &o_stSaveboat,string &i_strShipid);
};

typedef ACE_Singleton<eMOPUserMgrSvc, ACE_Null_Mutex> g_eMOPUserMgrSvc;
 


#endif
