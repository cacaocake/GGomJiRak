#pragma once
#include <BWAPI.h>
#include <list>
#include <queue>
#include <utility>
#include <BWTA.h>
#include <windows.h>

#include "../BaseInformation.h"
#include "../ZerglingControl.h"
#include "../TaehoModule.h"

extern bool analyzed;
extern bool analysis_just_finished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

typedef std::pair<BWAPI::TilePosition, BWAPI::UnitType> BuildingInfo;
typedef std::pair<BWAPI::Unit*, BuildingInfo> BuildingJob;

typedef enum _buildState{
	five_StartBuildingSpawningPool,
	five_BuildingSpawningPool,
	five_MorphDronUntill6,
	five_WaitingSpawningPool,
	five_MorphSixZerglings,
	five_MorphZergling,
	five_MorphOverload,
	five_Attack
} BuildState;

class ExampleAIModule : public BWAPI::AIModule
{
public:
  virtual void onStart();
  virtual void onEnd(bool isWinner);
  virtual void onFrame();
  virtual void onSendText(std::string text);
  virtual void onReceiveText(BWAPI::Player* player, std::string text);
  virtual void onPlayerLeft(BWAPI::Player* player);
  virtual void onNukeDetect(BWAPI::Position target);
  virtual void onUnitDiscover(BWAPI::Unit* unit);
  virtual void onUnitEvade(BWAPI::Unit* unit);
  virtual void onUnitShow(BWAPI::Unit* unit);
  virtual void onUnitHide(BWAPI::Unit* unit);
  virtual void onUnitCreate(BWAPI::Unit* unit);
  virtual void onUnitDestroy(BWAPI::Unit* unit);
  virtual void onUnitMorph(BWAPI::Unit* unit);
  virtual void onUnitRenegade(BWAPI::Unit* unit);
  virtual void onSaveGame(std::string gameName);
  virtual void onUnitComplete(BWAPI::Unit *unit);
  void drawStats(); //not part of BWAPI::AIModule
  void drawBullets();
  void drawVisibilityData();
  void drawTerrainData();
  void showPlayers();
  void showForces();
  bool show_bullets;
  bool show_visibility_data;

  //by kkk
  void spreadWorkers();
  BWAPI::Unit* building();

  void goOverloadGo();
  void turnOverloadTurn();

  //void morphZergling();

  void startBuildingSpawningPool();
  void addZerglings();
  void comeOnLetsGo();
  void comeOnLetsGo(BWAPI::Unit* u);
  void changingEnermyBase();
  void attackAllZergling();

  BaseInformation baseInformation;
  std::queue<BWAPI::Position*> scoutRoute;
  BWAPI::Unit* scoutOverload;
  bool isScouting;
  bool isZerglingAdded;
  
  BuildState buildState;  
  std::list<BuildingJob> buildingJobList;
  
  BWAPI::Unit* buildResult;
  BWAPI::Position* enermyBase;
  ZerglingControl zerglingControl;

  int DEBUG_COUNT;
};
