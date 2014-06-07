#ifndef __BLM_EMOPCOMMONDATA_SVR_H__
#define __BLM_EMOPCOMMONDATA_SVR_H__

#include <string>
#include "IBusinessService.h"
#include "kSQL.h"
#include "define.h"

using namespace std;


inline string GetCurrentTmStr()
{
	char timestr[20]="";
	time_t lt= time(NULL);
	struct tm *ptr = localtime(&lt);
	strftime(timestr,20,"%Y-%m-%d %H:%M:%S",ptr);

	return string(timestr);
}

inline std::string CurLocalDateSeq()
{
	time_t t = time(NULL);
	tm* aTm = localtime(&t);

	char curdate[20];
	snprintf(curdate, 20, "%04d%02d%02d", aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday);
	return std::string(curdate);
}


//备件基本信息结构 
struct SparePartsInfo
{
	string m_partid;//备件ID
	string m_model;//型号
	string m_makefactory;//厂商
	string m_name_cn;
	string m_name_en;
	string m_charno;//图号
	string m_standardno;//标准号

    int m_highreq;//高储值
	int m_lowreq;//低储值
	string m_unit;//计量单位
    string m_remarks;//备注
	string m_desc_cn;//备件参数描述（中文）
	string m_desc_en;//备件参数描述（英文）


	SparePartsInfo():m_partid(""),m_highreq(0),m_lowreq(0)
	{

	}
};

//设备基本信息结构
struct EquipBaseInfo
{
	string m_equipid;
	string m_model;//设备型号
	string m_cmpykey;
	string m_cmpyname;//厂商
	string m_typecode;//系列号
    string m_desc;//描述
};

//物料基本信息

struct MaterialBaseInfo
{
	string m_materialId; //物料ID
	string m_category; //所属分类ID
	string m_key1;
	string m_key2;
	string m_key3;
	string m_catname_cn;
	string m_catname_en;
	string m_catCode;//类别代码(物料代码)
	string m_name_cn;
	string m_name_en;
	string m_spec_cn;//规格
	string m_spec_en;//规格
	string m_lifeTime;//使用寿命
	string m_applyPeriod;//申领周期
	string m_unit_cn; //单位
	string m_unit_en; //单位
	int m_storeFlag; //入库标志
	string m_remark_cn; //备注
	string m_remark_en; //备注 
	string m_catCode1;//类别代码
	vector<string> picIds;

};



class eMOPCommDataSvr :ACE_Event_Handler
{
public:
	eMOPCommDataSvr();
	~eMOPCommDataSvr();

	 void Init();
	 virtual int  handle_timeout(const ACE_Time_Value &tv, const void *arg);

	 SparePartsInfo *GetPartItemById(string partid);
     void AddNewPartItem(SparePartsInfo &partItem);
	 void DeletePartItem(string partid);

     EquipBaseInfo *GetEquipItemById(string equipid);
     void AddNewEquipItem(EquipBaseInfo &equipItem);
	 void DeleteEquipItem(string equipid);
	 
	 MaterialBaseInfo *GetMaterialItemById(string materialId);
     void AddMaterialItem(MaterialBaseInfo &materialItem);
	 void DeleteMaterialItem(string materialId);
     
	 map<string,MaterialBaseInfo> &GetAllMaterials(){return m_mapAllMaterials;}

      
private:
	bool _refreshParts();
	bool _refreshEuqips();
	bool _refreshMaterials();
	int InitMaterial();
	int InitMaterialPics();
	int GetPics(MaterialBaseInfo &mbase);
private:
	map<string,SparePartsInfo> m_mapAllParts;//所有的备件信息（表T50_EMOP_PARTS_MODELS）
    string m_lastPartTimeStr;
	bool m_bpartLoading;
	ACE_RW_Thread_Mutex m_partsLock;

	map<string,EquipBaseInfo> m_mapAllEquips;//所有设备


	map<string,MaterialBaseInfo> m_mapAllMaterials;//所有的物料信息（表T50_EMOP_MATERIALS）
	string m_lastMaterialTimeStr;
	bool m_bmaterialLoading;
	//ACE_RW_Thread_Mutex m_materialsLock; 
};

typedef ACE_Singleton<eMOPCommDataSvr, ACE_Null_Mutex> g_eMOPCommDataSvr;


#endif
