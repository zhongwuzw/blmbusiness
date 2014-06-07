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


//����������Ϣ�ṹ 
struct SparePartsInfo
{
	string m_partid;//����ID
	string m_model;//�ͺ�
	string m_makefactory;//����
	string m_name_cn;
	string m_name_en;
	string m_charno;//ͼ��
	string m_standardno;//��׼��

    int m_highreq;//�ߴ�ֵ
	int m_lowreq;//�ʹ�ֵ
	string m_unit;//������λ
    string m_remarks;//��ע
	string m_desc_cn;//�����������������ģ�
	string m_desc_en;//��������������Ӣ�ģ�


	SparePartsInfo():m_partid(""),m_highreq(0),m_lowreq(0)
	{

	}
};

//�豸������Ϣ�ṹ
struct EquipBaseInfo
{
	string m_equipid;
	string m_model;//�豸�ͺ�
	string m_cmpykey;
	string m_cmpyname;//����
	string m_typecode;//ϵ�к�
    string m_desc;//����
};

//���ϻ�����Ϣ

struct MaterialBaseInfo
{
	string m_materialId; //����ID
	string m_category; //��������ID
	string m_key1;
	string m_key2;
	string m_key3;
	string m_catname_cn;
	string m_catname_en;
	string m_catCode;//������(���ϴ���)
	string m_name_cn;
	string m_name_en;
	string m_spec_cn;//���
	string m_spec_en;//���
	string m_lifeTime;//ʹ������
	string m_applyPeriod;//��������
	string m_unit_cn; //��λ
	string m_unit_en; //��λ
	int m_storeFlag; //����־
	string m_remark_cn; //��ע
	string m_remark_en; //��ע 
	string m_catCode1;//������
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
	map<string,SparePartsInfo> m_mapAllParts;//���еı�����Ϣ����T50_EMOP_PARTS_MODELS��
    string m_lastPartTimeStr;
	bool m_bpartLoading;
	ACE_RW_Thread_Mutex m_partsLock;

	map<string,EquipBaseInfo> m_mapAllEquips;//�����豸


	map<string,MaterialBaseInfo> m_mapAllMaterials;//���е�������Ϣ����T50_EMOP_MATERIALS��
	string m_lastMaterialTimeStr;
	bool m_bmaterialLoading;
	//ACE_RW_Thread_Mutex m_materialsLock; 
};

typedef ACE_Singleton<eMOPCommDataSvr, ACE_Null_Mutex> g_eMOPCommDataSvr;


#endif
