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
	//�ҷ�
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


	larva_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Larva);		//�׳�
	drone_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Drone);		//����
	zergling_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Zergling) * 2;	//С��
	hydralisk_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Hydralisk);//����
	lurker_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Lurker);		//�ش�
	ultralisk_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Ultralisk);//����
	defiler_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Defiler);	//Ы��
	overlord_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Overlord);	//����
	mutalisk_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Mutalisk);	//����
	scourge_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Scourge) * 2;	//�Ա���
	queen_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Queen);		//Ů��
	guardian_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Guardian);	//������
	devourer_in_queue = queue.unitCount(BWAPI::UnitTypes::Zerg_Devourer);	//������

	metabolic_boost_count = queue.upgradeCount(BWAPI::UpgradeTypes::Metabolic_Boost)
		+ BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Metabolic_Boost)
		+ BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Metabolic_Boost);
	lurker_aspect_count = queue.techCount(BWAPI::TechTypes::Lurker_Aspect)
		+ BWAPI::Broodwar->self()->hasResearched(BWAPI::TechTypes::Lurker_Aspect)
		+ BWAPI::Broodwar->self()->isResearching(BWAPI::TechTypes::Lurker_Aspect);
	adrenal_glands_count = queue.upgradeCount(BWAPI::UpgradeTypes::Adrenal_Glands)
		+ BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Adrenal_Glands)
		+ BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Adrenal_Glands);
	grooved_spines_count = queue.upgradeCount(BWAPI::UpgradeTypes::Grooved_Spines)
		+ BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Grooved_Spines)
		+ BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Grooved_Spines);
	muscular_arguments_count = queue.upgradeCount(BWAPI::UpgradeTypes::Muscular_Augments)
		+ BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Muscular_Augments)
		+ BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Muscular_Augments);
	melee_attacks_count = queue.upgradeCount(BWAPI::UpgradeTypes::Zerg_Melee_Attacks)
		+ BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Melee_Attacks)
		+ BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Zerg_Melee_Attacks);
	missile_attacks_count = queue.upgradeCount(BWAPI::UpgradeTypes::Zerg_Missile_Attacks)
		+ BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Missile_Attacks)
		+ BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Zerg_Missile_Attacks);
	ground_carapace_count = queue.upgradeCount(BWAPI::UpgradeTypes::Zerg_Carapace)
		+ BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Carapace)
		+ BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Zerg_Carapace);
	flyer_attacks_count = queue.upgradeCount(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks)
		+ BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks)
		+ BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks);
	flyer_carapace_count = queue.upgradeCount(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace)
		+ BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace)
		+ BWAPI::Broodwar->self()->isUpgrading(BWAPI::UpgradeTypes::Zerg_Flyer_Carapace);

	//����
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
	spire_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Spire, BWAPI::Broodwar->self());
	creep_colony_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Creep_Colony, BWAPI::Broodwar->self());
	extractor_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Extractor, BWAPI::Broodwar->self());
	spawning_pool_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Spawning_Pool, BWAPI::Broodwar->self());
	hydralisk_den_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Hydralisk_Den, BWAPI::Broodwar->self());
	queens_nest_completed = InformationManager::Instance().getNumConstructedUnits(BWAPI::UnitTypes::Zerg_Queens_Nest, BWAPI::Broodwar->self());

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
	
	//��������
	army_supply = 0.0;
	air_army_supply = 0.0;
	ground_army_supply = 0.0;

	//�жϺ�����
	//todo
	opponent_has_expanded = false;
	can_expand = false;
	force_expand = false;
	being_rushed = false;
	is_attacking = false;
	is_defending = false;
	default_upgrade = false;

	//�з�
	enemy_terran_unit_count = 0;
	enemy_protos_unit_count = 0;
	enemy_zerg_unit_count = 0;

	//���嵥λ
	enemy_marine_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Marine, BWAPI::Broodwar->enemy());						//��ǹ��
	enemy_firebat_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Firebat, BWAPI::Broodwar->enemy());					//�����
	enemy_medic_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Medic, BWAPI::Broodwar->enemy());						//ҽ�Ʊ�
	enemy_ghost_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Ghost, BWAPI::Broodwar->enemy());						//����
	enemy_vulture_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Vulture, BWAPI::Broodwar->enemy());					//�׳�
	enemy_tank_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode, BWAPI::Broodwar->enemy())
		+ InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode, BWAPI::Broodwar->enemy());						//̹��
	enemy_goliath_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Goliath, BWAPI::Broodwar->enemy());					//������
	enemy_wraith_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Wraith, BWAPI::Broodwar->enemy());						//����
	enemy_valkyrie_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Valkyrie, BWAPI::Broodwar->enemy());					//����������
	enemy_bc_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Battlecruiser, BWAPI::Broodwar->enemy());					//ս��Ѳ��
	enemy_science_vessel_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Science_Vessel, BWAPI::Broodwar->enemy());		//��ѧ��
	enemy_dropship_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Dropship, BWAPI::Broodwar->enemy());					//�����

	//���彨��
	enemy_bunker_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Bunker, BWAPI::Broodwar->enemy());						//�ر�
	enemy_barrack_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Barracks, BWAPI::Broodwar->enemy());					//��Ӫ
	enemy_factory_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Factory, BWAPI::Broodwar->enemy());					//����
	enemy_starport_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Terran_Starport, BWAPI::Broodwar->enemy());					//�ɻ���

	//���嵥λ
	enemy_zealot_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Zealot, BWAPI::Broodwar->enemy());					//������
	enemy_dragoon_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Dragoon, BWAPI::Broodwar->enemy());					//����
	enemy_ht_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_High_Templar, BWAPI::Broodwar->enemy());					//����ʥ��
	enemy_dt_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Dark_Templar, BWAPI::Broodwar->enemy());					//�ڰ�ʥ��
	enemy_reaver_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Reaver, BWAPI::Broodwar->enemy());					//��׳�
	enemy_shuttle_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Shuttle, BWAPI::Broodwar->enemy());					//�����
	enemy_carrier_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Carrier, BWAPI::Broodwar->enemy());					//��ĸ
	enemy_arbiter_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Arbiter, BWAPI::Broodwar->enemy());					//�ٲ���
	enemy_corsair_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Corsair, BWAPI::Broodwar->enemy());					//������
	enemy_scout_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Scout, BWAPI::Broodwar->enemy());						//����

	//���彨��
	enemy_cannon_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Photon_Cannon, BWAPI::Broodwar->enemy());				//��������
	enemy_gateway_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::Broodwar->enemy());					//��Ӫ
	enemy_stargate_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Stargate, BWAPI::Broodwar->enemy());				//����
	enemy_robotics_facility_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Protoss_Robotics_Facility, BWAPI::Broodwar->enemy());	//��е����

	//���嵥λ
	enemy_zergling_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Zergling, BWAPI::Broodwar->enemy());				//С��
	enemy_hydralisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hydralisk, BWAPI::Broodwar->enemy());				//����
	enemy_lurker_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Lurker, BWAPI::Broodwar->enemy());					//�ش�
	enemy_ultralisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Ultralisk, BWAPI::Broodwar->enemy());				//����
	enemy_defiler_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Defiler, BWAPI::Broodwar->enemy());				//Ы��
	enemy_mutalisk_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Mutalisk, BWAPI::Broodwar->enemy());				//����
	enemy_queen_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Queen, BWAPI::Broodwar->enemy());					//Ů��

	//���彨��
	enemy_spawning_pool_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Spawning_Pool, BWAPI::Broodwar->enemy());			//������
	enemy_hydralisk_den_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Hydralisk_Den, BWAPI::Broodwar->enemy());			//����Ѩ
	enemy_evolution_chamber_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Evolution_Chamber, BWAPI::Broodwar->enemy());		//����ǻ
	enemy_spire_count = InformationManager::Instance().getNumUnits(BWAPI::UnitTypes::Zerg_Spire, BWAPI::Broodwar->enemy());					//������

	//��������
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

	//ͨ�ý����͵�λ
	enemy_worker_count = 0;
	enemy_gas_count = 0;

	//�ҷ�
	real_base_count = WorkerManager::Instance().getMineralBases().size();

	//�з�
	for (const auto & u : InformationManager::Instance().getUnitInfo(BWAPI::Broodwar->enemy()))
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

		//��������
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
		if (type.isRefinery() || type == BWAPI::UnitTypes::Resource_Vespene_Geyser)
			++enemy_gas_count;
	}
}
