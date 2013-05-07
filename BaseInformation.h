#ifndef __BASE_INFORMATION_H__
#define __BASE_INFORMATION_H__

// 아 마음에 안든다;;
#define CLOSE_MINERAL_SIZE 5

#include <BWAPI.h>
#include <BWTA.h>


// 싱글톤으로 만들어 놔야 할거 같은디 만든적이 없어성ㅋ

class BaseInformation{

public:
	BaseInformation();

	BWAPI::Unit * baseHatchery;
	BWAPI::Unit * spawningPool;
	BWAPI::Unit* closeMinerals[CLOSE_MINERAL_SIZE];
	void init();
	bool initCloseMinerals();
	int closeMineralSize;
private:
	
};

#endif
