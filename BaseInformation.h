#ifndef __BASE_INFORMATION_H__
#define __BASE_INFORMATION_H__

// �� ������ �ȵ��;;
#define CLOSE_MINERAL_SIZE 5

#include <BWAPI.h>
#include <BWTA.h>


// �̱������� ����� ���� �Ұ� ������ �������� �����

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
