#ifndef __USER_REG_SVC_H__
#define __USER_REG_SVC_H__

#include "md5.h"
#include "define.h"
#include <string>
using namespace std;

inline string trimstr(string &s)
{
	s.erase(0,s.find_first_not_of(" ")); 
	s.erase(s.find_last_not_of(" ")+1);
	
	return s;
}

inline string MD5EncryptHex(string input)
{

	MD5 md;
	md.update((unsigned char*)input.c_str(),input.length());
	md.finalize();
	char *res=md.hex_digest();

	return string(res);
}


class UserRegSvc
{
public:
    UserRegSvc();
    ~UserRegSvc();
    bool Start();
    void ClearExpired();

public:
    int userEmailExists(const char* jsonString, std::stringstream& out);
    int registerUser(const char* jsonString, std::stringstream& out);
    int activateUser(const char* jsonString, std::stringstream& out);
    int sendActivation(const char* jsonString, std::stringstream& out);
    int registerInfo(const char* jsonString, std::stringstream& out);
	int mobileBound(const char* jsonString, std::stringstream& out);
    int userEmailPwd(const char* jsonString, std::stringstream& out);
	int userResetPwd(const char* jsonString, std::stringstream& out);

private:
    bool loadEmailCode();
    int  getRandomCode();

public:
    bool sendCode(const std::string& uid, const std::string& tel, const std::string& email, bool cnOrEn=true, bool bIsActivation=true);
	bool sendCode(const std::string& uid, const std::string& tel);
	int  verifyCode(const std::string& uid, const std::string& code, std::string& telno, std::string& email);

private:
    struct TCode
    {
        int			code;
        std::string uid;
        long		tt;
        int			type; // 0:email 1:tel
        std::string tel;  // 手机激活码对应的手机号
		std::string email;// 邮件激活码对应的邮箱

        TCode() : uid(""), code(0), tel(""), tt((long)time(NULL)), type(0) {}
        TCode(const std::string& _uid, int _code, int _type=0) : uid(_uid), code(_code), tel(""), tt((long)time(NULL)), type(_type) {}
    };

    void addTelCode(const std::string& uid, int nCode, const std::string& telno);
	void addEmailCode(const std::string& uid, int nCode, const std::string& email);

    void deleteFromUserMap(TCode* pCode);
    void addToUserMap(TCode* pCode);

    int  getEmailCode(const std::string& uid);

    typedef std::map<int, TCode*> MapCode;
    typedef MapCode::iterator MapCodeIter;

    typedef std::map<std::string, TCode*> MapUserTel;
    typedef std::map<std::string, TCode*> MapUserEmail;

    MapCode			m_mapCode;
    MapUserTel		m_mapUserTel;
    MapUserEmail	m_mapUserEmail;

	//StrIntMap		m_mapBot;

private:
    ACE_Recursive_Thread_Mutex	m_ThreadLock;
};

typedef ACE_Singleton<UserRegSvc, ACE_Null_Mutex> g_UserRegSvc;

#endif
