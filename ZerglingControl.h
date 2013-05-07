#ifndef __ZERGLING_CONTROL_H__
#define __ZERGLING_CONTROL_H__

#include <BWAPI.h>
#include <BWTA.h>
#include <list>

//Ŭ������ iterator ����°� ���� ����? �Ф�
//�� ��� ������...
typedef std::list<BWAPI::Unit*> ZerglingList;

class ZerglingControl{
public:
	void add(BWAPI::Unit* z);
	void remove(BWAPI::Unit* z);
	void unique();

	int getSize();
	
	bool attackAll(BWAPI::Position* target);
	bool attackAll(BWAPI::Unit* target);
	
	

	ZerglingList::iterator begin();
	ZerglingList::iterator end();

private:
	ZerglingList zl;
};
#endif