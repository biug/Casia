#include "ActionZVZZerglingLurker.h"
#include "BuildingManager.h"
#include "ActionHelper.h"
#include "InformationManager.h"

using namespace CasiaBot;
using namespace CasiaBot::ActionHelper;
using namespace std;

ActionZVZZerglingLurker::ActionZVZZerglingLurker()
{
}

void ActionZVZZerglingLurker::init()
{
	if (!isInitialized)
	{
		lastFrameCount = BWAPI::Broodwar->getFrameCount();
		lastFrameMineralAmount = BWAPI::Broodwar->self()->minerals();
		lastFrameGasAmount = BWAPI::Broodwar->self()->gas();
		isInitialized = true;
	}
}

bool ActionZVZZerglingLurker::canDeployAction()
{
	if (enemy_mutalisk_count <= 0 && enemy_hydralisk_count > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ActionZVZZerglingLurker::tick()
{
	if (enemy_hydralisk_count <= 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ActionZVZZerglingLurker::getBuildOrderList(CasiaBot::ProductionQueue & queue)
{
	// ��ǰ֡�����ۼƣ�
	int currentFrameCount = BWAPI::Broodwar->getFrameCount();

	if (currentFrameCount % 200 == 0)
	{
		int currentFrameMineralAmount = BWAPI::Broodwar->self()->minerals();
		int currentFrameGasAmount = BWAPI::Broodwar->self()->gas();
		int diffMineralAmount = currentFrameMineralAmount - lastFrameMineralAmount;
		int diffGasAmount = currentFrameGasAmount - lastFrameGasAmount;

		mineralNetIncrease.pop_front();
		mineralNetIncrease.push_back(diffMineralAmount);
		gasNetIncrease.pop_front();
		gasNetIncrease.push_back(diffGasAmount);
		lastFrameCount = currentFrameCount;
		lastFrameMineralAmount = currentFrameMineralAmount;
		lastFrameGasAmount = currentFrameGasAmount;
	}
	bool mineralDequePositive = IsDequeAllPositive(mineralNetIncrease);
	bool gasDequePositive = IsDequeNoneNegative(gasNetIncrease);

	// �ж�ǰ�Ὠ���Ƿ����
	bool isHiveExist = hive_being_built + hive_count + hive_in_queue > 0;
	bool isQueenNestExist = queens_nest_being_built + queens_nest_count + queens_nest_in_queue > 0;
	bool isLairExist = lair_being_built + lair_count + lair_in_queue > 0;
	bool isSpawningPoolExist = spawning_pool_being_built + spawning_pool_count + spawning_pool_in_queue > 0;
	if (!isSpawningPoolExist)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool), true);
	}

	bool isExtractorExist = extractor_being_built + extractor_count + extractor_in_queue > 0;
	if (!isExtractorExist && drone_count >= 12 && zergling_count > 8)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor), true);
	}

	bool isHydraliskDenExist = hydralisk_den_being_built + hydralisk_den_count + hydralisk_den_in_queue > 0;
	if (!isHydraliskDenExist)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk_Den));
	}

	//�Ƿ���Ҫ��������
	bool isSunkenColonyExist = sunken_colony_count + sunken_colony_being_built + sunken_colony_in_queue > 0;
	bool isCreepColonyExist = creep_colony_count + creep_colony_being_built + creep_colony_in_queue > 0;
	if (isCreepColonyExist)
	{
		if (sunken_colony_in_queue < creep_colony_completed)
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Sunken_Colony), true);
	}
	else if (!isCreepColonyExist && !isSunkenColonyExist) {
		if (zergling_completed > 0)
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Creep_Colony), true);
	}

	// �ж��Ƿ���Ҫ����ĸ��
	if (currentFrameCount % 200 == 0 && base_count + base_in_queue + base_being_built <= 5 && currentFrameCount > 10) {
		if (base_count + base_in_queue + base_being_built <= 1)
		{
			if (zergling_count >= 4)
			{
				ProductionItem item = MetaType(BWAPI::UnitTypes::Zerg_Hatchery);
				item.setNExpHatchery();
				queue.add(item);
			}
		}
		else if (base_count + base_in_queue + base_being_built <= 2)
		{
			if (zergling_count >= 8)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
			}
		}
		else if (base_count + base_in_queue + base_being_built <= 3)
		{
			if (mineralDequePositive)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
			}
		}
		else
		{
			if (isHiveExist && mineralDequePositive && gasDequePositive)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
			}
		}
	}



	bool notEnoughDrone = false;
	if (real_base_count == 1)
	{
		if (drone_count + drone_in_queue < 9)
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
		else if (zergling_count >= 6 && drone_count + drone_in_queue < 15)
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
		notEnoughDrone = drone_count + drone_in_queue < 12;
	}
	else
	{
		if (drone_count + drone_in_queue < real_base_count * 10)
		{
			queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone));
		}
		notEnoughDrone = drone_count + drone_in_queue < 8 * real_base_count;
	}

	// �ж���Ҫ������ٲ���
	int need_zergling_count = 0;
	if (isSpawningPoolExist)
	{
		//���ȸ��ݵз���λ�����ж�
		need_zergling_count = std::max(need_zergling_count, (int)(enemy_zergling_count * 1.5) - zergling_count - zergling_in_queue);
		if (need_zergling_count < 2) {
			//��֤����
			if (zergling_count + zergling_in_queue < 30)
				need_zergling_count = 2;
			//����Դ���������¼�������
			if (mineralDequePositive && isExtractorExist && gasDequePositive  && zergling_in_queue < 6)
				need_zergling_count = 2;
		}
		//���Ȳ�ũ��
		if (notEnoughDrone && zergling_count + zergling_in_queue >= 15)
		{
			need_zergling_count = 0;
		}

	}
	int need_lurker_count = 0;
	if (isHydraliskDenExist && isLairExist)
	{
		//���ȸ��ݵз���λ�����ж�
		need_lurker_count = std::max(need_lurker_count, need_zergling_count / 3);
		if (need_lurker_count < 1) {
			//��֤����
			if (lurker_count + lurker_in_queue < 6)
				need_lurker_count = 1;
			//����Դ���������¼�������
			if (isHiveExist && mineralDequePositive && isExtractorExist && gasDequePositive && lurker_in_queue < 2)
				need_lurker_count = 1;
		}
		//���Ȳ�ũ��
		if (notEnoughDrone && lurker_count + lurker_in_queue >= 3)
		{
			need_lurker_count = 0;
		}
	}

	// ���彨��Zergling��Lurker
	do
	{
		if (need_zergling_count > 0)
		{
			// 2��Zergling
			if (spawning_pool_count > 0)
			{
				if (currentFrameCount < 4800 && zergling_count + zergling_in_queue < 8)
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling), true);
				else
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Zergling));
			}
			need_zergling_count -= 2;
		}
		if (need_lurker_count > 0)
		{
			if (hydralisk_den_count > 0
				&& hydralisk_count + hydralisk_in_queue <= 5)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hydralisk));
			}
			if (BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
				&& lurker_count + lurker_in_queue < hydralisk_completed)
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lurker));
			}
			need_lurker_count--;
		}
		if (need_zergling_count <= 0 && need_lurker_count <= 0)
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
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Hive));
			}
		}
		else	// ���ʺ󳲲�����
		{
			if (isLairExist)	// ����Ѩ����
			{
				if (currentFrameCount > 9000)
				{
					queue.add(MetaType(BWAPI::UnitTypes::Zerg_Queens_Nest));
				}
			}
			else if (currentFrameCount > 4800)	// ����Ѩ������
			{
				queue.add(MetaType(BWAPI::UnitTypes::Zerg_Lair));
			}
		}
	}

	if (spawning_pool_count > 0 && queue.upgradeCount(BWAPI::UpgradeTypes::Metabolic_Boost) == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Metabolic_Boost));
	}
	if (hydralisk_den_count > 0 && lair_count > 0 && queue.techCount(BWAPI::TechTypes::Lurker_Aspect) == 0)
	{
		queue.add(MetaType(BWAPI::TechTypes::Lurker_Aspect));
	}
	if (hive_count > 0 && queue.upgradeCount(BWAPI::UpgradeTypes::Adrenal_Glands) == 0)
	{
		queue.add(MetaType(BWAPI::UpgradeTypes::Adrenal_Glands));
	}

	//������
	int extractorUpperBound = std::min(base_completed, 3);
	if (isExtractorExist && extractor_count + extractor_being_built + extractor_in_queue < extractorUpperBound)
	{
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	}
}

void ActionZVZZerglingLurker::updateCurrentState(ProductionQueue &queue)
{
	ActionZergBase::updateCurrentState(queue);
}