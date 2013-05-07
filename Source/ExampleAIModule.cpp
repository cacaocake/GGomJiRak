#include "ExampleAIModule.h"
using namespace BWAPI;

bool analyzed;
bool analysis_just_finished;
BWTA::Region* home;
BWTA::Region* enemy_base;

//std::queue<Unit*> workerQueue;

void ExampleAIModule::onStart()
{
	DEBUG_COUNT = 0;
  //Broodwar->sendText("Hello world!");
  Broodwar->printf("The map is %s, a %d player map",Broodwar->mapName().c_str(),Broodwar->getStartLocations().size());
  // Enable some cheat flags
  Broodwar->enableFlag(Flag::UserInput);
  // Uncomment to enable complete map information
  //Broodwar->enableFlag(Flag::CompleteMapInformation);

  //read map information into BWTA so terrain analysis can be done in another thread
  BWTA::readMap();
  analyzed=false;
  analysis_just_finished=false;

  show_bullets=false;
  show_visibility_data=false;

  if (Broodwar->isReplay())
  {
    Broodwar->printf("The following players are in this replay:");
    for(std::set<Player*>::iterator p=Broodwar->getPlayers().begin();p!=Broodwar->getPlayers().end();p++)
    {
      if (!(*p)->getUnits().empty() && !(*p)->isNeutral())
      {
        Broodwar->printf("%s, playing as a %s",(*p)->getName().c_str(),(*p)->getRace().getName().c_str());
      }
    }
  }
  else
  {
    Broodwar->printf("The match up is %s v %s",
      Broodwar->self()->getRace().getName().c_str(),
      Broodwar->enemy()->getRace().getName().c_str());
	
	//start init 이 부분을 함수로 따로 빼서 정리하는게 좋을듯
	spreadWorkers(); // 일꾼퍼뜨리기
	goOverloadGo(); // 오버로드 정찰 보내기
	
	buildState = five_StartBuildingSpawningPool; // 빌드 초기화
	enermyBase = NULL; // 적 위치 초기화
	isZerglingAdded = false;
	
  }
}

void ExampleAIModule::onEnd(bool isWinner)
{
  if (isWinner)
  {
    //log win to file
  }
}

void ExampleAIModule::onFrame()
{
	
	// 건물 지을거 있으면 짓게 하기... buildState와 연동해서 뭔가 하고 싶은데... 코드가 더러븜 ㅠㅠ
	buildResult = building();
	if(buildResult != NULL)
	{
		if(buildResult->getType() == UnitTypes::Zerg_Spawning_Pool)
		{
			buildState = five_MorphDronUntill6;			
			baseInformation.spawningPool = buildResult;
		}
	}
	/*
	if(isZerglingAdded)
	{
		addZerglings();
	}
	*/
	if(buildState == five_StartBuildingSpawningPool)
	{
		startBuildingSpawningPool();
	}
	else if(buildState == five_MorphDronUntill6)
	{
		if(Broodwar->self()->minerals()	> 49)
		{
			std::set<Unit*> myLarva=baseInformation.baseHatchery->getLarva();
			if (myLarva.size()>0)
			{
				Unit* larva=*myLarva.begin();
				larva->morph(UnitTypes::Zerg_Drone);
				Broodwar->printf("Spent Minerals: %d", Broodwar->self()->spentMinerals());
				
				//6마리가 되면 그만 처음 50원은 빼야 하나;;;				
				if(Broodwar->self()->spentMinerals() == 300)
				{					
					buildState = five_WaitingSpawningPool;					
				}
			}
		}
	}
	else if(buildState == five_WaitingSpawningPool)
	{
		if(baseInformation.spawningPool->isCompleted())
			buildState = five_MorphSixZerglings;
	}
	else if(buildState == five_MorphSixZerglings)
	{
		
		std::set<Unit*> myLarva=baseInformation.baseHatchery->getLarva();
		if (myLarva.size()>0)
		{
			
			for(std::set<Unit*>::iterator i = myLarva.begin(); i != myLarva.end(); i++)
			{
				Unit* larva = (*i);
				larva->morph(UnitTypes::Zerg_Zergling);
			}
			Broodwar->printf("five_MorphSixZerglings Done");			
			buildState = five_MorphOverload;
		}	

		
	}
	else if(buildState == five_MorphOverload)
	{
		std::set<Unit*> myLarva=baseInformation.baseHatchery->getLarva();
		if (myLarva.size()>0 && Broodwar->self()->minerals() > 99)
		{
			Unit* larva=*myLarva.begin();
			larva->morph(UnitTypes::Zerg_Overlord);

			Broodwar->printf("Spent Minerals: %d", Broodwar->self()->spentMinerals());
			if(Broodwar->self()->spentMinerals() > 450)
			{
				buildState = five_MorphZergling;
				Broodwar->printf("five_MorphOverload Done");

			}
			
		}
	}
	else if(buildState == five_MorphZergling)
	{
		
		if(Broodwar->self()->minerals()	> 49)
		{
			std::set<Unit*> myLarva=baseInformation.baseHatchery->getLarva();
			if (myLarva.size()>0)
			{
				Unit* larva=*myLarva.begin();
				larva->morph(UnitTypes::Zerg_Zergling);
			}
		}
	}
	// 일꾼 일시키기 - 이거 바꿔야 할듯 for문으로 ㅋ 
	
	
	// 오버로드 정찰 보내기
	if(isScouting)
	{			
		turnOverloadTurn();
	}
	

  if (show_visibility_data)
    drawVisibilityData();

  if (show_bullets)
    drawBullets();

  if (Broodwar->isReplay())
    return;

  drawStats();
  if (analyzed && Broodwar->getFrameCount()%30==0)
  {
    //order one of our workers to guard our chokepoint.
    for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
    {
      if ((*i)->getType().isWorker())
      {
        //get the chokepoints linked to our home region
        std::set<BWTA::Chokepoint*> chokepoints= home->getChokepoints();
        double min_length=10000;
        BWTA::Chokepoint* choke=NULL;

        //iterate through all chokepoints and look for the one with the smallest gap (least width)
        for(std::set<BWTA::Chokepoint*>::iterator c=chokepoints.begin();c!=chokepoints.end();c++)
        {
          double length=(*c)->getWidth();
          if (length<min_length || choke==NULL)
          {
            min_length=length;
            choke=*c;
          }
        }

        //order the worker to move to the center of the gap
        (*i)->rightClick(choke->getCenter());
        break;
      }
    }
  }
  if (analyzed)
    drawTerrainData();

  if (analysis_just_finished)
  {
    Broodwar->printf("Finished analyzing map.");
    analysis_just_finished=false;
  }

  
  

}

void ExampleAIModule::onSendText(std::string text)
{
  if (text=="/show bullets")
  {
    show_bullets = !show_bullets;
  } else if (text=="/show players")
  {
    showPlayers();
  } else if (text=="/show forces")
  {
    showForces();
  } else if (text=="/show visibility")
  {
    show_visibility_data=!show_visibility_data;
  } else if (text=="/analyze")
  {
    if (analyzed == false)
    {
      Broodwar->printf("Analyzing map... this may take a minute");
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
    }
  } else
  {
    Broodwar->printf("You typed '%s'!",text.c_str());
    Broodwar->sendText("%s",text.c_str());
  }
}

void ExampleAIModule::onReceiveText(BWAPI::Player* player, std::string text)
{
  Broodwar->printf("%s said '%s'", player->getName().c_str(), text.c_str());
}

void ExampleAIModule::onPlayerLeft(BWAPI::Player* player)
{
  //Broodwar->sendText("%s left the game.",player->getName().c_str());
}

void ExampleAIModule::onNukeDetect(BWAPI::Position target)
{
  if (target!=Positions::Unknown)
    Broodwar->printf("Nuclear Launch Detected at (%d,%d)",target.x(),target.y());
  else
    Broodwar->printf("Nuclear Launch Detected");
}

void ExampleAIModule::onUnitDiscover(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
  {
    //Broodwar->sendText("A %s [%x] has been discovered at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
	
	// 초반이니까... 적의 본진일듯
	if(unit->getType().isResourceDepot())
	{
		isScouting = false;
		enermyBase = new Position(unit->getTilePosition());
		Broodwar->sendText("Scouting Stop!");
	}
  }
}

void ExampleAIModule::onUnitEvade(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
  {
    //Broodwar->sendText("A %s [%x] was last accessible at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
  }
}

void ExampleAIModule::onUnitShow(BWAPI::Unit* unit)
{
	TaehoModule::getInstance()->onUnitShow(unit);
  if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
  {
    //Broodwar->sendText("A %s [%x] has been spotted at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());	
  }
}

void ExampleAIModule::onUnitHide(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
  {
    //Broodwar->sendText("A %s [%x] was last seen at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
  }
}

void ExampleAIModule::onUnitCreate(BWAPI::Unit* unit)
{
  if (Broodwar->getFrameCount()>1)
  {
    if (!Broodwar->isReplay())
	{
      //Broodwar->sendText("A %s [%x] has been created at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
		
	}
    else
    {
      /*if we are in a replay, then we will print out the build order
      (just of the buildings, not the units).*/
      if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false)
      {
        int seconds=Broodwar->getFrameCount()/24;
        int minutes=seconds/60;
        seconds%=60;
        //Broodwar->sendText("%.2d:%.2d: %s creates a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());		
      }
    }
  }
}

void ExampleAIModule::onUnitDestroy(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
  {
    //Broodwar->sendText("A %s [%x] has been destroyed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
	  TaehoModule::getInstance()->onUnitDestroy(unit);
  }
}

void ExampleAIModule::onUnitMorph(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay())
  {
    //Broodwar->sendText("A %s [%x] has been morphed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
	
	/*
		if (unit->getType().isWorker())
		{
			workerQueue.push(unit);			
		}
		
		
		
  }	
  else
  {
    /*if we are in a replay, then we will print out the build order
    (just of the buildings, not the units).*/
    if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false)
    {
      int seconds=Broodwar->getFrameCount()/24;
      int minutes=seconds/60;
      seconds%=60;
      //Broodwar->sendText("%.2d:%.2d: %s morphs a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());
	  
    }
  }
}

void ExampleAIModule::onUnitRenegade(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay())
    Broodwar->sendText("A %s [%x] is now owned by %s",unit->getType().getName().c_str(),unit,unit->getPlayer()->getName().c_str());
}

void ExampleAIModule::onSaveGame(std::string gameName)
{
  Broodwar->printf("The game was saved to \"%s\".", gameName.c_str());
}

DWORD WINAPI AnalyzeThread()
{
  BWTA::analyze();

  //self start location only available if the map has base locations
  if (BWTA::getStartLocation(BWAPI::Broodwar->self())!=NULL)
  {
    home       = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
  }
  //enemy start location only available if Complete Map Information is enabled.
  if (BWTA::getStartLocation(BWAPI::Broodwar->enemy())!=NULL)
  {
    enemy_base = BWTA::getStartLocation(BWAPI::Broodwar->enemy())->getRegion();
  }
  analyzed   = true;
  analysis_just_finished = true;
  return 0;
}

void ExampleAIModule::drawStats()
{
  std::set<Unit*> myUnits = Broodwar->self()->getUnits();
  //Broodwar->drawTextScreen(5,0,"I have %d units:",myUnits.size());
  std::map<UnitType, int> unitTypeCounts;
  for(std::set<Unit*>::iterator i=myUnits.begin();i!=myUnits.end();i++)
  {
    if (unitTypeCounts.find((*i)->getType())==unitTypeCounts.end())
    {
      unitTypeCounts.insert(std::make_pair((*i)->getType(),0));
    }
    unitTypeCounts.find((*i)->getType())->second++;
	
	if((*i)->getType() == UnitTypes::Zerg_Zergling)
	{
		
		if(isScouting)
		{
			// 갔는데 없음 ㅋ 
			//Broodwar->printf("distance: %d", (*i)->getDistance(*scoutRoute.front()));
			if((*i)->getDistance(*scoutRoute.front()) < 70)
			{
				
				changingEnermyBase();
				attackAllZergling();				
			}
		}
		// 공격 안하고 있으면
		if((*i)->isIdle())
		{
			//DEBUG_COUNT++;
			//Broodwar->printf("Attack! %d", DEBUG_COUNT);
			
			comeOnLetsGo((*i));
		}
	}
	else if(Broodwar->getFrameCount()>50 &&(*i)->getType().isWorker() && (*i)->isIdle())
	{
		(*i)->gather(baseInformation.closeMinerals[4]);
		
	}
  }
  int line=1;
	///
	// 내가 캔 자원
	Broodwar->drawTextScreen(5,16*line++,"- gathered by me : %d / %d", Broodwar->self()->gatheredMinerals(), Broodwar->self()->gatheredGas());
	//Broodwar->self()->spentMinerals();
	// 적 자원 예측
	Broodwar->drawTextScreen(5,16*line++,"- gathered by enemy : %d / %d", TaehoModule::getInstance()->calculate_enemy_gathered_mineral(), TaehoModule::getInstance()->calculate_enemy_gathered_gas());
	Broodwar->drawTextScreen(5,16*line++,"- spent by enemy    : %d / %d", TaehoModule::getInstance()->get_enemy_spend_mineral(), TaehoModule::getInstance()->get_enemy_spend_gas());

	// 현재 적유닛 갯수
	std::map<UnitType, int> enemy_unitTypeCounts = TaehoModule::getInstance()->get_enemy_alive_unit_types();
	Broodwar->drawTextScreen(5,16*line++,"enemy units:");

  //for(std::map<UnitType,int>::iterator i=unitTypeCounts.begin();i!=unitTypeCounts.end();i++)
 // {
//    Broodwar->drawTextScreen(5,16*line,"- %d %ss",(*i).second, (*i).first.getName().c_str());
 //   line++;	
  //}
  for(std::map<UnitType,int>::iterator i=enemy_unitTypeCounts.begin();i!=enemy_unitTypeCounts.end();i++)
  {
    Broodwar->drawTextScreen(5,16*line++,"- %d %ss",(*i).second, (*i).first.getName().c_str());
  }

  
}

void ExampleAIModule::drawBullets()
{
  std::set<Bullet*> bullets = Broodwar->getBullets();
  for(std::set<Bullet*>::iterator i=bullets.begin();i!=bullets.end();i++)
  {
    Position p=(*i)->getPosition();
    double velocityX = (*i)->getVelocityX();
    double velocityY = (*i)->getVelocityY();
    if ((*i)->getPlayer()==Broodwar->self())
    {
      Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Green);
      Broodwar->drawTextMap(p.x(),p.y(),"\x07%s",(*i)->getType().getName().c_str());
    }
    else
    {
      Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Red);
      Broodwar->drawTextMap(p.x(),p.y(),"\x06%s",(*i)->getType().getName().c_str());
    }
  }
}

void ExampleAIModule::drawVisibilityData()
{
  for(int x=0;x<Broodwar->mapWidth();x++)
  {
    for(int y=0;y<Broodwar->mapHeight();y++)
    {
      if (Broodwar->isExplored(x,y))
      {
        if (Broodwar->isVisible(x,y))
          Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Green);
        else
          Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Blue);
      }
      else
        Broodwar->drawDotMap(x*32+16,y*32+16,Colors::Red);
    }
  }
}

void ExampleAIModule::drawTerrainData()
{
  //we will iterate through all the base locations, and draw their outlines.
  for(std::set<BWTA::BaseLocation*>::const_iterator i=BWTA::getBaseLocations().begin();i!=BWTA::getBaseLocations().end();i++)
  {
    TilePosition p=(*i)->getTilePosition();
    Position c=(*i)->getPosition();

    //draw outline of center location
    Broodwar->drawBox(CoordinateType::Map,p.x()*32,p.y()*32,p.x()*32+4*32,p.y()*32+3*32,Colors::Blue,false);

    //draw a circle at each mineral patch
    for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getStaticMinerals().begin();j!=(*i)->getStaticMinerals().end();j++)
    {
      Position q=(*j)->getInitialPosition();
      Broodwar->drawCircle(CoordinateType::Map,q.x(),q.y(),30,Colors::Cyan,false);
    }

    //draw the outlines of vespene geysers
    for(std::set<BWAPI::Unit*>::const_iterator j=(*i)->getGeysers().begin();j!=(*i)->getGeysers().end();j++)
    {
      TilePosition q=(*j)->getInitialTilePosition();
      Broodwar->drawBox(CoordinateType::Map,q.x()*32,q.y()*32,q.x()*32+4*32,q.y()*32+2*32,Colors::Orange,false);
    }

    //if this is an island expansion, draw a yellow circle around the base location
    if ((*i)->isIsland())
      Broodwar->drawCircle(CoordinateType::Map,c.x(),c.y(),80,Colors::Yellow,false);
  }

  //we will iterate through all the regions and draw the polygon outline of it in green.
  for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
  {
    BWTA::Polygon p=(*r)->getPolygon();
    for(int j=0;j<(int)p.size();j++)
    {
      Position point1=p[j];
      Position point2=p[(j+1) % p.size()];
      Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Green);
    }
  }

  //we will visualize the chokepoints with red lines
  for(std::set<BWTA::Region*>::const_iterator r=BWTA::getRegions().begin();r!=BWTA::getRegions().end();r++)
  {
    for(std::set<BWTA::Chokepoint*>::const_iterator c=(*r)->getChokepoints().begin();c!=(*r)->getChokepoints().end();c++)
    {
      Position point1=(*c)->getSides().first;
      Position point2=(*c)->getSides().second;
      Broodwar->drawLine(CoordinateType::Map,point1.x(),point1.y(),point2.x(),point2.y(),Colors::Red);
    }
  }
}

void ExampleAIModule::showPlayers()
{
  std::set<Player*> players=Broodwar->getPlayers();
  for(std::set<Player*>::iterator i=players.begin();i!=players.end();i++)
  {
    Broodwar->printf("Player [%d]: %s is in force: %s",(*i)->getID(),(*i)->getName().c_str(), (*i)->getForce()->getName().c_str());
  }
}

void ExampleAIModule::showForces()
{
  std::set<Force*> forces=Broodwar->getForces();
  for(std::set<Force*>::iterator i=forces.begin();i!=forces.end();i++)
  {
    std::set<Player*> players=(*i)->getPlayers();
    Broodwar->printf("Force %s has the following players:",(*i)->getName().c_str());
    for(std::set<Player*>::iterator j=players.begin();j!=players.end();j++)
    {
      Broodwar->printf("  - Player [%d]: %s",(*j)->getID(),(*j)->getName().c_str());
    }
  }
}

void ExampleAIModule::onUnitComplete(BWAPI::Unit *unit)
{
  if (!Broodwar->isReplay() && Broodwar->getFrameCount()>1)
  {
    //Broodwar->sendText("A %s [%x] has been completed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
  }
}

void ExampleAIModule::spreadWorkers()
{
	//send each worker to the mineral field that is closest to it
	int currentClosestMineral = 0;
    for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
    {
      if ((*i)->getType().isWorker())
      {		
        if (currentClosestMineral < baseInformation.closeMineralSize)
          (*i)->rightClick(baseInformation.closeMinerals[currentClosestMineral++]);
      }
      else if ((*i)->getType().isResourceDepot())
      {

        //if this is a center, tell it to build the appropiate type of worker
        if ((*i)->getType().getRace()!=Races::Zerg)
        {
          (*i)->train(Broodwar->self()->getRace().getWorker());
        }
        else //if we are Zerg, we need to select a larva and morph it into a drone
        {
          std::set<Unit*> myLarva=(*i)->getLarva();
          if (myLarva.size()>0)
          {
            Unit* larva=*myLarva.begin();
            larva->morph(UnitTypes::Zerg_Drone);
          }
		  // set base hatchery		  
		  baseInformation.baseHatchery = *i;
        }
		
		
      }
    }	
}


Unit* ExampleAIModule::building()
{
	for(std::list<BuildingJob>::iterator i = buildingJobList.begin(); i != buildingJobList.end(); i++)
	{
		(*i).first->build((*i).second.first, (*i).second.second);
		//Broodwar->printf("%s", (*i).first->getType().c_str());
		if((*i).first->getType() == (*i).second.second)
		{
			buildingJobList.remove((*i));
			return (*i).first;
		}
		
	}
	return NULL;
}

void ExampleAIModule::goOverloadGo()
{
	isScouting = true;

	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		if( (*i)->getType() == UnitTypes::Zerg_Overlord)
		{
			scoutOverload = (*i);
		}
	}

	std::list<TilePosition*> sl;
	TilePosition* currentTile = &(Broodwar->self()->getStartLocation());
	TilePosition* closestTile = NULL;
	
	// init sl
	for(std::set<TilePosition>::iterator i = Broodwar->getStartLocations().begin(); i != Broodwar->getStartLocations().end(); i++)
	{
		//시작 지점이면 넣으면 안된다
		if(*currentTile == (*i))
			continue;
		//이 부분 확신이 안든다.... iterator가 나올거 같기도 하고 포인터가 나올거 같기도 하고;;;
		//sl.push_back(i);
		sl.push_back(&(*i));

		if(closestTile == NULL)
			closestTile = &(*i); 
		else if(currentTile->getDistance(*closestTile) > currentTile->getDistance(*i))
			closestTile = &(*i);		
	}
	Position* tempPosition = new Position(*closestTile);
	scoutOverload->rightClick((*tempPosition));
	scoutRoute.push(tempPosition);
	

	//일단은 보내는데 도착한 뒤에 다음 시작 지점으로 보내기 위해 queue에 저장해야 함 
	while(sl.size() > 1)
	{
		sl.remove(closestTile);
		currentTile = closestTile;
		closestTile = NULL;

		for(std::list<TilePosition*>::iterator i = sl.begin(); i != sl.end(); i++)
		{
			
			if(closestTile == NULL)
				closestTile = *i; 
			else if(currentTile->getDistance(*closestTile) > currentTile->getDistance(**i))
				closestTile = *i;
		}
		//이 구조가 굉장히 마음에 안드는데 어떻게 바꾸나 ㅠㅠ 
		tempPosition = new Position(*closestTile);
		scoutRoute.push(tempPosition);
		
	}
	
	tempPosition = new Position(*sl.back());
	scoutRoute.push(tempPosition);
}

void ExampleAIModule::turnOverloadTurn()
{
	if(scoutOverload->getDistance(*scoutRoute.front()) < 250)
	{
		changingEnermyBase();
	}	
}

void ExampleAIModule::startBuildingSpawningPool()
{
	if(Broodwar->self()->minerals()	> 183)
	{
		//드론 하나 집어서 건물을 짓게 해야 함 으아 ㅠㅠ
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if((*i)->getType().isWorker() && !(*i)->isCarryingMinerals())
			//if((*i)->getType().isWorker())
			{
				// 가장 가까운 미네랄과 점대칭으로 지으면 될거 같다. 점은 해처리 ㅋ
				
				int x = baseInformation.baseHatchery->getTilePosition().x() - (baseInformation.closeMinerals[0]->getTilePosition().x() - baseInformation.baseHatchery->getTilePosition().x());
				int y = baseInformation.baseHatchery->getTilePosition().y() - (baseInformation.closeMinerals[0]->getTilePosition().y() - baseInformation.baseHatchery->getTilePosition().y());
				//Broodwar->printf("pool (%d, %d)  hat(%d, %d)  cm(%d, %d)", x, y,baseInformation.baseHatchery->getTilePosition().x(), baseInformation.baseHatchery->getTilePosition().y(), baseInformation.closeMinerals[0]->getTilePosition().x(), baseInformation.closeMinerals[0]->getTilePosition().y());
				TilePosition poolPosition(x, y);
				if((*i)->build(poolPosition, UnitTypes::Zerg_Spawning_Pool))
					break;
				BuildingInfo bi(poolPosition, UnitTypes::Zerg_Spawning_Pool);
				BuildingJob bj((*i), bi);
				buildingJobList.push_back(bj);
				buildState = five_BuildingSpawningPool;
				break;
			}
		}
		//일단은 인구 6까지 드론 만드는거 해야 함 
	}
}

// 처음 나온 저글링 6마리 어디로 달릴지 정하는거
void ExampleAIModule::comeOnLetsGo()
{
	// 저글링을 다룰 때 list 같은 곳에 넣어두고 하면 좋겠는데
	// 죽었을 때 null 에러가 날려나?
	Broodwar->printf("comeOnLetsGo");
	// 아직 정찰정이라 적 위치가 발견되지 않았을 때
	if(enermyBase == NULL)
	{
		zerglingControl.attackAll(scoutRoute.back());
	}
	else
	{
		zerglingControl.attackAll(enermyBase);
	}
}

void ExampleAIModule::comeOnLetsGo(Unit* u)
{
	if(enermyBase == NULL)
	{
		u->attack(*scoutRoute.front());
	}
	else
	{
		u->attack(*enermyBase);
	}
}

void ExampleAIModule::addZerglings()
{
	bool isAdded = false;
	//여러번 add 됨 ㄷㄷㄷ -_-
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)	
	{
		if((*i)->getType() == UnitTypes::Zerg_Zergling)
		{
			zerglingControl.add((*i));
			isAdded = true;			
		}
		
	}

	if(isAdded)
	{
		Broodwar->printf("isAdded: %d", zerglingControl.getSize());
		zerglingControl.unique();		
		isZerglingAdded = false;
	}
}

void ExampleAIModule::changingEnermyBase()
{
	scoutRoute.pop();			
	scoutOverload->rightClick(*scoutRoute.front());
	
}

void ExampleAIModule::attackAllZergling()
{
	std::set<Unit*> myUnits = Broodwar->self()->getUnits();	
	
	for(std::set<Unit*>::iterator i=myUnits.begin();i!=myUnits.end();i++)
	{
		if((*i)->getType() == UnitTypes::Zerg_Zergling)
		{			
			comeOnLetsGo((*i));			
		}
	}
}