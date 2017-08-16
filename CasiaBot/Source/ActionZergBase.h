#pragma once
#include "InformationManager.h"
#include "ProductionQueue.h"
#include "BuildOrder.h"

namespace CasiaBot
{
	struct ActionStatus
	{
		//己方已生成单位
		int larva_count;						//幼虫
		int drone_count;						//工蜂
		int zergling_count;						//小狗
		int hydralisk_count;					//刺蛇
		int lurker_count;						//地刺
		int ultralisk_count;					//雷兽
		int defiler_count;						//蝎子
		int overlord_count;						//领主
		int mutalisk_count;						//飞龙
		int scourge_count;						//自爆蚊
		int queen_count;						//女王
		int guardian_count;						//守卫者
		int devourer_count;						//吞噬者

												//己方已完成单位
		int larva_completed;						//幼虫
		int drone_completed;						//工蜂
		int zergling_completed;						//小狗
		int hydralisk_completed;					//刺蛇
		int lurker_completed;						//地刺
		int ultralisk_completed;					//雷兽
		int defiler_completed;						//蝎子
		int overlord_completed;						//领主
		int mutalisk_completed;						//飞龙
		int scourge_completed;						//自爆蚊
		int queen_completed;						//女王
		int guardian_completed;						//守卫者
		int devourer_completed;						//吞噬者

													//己方队列中单位
		int larva_in_queue;                      //队列中幼虫
		int drone_in_queue;                      //队列中工蜂
		int zergling_in_queue;                   //队列中小狗
		int hydralisk_in_queue;                  //队列中刺蛇
		int lurker_in_queue;                     //队列中地刺
		int ultralisk_in_queue;                  //队列中雷兽
		int defiler_in_queue;                    //队列中蝎子
		int overlord_in_queue;                   //队列中领主
		int mutalisk_in_queue;                   //队列中飞龙
		int scourge_in_queue;                    //队列中自爆蚊
		int queen_in_queue;                      //队列中女王
		int guardian_in_queue;                   //队列中守卫者
		int devourer_in_queue;                   //队列中吞噬者

												 //己方单位总计
		int larva_total;						//幼虫
		int drone_total;						//工蜂
		int zergling_total;						//小狗
		int hydralisk_total;					//刺蛇
		int lurker_total;						//地刺
		int ultralisk_total;					//雷兽
		int defiler_total;						//蝎子
		int overlord_total;						//领主
		int mutalisk_total;						//飞龙
		int scourge_total;						//自爆蚊
		int queen_total;						//女王
		int guardian_total;						//守卫者
		int devourer_total;						//吞噬者

												// 科技
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

		//己方已建造建筑
		int base_count;							//基地
		int hatchery_count;						//母巢
		int lair_count;							//蜂房
		int hive_count;							//兽穴
		int extractor_count;                    //气矿
		int creep_colony_count;                 //殖体
		int sunken_colony_count;			    //地刺塔
		int spore_colony_count;			    	//孢子塔
		int spawning_pool_count;                //血池
		int hydralisk_den_count;                //刺蛇洞穴
		int queens_nest_count;                  //皇后巢
		int defiler_mound_count;                //蝎子巢
		int spire_count;                        //飞龙塔
		int greater_spire_count;                //大飞龙塔
		int nydus_canal_count;                  //蛹虫通道
		int ultralisk_cavern_count;             //巨兽之窟

												//己方已建成建筑
		int real_base_completed;					//实际意义上的基地
		int base_completed;							//基地
		int hatchery_completed;						//母巢
		int lair_completed;							//蜂房
		int hive_completed;							//兽穴
		int extractor_completed;                    //气矿
		int creep_colony_completed;                 //殖体
		int sunken_colony_completed;			    //地刺塔
		int spore_colony_completed;			    	//孢子塔
		int spawning_pool_completed;                //血池
		int hydralisk_den_completed;                //刺蛇洞穴
		int queens_nest_completed;                  //皇后巢
		int defiler_mound_completed;                //蝎子巢
		int spire_completed;                        //飞龙塔
		int greater_spire_completed;                //大飞龙塔
		int nydus_canal_completed;                  //蛹虫通道
		int ultralisk_cavern_completed;             //巨兽之窟

													//己方建设中建筑
		int base_being_built;						//基地
		int hatchery_being_built;					//母巢
		int lair_being_built;						//蜂房
		int hive_being_built;						//兽穴
		int extractor_being_built;                  //气矿
		int creep_colony_being_built;               //殖体
		int sunken_colony_being_built;			    //地刺塔
		int spore_colony_being_built;			   	//孢子塔
		int spawning_pool_being_built;              //血池
		int hydralisk_den_being_built;              //刺蛇洞穴
		int queens_nest_being_built;                //皇后巢
		int defiler_mound_being_built;              //蝎子巢
		int spire_being_built;                      //飞龙塔
		int greater_spire_being_built;              //大飞龙塔
		int nydus_canal_being_built;                //蛹虫通道
		int ultralisk_cavern_being_built;           //巨兽之窟

													//己方队列中建筑
		int base_in_queue;						//基地
		int hatchery_in_queue;					//母巢
		int lair_in_queue;						//蜂房
		int hive_in_queue;						//兽穴
		int extractor_in_queue;                 //气矿
		int creep_colony_in_queue;              //殖体
		int sunken_colony_in_queue;			    //地刺塔
		int spore_colony_in_queue;			    //孢子塔
		int spawning_pool_in_queue;             //血池
		int hydralisk_den_in_queue;             //刺蛇洞穴
		int queens_nest_in_queue;               //皇后巢
		int defiler_mound_in_queue;             //蝎子巢
		int spire_in_queue;                     //飞龙塔
		int greater_spire_in_queue;             //大飞龙塔
		int nydus_canal_in_queue;               //蛹虫通道
		int ultralisk_cavern_in_queue;          //巨兽之窟

												//己方建筑总计
		int base_total;							//基地
		int hatchery_total;						//母巢
		int lair_total;							//蜂房
		int hive_total;							//兽穴
		int extractor_total;                    //气矿
		int creep_colony_total;                 //殖体
		int sunken_colony_total;			    //地刺塔
		int spore_colony_total;			    	//孢子塔
		int spawning_pool_total;                //血池
		int hydralisk_den_total;                //刺蛇洞穴
		int queens_nest_total;                  //皇后巢
		int defiler_mound_total;                //蝎子巢
		int spire_total;                        //飞龙塔
		int greater_spire_total;                //大飞龙塔
		int nydus_canal_total;                  //蛹虫通道
		int ultralisk_cavern_total;             //巨兽之窟

												//军事力量
		int army_supply;
		int air_army_supply;
		int ground_army_supply;

		//判断和倾向
		bool being_rushed;

		//敌方
		int enemy_terran_unit_count;
		int enemy_protos_unit_count;
		int enemy_zerg_unit_count;

		//人族单位
		int enemy_marine_count;					//机枪兵
		int enemy_firebat_count;				//火焰兵
		int enemy_medic_count;					//医疗兵
		int enemy_ghost_count;					//幽灵
		int enemy_vulture_count;				//雷车
		int enemy_tank_count;					//坦克
		int enemy_goliath_count;				//机器人
		int enemy_wraith_count;					//隐飞
		int enemy_valkyrie_count;				//导弹护卫舰
		int enemy_bc_count;						//战列巡洋舰
		int enemy_science_vessel_count;			//科学球
		int enemy_dropship_count;				//运输机

												//人族建筑
		int enemy_bunker_count;					//地堡
		int enemy_barrack_count;				//兵营
		int enemy_factory_count;				//工厂
		int enemy_starport_count;				//飞机场

												//神族单位
		int enemy_zealot_count;					//狂热者
		int enemy_dragoon_count;				//龙骑
		int enemy_ht_count;						//光明圣堂
		int enemy_dt_count;						//黑暗圣堂
		int enemy_reaver_count;					//金甲虫
		int enemy_shuttle_count;				//运输机
		int enemy_carrier_count;				//航母
		int enemy_arbiter_count;				//仲裁者
		int enemy_corsair_count;				//海盗船
		int enemy_scout_count;					//侦察机
		int enemy_archon_count;					//光明执政官
		int enemy_dark_archon_count;			//黑暗执政官

												//神族建筑
		int enemy_cannon_count;					//光子炮塔
		int enemy_gateway_count;				//兵营
		int enemy_stargate_count;				//星门
		int enemy_robotics_facility_count;		//机械工厂

												//虫族单位
		int enemy_zergling_count;				//小狗
		int enemy_hydralisk_count;				//刺蛇
		int enemy_lurker_count;					//地刺
		int enemy_ultralisk_count;				//雷兽
		int enemy_defiler_count;				//蝎子
		int enemy_mutalisk_count;				//飞龙
		int enemy_queen_count;					//女王
		int enemy_scourge_count;				//自爆蚊
		int enemy_guardian_count;				//空中卫士
		int enemy_devourer_count;				//吞噬者

												//虫族建筑
		int enemy_spawning_pool_count;			//孵化池
		int enemy_hydralisk_den_count;			//刺蛇穴
		int enemy_evolution_chamber_count;		//进化腔
		int enemy_spire_count;					//飞龙塔

												//军事力量
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

		//通用建筑和单位
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
