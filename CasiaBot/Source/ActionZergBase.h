#pragma once
#include "InformationManager.h"
#include "ProductionQueue.h"
#include "BuildOrder.h"

namespace CasiaBot
{
	struct ActionStatus
	{
		//���������ɵ�λ
		int larva_count;						//�׳�
		int drone_count;						//����
		int zergling_count;						//С��
		int hydralisk_count;					//����
		int lurker_count;						//�ش�
		int ultralisk_count;					//����
		int defiler_count;						//Ы��
		int overlord_count;						//����
		int mutalisk_count;						//����
		int scourge_count;						//�Ա���
		int queen_count;						//Ů��
		int guardian_count;						//������
		int devourer_count;						//������

												//��������ɵ�λ
		int larva_completed;						//�׳�
		int drone_completed;						//����
		int zergling_completed;						//С��
		int hydralisk_completed;					//����
		int lurker_completed;						//�ش�
		int ultralisk_completed;					//����
		int defiler_completed;						//Ы��
		int overlord_completed;						//����
		int mutalisk_completed;						//����
		int scourge_completed;						//�Ա���
		int queen_completed;						//Ů��
		int guardian_completed;						//������
		int devourer_completed;						//������

													//���������е�λ
		int larva_in_queue;                      //�������׳�
		int drone_in_queue;                      //�����й���
		int zergling_in_queue;                   //������С��
		int hydralisk_in_queue;                  //�����д���
		int lurker_in_queue;                     //�����еش�
		int ultralisk_in_queue;                  //����������
		int defiler_in_queue;                    //������Ы��
		int overlord_in_queue;                   //����������
		int mutalisk_in_queue;                   //�����з���
		int scourge_in_queue;                    //�������Ա���
		int queen_in_queue;                      //������Ů��
		int guardian_in_queue;                   //������������
		int devourer_in_queue;                   //������������

												 //������λ�ܼ�
		int larva_total;						//�׳�
		int drone_total;						//����
		int zergling_total;						//С��
		int hydralisk_total;					//����
		int lurker_total;						//�ش�
		int ultralisk_total;					//����
		int defiler_total;						//Ы��
		int overlord_total;						//����
		int mutalisk_total;						//����
		int scourge_total;						//�Ա���
		int queen_total;						//Ů��
		int guardian_total;						//������
		int devourer_total;						//������

												// �Ƽ�
		int muscular_arguments_count;
		int grooved_spines_count;
		int metabolic_boost_count;
		int lurker_aspect_count;
		int adrenal_glands_count;
		int melee_attacks_count;
		int missile_attacks_count;
		int ground_carapace_count;
		int flyer_attacks_count;
		int flyer_carapace_count;

		//�����ѽ��콨��
		int base_count;							//����
		int hatchery_count;						//ĸ��
		int lair_count;							//�䷿
		int hive_count;							//��Ѩ
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

												//�����ѽ��ɽ���
		int real_base_completed;					//ʵ�������ϵĻ���
		int base_completed;							//����
		int hatchery_completed;						//ĸ��
		int lair_completed;							//�䷿
		int hive_completed;							//��Ѩ
		int extractor_completed;                    //����
		int creep_colony_completed;                 //ֳ��
		int sunken_colony_completed;			    //�ش���
		int spore_colony_completed;			    	//������
		int spawning_pool_completed;                //Ѫ��
		int hydralisk_den_completed;                //���߶�Ѩ
		int queens_nest_completed;                  //�ʺ�
		int defiler_mound_completed;                //Ы�ӳ�
		int spire_completed;                        //������
		int greater_spire_completed;                //�������
		int nydus_canal_completed;                  //Ӽ��ͨ��
		int ultralisk_cavern_completed;             //����֮��

													//���������н���
		int base_being_built;						//����
		int hatchery_being_built;					//ĸ��
		int lair_being_built;						//�䷿
		int hive_being_built;						//��Ѩ
		int extractor_being_built;                  //����
		int creep_colony_being_built;               //ֳ��
		int sunken_colony_being_built;			    //�ش���
		int spore_colony_being_built;			   	//������
		int spawning_pool_being_built;              //Ѫ��
		int hydralisk_den_being_built;              //���߶�Ѩ
		int queens_nest_being_built;                //�ʺ�
		int defiler_mound_being_built;              //Ы�ӳ�
		int spire_being_built;                      //������
		int greater_spire_being_built;              //�������
		int nydus_canal_being_built;                //Ӽ��ͨ��
		int ultralisk_cavern_being_built;           //����֮��

													//���������н���
		int base_in_queue;						//����
		int hatchery_in_queue;					//ĸ��
		int lair_in_queue;						//�䷿
		int hive_in_queue;						//��Ѩ
		int extractor_in_queue;                 //����
		int creep_colony_in_queue;              //ֳ��
		int sunken_colony_in_queue;			    //�ش���
		int spore_colony_in_queue;			    //������
		int spawning_pool_in_queue;             //Ѫ��
		int hydralisk_den_in_queue;             //���߶�Ѩ
		int queens_nest_in_queue;               //�ʺ�
		int defiler_mound_in_queue;             //Ы�ӳ�
		int spire_in_queue;                     //������
		int greater_spire_in_queue;             //�������
		int nydus_canal_in_queue;               //Ӽ��ͨ��
		int ultralisk_cavern_in_queue;          //����֮��

												//���������ܼ�
		int base_total;							//����
		int hatchery_total;						//ĸ��
		int lair_total;							//�䷿
		int hive_total;							//��Ѩ
		int extractor_total;                    //����
		int creep_colony_total;                 //ֳ��
		int sunken_colony_total;			    //�ش���
		int spore_colony_total;			    	//������
		int spawning_pool_total;                //Ѫ��
		int hydralisk_den_total;                //���߶�Ѩ
		int queens_nest_total;                  //�ʺ�
		int defiler_mound_total;                //Ы�ӳ�
		int spire_total;                        //������
		int greater_spire_total;                //�������
		int nydus_canal_total;                  //Ӽ��ͨ��
		int ultralisk_cavern_total;             //����֮��

												//��������
		int army_supply;
		int air_army_supply;
		int ground_army_supply;

		//�жϺ�����
		bool being_rushed;

		//�з�
		int enemy_terran_unit_count;
		int enemy_protos_unit_count;
		int enemy_zerg_unit_count;

		//���嵥λ
		int enemy_marine_count;					//��ǹ��
		int enemy_firebat_count;				//�����
		int enemy_medic_count;					//ҽ�Ʊ�
		int enemy_ghost_count;					//����
		int enemy_vulture_count;				//�׳�
		int enemy_tank_count;					//̹��
		int enemy_goliath_count;				//������
		int enemy_wraith_count;					//����
		int enemy_valkyrie_count;				//����������
		int enemy_bc_count;						//ս��Ѳ��
		int enemy_science_vessel_count;			//��ѧ��
		int enemy_dropship_count;				//�����

												//���彨��
		int enemy_bunker_count;					//�ر�
		int enemy_barrack_count;				//��Ӫ
		int enemy_factory_count;				//����
		int enemy_starport_count;				//�ɻ���

												//���嵥λ
		int enemy_zealot_count;					//������
		int enemy_dragoon_count;				//����
		int enemy_ht_count;						//����ʥ��
		int enemy_dt_count;						//�ڰ�ʥ��
		int enemy_reaver_count;					//��׳�
		int enemy_shuttle_count;				//�����
		int enemy_carrier_count;				//��ĸ
		int enemy_arbiter_count;				//�ٲ���
		int enemy_corsair_count;				//������
		int enemy_scout_count;					//����
		int enemy_archon_count;					//����ִ����
		int enemy_dark_archon_count;			//�ڰ�ִ����

												//���彨��
		int enemy_cannon_count;					//��������
		int enemy_gateway_count;				//��Ӫ
		int enemy_stargate_count;				//����
		int enemy_robotics_facility_count;		//��е����

												//���嵥λ
		int enemy_zergling_count;				//С��
		int enemy_hydralisk_count;				//����
		int enemy_lurker_count;					//�ش�
		int enemy_ultralisk_count;				//����
		int enemy_defiler_count;				//Ы��
		int enemy_mutalisk_count;				//����
		int enemy_queen_count;					//Ů��
		int enemy_scourge_count;				//�Ա���
		int enemy_guardian_count;				//������ʿ
		int enemy_devourer_count;				//������

												//���彨��
		int enemy_spawning_pool_count;			//������
		int enemy_hydralisk_den_count;			//����Ѩ
		int enemy_evolution_chamber_count;		//����ǻ
		int enemy_spire_count;					//������

												//��������
		int enemy_army_supply;
		int enemy_air_army_supply;
		int enemy_ground_army_supply;
		int enemy_ground_large_army_supply;
		int enemy_ground_small_army_supply;
		int enemy_anti_air_army_supply;
		int enemy_biological_army_supply;
		int enemy_attacking_army_supply;
		int enemy_static_defence_count;
		int enemy_proxy_building_count;
		int enemy_attacking_worker_count;
		int enemy_cloaked_unit_count;

		//ͨ�ý����͵�λ
		int enemy_base_count;
		int enemy_worker_count;

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
	protected:
		static ActionStatus _status;
	};
}
