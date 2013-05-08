#include "TaehoModule.h"

using namespace BWAPI;

// singleton
static TaehoModule* _instance = NULL;
TaehoModule* TaehoModule::getInstance()
{
	if (_instance == NULL)
	{
		_instance = new TaehoModule();
		_instance->init();
	}
	return _instance;
}

void TaehoModule::init()
{
	used_mineral = 0;
	used_gas = 0;
	gathered_mineral = 0;
	gathered_gas = 0;
}

Unit* TaehoModule::find_unit_by_type(Player* player, UnitType type)
{
	for(std::set<Unit*>::const_iterator i=player->getUnits().begin();i!=player->getUnits().end();i++)
	{
		if((*i)->getType() == type) return *i;
	}
	return NULL;
}

void TaehoModule::add_unit_id(int id, Unit* unit)
{
	// add unit into queue
	if(unit->getPlayer() == Broodwar->enemy())
	{
		if(!enemy_alive_units[id])
		{
			enemy_alive_units[id] = unit;
			enemy_alive_unit_types[id] = unit->getType();

			// calculate spend money
			used_mineral += unit->getType().mineralPrice();
			used_gas += unit->getType().gasPrice();
			//Broodwar->sendText("1 - %d %ss",unit->getID(), unit->getType().getName().c_str());
		}
	} else 
	{
		if(!self_alive_units[id])
		{
			self_alive_units[id] = unit;

			// calculate spend money
			//used_mineral += unit->getType().mineralPrice();
			//used_gas += unit->getType().gasPrice();
		}
	}
}


void TaehoModule::add_mineral_id(int id, Unit* unit)
{
	if(!alive_minerals[id])
	{
		alive_minerals[id] = unit;
	}
}

void TaehoModule::add_gas_id(int id, Unit* unit)
{
	if(!alive_gas[id])
	{
		alive_gas[id] = unit;
	}
}
void TaehoModule::remove_unit_id(int id)
{
	enemy_alive_units.erase(id);
	enemy_alive_unit_types.erase(id);
	self_alive_units.erase(id);
}
void TaehoModule::remove_mineral_id(int id)
{
	alive_minerals.erase(id);
}
void TaehoModule::remove_gas_id(int id)
{
	alive_gas.erase(id);
}

//int calculate_gathered_mineral();
//	int calculate_gathered_gas();
int TaehoModule::calculate_enemy_gathered_mineral()
{
	int temp_gathered_mineral = 0;
	for(std::map<int, Unit*>::iterator i=alive_minerals.begin();i!=alive_minerals.end();i++)
	{
		if((*i).second->isVisible())
			temp_gathered_mineral += (*i).second->getInitialResources() - (*i).second->getResources();
//(*i).second->get
	}
	temp_gathered_mineral = temp_gathered_mineral - Broodwar->self()->gatheredMinerals() + 50 + 50;

	if(this->gathered_mineral < temp_gathered_mineral)
		this->gathered_mineral = temp_gathered_mineral;
	return this->gathered_mineral;
}

int TaehoModule::calculate_enemy_gathered_gas()
{
	int temp_gathered_gas = 0;
	for(std::map<int, Unit*>::iterator i=alive_gas.begin();i!=alive_gas.end();i++)
	{
		if((*i).second->isVisible())
			temp_gathered_gas += (*i).second->getInitialResources() - (*i).second->getResources();
	}
	temp_gathered_gas = temp_gathered_gas - Broodwar->self()->gatheredGas();

	if(this->gathered_gas < temp_gathered_gas)
		this->gathered_gas = temp_gathered_gas;

	return this->gathered_gas;
}

int TaehoModule::get_enemy_spend_mineral()
{
	return used_mineral;
}

int TaehoModule::get_enemy_spend_gas()
{
	return used_gas;
}

// add unit queue
	// assume resource usage
	
	
	// add mineral queue
	// assume resource gathered


/*
 std::map<UnitType, int> unitTypeCounts;
  for(std::set<Unit*>::iterator i=myUnits.begin();i!=myUnits.end();i++)
  {
    if (unitTypeCounts.find((*i)->getType())==unitTypeCounts.end())
    {
      unitTypeCounts.insert(std::make_pair((*i)->
	  
	  getType(),0));
    }
    unitTypeCounts.find((*i)->getType())->second++;
  }
  int line=1;
  for(std::map<UnitType,int>::iterator i=unitTypeCounts.begin();i!=unitTypeCounts.end();i++)
  {
    Broodwar->drawTextScreen(5,16*line,"- %d %ss",(*i).second, (*i).first.getName().c_str());
    line++;
  }
  */


// event
void TaehoModule::onUnitShow(BWAPI::Unit* unit)
{
//  if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
//    Broodwar->sendText("A %s [%x] has been spotted at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
  
  if(unit->getType() == UnitTypes::Resource_Mineral_Field || unit->getType() == UnitTypes::Resource_Mineral_Field_Type_2 || unit->getType() == UnitTypes::Resource_Mineral_Field_Type_3)
  {
	  TaehoModule::getInstance()->add_mineral_id(unit->getID(), unit);
  }
  else if(unit->getType() == UnitTypes::Resource_Vespene_Geyser)
  {
	  TaehoModule::getInstance()->add_gas_id(unit->getID(), unit);
  }
  else
  {
	  TaehoModule::getInstance()->add_unit_id(unit->getID(), unit);
  }
}
void TaehoModule::onUnitDestroy(BWAPI::Unit* unit)
{
  if(unit->getType() == UnitTypes::Resource_Mineral_Field || unit->getType() == UnitTypes::Resource_Mineral_Field_Type_2 || unit->getType() == UnitTypes::Resource_Mineral_Field_Type_3)
  {
	  TaehoModule::getInstance()->remove_mineral_id(unit->getID());
  }
  else if(unit->getType() == UnitTypes::Resource_Vespene_Geyser)
  {
	  TaehoModule::getInstance()->remove_gas_id(unit->getID());
  }
  else
  {
	  TaehoModule::getInstance()->remove_unit_id(unit->getID());
  }
}

int TaehoModule::drawStats(int start_line)
{
int line=start_line;
	///
	// 내가 캔 자원
	Broodwar->drawTextScreen(5,16*line++,"- gathered by me : %d / %d", Broodwar->self()->gatheredMinerals(), Broodwar->self()->gatheredGas());
	//Broodwar->self()->spentMinerals();
	// 적 자원 예측
	Broodwar->drawTextScreen(5,16*line++,"- gathered by enemy : %d / %d", TaehoModule::getInstance()->calculate_enemy_gathered_mineral(), TaehoModule::getInstance()->calculate_enemy_gathered_gas());
	Broodwar->drawTextScreen(5,16*line++,"- spent by enemy    : %d / %d", TaehoModule::getInstance()->get_enemy_spend_mineral(), TaehoModule::getInstance()->get_enemy_spend_gas());

	// 점수 계산
	Broodwar->drawTextScreen(5,16*line++,"- my attack score : %lf", TaehoModule::getInstance()->analysis_my_attack_score());
	Broodwar->drawTextScreen(5,16*line++,"- enemy defense score : %lf", TaehoModule::getInstance()->analysis_enemy_defense_score());


	// 현재 적유닛 갯수
	std::map<UnitType, int> enemy_unitTypeCounts = TaehoModule::getInstance()->get_enemy_alive_unit_types();
	Broodwar->drawTextScreen(5,16*line++,"enemy units:");

  for(std::map<UnitType,int>::iterator i=enemy_unitTypeCounts.begin();i!=enemy_unitTypeCounts.end();i++)
  {
    Broodwar->drawTextScreen(5,16*line++,"- %d %ss",(*i).second, (*i).first.getName().c_str());
  }

  // 저글링 / 질럿의 점수
  // 우리의 공격점수 / 상대의 방어점수

  return line;
}


// getter and setter
std::map<UnitType, int> TaehoModule::get_enemy_alive_unit_types()
{
	std::map<UnitType, int> enemy_unitTypeCounts;
  for(std::map<int, UnitType>::iterator i=enemy_alive_unit_types.begin();i!=enemy_alive_unit_types.end();i++)
  {
	  if((*i).second)
	  {
		  if (enemy_unitTypeCounts.find((*i).second)==enemy_unitTypeCounts.end())
    {
		enemy_unitTypeCounts.insert(std::make_pair((*i).second,0));
    }
    enemy_unitTypeCounts.find((*i).second)->second++;
	  }
  }
  return enemy_unitTypeCounts;
}

// analysis
double TaehoModule::analysis_my_attack_score()
{
	double score = 0;

	std::map<BWAPI::UnitType, double> score_map;
	score_map[UnitTypes::Zerg_Zergling] = 3;

	for(std::map<int, BWAPI::Unit*>::const_iterator i=self_alive_units.begin();i!=self_alive_units.end();i++)
	{
		BWAPI::UnitType type = (*i).second->getType();
		
		if(score_map[type])
			score += score_map[type];
	}

	return score;
}
double TaehoModule::analysis_my_defense_score()
{
	double score = 0;

	std::map<BWAPI::UnitType, double> score_map;
	score_map[UnitTypes::Zerg_Zergling] = 2;

	for(std::map<int, BWAPI::Unit*>::const_iterator i=self_alive_units.begin();i!=self_alive_units.end();i++)
	{
		BWAPI::UnitType type = (*i).second->getType();
		
		if(score_map[type])
			score += score_map[type];
	}

	return score;
}
double TaehoModule::analysis_enemy_attack_score()
{
	double score = 0;

	std::map<BWAPI::UnitType, double> score_map;
	score_map[UnitTypes::Protoss_Zealot] = 9;

	for(std::map<int, BWAPI::UnitType>::const_iterator i=enemy_alive_unit_types.begin();i!=enemy_alive_unit_types.end();i++)
	{
		BWAPI::UnitType type = (*i).second;
		
		if(score_map[type])
			score += score_map[type];
	}

	return score;
}
double TaehoModule::analysis_enemy_defense_score()
{
	double score = 0;

	std::map<BWAPI::UnitType, double> score_map;
	score_map[UnitTypes::Protoss_Zealot] = 11;

	for(std::map<int, BWAPI::UnitType>::const_iterator i=enemy_alive_unit_types.begin();i!=enemy_alive_unit_types.end();i++)
	{
		BWAPI::UnitType type = (*i).second;
		
		if(score_map[type])
			score += score_map[type];
	}

	return score;
}