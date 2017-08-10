#include "ActionZVTBarracksUnits.h"
#include "BuildingManager.h"
#include "ActionHelper.h"
#include "InformationManager.h"

using namespace CasiaBot;
using namespace CasiaBot::ActionHelper;
using namespace std;

ActionZVTBarracksUnits::ActionZVTBarracksUnits()
{
}

void ActionZVTBarracksUnits::init()
{
	if (!isInitialized)
	{
		lastFrameCount = BWAPI::Broodwar->getFrameCount();
		lastFrameMineralAmount = BWAPI::Broodwar->self()->minerals();
		lastFrameGasAmount = BWAPI::Broodwar->self()->gas();
		isInitialized = true;
	}
}

bool ActionZVTBarracksUnits::canDeployAction()
{
	if (enemyTerranMechanizationRate < 0.8 || enemyTerranFactoryUnitsAmount <= 8)
	{
		return true;
	}
	else
		return false;
}

bool ActionZVTBarracksUnits::tick()
{
	if (enemyTerranMechanizationRate >= 0.8 || enemyTerranFactoryUnitsAmount > 8)
	{
		return true;
	}
	else
		return false;
}

void ActionZVTBarracksUnits::getBuildOrderList(CasiaBot::ProductionQueue & queue)
{
	being_rushed = InformationManager::Instance().isEncounterRush();
	BWAPI::Broodwar->drawTextScreen(480, 260, being_rushed ? "rush" : "not rush");
	// ��ǰ֡�����ۼƣ�
	int gas = BWAPI::Broodwar->self()->gas();
	int minerals = BWAPI::Broodwar->self()->minerals();
	int currentFrameCount = BWAPI::Broodwar->getFrameCount();

	// ��rush�ŵر�
	int creep_need = std::min(4, std::max(2, enemy_marine_count / 6));
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

	// �ж�Ѫ���Ƿ����
	bool isSpawningPoolExist = spawning_pool_being_built + spawning_pool_count + spawning_pool_in_queue > 0;
	if (!isSpawningPoolExist)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	// �жϵ�һ�������Ƿ����
	bool isExtractorExist = extractor_being_built + extractor_count + extractor_in_queue > 0;
	if (!isExtractorExist && drone_count >= 10 && spawning_pool_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	}

	// �жϴ��߶��Ƿ����
	bool isHydraliskDenExist = hydralisk_den_being_built + hydralisk_den_count + hydralisk_den_in_queue > 0;
	if (!isHydraliskDenExist && gas >= 50 && lair_count > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den, "50%Lair"));
	}

	// �ж��Ƿ����
	if (lair_count + lair_being_built + lair_in_queue == 0	// û�ж���
		&& spawning_pool_completed > 0 && gas >= 100		// ��Դ��ǰ������
		&& (being_rushed ? (sunken_colony_count >= 2) : true)// ��rushҪ�ر�����
		&& currentFrameCount > 3600)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lair), true);	// ��������
	}

	// ���ֿ�
	if (BuildingManager::Instance().getReservedGas() > 400 && base_in_queue + base_being_built == 0)
	{
		BWAPI::Broodwar->printf("add a hatchery");
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
	}

	// ����
	if (lair_completed > 0)
	{
		if (queens_nest_count + queens_nest_being_built + queens_nest_in_queue == 0)
		{
			if (currentFrameCount > 15400)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
			}
		}
		if (queens_nest_completed > 0)
		{
			if (hive_count + hive_being_built + hive_in_queue == 0)
			{
				if (currentFrameCount > 17280)
				{
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hive));
				}
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

	// ������Ҫ��ũ������
	int numTotalPatch = 0;
	for (const auto & base : InformationManager::Instance().getSelfBases())
	{
		numTotalPatch += WorkerManager::Instance().getMineralPatches(base).size();
	}
	// ���ũ���㣬��Ҫ�������ȶ���
	bool notEnoughDrone = (drone_count + drone_in_queue) < (int)((float)numTotalPatch * 1.3);
	if (drone_count + drone_in_queue < numTotalPatch * Config::Macro::WorkersPerMineralPatch)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), notEnoughDrone);
	}

	// �ֿ������
	int extractorNeed = std::min(base_completed, 3);
	if (!notEnoughDrone
		&& extractor_count + extractor_being_built + extractor_in_queue < extractorNeed)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
		// ��һ��ũ��
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	}

	// �ж���Ҫ������ٲ���
	// С��
	int zerglingMax = being_rushed || currentFrameCount < 6400 ? 12 : 48;
	int zerglingNeed = (zerglingMax - zergling_count - zergling_in_queue) / 2;
	if (zerglingNeed > 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling));
	}

	// ����
	if (hydralisk_count + hydralisk_in_queue < 3)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
	}
	// lurker�������ȶ���
	if (hydralisk_completed > 0
		&& BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
		&& lurker_in_queue == 0)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lurker), true);
	}
}

void ActionZVTBarracksUnits::updateCurrentState(ProductionQueue &queue)
{
	ActionZergBase::updateCurrentState(queue);

	enemyTerranBarrackUnitsAmount = enemy_marine_count + enemy_firebat_count + enemy_ghost_count + enemy_medic_count;
	enemyTerranFactoryUnitsAmount = enemy_vulture_count * 2 + enemy_tank_count * 2 + enemy_goliath_count * 2;
	enemyTerranMechanizationRate = enemyTerranBarrackUnitsAmount == 0 ? 10 : (double)enemyTerranFactoryUnitsAmount / (double)enemyTerranBarrackUnitsAmount;
	if (enemyTerranFactoryUnitsAmount == 0) enemyTerranMechanizationRate = 0;
}

void ActionZVTBarracksUnits::tryAddInQueue(ProductionQueue & queue, const ProductionItem & item, bool priority)
{

}
