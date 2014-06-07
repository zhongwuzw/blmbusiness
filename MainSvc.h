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
    std::string  username;				//�û�����
    std::string  nickname;				//�û��ǳ�
    std::string  firstname;				//�û���
    std::string  lastname;				//�û���
    std::string  birthday;				//����
    char		 gender[2];				//�Ա�
    std::string  salutation;			//�ƺ�
    std::string  sign;					//�û�ǩ��
    char		 country[4];			//����
    std::string  email;					//Email
    std::string  telno;					//�绰
    std::string  mobile;				//�ƶ��绰
    std::string  zip;					//ZIP
    std::string  fax;					//����
    std::string  address;				//��ַ
    std::string  city;					//����
    int			 avatar_id;				//ͷ��ID
    std::string	 servicetype_code;		//������
    std::string	 compid;                //��˾ID
    std::string  compname;              //��˾����
    int			 nusertype;             //�û�����
    int			 email_verified;		//�����Ƿ���֤
    int			 mobile_verified;		//�ֻ��Ƿ���֤
};

#endif
