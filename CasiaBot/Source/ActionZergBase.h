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
		virtual void tryAddInQueue(CasiaBot::ProductionQueue &queue, const CasiaBot::ProductionItem & item, bool priority = false) = 0;

	protected:
		//己方已生成单位
		int larva_count = 0;						//幼虫
		int drone_count = 0;						//工蜂
		int zergling_count = 0;						//小狗
		int hydralisk_count = 0;					//刺蛇
		int lurker_count = 0;						//地刺
		int ultralisk_count = 0;					//雷兽
		int defiler_count = 0;						//蝎子
		int overlord_count = 0;						//领主
		int mutalisk_count = 0;						//飞龙
		int scourge_count = 0;						//自爆蚊
		int queen_count = 0;						//女王
		int guardian_count = 0;						//守卫者
		int devourer_count = 0;						//吞噬者

		int zergling_completed = 0;
		int hydralisk_completed = 0;
		int lurker_completed = 0;
		int mutalisk_completed = 0;

		//己方队列中单位
		int larva_in_queue = 0;                      //队列中幼虫
		int drone_in_queue = 0;                      //队列中工蜂
		int zergling_in_queue = 0;                   //队列中小狗
		int hydralisk_in_queue = 0;                  //队列中刺蛇
		int lurker_in_queue = 0;                     //队列中地刺
		int ultralisk_in_queue = 0;                  //队列中雷兽
		int defiler_in_queue = 0;                    //队列中蝎子
		int overlord_in_queue = 0;                   //队列中领主
		int mutalisk_in_queue = 0;                   //队列中飞龙
		int scourge_in_queue = 0;                    //队列中自爆蚊
		int queen_in_queue = 0;                      //队列中女王
		int guardian_in_queue = 0;                   //队列中守卫者
		int devourer_in_queue = 0;                   //队列中吞噬者

		bool muscular_argument_completing;
		bool grooved_spines_completing;
		int metabolic_boost_count;
		int lurker_aspect_count;
		int adrenal_glands_count;

		//己方已建成建筑
		int real_base_count;				//实际意义上的基地
		int base_count;						//基地
		int hatchery_count;					//母巢
		int lair_count;						//蜂房
		int hive_count;						//兽穴
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

		//己方建设中建筑
		int base_being_built;						//基地
		int hatchery_being_built;					//母巢
		int lair_being_built;						//蜂房
		int hive_being_built;						//兽穴
		int extractor_being_built;                    //气矿
		int creep_colony_being_built;                 //殖体
		int sunken_colony_being_built;			    //地刺塔
		int spore_colony_being_built;			   	//孢子塔
		int spawning_pool_being_built;                //血池
		int hydralisk_den_being_built;                //刺蛇洞穴
		int queens_nest_being_built;                  //皇后巢
		int defiler_mound_being_built;                //蝎子巢
		int spire_being_built;                        //飞龙塔
		int greater_spire_being_built;                //大飞龙塔
		int nydus_canal_being_built;                  //蛹虫通道
		int ultralisk_cavern_being_built;             //巨兽之窟

		//己方队列中建筑
		int base_in_queue;						//基地
		int hatchery_in_queue;					//母巢
		int lair_in_queue;						//蜂房
		int hive_in_queue;						//兽穴
		int extractor_in_queue;                    //气矿
		int creep_colony_in_queue;                 //殖体
		int sunken_colony_in_queue;			    //地刺塔
		int spore_colony_in_queue;			    	//孢子塔
		int spawning_pool_in_queue;                //血池
		int hydralisk_den_in_queue;                //刺蛇洞穴
		int queens_nest_in_queue;                  //皇后巢
		int defiler_mound_in_queue;                //蝎子巢
		int spire_in_queue;                        //飞龙塔
		int greater_spire_in_queue;                //大飞龙塔
		int nydus_canal_in_queue;                  //蛹虫通道
		int ultralisk_cavern_in_queue;             //巨兽之窟


												//军事力量
		double army_supply = 0;
		double air_army_supply = 0;
		double ground_army_supply = 0;

		//判断和倾向
		bool opponent_has_expanded = false;
		bool can_expand = false;
		bool force_expand = false;
		bool being_rushed = false;
		bool is_attacking = false;
		bool is_defending = false;
		bool default_upgrade = false;

		//敌方
		int enemy_terran_unit_count = 0;
		int enemy_protos_unit_count = 0;
		int enemy_zerg_unit_count = 0;

		//人族单位
		int enemy_marine_count = 0;					//机枪兵
		int enemy_firebat_count = 0;				//火焰兵
		int enemy_medic_count = 0;					//医疗兵
		int enemy_ghost_count = 0;					//幽灵
		int enemy_vulture_count = 0;				//雷车
		int enemy_tank_count = 0;					//坦克
		int enemy_goliath_count = 0;				//机器人
		int enemy_wraith_count = 0;					//隐飞
		int enemy_valkyrie_count = 0;				//导弹护卫舰
		int enemy_bc_count = 0;						//战列巡洋舰
		int enemy_science_vessel_count = 0;			//科学球
		int enemy_dropship_count = 0;				//运输机

													//人族建筑
		int enemy_bunker_count = 0;					//地堡
		int enemy_barrack_count = 0;				//兵营
		int enemy_factory_count = 0;				//工厂
		int enemy_starport_count = 0;				//飞机场

													//神族单位
		int enemy_zealot_count = 0;					//狂热者
		int enemy_dragoon_count = 0;				//龙骑
		int enemy_ht_count = 0;						//光明圣堂
		int enemy_dt_count = 0;						//黑暗圣堂
		int enemy_reaver_count = 0;					//金甲虫
		int enemy_shuttle_count = 0;				//运输机
		int enemy_carrier_count = 0;				//航母
		int enemy_arbiter_count = 0;				//仲裁者
		int enemy_corsair_count = 0;				//海盗船
		int enemy_scout_count = 0;					//侦察机

													//神族建筑
		int enemy_cannon_count = 0;					//光子炮塔
		int enemy_gateway_count = 0;				//兵营
		int enemy_stargate_count = 0;				//星门
		int enemy_robotics_facility_count = 0;		//机械工厂

													//虫族单位
		int enemy_zergling_count = 0;				//小狗
		int enemy_hydralisk_count = 0;				//刺蛇
		int enemy_lurker_count = 0;					//地刺
		int enemy_ultralisk_count = 0;				//雷兽
		int enemy_defiler_count = 0;				//蝎子
		int enemy_mutalisk_count = 0;				//飞龙
		int enemy_queen_count = 0;					//女王

													//虫族建筑
		int enemy_spawning_pool_count = 0;			//孵化池
		int enemy_hydralisk_den_count = 0;			//刺蛇穴
		int enemy_evolution_chamber_count = 0;		//进化腔
		int enemy_spire_count = 0;					//飞龙塔

													//军事力量
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

		//通用建筑和单位
		int enemy_worker_count = 0;
		int enemy_gas_count = 0;

		// 通用成员变量
		bool isInitialized = false;
		int lastFrameCount = 0;
		int lastFrameMineralAmount = 0;
		int lastFrameGasAmount = 0;
		std::deque<int> mineralNetIncrease;
		std::deque<int> gasNetIncrease;
	};
}
