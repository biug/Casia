#include "ActionZergBase.h"
#include "BuildingManager.h"

using namespace CasiaBot;

ActionStatus::ActionStatus()
{
}

void ActionStatus::checkCurrentQueue(CasiaBot::ProductionQueue &queue)
{
	// 没血池造小狗
	if (spawning_pool_completed == 0 && zergling_in_queue > 0)
	{
		queue.clear();
	}
	// 没基地升二本
	else if (hatchery_completed == 0 && lair_in_queue > 0)
	{
		queue.clear();
	}
	// 没二本升三本
	else if (lair_completed == 0 && hive_in_queue > 0)
	{
		queue.clear();
	}
	// 没二本造spire
	else if (lair_completed == 0 && spire_in_queue > 0)
	{
		queue.clear();
	}
	// 没刺蛇洞造刺蛇
	else if (hydralisk_den_completed == 0 && hydralisk_in_queue > 0)
	{
		queue.clear();
	}
	// 没刺蛇造lurker
	else if (lurker_in_queue > hydralisk_completed)
	{
		queue.clear();
	}
	// 没creep造sunken
	else if (creep_colony_completed == 0 && sunken_colony_in_queue > 0)
	{
		queue.clear();
	}
	// 没spire造飞龙
	else if (spire_completed == 0 && mutalisk_in_queue > 0)
	{
		queue.clear();
	}
	// 农民过少时，补农民
	if (drone_count < 3)
	{
		queue.clear();
		queue.add(MetaType(BWAPI::UnitTypes::Zerg_Drone), true);
	}
}

void ActionStatus::updateCurrentState(CasiaBot::ProductionQueue &queue)
{
	auto self = BWAPI::Broodwar->self();
	auto enemy = BWAPI::Broodwar->enemy();

	//我方
	larva_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Larva, self);
	drone_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Drone, self);
	zergling_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Zergling, self);
	hydralisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hydralisk, self);
	lurker_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Lurker, self);
	ultralisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Ultralisk, self);
	defiler_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Defiler, self);
	overlord_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Overlord, self);
	mutalisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Mutalisk, self);
	scourge_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Scourge, self);
	queen_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Queen, self);
	guardian_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Guardian, self);
	devourer_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Devourer, self);

	larva_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Larva, self);
	drone_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Drone, self);
	zergling_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Zergling, self);
	hydralisk_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Hydralisk, self);
	lurker_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Lurker, self);
	ultralisk_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Ultralisk, self);
	defiler_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Defiler, self);
	overlord_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Overlord, self);
	mutalisk_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Mutalisk, self);
	scourge_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Scourge, self);
	queen_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Queen, self);
	guardian_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Guardian, self);
	devourer_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Devourer, self);

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

	larva_total = larva_count + larva_in_queue;
	drone_total = drone_count + drone_in_queue;
	zergling_total = zergling_count + zergling_in_queue;
	hydralisk_total = hydralisk_count + hydralisk_in_queue;
	lurker_total = lurker_count + lurker_in_queue;
	ultralisk_total = ultralisk_count + ultralisk_in_queue;
	defiler_total = defiler_count + defiler_in_queue;
	overlord_total = overlord_count + overlord_in_queue;
	mutalisk_total = mutalisk_count + mutalisk_in_queue;
	scourge_total = scourge_count + scourge_in_queue;
	queen_total = queen_count + queen_in_queue;
	guardian_total = guardian_count + guardian_in_queue;
	devourer_total = devourer_count + devourer_in_queue;

	metabolic_boost_count = queue.upgradeCount(BWAPI::UpgradeTypes::Metabolic_Boost)
		+ self->getUpgradeLevel(BWAPI::UpgradeTypes::Metabolic_Boost)
		+ self->isUpgrading(BWAPI::UpgradeTypes::Metabolic_Boost);
	lurker_aspect_count = queue.techCount(BWAPI::TechTypes::Lurker_Aspect)
		+ self->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
		+ self->isResearching(BWAPI::TechTypes::Lurker_Aspect);
	adrenal_glands_count = queue.upgradeCount(BWAPI::UpgradeTypes::Adrenal_Glands)
		+ self->getUpgradeLevel(BWAPI::UpgradeTypes::Adrenal_Glands)
		+ self->isUpgrading(BWAPI::UpgradeTypes::Adrenal_Glands);
	grooved_spines_count = queue.upgradeCount(BWAPI::UpgradeTypes::Grooved_Spines)
		+ self->getUpgradeLevel(BWAPI::UpgradeTypes::Grooved_Spines)
		+ self->isUpgrading(BWAPI::UpgradeTypes::Grooved_Spines);
	muscular_arguments_count = queue.upgradeCount(BWAPI::UpgradeTypes::Muscular_Augments)
		+ self->getUpgradeLevel(BWAPI::UpgradeTypes::Muscular_Augments)
		+ self->isUpgrading(BWAPI::UpgradeTypes::Muscular_Augments);
	melee_attacks_count = queue.upgradeCount(BWAPI::UpgradeTypes::Zerg_Melee_Attacks)
		+ self->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Melee_Attacks)
		+ self->isUpgrading(BWAPI::UpgradeTypes::Zerg_Melee_Attacks);
	missile_attacks_count = queue.upgradeCount(BWAPI::UpgradeTypes::Zerg_Missile_Attacks)
		+ self->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Missile_Attacks)
		+ self->isUpgrading(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
	ground_carapace_count = queue.upgradeCount(BWAPI::UpgradeTypes::Zerg_Carapace)
		+ self->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Carapace)
		+ self->isUpgrading(BWAPI::UpgradeTypes::Zerg_Carapace);
	flyer_attacks_count = queue.upgradeCount(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks)
		+ self->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks)
		+ self->isUpgrading(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks);
	flyer_carapace_count = queue.upgradeCount(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace)
		+ self->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace)
		+ self->isUpgrading(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace);

	//建筑
	hatchery_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hatchery, self);
	lair_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Lair, self);
	hive_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hive, self);
	base_count = hatchery_count + lair_count + hive_count;
	extractor_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Extractor, self);
	creep_colony_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Creep_Colony, self);
	sunken_colony_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Sunken_Colony, self);
	spore_colony_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Spore_Colony, self);
	spawning_pool_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Spawning_Pool, self);
	hydralisk_den_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hydralisk_Den, self);
	queens_nest_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Queens_Nest, self);
	defiler_mound_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Defiler_Mound, self);
	spire_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Spire, self);
	greater_spire_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Greater_Spire, self);
	nydus_canal_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Nydus_Canal, self);
	ultralisk_cavern_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Ultralisk_Cavern, self);

	hatchery_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Hatchery, self);
	lair_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Lair, self);
	hive_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Hive, self);
	base_completed = hatchery_completed + lair_completed + hive_completed;
	extractor_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Extractor, self);
	creep_colony_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Creep_Colony, self);
	sunken_colony_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Sunken_Colony, self);
	spore_colony_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Spore_Colony, self);
	spawning_pool_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Spawning_Pool, self);
	hydralisk_den_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Hydralisk_Den, self);
	queens_nest_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Queens_Nest, self);
	defiler_mound_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Defiler_Mound, self);
	spire_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Spire, self);
	greater_spire_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Greater_Spire, self);
	nydus_canal_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Nydus_Canal, self);
	ultralisk_cavern_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Ultralisk_Cavern, self);

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

	hatchery_total = hatchery_count + hatchery_in_queue + hatchery_being_built;
	lair_total = lair_count + lair_in_queue + lair_being_built;
	hive_total = hive_count + hive_in_queue + hive_being_built;
	base_total = hatchery_total + lair_total + hive_total;
	extractor_total = extractor_count + extractor_in_queue + extractor_being_built;
	creep_colony_total = creep_colony_count + creep_colony_in_queue + creep_colony_being_built;
	sunken_colony_total = sunken_colony_count + sunken_colony_in_queue + sunken_colony_being_built;
	spore_colony_total = spore_colony_count + spore_colony_in_queue + spore_colony_being_built;
	spawning_pool_total = spawning_pool_count + spawning_pool_in_queue + spawning_pool_being_built;
	hydralisk_den_total = hydralisk_den_count + hydralisk_den_in_queue + hydralisk_den_being_built;
	queens_nest_total = queens_nest_count + queens_nest_in_queue + queens_nest_being_built;
	defiler_mound_total = defiler_mound_count + defiler_mound_in_queue + defiler_mound_being_built;
	spire_total = spire_count + spire_in_queue + spire_being_built;
	greater_spire_total = greater_spire_count + greater_spire_in_queue + greater_spire_being_built;
	nydus_canal_total = nydus_canal_count + nydus_canal_in_queue + nydus_canal_being_built;
	ultralisk_cavern_total = ultralisk_cavern_count + ultralisk_cavern_in_queue + ultralisk_cavern_being_built;

	//军事力量
	army_supply = 0;
	air_army_supply = 0;
	ground_army_supply = 0;

	//判断和倾向
	being_rushed = InformationManager::Instance().isEncounterRush();

	//敌方
	enemy_terran_unit_count = 0;
	enemy_protos_unit_count = 0;
	enemy_zerg_unit_count = 0;

	enemy_marine_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Marine, enemy);
	enemy_firebat_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Firebat, enemy);
	enemy_medic_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Medic, enemy);
	enemy_ghost_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Ghost, enemy);
	enemy_vulture_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Vulture, enemy);
	enemy_tank_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode, enemy)
		+ InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, enemy);
	enemy_goliath_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Goliath, enemy);
	enemy_wraith_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Wraith, enemy);
	enemy_valkyrie_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Valkyrie, enemy);
	enemy_bc_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Battlecruiser, enemy);
	enemy_science_vessel_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Science_Vessel, enemy);
	enemy_dropship_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Dropship, enemy);

	enemy_bunker_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Bunker, enemy);
	enemy_barrack_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Barracks, enemy);
	enemy_factory_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Factory, enemy);
	enemy_starport_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Starport, enemy);

	enemy_zealot_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Zealot, enemy);					//狂热者
	enemy_dragoon_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Dragoon, enemy);					//龙骑
	enemy_ht_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_High_Templar, enemy);					//光明圣堂
	enemy_dt_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Dark_Templar, enemy);					//黑暗圣堂
	enemy_reaver_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Reaver, enemy);					//金甲虫
	enemy_shuttle_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Shuttle, enemy);					//运输机
	enemy_carrier_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Carrier, enemy);					//航母
	enemy_arbiter_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Arbiter, enemy);					//仲裁者
	enemy_corsair_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Corsair, enemy);					//海盗船
	enemy_scout_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Scout, enemy);						//侦察机

	enemy_cannon_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Photon_Cannon, enemy);
	enemy_gateway_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Gateway, enemy);
	enemy_stargate_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Stargate, enemy);
	enemy_robotics_facility_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Robotics_Facility, enemy);

	enemy_zergling_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Zergling, enemy);				//小狗
	enemy_hydralisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hydralisk, enemy);			//刺蛇
	enemy_lurker_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Lurker, enemy);					//地刺
	enemy_ultralisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Ultralisk, enemy);			//雷兽
	enemy_defiler_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Defiler, enemy);				//蝎子
	enemy_mutalisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Mutalisk, enemy);				//飞龙
	enemy_queen_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Queen, enemy);					//女王

	enemy_spawning_pool_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Spawning_Pool, enemy);
	enemy_hydralisk_den_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hydralisk_Den, enemy);
	enemy_evolution_chamber_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Evolution_Chamber, enemy);
	enemy_spire_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Spire, enemy);

	enemy_army_supply = 0;
	enemy_air_army_supply = 0;
	enemy_ground_army_supply = 0;
	enemy_ground_large_army_supply = 0;
	enemy_ground_small_army_supply = 0;
	enemy_anti_air_army_supply = 0;
	enemy_biological_army_supply = 0;
	enemy_static_defence_count = 0;
	enemy_proxy_building_count = 0;
	enemy_attacking_army_supply = 0;
	enemy_attacking_worker_count = 0;
	enemy_cloaked_unit_count = 0;

	//通用建筑和单位
	enemy_worker_count = 0;
	enemy_base_count = 0;

	//我方
	real_base_completed = WorkerManager::Instance().getMineralBases().size();

	//敌方
	for (const auto & u : InformationManager::Instance().getUnitInfo(enemy))
	{
		auto info = u.second;
		auto type = info.type;
		switch (type.getRace().getID())
		{
		case BWAPI::Races::Enum::Terran:
			++enemy_terran_unit_count;
			break;
		case BWAPI::Races::Enum::Protoss:
			++enemy_protos_unit_count;
			break;
		case BWAPI::Races::Enum::Zerg:
			++enemy_zerg_unit_count;
			break;
		}

		//军事力量
		if (!type.isWorker() && !type.isBuilding() && type.canAttack()) {
			if (type.isFlyer()) {
				enemy_air_army_supply += type.supplyRequired();
			}
			else {
				enemy_ground_army_supply += type.supplyRequired();
				switch (type.getID())
				{
				case BWAPI::UnitSizeTypes::Enum::Large:
					enemy_ground_large_army_supply += type.supplyRequired();
					break;
				case BWAPI::UnitSizeTypes::Enum::Small:
					enemy_ground_small_army_supply += type.supplyRequired();
					break;
				}
			}
			enemy_army_supply += type.supplyRequired();
			if (type.airWeapon())
				enemy_anti_air_army_supply += type.supplyRequired();
			if (type.isOrganic())
				enemy_biological_army_supply += type.supplyRequired();

		}
		switch (type.getID())
		{
		case BWAPI::UnitTypes::Enum::Terran_Missile_Turret:
			++enemy_static_defence_count;
			break;
		case BWAPI::UnitTypes::Enum::Protoss_Photon_Cannon:
			++enemy_static_defence_count;
			break;
		case BWAPI::UnitTypes::Enum::Zerg_Spore_Colony:
			++enemy_static_defence_count;
			break;
		case BWAPI::UnitTypes::Enum::Zerg_Sunken_Colony:
			++enemy_static_defence_count;
			break;

		}
		if (type.isWorker())
			++enemy_worker_count;
		else if (type.isResourceDepot())
			++enemy_base_count;
	}
	enemy_army_supply /= 2;
	enemy_air_army_supply /= 2;
	enemy_ground_army_supply /= 2;
	enemy_ground_large_army_supply /= 2;
	enemy_ground_small_army_supply /= 2;
	enemy_anti_air_army_supply /= 2;
	enemy_biological_army_supply /= 2;
	enemy_attacking_army_supply /= 2;
}

ActionStatus ActionZergBase::_status;

ActionZergBase::ActionZergBase()
{
}


ActionZergBase::~ActionZergBase()
{
}

void ActionZergBase::updateStatus(CasiaBot::ProductionQueue &queue)
{
	_status.checkCurrentQueue(queue);
	_status.updateCurrentState(queue);
}
