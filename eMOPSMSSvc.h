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



//smsѧϰ*************************************************************
 //����ѧϰ�б�
int GetSMSList(const char* pUid, const char* jsonString, std::stringstream& out);
//����ѧϰ��ϸ��Ϣ
int GetSMSDetail(const char* pUid, const char* jsonString, std::stringstream& out);
//1.3	�½�ѧϰ�б�
int NewSMSStudy(const char* pUid, const char* jsonString, std::stringstream& out);
//1.4	�������ӵ��ļ����߲��Ż��ߴ���
int GetAddableFileDeparShip(const char* pUid, const char* jsonString, std::stringstream& out);
//1.5	ɾ�����������ļ����߲��Ż��ߴ���
int AddDelFileDeparShip(const char* pUid, const char* jsonString, std::stringstream& out);
//1.6	״̬�ı�
int UpdateState(const char* pUid, const char* jsonString, std::stringstream& out);


//��Ƽ��*******************************************************************
//2.1	������Ƽ���б�
int GetAuditList(const char* pUid, const char* jsonString, std::stringstream& out);
//2.2	������ϸ�����Ϣ
int GetAuditDetail(const char* pUid, const char* jsonString, std::stringstream& out);
//2.3	�½������Ϣ
int NewAudit(const char* pUid, const char* jsonString, std::stringstream& out);
//2.4	����ȱ�ݼ�¼
int AddFlaw(const char* pUid, const char* jsonString, std::stringstream& out);

//SMS�ļ�����******************************************************************
//��ȡ�ļ��б�
int GetFile(const char* pUid, const char* jsonString, std::stringstream& out);
//ɾ���ļ�
int DeleteFile(const char* pUid, const char* jsonString, std::stringstream& out);
//��ȡ�ļ����н���
int GetSuggest(const char* pUid, const char* jsonString, std::stringstream& out);
//ɾ������
int DeleteSuggest(const char* pUid, const char* jsonString, std::stringstream& out);
//�ļ�����汾�ı�
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
