#include "ActionZVTBarracksUnits.h"
#include "BuildingManager.h"
#include "ActionHelper.h"
#include "InformationManager.h"

using namespace CasiaBot;
using namespace CasiaBot::ActionHelper;
using namespace std;

ActionZVTBarracksUnits::ActionZVTBarracksUnits()
	: thirdBase(true)
{
}

void ActionZVTBarracksUnits::init()
{
}

bool ActionZVTBarracksUnits::canDeployAction()
{
	int enemyTerranBarrackUnitsAmount =
		_status.enemy_marine_count + _status.enemy_firebat_count + _status.enemy_ghost_count + _status.enemy_medic_count;
	int enemyTerranFactoryUnitsAmount =
		_status.enemy_vulture_count * 2 + _status.enemy_tank_count * 2 + _status.enemy_goliath_count * 2;
	double enemyTerranMechanizationRate =
		enemyTerranBarrackUnitsAmount == 0 ?
		(enemyTerranFactoryUnitsAmount == 0 ? 0 :10) :
		(double)enemyTerranFactoryUnitsAmount / (double)enemyTerranBarrackUnitsAmount;
	return enemyTerranMechanizationRate < 0.8;
}

bool ActionZVTBarracksUnits::tick()
{
	return !canDeployAction();
}

void ActionZVTBarracksUnits::getBuildOrderList(CasiaBot::ProductionQueue & queue)
{
	int currentFrameCount = BWAPI::Broodwar->getFrameCount();
	int freeMineral =
		BWAPI::Broodwar->self()->minerals()
		- BuildingManager::Instance().getReservedMinerals();
	int freeGas =
		BWAPI::Broodwar->self()->gas()
		- BuildingManager::Instance().getReservedGas();
	bool being_rushed = _status.being_rushed;

	// ��rush�ŵر�
	int creep_need = std::min(3, std::max(2, (_status.enemy_marine_count + _status.enemy_firebat_count + _status.enemy_medic_count) / 6));
	if (_status.spawning_pool_count > 0	// ����Ѫ��
		&& _status.creep_colony_being_built + _status.creep_colony_in_queue == 0	// ������û�еر�
		&& _status.creep_colony_total + _status.sunken_colony_total < (being_rushed ? creep_need : 0))
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
	}

	if (_status.creep_colony_completed > 0 && _status.spawning_pool_completed > 0 &&
		_status.sunken_colony_being_built + _status.sunken_colony_in_queue < _status.creep_colony_completed)
		//�����creep�ͱ�ر�
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony), true);
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);	// ���ر�ҲҪ��ũ��
	}

	// �ж�Ѫ���Ƿ����
	if (_status.spawning_pool_total == 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	// �жϵ�һ�������Ƿ����
	if (_status.extractor_total == 0
		&& _status.drone_count >= 10
		&& _status.spawning_pool_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	}

	// �ж��Ƿ����
	if (_status.lair_total == 0	// û�ж���
		&& _status.spawning_pool_completed > 0 && freeGas > 50		// ��Դ��ǰ������
		&& (!being_rushed || _status.sunken_colony_count >= 2)		// ��rushҪ�ر�����
		&& _status.zergling_count >= 8
		&& currentFrameCount >= 5400)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lair), true);	// ��������
	}

	// �жϴ��߶��Ƿ����
	if (_status.hydralisk_den_total == 0
		&& freeGas >= 50 && _status.lair_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den, "50%Lair"));
	}

	// ��˫
	if (_status.lurker_count > 0
		&& freeMineral > 300
		&& _status.base_in_queue + _status.base_being_built == 0
		&& thirdBase)
	{
		// ��˫����
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery, "Main"), true);
		thirdBase = false;
	}

	// ���ֿ�
	if (freeMineral > 400
		&& _status.base_in_queue + _status.base_being_built == 0
		&& _status.base_count < 5
		&& _status.lurker_count > 0)
	{
		BWAPI::Broodwar->printf("add a hatchery");
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
	}

	// ����
	if (_status.lair_completed > 0)
	{
		if (_status.queens_nest_total == 0 && _status.spire_completed > 0)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
		}
		if (_status.queens_nest_completed > 0 && _status.hive_total == 0)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hive));
		}
	}

	// lurker�Ƽ�
	if (_status.hydralisk_den_completed > 0
		&& _status.lair_completed > 0
		&& _status.lurker_aspect_count == 0)
	{
		queue.add(MetaType(BWAPI::TechTypes::Lurker_Aspect), true);
	}
	// С���Ƽ�
	if (_status.spawning_pool_completed > 0 && _status.metabolic_boost_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Metabolic_Boost));
	}
	if (_status.hive_completed > 0 && _status.adrenal_glands_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Adrenal_Glands));
	}

	// ������Ҫ��ũ������
	int numTotalPatch = 0;
	for (const auto & base : InformationManager::Instance().getSelfBases())
	{
		numTotalPatch += WorkerManager::Instance().getMineralPatches(base).size();
	}
	// ���ũ���㣬��Ҫ�������ȶ���
	bool notEnoughDrone = (_status.drone_total) < (int)((float)numTotalPatch * 1.3);
	if (_status.drone_total < numTotalPatch * Config::Macro::WorkersPerMineralPatch)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), notEnoughDrone);
	}

	// �ֿ������
	int numMineralBases = WorkerManager::Instance().getMineralBases().size();
	int extractorNeed = std::min(numMineralBases, 3);
	if (!notEnoughDrone && _status.extractor_total < extractorNeed)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
		// ��һ��ũ��
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	}

	// �ж���Ҫ������ٲ���
	// С��
	int zerglingMax = being_rushed || currentFrameCount < 6400 ? 16 : 48;
	int zerglingNeed = (zerglingMax - _status.zergling_total) / 2;
	if (zerglingNeed > 0 && _status.zergling_in_queue <= 6)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling));
	}

	// ����
	if (_status.hydralisk_den_completed > 0 && _status.hydralisk_total < 3)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
	}
	// lurker�������ȶ���
	if (_status.hydralisk_completed > 0
		&& BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
		&& _status.lurker_in_queue == 0)
	{
		int hydra_reserved = _status.enemy_air_army_supply - _status.hydralisk_count;
		if (hydra_reserved <= 0)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lurker), true);
		}
	}
}
