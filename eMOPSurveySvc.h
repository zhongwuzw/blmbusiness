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
	//请求用户管理的所有船舶的所有检验信息
	int GetAllShipsSurvey(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求用户管理的所有船舶的所有证书信息
	int GetAllShipsCertificate(const char* pUid, const char* jsonString, std::stringstream& out);
	//请求船舶的检验项的所有证书信息
	int GetShipSurveyCertificate(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//新增船舶的检验
	int AddShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out);
	//查看船舶的检验
	int GetShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out);
	//修改船舶的检验
	int UpdateShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除船舶的检验
	int DeleteShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out);
	//完成检验
	int FinishShipSurvey(const char* pUid, const char* jsonString, std::stringstream& out);

	//新增船舶证书
	int AddShipCertificate(const char* pUid, const char* jsonString, std::stringstream& out);
	//查看证书
	int GetShipCertificate(const char* pUid, const char* jsonString, std::stringstream& out);
	//修改证书
	int UpdateShipCertificate(const char* pUid, const char* jsonString, std::stringstream& out);
	//删除证书
	int DeleteShipCertificate(const char* pUid, const char* jsonString, std::stringstream& out);

	//显示预警模板******************************************************************
	//请求模板信息
	int GetTemplateInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//检验-证书（添加，删除）********************************************************
	//请求检验证书
	int GetSurveyInfo(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//请求预警设置的所有模板信息
	int GetAlertTemplateInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//证书1
	int getCerts(int id,std::string &outStr);

	//预警1
	int getLimit(int id,std::string &outStr);
	
	//船舶信息
	int getShips(int id,std::string &outStr);

	//新建预警模板信息
	int AddAlertTemplateInfo(const char* pUid, const char* jsonString, std::stringstream& out);
		
	//删除预警模板信息
	int DeleteAlertTemplate(const char* pUid, const char* jsonString, std::stringstream& out);

	//修改预警模板信息
	int EditAlertTemplateInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//请求船舶信息
	int GetShipsInfo(const char* pUid, const char* jsonString, std::stringstream& out);

	//添加关联证书
	int AddRelateCERT(const char* pUid, const char* jsonString, std::stringstream& out);

	//删除关联证书
	int DeleteRelateCERT(const char* pUid, const char* jsonString, std::stringstream& out);

	//请求工程单列表
	int GetEngineerList(const char* pUid, const char* jsonString, std::stringstream& out);

	//添加关联工程单
	int AddRelateEngineering(const char* pUid, const char* jsonString, std::stringstream& out);

	//删除关联工程单
	int DeleteRelateEngineering(const char* pUid, const char* jsonString, std::stringstream& out);

	
	//请求检验证书的相关检验
	int GetSurCERTRelate(const char* pUid, const char* jsonString, std::stringstream& out);
	
	//添加相关检验
	int AddRelateSur(const char* pUid, const char* jsonString, std::stringstream& out);

	//删除相关检验
	int DeleteRelateSur(const char* pUid, const char* jsonString, std::stringstream& out);

	//请求检验码表
	int GetSurveyMaTable(const char* pUid, const char* jsonString, std::stringstream& out);

	//请求证书码表
	int GetCERTMaTable(const char* pUid, const char* jsonString, std::stringstream& out);

	 //根据输入的时间获取天数差值
	 int getRemainDay(int startTime,int endTime);
	
};

typedef ACE_Singleton<eMOPSurveySvc, ACE_Null_Mutex> g_eMOPSurveySvc;

#endif // AREASVC_H
