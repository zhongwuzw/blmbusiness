#ifndef __EMOPSEQUENCEMANAGER_H__
#define __EMOPSEQUENCEMANAGER_H__

#include "kSQL.h"
#include <map>
#include "define.h"

using namespace std;


#define  GET_EMOP_SEQUENCE_INT(CODE,SEQUENCE){SEQUENCE=g_Sequence::instance()->GetSequence(#CODE);}
#define  GET_EMOP_SEQUENCE_STRING(CODE,SEQUENCE){SEQUENCE=g_Sequence::instance()->GetSequenceStr(#CODE);}

class SequeceManager
{
public:
	SequeceManager()
	{

	}
	~SequeceManager()
	{

	}


public:
	bool Init()
	{

		MySql* psql = CREATE_MYSQL;
		char sql[256]="select ID,SEQUENCE from blm_emop_etl.t91_sequence where IS_VALID='1'";
		CHECK_MYSQL_STATUS(psql->Query(sql), 1);

		char id[64]="";
		int seq=10000;
		while(psql->NextRow())
		{
			READMYSQL_STR(ID,id)
			READMYSQL_INT(SEQUENCE,seq,10000)
            map_Sequence[string(id)]=seq;
		}
      
		RELEASE_MYSQL_NORETURN(psql);
       
		return true;
	}
	int GetSequence(string code)
	{
		int sequence=_GetSequence(code);

		MySql* psql = CREATE_MYSQL;
		char sql[512]="";
		sprintf (sql, "REPLACE INTO blm_emop_etl.t91_sequence(ID,SEQUENCE,IS_VALID) VALUES('%s',%d,'1')",code.c_str(),sequence);
		CHECK_MYSQL_STATUS(psql->Execute(sql)>=0, 3);
       
		RELEASE_MYSQL_NORETURN(psql);
		return sequence;
	}

	string GetSequenceStr(string code)
	{
        int sequence=GetSequence(code);
		return IntToStr(sequence);
	}

private:
	int _GetSequence(string code)
	{

		ACE_Guard<ACE_Thread_Mutex> guard(g_cs);
		int sequence=10000;

		if (map_Sequence.find(code)==map_Sequence.end())
		{
			map_Sequence[code]=sequence;//从10000开始
		}
		else
		{
			map_Sequence[code]++;
			sequence=map_Sequence[code];
		}
        return sequence;
	}
private:
	map<string,int> map_Sequence;
	//临界资源
	ACE_Thread_Mutex g_cs;

};

typedef ACE_Singleton<SequeceManager, ACE_Null_Mutex> g_Sequence;



#endif
