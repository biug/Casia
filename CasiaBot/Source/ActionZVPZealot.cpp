#include "ActionZVPZealot.h"
#include "BuildingManager.h"
#include "ActionHelper.h"
#include "InformationManager.h"

using namespace CasiaBot;
using namespace CasiaBot::ActionHelper;
using namespace std;

ActionZVPZealot::ActionZVPZealot()
{
}

void ActionZVPZealot::init()
{
	if (!isInitialized)
	{
		lastFrameCount = BWAPI::Broodwar->getFrameCount();
		lastFrameMineralAmount = BWAPI::Broodwar->self()->minerals();
		lastFrameGasAmount = BWAPI::Broodwar->self()->gas();
		isInitialized = true;
	}
}

bool ActionZVPZealot::canDeployAction()
{
	if (enemyDragoonOverZealotRate <= 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ActionZVPZealot::tick()
{
	if (enemyDragoonOverZealotRate > 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ActionZVPZealot::getBuildOrderList(CasiaBot::ProductionQueue & queue)
{
	// ��ǰ֡�����ۼƣ�
	int currentFrameCount = BWAPI::Broodwar->getFrameCount();
	int freeMineral =
		BWAPI::Broodwar->self()->minerals()
		- BuildingManager::Instance().getReservedMinerals();
	int freeGas =
		BWAPI::Broodwar->self()->gas()
		- BuildingManager::Instance().getReservedGas();
	being_rushed = InformationManager::Instance().isEncounterRush();

	bool isSunkenColonyExist = sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue > 0;
	bool isCreepColonyExist = creep_colony_count + creep_colony_being_built + creep_colony_in_queue > 0;
	int sunken_colony_total = sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue;
	int creep_colony_total = creep_colony_count + creep_colony_being_built + creep_colony_in_queue;

	// ÿ10�������һ��
	if ((currentFrameCount + 150) % 150 == 0)
	{
		if (spawning_pool_completed == 0 && zergling_in_queue > 0)
		{
			queue.clear();
		}
		else if (hatchery_completed == 0 && lair_in_queue > 0)
		{
			queue.clear();
		}
		else if (lair_completed == 0 && hive_in_queue > 0)
		{
			queue.clear();
		}
		else if (hydralisk_den_completed == 0 && hydralisk_in_queue > 0)
		{
			queue.clear();
		}
		else if (lurker_in_queue > hydralisk_completed)
		{
			queue.clear();
		}
		else if (creep_colony_completed == 0 && sunken_colony_in_queue > 0)
		{
			queue.clear();
		}
		else if (spire_completed == 0 && mutalisk_in_queue > 0)
		{
			queue.clear();
		}
		if (zergling_count + zergling_in_queue > zerglingLimit)
		{
			queue.clear();
		}
		// ũ�����ʱ
		if (drone_count < 3)
		{
			queue.clear();
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
		}
	}

	// ��rush�ŵر�
	int creep_need = std::min(5, std::max(2, (int)(enemy_zealot_count / 1.6)));
	if (spawning_pool_count > 0	// ����Ѫ��
		&& creep_colony_being_built + creep_colony_in_queue == 0	// ������û�еر�
		&& creep_colony_count + creep_colony_being_built + creep_colony_in_queue +
		sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue < (being_rushed ? creep_need : 0))
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
	}

	if (creep_colony_completed > 0 && spawning_pool_completed > 0 &&
		sunken_colony_being_built + sunken_colony_in_queue < creep_colony_completed)	//�����creep�ͱ�ر�
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony), true);
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);	// ���ر�ҲҪ��ũ��
	}

	// �ж�ǰ�Ὠ���Ƿ����
	if (spawning_pool_count + spawning_pool_in_queue + spawning_pool_being_built == 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	if (extractor_count + extractor_in_queue + extractor_being_built == 0
		&& drone_count >= 10 && spawning_pool_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	}

	// �ж��Ƿ����
	if (lair_count + lair_being_built + lair_in_queue == 0	// û�ж���
		&& spawning_pool_completed > 0 && freeGas > 50		// ��Դ��ǰ������
		&& (!being_rushed || sunken_colony_count >= 2)		// ��rushҪ�ر�����
		&& zergling_count >= 8
		&& currentFrameCount >= 5400)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lair), true);	// ��������
	}

	if (hydralisk_den_count + hydralisk_den_in_queue + hydralisk_den_being_built == 0
		&& freeGas >= 50 && lair_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den, "50%Lair"));
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	}

	// �������������ڣ����ҳ���lurker
	if (spire_count + spire_in_queue + spire_being_built == 0
		&& (lair_completed > 0 || hive_completed > 0)
		&& lurker_completed > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spire));
	}

	// ���ֿ�
	if (freeMineral > 400
		&& base_in_queue + base_being_built == 0
		&& base_count < 5
		&& lurker_count > 0
		&& !being_rushed)
	{
		BWAPI::Broodwar->printf("add a hatchery");
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
	}

	// ����
	if (lair_completed > 0)
	{
		if (queens_nest_count + queens_nest_being_built + queens_nest_in_queue == 0
			&& spire_completed > 0)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
		}
		if (queens_nest_completed > 0)
		{
			if (hive_count + hive_being_built + hive_in_queue == 0)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hive));
			}
		}
	}

	// lurker�Ƽ�
	if (hydralisk_den_completed > 0 && lair_completed > 0 && lurker_aspect_count == 0)
	{
		queue.add(MetaType(BWAPI::TechTypes::Lurker_Aspect), true);
	}
	// С���Ƽ�
	if (spawning_pool_completed > 0 && metabolic_boost_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Metabolic_Boost));
	}
	if (hive_completed > 0 && adrenal_glands_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Adrenal_Glands));
	}
	// �����Ƽ�
	if (spire_completed > 0 && mutalisk_count > 0 && flyer_carapace_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace));
	}
	// ���߿Ƽ�
	if (hydralisk_den_completed > 0 && hydralisk_completed > 0 && muscular_arguments_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Muscular_Augments));
	}
	if (hydralisk_den_completed > 0 && hydralisk_completed > 0 && grooved_spines_count == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Grooved_Spines));
	}

	// ������Ҫ��ũ������
	int numTotalPatch = 0;
	int minNumWorkers = 0;
	for (const auto & base : InformationManager::Instance().getSelfBases())
	{
		int numWorkers = WorkerManager::Instance().getMineralPatches(base).size();
		numTotalPatch += numWorkers;
		minNumWorkers += numWorkers + 3;
	}
	// ���ũ���㣬��Ҫ�������ȶ���
	bool notEnoughDrone = (drone_count + drone_in_queue) < minNumWorkers;
	if (drone_count + drone_in_queue < numTotalPatch * Config::Macro::WorkersPerMineralPatch)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), notEnoughDrone);
	}

	// �ֿ������
	int numMineralBases = WorkerManager::Instance().getMineralBases().size();
	int extractorNeed = std::min(numMineralBases, 3);
	if (!notEnoughDrone
		&& extractor_count + extractor_being_built + extractor_in_queue < extractorNeed)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
		// ��һ��ũ��
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	}

	// �ж���Ҫ������ٲ���
	// С��
	int zerglingMax = being_rushed || currentFrameCount < 6400 ? 16 : 48;
	int zerglingNeed = (zerglingMax - zergling_count - zergling_in_queue) / 2;
	if (zerglingNeed > 0 && zergling_in_queue < 6)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling));
	}

	// ����
	if (hydralisk_den_completed > 0
		&& hydralisk_in_queue < 3
		&& hydralisk_count + hydralisk_in_queue < enemy_air_army_supply + 3)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
	}
	// lurker�������ȶ���
	if (hydralisk_completed > 0
		&& BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
		&& lurker_in_queue == 0)
	{
		int hydra_reserved = enemy_air_army_supply - hydralisk_count;
		if (hydra_reserved <= 0)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lurker), true);
		}
	}
	// ����
	if (spire_completed > 0 && lurker_completed > 0
		&& mutalisk_count + mutalisk_in_queue < 12)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Mutalisk), true);
	}
}

void ActionZVPZealot::updateCurrentState(ProductionQueue &queue)
{
	ActionZergBase::updateCurrentState(queue);

	enemyDragoonOverZealotRate = enemy_zealot_count == 0 ? 10 : (double)enemy_dragoon_count / (double)enemy_zealot_count;
	if (enemy_dragoon_count == 0) enemyDragoonOverZealotRate = 0;
	droneLimit = real_base_count == 1 ? 16 : real_base_count * 10;

	zerglingLimit = lurker_completed * 3;
	if (zerglingLimit < 20)
	{
		zerglingLimit = 20;
	}
	else if (zerglingLimit > 45)
	{
		zerglingLimit = 45;
	}

	enemyAirForceCount = enemy_scout_count + enemy_corsair_count + enemy_carrier_count + enemy_arbiter_count;
}

void ActionZVPZealot::tryAddInQueue(ProductionQueue & queue, const ProductionItem & item, bool priority)
{
}
