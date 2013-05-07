#pragma once
#include <BWAPI.h>
#include <BWTA.h>

class TaehoModule
{
public:
	// singleton
	static TaehoModule* getInstance();
	void init();
	
//	std::map<UnitType, int> unitTypeCounts;

	BWAPI::Unit* find_unit_by_type(BWAPI::Player* player, BWAPI::UnitType type);

	// add unit queue
	// assume resource usage
	// add mineral queue
	// assume resource gathered
	void add_unit_id(int id, BWAPI::Unit* unit);
	void add_mineral_id(int id, BWAPI::Unit* unit);
	void add_gas_id(int id, BWAPI::Unit* unit);
	void remove_unit_id(int id);
	void remove_mineral_id(int id);
	void remove_gas_id(int id);

	int calculate_enemy_gathered_mineral();
	int calculate_enemy_gathered_gas();
	int get_enemy_spend_mineral();
	int get_enemy_spend_gas();

	// event
	void onUnitShow(BWAPI::Unit* unit);
	void onUnitDestroy(BWAPI::Unit* unit);

	// 미네랄은 전체 미네랄 캔것 - 우리것 으로 계산하기

	// getter and setter
	std::map<BWAPI::UnitType, int> get_enemy_alive_unit_types();
protected:
	std::map<int, BWAPI::Unit*> self_alive_units;
	std::map<int, BWAPI::Unit*> enemy_alive_units;
	std::map<int, BWAPI::UnitType> enemy_alive_unit_types;
	std::map<int, BWAPI::Unit*> alive_minerals;
	std::map<int, BWAPI::Unit*> alive_gas;
	int used_mineral;
	int used_gas;
	int gathered_mineral;
	int gathered_gas;
};