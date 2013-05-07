#include "BaseInformation.h"
using namespace BWAPI;

BaseInformation::BaseInformation()
{	
	init();	
}
void BaseInformation::init()
{
	closeMineralSize = CLOSE_MINERAL_SIZE;

	//가까운 미네랄 초기화
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
				// 가까운 미네랄이 1개 밖에 없으면 나머지 3마리는 갈대가 없어지기 때문에
				// 일단 NULL은 처음값으로 다 채우게 될듯
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
