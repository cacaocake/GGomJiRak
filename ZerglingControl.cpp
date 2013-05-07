#include "ZerglingControl.h"

void ZerglingControl::add(BWAPI::Unit *z)
{
	zl.push_back(z);
}

void ZerglingControl::remove(BWAPI::Unit *z)
{
	this->zl.remove(z);
}

int ZerglingControl::getSize()
{
	return zl.size();
}

bool ZerglingControl::attackAll(BWAPI::Position* target)
{
	bool rtn = true;

	for(ZerglingList::iterator i = zl.begin(); i != zl.end(); i++)
	{
		if(rtn)
			rtn = (*i)->attack(*target);
		else
			(*i)->attack(*target);		
	}

	return rtn;
}

bool ZerglingControl::attackAll(BWAPI::Unit* target)
{
	bool rtn = true;

	for(ZerglingList::iterator i = zl.begin(); i != zl.end(); i++)
	{
		if(rtn)
			rtn = (*i)->attack(target);
		else
			(*i)->attack(target);		
	}

	return rtn;
}

ZerglingList::iterator ZerglingControl::begin()
{
	return zl.begin();
}

ZerglingList::iterator ZerglingControl::end()
{
	return zl.end();
}

void ZerglingControl::unique()
{
	zl.unique();
}