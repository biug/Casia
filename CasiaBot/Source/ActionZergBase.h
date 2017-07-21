#pragma once
#include "InformationManager.h"
#include "ProductionQueue.h"
#include "BuildOrder.h"

namespace CasiaBot
{
	class ActionZergBase
	{
	public:
		ActionZergBase();
		~ActionZergBase();
		virtual bool canDeployAction() = 0;
		virtual void init() = 0;
		virtual bool tick() = 0;
		virtual void getBuildOrderList(CasiaBot::ProductionQueue &queue) = 0;
		virtual void updateCurrentState(CasiaBot::ProductionQueue &queue);

	protected:
		std::pair<BWAPI::TilePosition, BWAPI::TilePosition> getClosestOpponentBaseLocation();
		double unitPathingDistance(BWAPI::UnitType type, std::pair<BWAPI::TilePosition, BWAPI::TilePosition> fromto);

	protected:
		//���������ɵ�λ
		int larva_count = 0;						//�׳�
		int drone_count = 0;						//����
		int zergling_count = 0;						//С��
		int hydralisk_count = 0;					//����
		int lurker_count = 0;						//�ش�
		int ultralisk_count = 0;					//����
		int defiler_count = 0;						//Ы��
		int overlord_count = 0;						//����
		int mutalisk_count = 0;						//����
		int scourge_count = 0;						//�Ա���
		int queen_count = 0;						//Ů��
		int guardian_count = 0;						//������
		int devourer_count = 0;						//������

		int zergling_completed = 0;
		int hydralisk_completed = 0;
		int lurker_completed = 0;
		int mutalisk_completed = 0;

		//���������е�λ
		int larva_in_queue = 0;                      //�������׳�
		int drone_in_queue = 0;                      //�����й���
		int zergling_in_queue = 0;                   //������С��
		int hydralisk_in_queue = 0;                  //�����д���
		int lurker_in_queue = 0;                     //�����еش�
		int ultralisk_in_queue = 0;                  //����������
		int defiler_in_queue = 0;                    //������Ы��
		int overlord_in_queue = 0;                   //����������
		int mutalisk_in_queue = 0;                   //�����з���
		int scourge_in_queue = 0;                    //�������Ա���
		int queen_in_queue = 0;                      //������Ů��
		int guardian_in_queue = 0;                   //������������
		int devourer_in_queue = 0;                   //������������

		int metabolic_boost_count;
		int lurker_aspect_count;
		int adrenal_glands_count;

		//�����ѽ��ɽ���
		int real_base_count;				//ʵ�������ϵĻ���
		int base_count;						//����
		int hatchery_count;					//ĸ��
		int lair_count;						//�䷿
		int hive_count;						//��Ѩ
		int extractor_count;                    //����
		int creep_colony_count;                 //ֳ��
		int sunken_colony_count;			    //�ش���
		int spore_colony_count;			    	//������
		int spawning_pool_count;                //Ѫ��
		int hydralisk_den_count;                //���߶�Ѩ
		int queens_nest_count;                  //�ʺ�
		int defiler_mound_count;                //Ы�ӳ�
		int spire_count;                        //������
		int greater_spire_count;                //�������
		int nydus_canal_count;                  //Ӽ��ͨ��
		int ultralisk_cavern_count;             //����֮��

		int base_completed;
		int hatchery_completed;
		int lair_completed;
		int hive_completed;
		int spire_complete;
		int creep_colony_completed;
		int extractor_completed;
		int spawning_pool_completed;
		int hydralisk_den_completed;
		int queens_nest_completed;
		int spire_completed;

		//���������н���
		int base_being_built;						//����
		int hatchery_being_built;					//ĸ��
		int lair_being_built;						//�䷿
		int hive_being_built;						//��Ѩ
		int extractor_being_built;                    //����
		int creep_colony_being_built;                 //ֳ��
		int sunken_colony_being_built;			    //�ش���
		int spore_colony_being_built;			   	//������
		int spawning_pool_being_built;                //Ѫ��
		int hydralisk_den_being_built;                //���߶�Ѩ
		int queens_nest_being_built;                  //�ʺ�
		int defiler_mound_being_built;                //Ы�ӳ�
		int spire_being_built;                        //������
		int greater_spire_being_built;                //�������
		int nydus_canal_being_built;                  //Ӽ��ͨ��
		int ultralisk_cavern_being_built;             //����֮��

		//���������н���
		int base_in_queue;						//����
		int hatchery_in_queue;					//ĸ��
		int lair_in_queue;						//�䷿
		int hive_in_queue;						//��Ѩ
		int extractor_in_queue;                    //����
		int creep_colony_in_queue;                 //ֳ��
		int sunken_colony_in_queue;			    //�ش���
		int spore_colony_in_queue;			    	//������
		int spawning_pool_in_queue;                //Ѫ��
		int hydralisk_den_in_queue;                //���߶�Ѩ
		int queens_nest_in_queue;                  //�ʺ�
		int defiler_mound_in_queue;                //Ы�ӳ�
		int spire_in_queue;                        //������
		int greater_spire_in_queue;                //�������
		int nydus_canal_in_queue;                  //Ӽ��ͨ��
		int ultralisk_cavern_in_queue;             //����֮��


												//��������
		double army_supply = 0;
		double air_army_supply = 0;
		double ground_army_supply = 0;

		//�жϺ�����
		bool opponent_has_expanded = false;
		bool can_expand = false;
		bool force_expand = false;
		bool being_rushed = false;
		bool is_attacking = false;
		bool is_defending = false;
		bool default_upgrade = false;

		//�з�
		int enemy_terran_unit_count = 0;
		int enemy_protos_unit_count = 0;
		int enemy_zerg_unit_count = 0;

		//���嵥λ
		int enemy_marine_count = 0;					//��ǹ��
		int enemy_firebat_count = 0;				//�����
		int enemy_medic_count = 0;					//ҽ�Ʊ�
		int enemy_ghost_count = 0;					//����
		int enemy_vulture_count = 0;				//�׳�
		int enemy_tank_count = 0;					//̹��
		int enemy_goliath_count = 0;				//������
		int enemy_wraith_count = 0;					//����
		int enemy_valkyrie_count = 0;				//����������
		int enemy_bc_count = 0;						//ս��Ѳ��
		int enemy_science_vessel_count = 0;			//��ѧ��
		int enemy_dropship_count = 0;				//�����

													//���彨��
		int enemy_bunker_count = 0;					//�ر�
		int enemy_barrack_count = 0;				//��Ӫ
		int enemy_factory_count = 0;				//����
		int enemy_starport_count = 0;				//�ɻ���

													//���嵥λ
		int enemy_zealot_count = 0;					//������
		int enemy_dragoon_count = 0;				//����
		int enemy_ht_count = 0;						//����ʥ��
		int enemy_dt_count = 0;						//�ڰ�ʥ��
		int enemy_reaver_count = 0;					//��׳�
		int enemy_shuttle_count = 0;				//�����
		int enemy_carrier_count = 0;				//��ĸ
		int enemy_arbiter_count = 0;				//�ٲ���
		int enemy_corsair_count = 0;				//������

													//���彨��
		int enemy_cannon_count = 0;					//��������
		int enemy_gateway_count = 0;				//��Ӫ
		int enemy_stargate_count = 0;				//����
		int enemy_robotics_facility_count = 0;		//��е����

													//���嵥λ
		int enemy_zergling_count = 0;				//С��
		int enemy_hydralisk_count = 0;				//����
		int enemy_lurker_count = 0;					//�ش�
		int enemy_ultralisk_count = 0;				//����
		int enemy_defiler_count = 0;				//Ы��
		int enemy_mutalisk_count = 0;				//����
		int enemy_queen_count = 0;					//Ů��

													//���彨��
		int enemy_spawning_pool_count = 0;			//������
		int enemy_hydralisk_den_count = 0;			//����Ѩ
		int enemy_evolution_chamber_count = 0;		//����ǻ
		int enemy_spire_count = 0;					//������

													//��������
		double enemy_army_supply = 0;
		double enemy_air_army_supply = 0;
		double enemy_ground_army_supply = 0;
		double enemy_ground_large_army_supply = 0;
		double enemy_ground_small_army_supply = 0;
		double enemy_anti_air_army_supply = 0;
		double enemy_biological_army_supply = 0;
		int enemy_static_defence_count = 0;
		int enemy_proxy_building_count = 0;
		double enemy_attacking_army_supply = 0;
		int enemy_attacking_worker_count = 0;
		int enemy_cloaked_unit_count = 0;

		//ͨ�ý����͵�λ
		int enemy_worker_count = 0;
		int enemy_gas_count = 0;

		// ͨ�ó�Ա����
		bool isInitialized = false;
		int lastFrameCount = 0;
		int lastFrameMineralAmount = 0;
		int lastFrameGasAmount = 0;
		std::deque<int> mineralNetIncrease;
		std::deque<int> gasNetIncrease;
	};
}
