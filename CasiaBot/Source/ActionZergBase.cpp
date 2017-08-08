#include "ActionZergBase.h"
#include "BuildingManager.h"

using namespace CasiaBot;

ActionZergBase::ActionZergBase()
{
	mineralNetIncrease = { 0,0,0,0,0 };
	gasNetIncrease = { 0,0,0,0,0 };
}


ActionZergBase::~ActionZergBase()
{
}

void ActionZergBase::updateCurrentState(ProductionQueue &queue)
{
	//我方
	larva_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Larva, BWAPI::Broodwar->self());
	drone_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Drone, BWAPI::Broodwar->self());
	zergling_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Zergling, BWAPI::Broodwar->self());
	hydralisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hydralisk, BWAPI::Broodwar->self());
	lurker_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Lurker, BWAPI::Broodwar->self());
	ultralisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Ultralisk, BWAPI::Broodwar->self());
	defiler_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Defiler, BWAPI::Broodwar->self());
	overlord_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Overlord, BWAPI::Broodwar->self());
	mutalisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Mutalisk, BWAPI::Broodwar->self());
	scourge_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Scourge, BWAPI::Broodwar->self());
	queen_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Queen, BWAPI::Broodwar->self());
	guardian_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Guardian, BWAPI::Broodwar->self());
	devourer_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Devourer, BWAPI::Broodwar->self());

	zergling_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Zergling, BWAPI::Broodwar->self());
	hydralisk_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Hydralisk, BWAPI::Broodwar->self());
	lurker_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Lurker, BWAPI::Broodwar->self());
	mutalisk_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Mutalisk, BWAPI::Broodwar->self());


	larva_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Larva);		//幼虫
	drone_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Drone);		//工蜂
	zergling_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Zergling) * 2;	//小狗
	hydralisk_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Hydralisk);//刺蛇
	lurker_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Lurker);		//地刺
	ultralisk_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Ultralisk);//雷兽
	defiler_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Defiler);	//蝎子
	overlord_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Overlord);	//领主
	mutalisk_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Mutalisk);	//飞龙
	scourge_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Scourge) * 2;	//自爆蚊
	queen_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Queen);		//女王
	guardian_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Guardian);	//守卫者
	devourer_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Devourer);	//吞噬者

	metabolic_boost_count = queue.upgradeCount(BWAPI::UpgradeTypes::Metabolic_Boost)
		+ BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Metabolic_Boost)
		+ (BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Metabolic_Boost) ? 1 : 0);
	lurker_aspect_count = queue.techCount(BWAPI::TechTypes::Lurker_Aspect)
		+ BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
		+ (BWAPI::Broodwar->self()->isResearching(BWAPI::TechTypes::Lurker_Aspect) ? 1 : 0);
	adrenal_glands_count = queue.upgradeCount(BWAPI::UpgradeTypes::Adrenal_Glands)
		+ BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Adrenal_Glands)
		+ (BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Adrenal_Glands) ? 1 : 0);

	//建筑
	hatchery_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hatchery, BWAPI::Broodwar->self());
	lair_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Lair, BWAPI::Broodwar->self());
	hive_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hive, BWAPI::Broodwar->self());
	base_count = hatchery_count + lair_count + hive_count;
	extractor_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Extractor, BWAPI::Broodwar->self());
	creep_colony_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Creep_Colony, BWAPI::Broodwar->self());
	sunken_colony_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Sunken_Colony, BWAPI::Broodwar->self());
	spore_colony_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Spore_Colony, BWAPI::Broodwar->self());
	spawning_pool_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Spawning_Pool, BWAPI::Broodwar->self());
	hydralisk_den_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hydralisk_Den, BWAPI::Broodwar->self());
	queens_nest_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Queens_Nest, BWAPI::Broodwar->self());
	defiler_mound_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Defiler_Mound, BWAPI::Broodwar->self());
	spire_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Spire, BWAPI::Broodwar->self());
	greater_spire_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Greater_Spire, BWAPI::Broodwar->self());
	nydus_canal_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Nydus_Canal, BWAPI::Broodwar->self());
	ultralisk_cavern_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Ultralisk_Cavern, BWAPI::Broodwar->self());

	hatchery_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Hatchery, BWAPI::Broodwar->self());
	lair_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Lair, BWAPI::Broodwar->self());
	hive_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Hive, BWAPI::Broodwar->self());
	base_completed = hatchery_completed + lair_completed + hive_completed;
	spire_complete = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Spire, BWAPI::Broodwar->self());
	creep_colony_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Creep_Colony, BWAPI::Broodwar->self());
	extractor_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Extractor, BWAPI::Broodwar->self());
	spawning_pool_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Spawning_Pool, BWAPI::Broodwar->self());
	hydralisk_den_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Hydralisk_Den, BWAPI::Broodwar->self());
	spire_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Spire, BWAPI::Broodwar->self());

	hatchery_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Hatchery);
	lair_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Lair);
	hive_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Hive);
	base_in_queue = hatchery_in_queue + lair_in_queue + hive_in_queue;
	extractor_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Extractor);
	creep_colony_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Creep_Colony);
	sunken_colony_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Sunken_Colony);	
	spore_colony_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Spore_Colony);
	spawning_pool_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Spawning_Pool);
	hydralisk_den_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Hydralisk_Den);
	queens_nest_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Queens_Nest);
	defiler_mound_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Defiler_Mound);
	spire_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Spire);
	greater_spire_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Greater_Spire);
	nydus_canal_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Nydus_Canal);
	ultralisk_cavern_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Ultralisk_Cavern);

	hatchery_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Hatchery);
	lair_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Lair);
	hive_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Hive);
	base_being_built = hatchery_being_built + lair_being_built + hive_being_built;
	extractor_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Extractor);
	creep_colony_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Creep_Colony);
	sunken_colony_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Sunken_Colony);
	spore_colony_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Spore_Colony);
	spawning_pool_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Spawning_Pool);
	hydralisk_den_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Hydralisk_Den);
	queens_nest_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Queens_Nest);
	defiler_mound_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Defiler_Mound);
	spire_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Spire);
	greater_spire_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Greater_Spire);
	nydus_canal_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Nydus_Canal);
	ultralisk_cavern_being_built = BuildingManager::Instance().numBeingBuilt(BWAPI::UnitTypes::Zerg_Ultralisk_Cavern);
	
	
	//军事力量
	army_supply = 0.0;
	air_army_supply = 0.0;
	ground_army_supply = 0.0;

	//判断和倾向
	//todo
	opponent_has_expanded = false;
	can_expand = false;
	force_expand = false;
	being_rushed = false;
	is_attacking = false;
	is_defending = false;
	default_upgrade = false;

	//敌方
	enemy_terran_unit_count = 0;
	enemy_protos_unit_count = 0;
	enemy_zerg_unit_count = 0;

	//人族单位
	enemy_marine_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Marine, BWAPI::Broodwar->enemy());						//机枪兵
	enemy_firebat_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Firebat, BWAPI::Broodwar->enemy());					//火焰兵
	enemy_medic_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Medic, BWAPI::Broodwar->enemy());						//医疗兵
	enemy_ghost_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Ghost, BWAPI::Broodwar->enemy());						//幽灵
	enemy_vulture_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Vulture, BWAPI::Broodwar->enemy());					//雷车
	enemy_tank_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode, BWAPI::Broodwar->enemy())
		+ InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, BWAPI::Broodwar->enemy());						//坦克
	enemy_goliath_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Goliath, BWAPI::Broodwar->enemy());					//机器人
	enemy_wraith_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Wraith, BWAPI::Broodwar->enemy());						//隐飞
	enemy_valkyrie_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Valkyrie, BWAPI::Broodwar->enemy());					//导弹护卫舰
	enemy_bc_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Battlecruiser, BWAPI::Broodwar->enemy());					//战列巡洋舰
	enemy_science_vessel_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Science_Vessel, BWAPI::Broodwar->enemy());		//科学球
	enemy_dropship_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Dropship, BWAPI::Broodwar->enemy());					//运输机

	//人族建筑
	enemy_bunker_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Bunker, BWAPI::Broodwar->enemy());						//地堡
	enemy_barrack_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Barracks, BWAPI::Broodwar->enemy());					//兵营
	enemy_factory_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Factory, BWAPI::Broodwar->enemy());					//工厂
	enemy_starport_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Starport, BWAPI::Broodwar->enemy());					//飞机场

	//神族单位
	enemy_zealot_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Zealot, BWAPI::Broodwar->enemy());					//狂热者
	enemy_dragoon_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Dragoon, BWAPI::Broodwar->enemy());					//龙骑
	enemy_ht_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_High_Templar, BWAPI::Broodwar->enemy());					//光明圣堂
	enemy_dt_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Dark_Templar, BWAPI::Broodwar->enemy());					//黑暗圣堂
	enemy_reaver_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Reaver, BWAPI::Broodwar->enemy());					//金甲虫
	enemy_shuttle_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Shuttle, BWAPI::Broodwar->enemy());					//运输机
	enemy_carrier_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Carrier, BWAPI::Broodwar->enemy());					//航母
	enemy_arbiter_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Arbiter, BWAPI::Broodwar->enemy());					//仲裁者
	enemy_corsair_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Corsair, BWAPI::Broodwar->enemy());					//海盗船
	enemy_scout_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Scout, BWAPI::Broodwar->enemy());						//侦察机

	//神族建筑
	enemy_cannon_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Photon_Cannon, BWAPI::Broodwar->enemy());				//光子炮塔
	enemy_gateway_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->enemy());					//兵营
	enemy_stargate_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Stargate, BWAPI::Broodwar->enemy());				//星门
	enemy_robotics_facility_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Robotics_Facility, BWAPI::Broodwar->enemy());	//机械工厂

	//虫族单位
	enemy_zergling_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Zergling, BWAPI::Broodwar->enemy());				//小狗
	enemy_hydralisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hydralisk, BWAPI::Broodwar->enemy());				//刺蛇
	enemy_lurker_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Lurker, BWAPI::Broodwar->enemy());					//地刺
	enemy_ultralisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Ultralisk, BWAPI::Broodwar->enemy());				//雷兽
	enemy_defiler_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Defiler, BWAPI::Broodwar->enemy());				//蝎子
	enemy_mutalisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Mutalisk, BWAPI::Broodwar->enemy());				//飞龙
	enemy_queen_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Queen, BWAPI::Broodwar->enemy());					//女王

	//虫族建筑
	enemy_spawning_pool_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Spawning_Pool, BWAPI::Broodwar->enemy());			//孵化池
	enemy_hydralisk_den_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hydralisk_Den, BWAPI::Broodwar->enemy());			//刺蛇穴
	enemy_evolution_chamber_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Evolution_Chamber, BWAPI::Broodwar->enemy());		//进化腔
	enemy_spire_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Spire, BWAPI::Broodwar->enemy());					//飞龙塔

	//军事力量
	enemy_army_supply = 0.0;
	enemy_air_army_supply = 0.0;
	enemy_ground_army_supply = 0.0;
	enemy_ground_large_army_supply = 0.0;
	enemy_ground_small_army_supply = 0.0;
	enemy_anti_air_army_supply = 0.0;
	enemy_biological_army_supply = 0.0;
	enemy_static_defence_count = 0;
	enemy_proxy_building_count = 0;
	enemy_attacking_army_supply = 0.0;
	enemy_attacking_worker_count = 0;
	enemy_cloaked_unit_count = 0;

	//通用建筑和单位
	enemy_worker_count = 0;
	enemy_gas_count = 0;

	//我方
	resDepots.clear();
	std::set<BWTA::BaseLocation *> base_set;

	for (auto &unit : BWAPI::Broodwar->self()->getUnits()) {
		if (!unit->getType().isWorker() && !unit->getType().isBuilding()) {
			if (unit->getType().isFlyer()) {
				air_army_supply += unit->getType().supplyRequired();
			}
			else {
				ground_army_supply += unit->getType().supplyRequired();
			}
			army_supply += unit->getType().supplyRequired();
		}
		
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Hatchery
			|| unit->getType() == BWAPI::UnitTypes::Zerg_Lair
			|| unit->getType() == BWAPI::UnitTypes::Zerg_Hive) {

			if (unit->isCompleted())
				resDepots.insert(unit);

			BWTA::BaseLocation *nearest;
			double dis = -1.0, curl;
			for (auto &base : BWTA::getBaseLocations()) {
				curl = (unit->getPosition() - base->getPosition()).getLength();
				if (dis < 0 || curl < dis) {
					dis = curl;
					nearest = base;
				}
			}
			base_set.insert(nearest);
		}
	}
	real_base_count = base_set.size();

	//敌方
	for (auto &unit : BWAPI::Broodwar->enemy()->getUnits()) {
		if (unit->getType().getRace() == BWAPI::Races::Terran)
			++enemy_terran_unit_count;
		if (unit->getType().getRace() == BWAPI::Races::Protoss)
			++enemy_protos_unit_count;
		if (unit->getType().getRace() == BWAPI::Races::Zerg)
			++enemy_zerg_unit_count;

		//军事力量
		if (!unit->getType().isWorker() && !unit->getType().isBuilding()) {
			if (unit->getType().isFlyer()) {
				enemy_air_army_supply += unit->getType().supplyRequired();
			}
			else {
				enemy_ground_army_supply += unit->getType().supplyRequired();
				if (unit->getType().size() == BWAPI::UnitSizeTypes::Large)
					enemy_ground_large_army_supply += unit->getType().supplyRequired();
				if (unit->getType().size() == BWAPI::UnitSizeTypes::Small)
					enemy_ground_small_army_supply += unit->getType().supplyRequired();
			}
			enemy_army_supply += unit->getType().supplyRequired();
			if (unit->getType().airWeapon())
				enemy_anti_air_army_supply += unit->getType().supplyRequired();
			if (unit->getType().isOrganic())
				enemy_biological_army_supply += unit->getType().supplyRequired();

		}
		if (unit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret)
			++enemy_static_defence_count;
		if (unit->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon)
			++enemy_static_defence_count;
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Spore_Colony)
			++enemy_static_defence_count;
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Sunken_Colony)
			++enemy_static_defence_count;
		if (unit->getType().isWorker())
			++enemy_worker_count;
		if (unit->getType().isRefinery() || unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser)
			++enemy_gas_count;
	}
}
