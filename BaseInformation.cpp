#include "BaseInformation.h"
using namespace BWAPI;

BaseInformation::BaseInformation()
{	
	init();	
}
void BaseInformation::init()
{
	closeMineralSize = CLOSE_MINERAL_SIZE;

	//����� �̳׶� �ʱ�ȭ
	if(!initCloseMinerals())
	{
		Broodwar->sendText("Error: init close Minerals");
		return;
	}
	
}
bool BaseInformation::initCloseMinerals()
{
	for(int i = 0; i < CLOSE_MINERAL_SIZE; i++)
	{
		closeMinerals[i] = NULL;
	}

	//////////////////////////////////////////////////////
	// Find 4 closest minerals
	//////////////////////////////////////////////////////		
	for(std::set<Unit*>::iterator m=Broodwar->getMinerals().begin();m!=Broodwar->getMinerals().end();m++)
	{
		Unit* currentM = *m;
		Position sl(Broodwar->self()->getStartLocation());

		for(int cm = 0; cm < CLOSE_MINERAL_SIZE; cm++)
		{
			if (closeMinerals[cm] == NULL)
			{
				// ����� �̳׶��� 1�� �ۿ� ������ ������ 3������ ���밡 �������� ������
				// �ϴ� NULL�� ó�������� �� ä��� �ɵ�
				closeMinerals[cm] = currentM;						
			}					
			else if(sl.getDistance(currentM->getPosition())< sl.getDistance(closeMinerals[cm]->getPosition()))
			{						
				Unit* swapTemp = closeMinerals[cm];
				closeMinerals[cm] = currentM;
				currentM = swapTemp;
			}
		}
	}

	return true;
}
