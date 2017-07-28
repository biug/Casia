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
	if (enemyDragoonOverZealotRate <= 1 && enemy_dragoon_count <= 5)
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
	if (enemyDragoonOverZealotRate > 1 || enemy_dragoon_count > 5)
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
	int gas = BWAPI::Broodwar->self()->gas();
	int minerals = BWAPI::Broodwar->self()->minerals();

	if (currentFrameCount % 200 == 0)
	{

		int diffMineralAmount = minerals - lastFrameMineralAmount;
		int diffGasAmount = gas - lastFrameGasAmount;
		mineralNetIncrease.pop_front();
		mineralNetIncrease.push_back(diffMineralAmount);
		gasNetIncrease.pop_front();
		gasNetIncrease.push_back(diffGasAmount);
		lastFrameCount = currentFrameCount;
		lastFrameMineralAmount = minerals;
		lastFrameGasAmount = gas;
	}
	bool mineralDequePositive = IsDequeAllPositive(mineralNetIncrease);
	bool gasDequePositive = IsDequeNoneNegative(gasNetIncrease);

	bool isSunkenColonyExist = sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue > 0;
	bool isCreepColonyExist = creep_colony_count + creep_colony_being_built + creep_colony_in_queue > 0;
	int sunken_colony_total = sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue;
	int creep_colony_total = creep_colony_count + creep_colony_being_built + creep_colony_in_queue;
	bool isHatcheryExist = hatchery_being_built + hatchery_count + hatchery_in_queue > 0;
	bool isHiveExist = hive_being_built + hive_count + hive_in_queue > 0;
	bool isQueenNestExist = queens_nest_being_built + queens_nest_count + queens_nest_in_queue > 0;
	bool isLairExist = lair_being_built + lair_count + lair_in_queue > 0;
	bool isSpireExist = spire_count + spire_in_queue + spire_being_built > 0;
	bool isSpawningPoolExist = spawning_pool_being_built + spawning_pool_count + spawning_pool_in_queue > 0;
	bool isExtractorExist = extractor_being_built + extractor_count + extractor_in_queue > 0;

	// ÿ200֡������һ��
	if ((currentFrameCount + 100) % 200 == 0)
	{
		if (!isSpawningPoolExist && zergling_in_queue > 0)
		{
			queue.clear();
		}
		else if (!isHatcheryExist && lair_in_queue > 0)
		{
			queue.clear();
		}
		else if (!isLairExist && hive_in_queue > 0)
		{
			queue.clear();
		}
		else if (!isCreepColonyExist && sunken_colony_in_queue > 0)
		{
			queue.clear();
		}
		else if (!isSpireExist && mutalisk_in_queue > 0)
		{
			queue.clear();
		}
		else if (!isSpireExist && greater_spire_in_queue > 0)
		{
			queue.clear();
		}
	}

	//�Ƿ���Ҫ��������
	if (isCreepColonyExist)
	{
		if (sunken_colony_in_queue < creep_colony_completed)
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony), true);
		if (creep_colony_total + sunken_colony_total < 2)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone));
		}
		else if (creep_colony_total + sunken_colony_total < 5)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony));
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone));
		}
	}
	else if (!isCreepColonyExist && !isSunkenColonyExist) {
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone));
	}

	// �ж�ǰ�Ὠ���Ƿ����
	if (!isSpawningPoolExist)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	if (!isExtractorExist && drone_count >= 10 && spawning_pool_count > 0)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Extractor), true);
	}

	if (!isSpireExist && (isLairExist || isHiveExist) && currentFrameCount > 6000)	// ��������������
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Spire));
	}

	// �ж��Ƿ���Ҫ����ĸ��
	if (currentFrameCount % 200 == 0 && base_in_queue < 1 && base_count + base_in_queue + base_being_built <= 4)
	{
		if (base_count + base_in_queue + base_being_built <= 1)
		{
			if (zergling_count >= 4)
			{
				ProductionItem item = MetaType(BWAPI::UnitTypes::Zerg_Hatchery, "Main");
				tryAddInQueue(queue, item);
			}
		}
		else if (base_count + base_in_queue + base_being_built <= 2)
		{
			if ((lair_completed > 0 || hive_completed > 0) && zergling_count >= 8 && minerals > 400)
			{
				if (real_base_count == base_count + base_in_queue + base_being_built)
				{
					ProductionItem item = MetaType(BWAPI::UnitTypes::Zerg_Hatchery, "Main");
					tryAddInQueue(queue, item);
				}
				else
				{
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
				}
			}
		}
		else if (base_count + base_in_queue + base_being_built <= 3)
		{
			if (mineralDequePositive && minerals > 500)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
			}
		}
		else
		{
			if (isHiveExist && mineralDequePositive && gasDequePositive)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
			}
		}
	}

	bool notEnoughDrone = false;
	if (real_base_count == 1)
	{
		if (drone_count + drone_in_queue < 9)
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone));
		else if (zergling_count >= 6 && drone_count + drone_in_queue < 15)
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone));
		notEnoughDrone = drone_count + drone_in_queue < 12;
	}
	else
	{
		if (drone_count + drone_in_queue < real_base_count * 10)
		{
			tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Drone));
		}
		notEnoughDrone = drone_count + drone_in_queue < 8 * real_base_count;
	}

	// �ж���Ҫ������ٲ���
	int need_zergling_count = 0;
	if (isSpawningPoolExist)
	{
		//���ȸ��ݵз���λ�����ж�
		need_zergling_count = std::max(need_zergling_count, enemy_zealot_count * 4 + enemy_dragoon_count * 3 - zergling_count - zergling_in_queue);
		if (need_zergling_count < 2) {
			//��֤����
			if (zergling_count + zergling_in_queue < 20)
				need_zergling_count = 2;
			//����Դ���������¼�������
			if (mineralDequePositive && isExtractorExist && gasDequePositive && zergling_in_queue < 6)
				need_zergling_count = 2;
		}
		else if (need_zergling_count > 4)
		{
			need_zergling_count = 4;
		}
		// С��̫��ʱֹͣ��С��
		if (zergling_count + zergling_in_queue > zerglingLimit)
		{
			need_zergling_count = 0;
		}

		//���Ȳ�ũ��
		if (notEnoughDrone && zergling_count + zergling_in_queue >= 15)
		{
			need_zergling_count = 0;
		}
	}
	int need_mutalisk_count = 0;
	if (isSpireExist)
	{
		//���ȸ��ݵз���λ�����ж�
		need_mutalisk_count = std::max(need_mutalisk_count, need_zergling_count / 3);
		if (need_mutalisk_count < 1) {
			//��֤����
			if (mutalisk_count + mutalisk_in_queue < 10)
				need_mutalisk_count = 1;
			//����Դ���������¼�������
			if (mineralDequePositive && isExtractorExist && gasDequePositive && mutalisk_in_queue < 2)
				need_mutalisk_count = 1;
		}
		//���Ȳ�ũ��
		if (notEnoughDrone && mutalisk_count + mutalisk_in_queue >= 3)
		{
			need_mutalisk_count = 0;
		}
	}

	// ���彨��Zergling��Mutalisk
	do
	{
		if (need_zergling_count > 0)
		{
			// 2��Zergling
			if (spawning_pool_count > 0)
			{
				if (currentFrameCount < 4800 && zergling_count + zergling_in_queue < 8)
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Zergling), true);
				else
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Zergling));
			}
			need_zergling_count -= 2;
		}
		if (need_mutalisk_count > 0)
		{
			if (spire_count + greater_spire_count > 0)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Mutalisk));
			}
			need_mutalisk_count--;
		}
		if (need_zergling_count <= 0 && need_mutalisk_count <= 0)
		{
			break;
		}

	} while (true);

	if (!isHiveExist)	// ���䳲������
	{
		if (isQueenNestExist)	// ���ʺ󳲴���
		{
			if (currentFrameCount > 10800)
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Hive));
			}
		}
		else	// ���ʺ󳲲�����
		{
			if (isLairExist)	// ����Ѩ����
			{
				if (currentFrameCount > 9000)
				{
					tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
				}
			}
			else if (currentFrameCount > 4800)	// ����Ѩ������
			{
				tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Lair));
			}
		}
	}

	if (spawning_pool_completed > 0 && metabolic_boost_count == 0)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Metabolic_Boost));
	}
	if (hive_completed > 0 && adrenal_glands_count == 0)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UpgradeTypes::Adrenal_Glands));
	}

	//������
	int extractorUpperBound = std::min(base_completed, 3);
	if (isExtractorExist && extractor_count + extractor_being_built + extractor_in_queue < extractorUpperBound)
	{
		tryAddInQueue(queue, MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	}
}

void ActionZVPZealot::updateCurrentState(ProductionQueue &queue)
{
	ActionZergBase::updateCurrentState(queue);

	enemyDragoonOverZealotRate = enemy_zealot_count == 0 ? 10 : (double)enemy_dragoon_count / (double)enemy_zealot_count;
	if (enemy_dragoon_count == 0) enemyDragoonOverZealotRate = 0;
}

void ActionZVPZealot::tryAddInQueue(ProductionQueue & queue, const ProductionItem & item, bool priority)
{
	const MetaType & unit = item._unit;
	auto unitType = unit.getUnitType();
	auto upgradeType = unit.getUpgradeType();
	auto techType = unit.getTechType();

	if (unitType == BWAPI::UnitTypes::Zerg_Drone)
	{
		if (drone_count + drone_in_queue < droneLimit)
		{
			queue.add(item, priority);
			drone_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Zergling)
	{
		if (spawning_pool_count > 0 && zergling_count + zergling_in_queue < zerglingLimit)
		{
			queue.add(item, priority);
			zergling_in_queue += 2;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Mutalisk)
	{
		if (mutalisk_count + mutalisk_in_queue < mutaliskLimit)
		{
			queue.add(item, priority);
			mutalisk_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Creep_Colony)
	{
		if (creep_colony_count + creep_colony_being_built + creep_colony_in_queue < creepColonyLimit)
		{
			queue.add(item, priority);
			creep_colony_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Sunken_Colony)
	{
		if (sunken_colony_in_queue < creep_colony_completed
			&& sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue < sunkenColonyLimit)
		{
			queue.add(item, priority);
			sunken_colony_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Spawning_Pool)
	{
		if (spawning_pool_being_built + spawning_pool_count + spawning_pool_in_queue < spawningPoolLimit)
		{
			queue.add(item, priority);
			spawning_pool_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Extractor)
	{
		if (extractor_being_built + extractor_count + extractor_in_queue < extractorLimit)
		{
			queue.add(item, priority);
			extractor_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Spire)
	{
		if (spire_count + spire_in_queue + spire_being_built < spireLimit)
		{
			queue.add(item, priority);
			spire_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Hatchery)
	{
		if (hatchery_being_built + hatchery_count + hatchery_in_queue < hatcheryLimit)
		{
			queue.add(item, priority);
			hatchery_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Lair)
	{
		if (lair_being_built + lair_count + lair_in_queue < lairLimit
			&& hive_being_built + hive_count + hive_in_queue < hiveLimit)
		{
			queue.add(item, priority);
			lair_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Hive)
	{
		if (hive_being_built + hive_count + hive_in_queue < hiveLimit)
		{
			queue.add(item, priority);
			hive_in_queue++;
		}
	}
	else if (unitType == BWAPI::UnitTypes::Zerg_Queens_Nest)
	{
		if (queens_nest_being_built + queens_nest_count + queens_nest_in_queue < queenNestLimit)
		{
			queue.add(item, priority);
			queens_nest_in_queue++;
		}
	}
	else if (upgradeType == BWAPI::UpgradeTypes::Metabolic_Boost)
	{
		if (spawning_pool_completed > 0 && metabolic_boost_count < metabolicBoostLimit)
		{
			queue.add(item, priority);
			metabolic_boost_count++;
		}
	}
	else if (upgradeType == BWAPI::UpgradeTypes::Adrenal_Glands)
	{
		if (hive_completed > 0 && adrenal_glands_count < adrenalGlandsLimit)
		{
			queue.add(item, priority);
			adrenal_glands_count++;
		}
	}
	else
	{
		queue.add(item, priority);
	}
}
