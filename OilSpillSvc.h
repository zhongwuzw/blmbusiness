#ifndef __OILSPILL_SVC_H__
#define __OILSPILL_SVC_H__

#include "IBusinessService.h"
#include "kSQL.h"
#include <string>
using namespace std;

struct TaskAreaItem  
{
	string areaid;
	string areaname;
	string areatype;
	string points;
	string zarea;//�������
	string surplus;
};

struct TaskPointThick
{
	float xpos;
	float ypos;
	float thickval;

	TaskPointThick():xpos(361),ypos(361),thickval(0)
	{

	}
};

struct EqualLevelItem 
{
	char id[32];
	string location;
	int level;
};

class OilSpillSvc : IBusinessService
{
public:
	OilSpillSvc(void);
	~OilSpillSvc(void);
	virtual bool Start();
	virtual bool ExecuteCmd(const char* pUid, uint16 u2Sid, const char* jsonString, ACE_Message_Block*& pRes);

	DECLARE_SERVICE_MAP(OilSpillSvc)

	 int GetOilSpillTaskList(const char* pUid, const char* jsonString, std::stringstream& out);
     int GetOilSpillSubTaskList(const char* pUid, const char* jsonString, std::stringstream& out);
     int AddUpdOilSpillTask(const char* pUid, const char* jsonString, std::stringstream& out);
	 int GetOilSpillTaskDetail(const char* pUid, const char* jsonString, std::stringstream& out);
     int DelOilSpillTask(const char* pUid, const char* jsonString, std::stringstream& out);

     
     int GetOilDetail(const char* pUid, const char* jsonString, std::stringstream& out);
     int GetOilSpillForcast(const char* pUid, const char* jsonString, std::stringstream& out);
	 int GetOilSpillAnalysis(const char* pUid, const char* jsonString, std::stringstream& out);


	  int AddUpdSubTaskEmergency(const char* pUid, const char* jsonString, std::stringstream& out);
      int GetSubTaskEmergencyDetail(const char* pUid, const char* jsonString, std::stringstream& out);

	  int AddUpdSubTaskObervation(const char* pUid, const char* jsonString, std::stringstream& out);
	  int GetSubTaskObervationDetail(const char* pUid, const char* jsonString, std::stringstream& out);


	  int AddUpdSubTaskVideo(const char* pUid, const char* jsonString, std::stringstream& out);
	  int GetSubTaskVideoDetail(const char* pUid, const char* jsonString, std::stringstream& out);

	  int DelOilSpillSubTask(const char* pUid, const char* jsonString, std::stringstream& out);
	  int GetOilSpillTaskStatus(const char* pUid, const char* jsonString, std::stringstream& out);
	  int GetOilSpillTaskCalStatus(const char* pUid, const char* jsonString, std::stringstream& out);


	  int GetOilSpillTaskTrace(const char* pUid, const char* jsonString, std::stringstream& out);

	  int WtiGetOilTrace(const char* pUid, const char* jsonString, std::stringstream& out);
      int WtiGetDispPoints(const char* pUid, const char* jsonString, std::stringstream& out);
	  int WtiGetPointDetail(const char* pUid, const char* jsonString, std::stringstream& out);

	  //------------------------------���ʹ��üƻ���ؽӿ�----------------------------------------

	  //��������������ҵ�����б�0x41
	  int GetOilShiplist(const char* pUid, const char* jsonString, std::stringstream& out);
	  //����������ҵ������ϸ��Ϣ0x22
	  int GetOilShipRelations(const char* pUid, const char* jsonString, std::stringstream& out);
      //����������ͷ�б�0x42
	  int GetOilGoodsDocklist(const char* pUid, const char* jsonString, std::stringstream& out);
	  //�޸Ĵ�����Ϣ0x43
	  int ModifyShipInfo(const char* pUid, const char* jsonString, std::stringstream& out);

      //�������ʹ��üƻ��б� 0x30
	  int GetOilPlanlist(const char* pUid, const char* jsonString, std::stringstream& out);
	  //���ʹ��üƻ���ϸ��Ϣ 0x31
      int GetOilPlanDetail(const char* pUid, const char* jsonString, std::stringstream& out);
      //�½�(�޸�)���ʹ��üƻ� 0x32
	  int NewModifyOilPlan(const char* pUid, const char* jsonString, std::stringstream& out);
	  //���ʹ��üƻ���������������ʼ��ҵ��������ҵ��ɾ���ƻ��� 0x33
	  int OperatOilPlan(const char* pUid, const char* jsonString, std::stringstream& out);
	  //�½����޸ģ����ϼƻ����� 0x34
	  int NewModifySuppPlanApply(const char* pUid, const char* jsonString, std::stringstream& out);
	  //ɾ�����ϼƻ����� 0x35
	  int DeleteSuppPlanApply(const char* pUid, const char* jsonString, std::stringstream& out);

	  //�½����޸ģ�ת���ƻ����� 0x36
	  int NewModifyTransferPlanApply(const char* pUid, const char* jsonString, std::stringstream& out);
	  //ɾ��ת���ƻ����� 0x37
	  int DeleteTransferPlanApply(const char* pUid, const char* jsonString, std::stringstream& out);
	  //�������д������ϼƻ� 0x38
	  int GetAllSuppPlanApply(const char* pUid, const char* jsonString, std::stringstream& out);
	  //�½����޸ģ����ͱ��ϼƻ� 0x39
	  int NewModifySuppPlanItem(const char* pUid, const char* jsonString, std::stringstream& out);
      //���ͱ��ϼƻ���ϸ��Ϣ0x3A
	  int GetOilSuppPlanDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	  //ɾ�����ϼƻ�0x3B
	  int DeleteSuppPlanItem(const char* pUid, const char* jsonString, std::stringstream& out);

	  //�������д�����ת���ƻ� 0x3C
	  int GetAllTransferPlanApply(const char* pUid, const char* jsonString, std::stringstream& out);
	  //�½����޸ģ�����ת���ƻ� 0x3D
	  int NewModifyTransferPlanItem(const char* pUid, const char* jsonString, std::stringstream& out);
	  //����ת���ƻ���ϸ��Ϣ3E
       int GetOilTransferPlanDetail(const char* pUid, const char* jsonString, std::stringstream& out);
	   //ɾ�����ϼƻ�0x3F
	   int DeleteTransferPlanItem(const char* pUid, const char* jsonString, std::stringstream& out);
	   //�������;�������0x40
	   int SetOilSpillAlertArea(const char* pUid, const char* jsonString, std::stringstream& out);



private:
	//ҵ������


};

typedef ACE_Singleton<OilSpillSvc, ACE_Null_Mutex> g_OilSpillSvc;

inline string OilSpillGetCurrentTmStr()
{
	char timestr[20]="";
	time_t lt= time(NULL);
	struct tm *ptr = localtime(&lt);
	strftime(timestr,20,"%Y-%m-%d %H:%M:%S",ptr);

	return string(timestr);
}

#endif
