#ifndef __EMOPSURVEY_SVC_H__
#define __EMOPSURVEY_SVC_H__
#include "IBusinessService.h"
#include "kSQL.h"
#include "define.h"
 

class eMOPSurveySvc : IBusinessService
{
public:
    eMOPSurveySvc();
    virtual ~eMOPSurveySvc();
    virtual bool Start();
    virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

    DECLARE_SERVICE_MAP(eMOPSurveySvc)

private: 
	//�����û���������д��������м�����Ϣ
	int GetAllShipsSurvey(const char* pUid, const char* jsonString, std::stringstream& out);
	//�����û���������д���������֤����Ϣ
	int GetAllShipsCertificate(const char* pUid, const char* jsonString, std::stringstream& out);
	//���󴬲��ļ����������֤����Ϣ
	int GetShipSurveyCertificate(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//���������ļ���
	int AddShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out);
	//�鿴�����ļ���
	int GetShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out);
	//�޸Ĵ����ļ���
	int UpdateShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ�������ļ���
	int DeleteShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ɼ���
	int FinishShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out);

	//��������֤��
	int AddShipCertificate(const char* pUid, const char* jsonString, std::stringstream& out);
	//�鿴֤��
	int GetShipCertificate(const char* pUid, const char* jsonString, std::stringstream& out);
	//�޸�֤��
	int UpdateShipCertificate(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ��֤��
	int DeleteShipCertificate(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ʾԤ��ģ��******************************************************************
	//����ģ����Ϣ
	int GetTemplateInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//����-֤�飨��ӣ�ɾ����********************************************************
	//�������֤��
	int GetSurveyInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//����Ԥ�����õ�����ģ����Ϣ
	int GetAlertTemplateInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//֤��1
	int getCerts(int id,std::string &outStr);

	//Ԥ��1
	int getLimit(int id,std::string &outStr);
	
	//������Ϣ
	int getShips(int id,std::string &outStr);

	//�½�Ԥ��ģ����Ϣ
	int AddAlertTemplateInfo(const char* pUid, const char* jsonString, std::stringstream& out);
		
	//ɾ��Ԥ��ģ����Ϣ
	int DeleteAlertTemplate(const char* pUid, const char* jsonString, std::stringstream& out);

	//�޸�Ԥ��ģ����Ϣ
	int EditAlertTemplateInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//���󴬲���Ϣ
	int GetShipsInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ӹ���֤��
	int AddRelateCERT(const char* pUid, const char* jsonString, std::stringstream& out);

	//ɾ������֤��
	int DeleteRelateCERT(const char* pUid, const char* jsonString, std::stringstream& out);

	//���󹤳̵��б�
	int GetEngineerList(const char* pUid, const char* jsonString, std::stringstream& out);

	//��ӹ������̵�
	int AddRelateEngineering(const char* pUid, const char* jsonString, std::stringstream& out);

	//ɾ���������̵�
	int DeleteRelateEngineering(const char* pUid, const char* jsonString, std::stringstream& out);

	
	//�������֤�����ؼ���
	int GetSurCERTRelate(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//�����ؼ���
	int AddRelateSur(const char* pUid, const char* jsonString, std::stringstream& out);

	//ɾ����ؼ���
	int DeleteRelateSur(const char* pUid, const char* jsonString, std::stringstream& out);

	//����������
	int GetSurveyMaTable(const char* pUid, const char* jsonString, std::stringstream& out);

	//����֤�����
	int GetCERTMaTable(const char* pUid, const char* jsonString, std::stringstream& out);

	 //���������ʱ���ȡ������ֵ
	 int getRemainDay(int startTime,int endTime);
	
};

typedef ACE_Singleton<eMOPSurveySvc, ACE_Null_Mutex> g_eMOPSurveySvc;

#endif // AREASVC_H
