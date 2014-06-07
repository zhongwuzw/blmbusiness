#ifndef __MAIN_SVC_H__
#define __MAIN_SVC_H__

#include "IBusinessService.h"

class MainSvc : IBusinessService, ACE_Event_Handler
{
public:
    MainSvc();
    ~MainSvc();

    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

    DECLARE_SERVICE_MAP(MainSvc)

private:
    int getUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int updateUserInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int manageRemark(const char* pUid, const char* jsonString, std::stringstream& out);
    int getDaylayTip(const char* pUid, const char* jsonString, std::stringstream& out);
    int getIpInfo(const char* pUid, const char* jsonString, std::stringstream& out);
    int getUserIpinfo(const char* pUid, const char* jsonString, std::stringstream& out);
	int getUserCountry(const char* pUid, const char* jsonString, std::stringstream& out);
    int getNotice2(const char* pUid, const char* jsonString, std::stringstream& out);
    int getUserAuthority(const char* pUid, const char* jsonString, std::stringstream& out);
    int getUpdate(const char* pUid, const char* jsonString, std::stringstream& out);

};

typedef ACE_Singleton<MainSvc, ACE_Null_Mutex> g_MainSvc;

struct Personality
{
    std::string  username;				//用户姓名
    std::string  nickname;				//用户昵称
    std::string  firstname;				//用户姓
    std::string  lastname;				//用户名
    std::string  birthday;				//生日
    char		 gender[2];				//性别
    std::string  salutation;			//称呼
    std::string  sign;					//用户签名
    char		 country[4];			//国家
    std::string  email;					//Email
    std::string  telno;					//电话
    std::string  mobile;				//移动电话
    std::string  zip;					//ZIP
    std::string  fax;					//传真
    std::string  address;				//地址
    std::string  city;					//城市
    int			 avatar_id;				//头像ID
    std::string	 servicetype_code;		//服务码
    std::string	 compid;                //公司ID
    std::string  compname;              //公司名称
    int			 nusertype;             //用户类型
    int			 email_verified;		//邮箱是否验证
    int			 mobile_verified;		//手机是否验证
};

#endif
