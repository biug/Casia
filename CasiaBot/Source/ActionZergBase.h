#pragma once
#include "InformationManager.h"
#include "ProductionQueue.h"
#include "BuildOrder.h"

namespace CasiaBot
{
	struct ActionStatus
	{
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

		//��������ɵ�λ
		int larva_completed = 0;						//�׳�
		int drone_completed = 0;						//����
		int zergling_completed = 0;						//С��
		int hydralisk_completed = 0;					//����
		int lurker_completed = 0;						//�ش�
		int ultralisk_completed = 0;					//����
		int defiler_completed = 0;						//Ы��
		int overlord_completed = 0;						//����
		int mutalisk_completed = 0;						//����
		int scourge_completed = 0;						//�Ա���
		int queen_completed = 0;						//Ů��
		int guardian_completed = 0;						//������
		int devourer_completed = 0;						//������

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

		//������λ�ܼ�
		int larva_total = 0;						//�׳�
		int drone_total = 0;						//����
		int zergling_total = 0;						//С��
		int hydralisk_total = 0;					//����
		int lurker_total = 0;						//�ش�
		int ultralisk_total = 0;					//����
		int defiler_total = 0;						//Ы��
		int overlord_total = 0;						//����
		int mutalisk_total = 0;						//����
		int scourge_total = 0;						//�Ա���
		int queen_total = 0;						//Ů��
		int guardian_total = 0;						//������
		int devourer_total = 0;						//������

		// �Ƽ�
		int muscular_arguments_total = 0;
		int grooved_spines_total = 0;
		int metabolic_boost_total = 0;
		int lurker_aspect_total = 0;
		int adrenal_glands_total = 0;
		int melee_attacks_total = 0;
		int missile_attacks_total = 0;
		int ground_carapace_total = 0;
		int flyer_attacks_total = 0;
		int flyer_carapace_total = 0;

		int muscular_arguments_in_queue = 0;
		int grooved_spines_in_queue = 0;
		int metabolic_boost_in_queue = 0;
		int lurker_aspect_in_queue = 0;
		int adrenal_glands_in_queue = 0;
		int melee_attacks_in_queue = 0;
		int missile_attacks_in_queue = 0;
		int ground_carapace_in_queue = 0;
		int flyer_attacks_in_queue = 0;
		int flyer_carapace_in_queue = 0;

		//�����ѽ��콨��
		int base_count = 0;							//����
		int hatchery_count = 0;						//ĸ��
		int lair_count = 0;							//�䷿
		int hive_count = 0;							//��Ѩ
		int extractor_count = 0;                    //����
		int creep_colony_count = 0;                 //ֳ��
		int sunken_colony_count = 0;			    //�ش���
		int spore_colony_count = 0;			    	//������
		int spawning_pool_count = 0;                //Ѫ��
		int hydralisk_den_count = 0;                //���߶�Ѩ
		int queens_nest_count = 0;                  //�ʺ�
		int defiler_mound_count = 0;                //Ы�ӳ�
		int spire_count = 0;                        //������
		int greater_spire_count = 0;                //�������
		int nydus_canal_count = 0;                  //Ӽ��ͨ��
		int ultralisk_cavern_count = 0;             //����֮��

		//�����ѽ��ɽ���
		int real_base_completed = 0;					//ʵ�������ϵĻ���
		int base_completed = 0;							//����
		int hatchery_completed = 0;						//ĸ��
		int lair_completed = 0;							//�䷿
		int hive_completed = 0;							//��Ѩ
		int extractor_completed = 0;                    //����
		int creep_colony_completed = 0;                 //ֳ��
		int sunken_colony_completed = 0;			    //�ش���
		int spore_colony_completed = 0;			    	//������
		int spawning_pool_completed = 0;                //Ѫ��
		int hydralisk_den_completed = 0;                //���߶�Ѩ
		int queens_nest_completed = 0;                  //�ʺ�
		int defiler_mound_completed = 0;                //Ы�ӳ�
		int spire_completed = 0;                        //������
		int greater_spire_completed = 0;                //�������
		int nydus_canal_completed = 0;                  //Ӽ��ͨ��
		int ultralisk_cavern_completed = 0;             //����֮��

		//���������н���
		int base_being_built = 0;						//����
		int hatchery_being_built = 0;					//ĸ��
		int lair_being_built = 0;						//�䷿
		int hive_being_built = 0;						//��Ѩ
		int extractor_being_built = 0;                  //����
		int creep_colony_being_built = 0;               //ֳ��
		int sunken_colony_being_built = 0;			    //�ش���
		int spore_colony_being_built = 0;			   	//������
		int spawning_pool_being_built = 0;              //Ѫ��
		int hydralisk_den_being_built = 0;              //���߶�Ѩ
		int queens_nest_being_built = 0;                //�ʺ�
		int defiler_mound_being_built = 0;              //Ы�ӳ�
		int spire_being_built = 0;                      //������
		int greater_spire_being_built = 0;              //�������
		int nydus_canal_being_built = 0;                //Ӽ��ͨ��
		int ultralisk_cavern_being_built = 0;           //����֮��

		//���������н���
		int base_in_queue = 0;						//����
		int hatchery_in_queue = 0;					//ĸ��
		int lair_in_queue = 0;						//�䷿
		int hive_in_queue = 0;						//��Ѩ
		int extractor_in_queue = 0;                 //����
		int creep_colony_in_queue = 0;              //ֳ��
		int sunken_colony_in_queue = 0;			    //�ش���
		int spore_colony_in_queue = 0;			    //������
		int spawning_pool_in_queue = 0;             //Ѫ��
		int hydralisk_den_in_queue = 0;             //���߶�Ѩ
		int queens_nest_in_queue = 0;               //�ʺ�
		int defiler_mound_in_queue = 0;             //Ы�ӳ�
		int spire_in_queue = 0;                     //������
		int greater_spire_in_queue = 0;             //�������
		int nydus_canal_in_queue = 0;               //Ӽ��ͨ��
		int ultralisk_cavern_in_queue = 0;          //����֮��

		// �ȴ��еĽ���
		int base_waiting = 0;						//����
		int hatchery_waiting = 0;					//ĸ��
		int lair_waiting = 0;						//�䷿
		int hive_waiting = 0;						//��Ѩ
		int extractor_waiting = 0;                 	//����
		int creep_colony_waiting = 0;              	//ֳ��
		int sunken_colony_waiting = 0;			   	//�ش���
		int spore_colony_waiting = 0;			   	//������
		int spawning_pool_waiting = 0;             	//Ѫ��
		int hydralisk_den_waiting = 0;             	//���߶�Ѩ
		int queens_nest_waiting = 0;               	//�ʺ�
		int defiler_mound_waiting = 0;             	//Ы�ӳ�
		int spire_waiting = 0;                     	//������
		int greater_spire_waiting = 0;             	//�������
		int nydus_canal_waiting = 0;               	//Ӽ��ͨ��
		int ultralisk_cavern_waiting = 0;          	//����֮��

		//���������ܼ�
		int base_total = 0;							//����
		int hatchery_total = 0;						//ĸ��
		int lair_total = 0;							//�䷿
		int hive_total = 0;							//��Ѩ
		int extractor_total = 0;                    //����
		int creep_colony_total = 0;                 //ֳ��
		int sunken_colony_total = 0;			    //�ش���
		int spore_colony_total = 0;			    	//������
		int spawning_pool_total = 0;                //Ѫ��
		int hydralisk_den_total = 0;                //���߶�Ѩ
		int queens_nest_total = 0;                  //�ʺ�
		int defiler_mound_total = 0;                //Ы�ӳ�
		int spire_total = 0;                        //������
		int greater_spire_total = 0;                //�������
		int nydus_canal_total = 0;                  //Ӽ��ͨ��
		int ultralisk_cavern_total = 0;             //����֮��

		//��������
		int army_supply = 0;
		int air_army_supply = 0;
		int ground_army_supply = 0;

		//�жϺ�����
		bool being_rushed = false;

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
		int enemy_scout_count = 0;					//����
		int enemy_archon_count = 0;					//����ִ����
		int enemy_dark_archon_count = 0;			//�ڰ�ִ����

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
		int enemy_scourge_count = 0;				//�Ա���
		int enemy_guardian_count = 0;				//������ʿ
		int enemy_devourer_count = 0;				//������

		//���彨��
		int enemy_spawning_pool_count = 0;			//������
		int enemy_hydralisk_den_count = 0;			//����Ѩ
		int enemy_evolution_chamber_count = 0;		//����ǻ
		int enemy_spire_count = 0;					//������

		//��������
		int enemy_army_supply = 0;
		int enemy_air_army_supply = 0;
		int enemy_ground_army_supply = 0;
		int enemy_ground_large_army_supply = 0;
		int enemy_ground_small_army_supply = 0;
		int enemy_anti_air_army_supply = 0;
		int enemy_biological_army_supply = 0;
		int enemy_attacking_army_supply = 0;
		int enemy_static_defence_count = 0;
		int enemy_proxy_building_count = 0;
		int enemy_attacking_worker_count = 0;
		int enemy_cloaked_unit_count = 0;

		//ͨ�ý����͵�λ
		int enemy_base_count = 0;
		int enemy_worker_count = 0;

		ActionStatus();
		void checkCurrentQueue(CasiaBot::ProductionQueue &queue);
		void updateCurrentState(CasiaBot::ProductionQueue &queue);
	};

	class ActionZergBase
	{
	public:
		ActionZergBase();
		~ActionZergBase();
		virtual void init() = 0;
		virtual bool tick() = 0;
		virtual bool canDeployAction() = 0;
		virtual void getBuildOrderList(CasiaBot::ProductionQueue &queue) = 0;

		static void updateStatus(CasiaBot::ProductionQueue &queue);
		static ActionStatus _status;
	};
}
