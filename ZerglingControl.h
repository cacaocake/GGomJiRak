#ifndef __ZERGLING_CONTROL_H__
#define __ZERGLING_CONTROL_H__

#include <BWAPI.h>
#include <BWTA.h>
#include <list>

//클래스에 iterator 만드는거 어케 하지? ㅠㅠ
//걍 상속 받을까...
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