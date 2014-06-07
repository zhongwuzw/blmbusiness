#ifndef __BLM_SAR_SVR_H__
#define __BLM_SAR_SVR_H__

#include <string>
#include "IBusinessService.h"
#include "kSQL.h"
#include "define.h"

using namespace std;

class SARSvc: IBusinessService
{
public:
	SARSvc();
	~SARSvc();

	virtual bool Start(); 
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);
	DECLARE_SERVICE_MAP(SARSvc)

private:
	int getAccidents(const char* pUid, const char* jsonString, std::stringstream& out);
	int modifyAccident(const char* pUid, const char* jsonString, std::stringstream& out);
	int getAccident(const char* pUid, const char* jsonString, std::stringstream& out);
	int deleteAccident(const char* pUid, const char* jsonString, std::stringstream& out);
	int getLKP(const char* pUid, const char* jsonString, std::stringstream& out);
	int modifyLKP(const char* pUid, const char* jsonString, std::stringstream& out);
	int deleteLKP(const char* pUid, const char* jsonString, std::stringstream& out);
	int modifyForecast(const char* pUid, const char* jsonString, std::stringstream& out);
	int deleteForecast(const char* pUid, const char* jsonString, std::stringstream& out);
	int getForecast(const char* pUid, const char* jsonString, std::stringstream& out);
	int getForecastTrack(const char* pUid, const char* jsonString, std::stringstream& out);
	int modifyForecastTrack(const char* pUid, const char* jsonString, std::stringstream& out);
	int customWind(const char* pUid, const char* jsonString, std::stringstream& out);

	//�½�(�޸�)�¹� 0x01
	int NewModifyAccident(const char* pUid, const char* jsonString, std::stringstream& out);
    //ɾ���¹� 0x02
	int DeleteAccident(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡ�¹��б� 0x03
	int GetAccidentList(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ȡĳ�¹������е�Ư��Ԥ���б� 0x04
	int GetAccForcastList(const char* pUid, const char* jsonString, std::stringstream& out);
    //�½�(�޸�)Ư��Ԥ�� 0x05
    int NewModifyAccForcast(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ��Ư��Ԥ�� 0x06
	int DeleteAccForcast(const char* pUid, const char* jsonString, std::stringstream& out);
    //����Ư��Ԥ����ϸ��Ϣ 0x07
	int GetForcastDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	//����Ư��Ԥ��켣 0x08
	int GetForcastTraceInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	//�����Ѿȼƻ��б� 0x09
	int GetPlanList(const char* pUid, const char* jsonString, std::stringstream& out);
	//�½�(�޸�)�Ѿȼƻ� 0x10
	int NewModifyPlan(const char* pUid, const char* jsonString, std::stringstream& out);
	//�����Ѿȼƻ����� 0x11
	int GetPlanReport(const char* pUid, const char* jsonString, std::stringstream& out);	
	//�½�(�޸�)�Ѿȼƻ����� 0x12
	int NewModifyPlanReport(const char* pUid, const char* jsonString, std::stringstream& out);
	//����SRU LIST 0x13
	int GetSRUList(const char* pUid, const char* jsonString, std::stringstream& out);
	//�½�(�޸�)SRU 0x14
	int NewModifySRUList(const char* pUid, const char* jsonString, std::stringstream& out);
	//����������ϸ��Ϣ 0x15
	int GetPlanDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	//�����Ѿȼƻ�״̬ 0x16
	int UpdatePlanStatus(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ���Ѿȼƻ� 0x17
	int DeletePlan(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ���ƻ�SRU 0x18
	int DeletePlanSRU(const char* pUid, const char* jsonString, std::stringstream& out);
	//�������µ�һ���¹� 0x20
	int GetNewestAccident(const char* pUid, const char* jsonString, std::stringstream& out); 
	//��������б� 0x32
	int GetBaseList(const char* pUid, const char* jsonString, std::stringstream& out);
	//������͵��б� 0x33
	int GetStationList(const char* pUid, const char* jsonString, std::stringstream& out);
	//���������ϸ��Ϣ 0x34
	int GetBaseInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	//������͵���ϸ��Ϣ 0x35
	int GetStationInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ӻ��޸�һ��������ϸ��Ϣ 0x36
	int AddOrModBase(const char* pUid, const char* jsonString, std::stringstream& out);
	//��ӻ��޸�һ�����͵���ϸ��Ϣ 0x37
	int AddOrModStation(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ��һ��������Ϣ  0x38
	int DeleteBase(const char* pUid, const char* jsonString, std::stringstream& out);
	//ɾ��һ�����͵���Ϣ 0x39
	int DeleteStation(const char* pUid, const char* jsonString, std::stringstream& out);

	//����SRU�б�0x61
	int GetSRUAndTypeList(const char* pUid, const char* jsonString, std::stringstream& out);

	//����SRU��ϸ��Ϣ0x64
	int GetSRUInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//�½����޸ģ�SRU����0x62
	int NewModifySRUType(const char* pUid, const char* jsonString, std::stringstream& out);

	//�½����޸ģ�SRU0x65
	int NewModifySRU(const char* pUid, const char* jsonString, std::stringstream& out);

	//ɾ��SRU����0x63
	int DeleteSRUType(const char* pUid, const char* jsonString, std::stringstream& out);

	//ɾ��SRU0x66
	int DeleteSRU(const char* pUid, const char* jsonString, std::stringstream& out);

	//�Ѿȴ����б�0x90
	int GetRescueShipStatistic(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//�Ѿȴ����б�0x91
	int GetRescuePlaneStatistic(const char* pUid, const char* jsonString, std::stringstream& out);

private:
	std::string getAccidentID();
	std::string getPersionID(const std::string& accid);
	std::string getLkpID();
	std::string getForecastID();
};

typedef ACE_Singleton<SARSvc, ACE_Null_Mutex> g_SARSvc;
#endif
