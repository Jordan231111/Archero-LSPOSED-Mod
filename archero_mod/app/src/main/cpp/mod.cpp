#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "And64InlineHook.hpp"

#if defined(MOD_ENABLE_DEBUG_LOGS) && MOD_ENABLE_DEBUG_LOGS
#include <android/log.h>
#define LOG_TAG "ArcheroMod"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else
#define LOGD(...) ((void)0)
#endif

// v7.9.1 IL2CPP RVAs. GetHeadShot/GetMiss are report-validated; the
// gold/material hooks are dump-derived candidates and can be toggled from
// archero_mod_config.txt without rebuilding.
namespace rva {
static constexpr uintptr_t EntityData_GetHeadShot = 0x4F7A2E8;
static constexpr uintptr_t EntityData_GetMiss = 0x4F6D280;
static constexpr uintptr_t TableTool_Weapon_weapon_get_Speed = 0x58E0F80;
static constexpr uintptr_t TableTool_Weapon_weapon_get_AttackSpeed = 0x58E0FB4;
static constexpr uintptr_t TableTool_Weapon_weapon_get_bThroughWall = 0x58E2290;
static constexpr uintptr_t TableTool_Weapon_weapon_get_bThroughInsideWall = 0x58E22D8;
static constexpr uintptr_t BulletTransmit_Init_Simple = 0x33402BC;
static constexpr uintptr_t BulletTransmit_Init_Full = 0x3340890;
static constexpr uintptr_t BulletTransmit_get_ThroughWall = 0x3341BB4;
static constexpr uintptr_t BulletBase_HitWall = 0x5FEF35C;
static constexpr uintptr_t BulletBase_TriggerEnter1_HitWallInternal = 0x5FF35E8;
static constexpr uintptr_t EntityBase_SetFlyWater = 0x4C1C294;
static constexpr uintptr_t EntityBase_GetFlyWater = 0x4C1C4C4;
static constexpr uintptr_t EntityBase_SetFlyStone = 0x4C1C418;
static constexpr uintptr_t EntityBase_GetOnCalCanMove = 0x4C1FD90;
static constexpr uintptr_t EntityBase_SetCollider = 0x4C23850;
static constexpr uintptr_t EntityBase_SetFlyAll = 0x4C23A34;
static constexpr uintptr_t EntityBase_CheckPos = 0x4C29A60;
static constexpr uintptr_t EntityBase_SelfMoveBy = 0x4C2979C;
static constexpr uintptr_t EntityBase_AddSkill = 0x4C320FC;
static constexpr uintptr_t EntityBase_ContainsSkill = 0x4C23308;
static constexpr uintptr_t EntityBase_AddInitSkills = 0x4C33C78;
static constexpr uintptr_t EntityHitCtrl_SetFlyOne = 0x53094B4;
static constexpr uintptr_t MoveControl_UpdateProgress = 0x5538124;
static constexpr uintptr_t AdCallbackControl_IsLoaded = 0x5897EB4;
static constexpr uintptr_t AdCallbackControl_Show = 0x589851C;
static constexpr uintptr_t AdCallbackControl_onClose = 0x5898D64;
static constexpr uintptr_t AdCallbackControl_onReward = 0x5899010;
static constexpr uintptr_t AdsRequestHelper_ALMaxRewardedDriver_isLoaded = 0x589F24C;
static constexpr uintptr_t AdsRequestHelper_ALMaxRewardedDriver_Show = 0x589F478;
static constexpr uintptr_t AdsRequestHelper_WrappedDriver_onClose = 0x58A1038;
static constexpr uintptr_t AdsRequestHelper_WrappedDriver_onReward = 0x58A1200;
static constexpr uintptr_t AdsRequestHelper_CombinedDriver_onClose = 0x58A32E0;
static constexpr uintptr_t AdsRequestHelper_CombinedDriver_onReward = 0x58A3490;
static constexpr uintptr_t AdsRequestHelper_CallbackRouter_onClose = 0x58A4090;
static constexpr uintptr_t AdsRequestHelper_CallbackRouter_onReward = 0x58A4758;
static constexpr uintptr_t AdsRequestHelper_WrappedAdapter_isLoaded = 0x58A4ABC;
static constexpr uintptr_t AdsRequestHelper_WrappedAdapter_Show = 0x58A4D14;
static constexpr uintptr_t AdsRequestHelper_WrappedAdapter_Show_Callback = 0x58A4E5C;
static constexpr uintptr_t AdsRequestHelper_WrappedAdapter_Show_Callback_Source = 0x58A4FB0;
static constexpr uintptr_t AdsRequestHelper_rewarded_high_eCPM_isLoaded = 0x589C404;
static constexpr uintptr_t AdsRequestHelper_rewarded_high_eCPM_show = 0x589C518;
static constexpr uintptr_t TableTool_PlayerCharacter_UpgradeModel_GetATKBase = 0x5904E60;
static constexpr uintptr_t TableTool_PlayerCharacter_UpgradeModel_GetHPMaxBase = 0x5905134;
static constexpr uintptr_t UnityEngine_Time_get_deltaTime = 0x84259B4;
static constexpr uintptr_t UnityEngine_Time_get_timeScale = 0x8425C64;
static constexpr uintptr_t UnityEngine_Time_set_timeScale = 0x8425C8C;
static constexpr uintptr_t EntityData_getGold = 0x4F69AA4;
static constexpr uintptr_t DropGold_OnGetHittedList = 0x34807F4;
static constexpr uintptr_t DropGold_OnGetDropDead = 0x3480994;
static constexpr uintptr_t GameLogic_GetPureGoldList = 0x58D328C;
static constexpr uintptr_t GameLogic_CanSaveGoldInRealTime = 0x58D5CDC;
static constexpr uintptr_t GameConfig_GetCoin1Wave = 0x58BC0B0;
static constexpr uintptr_t GameConfig_GetBoxDropGold = 0x58BC528;
static constexpr uintptr_t GameConfig_GetBoxChooseGold = 0x58BC580;
static constexpr uintptr_t DeadGoodMgr_StartDrop = 0x4DF40E4;
static constexpr uintptr_t DeadGoodMgr_GetGoldNum = 0x4DF4568;
static constexpr uintptr_t LocalSave_BattleIn_UpdateGold = 0x5A4CB30;
static constexpr uintptr_t LocalSave_BattleIn_GetGold = 0x5A4DFD0;
static constexpr uintptr_t BattleModuleData_AddGold_Float = 0x372D16C;
static constexpr uintptr_t BattleModuleData_AddGold_Int = 0x372D3CC;
static constexpr uintptr_t BattleModuleData_GetGold = 0x372D410;
static constexpr uintptr_t Drop_DropModel_GetGoldDropPercent = 0x58F4F70;
static constexpr uintptr_t Drop_DropModel_GetDropGold = 0x58F52E4;
static constexpr uintptr_t IStageLayerManager_GetEquipMaxDrop = 0x347BD20;
static constexpr uintptr_t IStageLayerManager_GetMPMaxDrop = 0x347BD28;
static constexpr uintptr_t IStageLayerManager_GetScrollMaxDrop = 0x347BDA8;
static constexpr uintptr_t IStageLayerManager_GetRuneStoneMaxDrop = 0x347BE18;
static constexpr uintptr_t IStageLayerManager_GetAdventureCoinsMaxDrop = 0x347BE88;
static constexpr uintptr_t IStageLayerManager_GetLoupeMaxDrop = 0x347BF60;
static constexpr uintptr_t IStageLayerManager_GetStoneMaxDrop = 0x347C0A0;
static constexpr uintptr_t IStageLayerManager_GetBloodStoneMaxDrop = 0x347C0A8;
static constexpr uintptr_t IStageLayerManager_GetSkillStoneMaxDrop = 0x347C180;
static constexpr uintptr_t IStageLayerManager_GetFetterBadgeMaxDrop = 0x347C258;
static constexpr uintptr_t IStageLayerManager_GetCommonItemMaxDrop = 0x347C330;
static constexpr uintptr_t IStageLayerManager_GetAct4thItemsMaxDrop = 0x347C408;
static constexpr uintptr_t IStageLayerManager_GetAct4thExchangeItemsMaxDrop = 0x347C4E0;
static constexpr uintptr_t IStageLayerManager_GetWishCoinMaxDrop = 0x347C5B8;
static constexpr uintptr_t IStageLayerManager_GetActivityPropMaxDrop = 0x347C628;
static constexpr uintptr_t IStageLayerManager_GetCookieMaxDrop = 0x347C700;
static constexpr uintptr_t IStageLayerManager_GetSoulStoneMaxDrop = 0x347C770;
static constexpr uintptr_t IStageLayerManager_GetHonorStoneMaxDrop = 0x347C778;
static constexpr uintptr_t IStageLayerManager_GetBoneMaxDrop = 0x347C850;
static constexpr uintptr_t IStageLayerManager_GetHornMaxDrop = 0x347C928;
static constexpr uintptr_t IStageLayerManager_GetSoulPointMaxDrop = 0x347C950;
static constexpr uintptr_t IStageLayerManager_GetMagicStoneMaxDrop = 0x347CA28;
static constexpr uintptr_t IStageLayerManager_GetDragonCoinMaxDrop = 0x347CB00;
static constexpr uintptr_t IStageLayerManager_GetStarLightStoneMaxDrop = 0x347CBD8;
static constexpr uintptr_t IStageLayerManager_GetModstoneMaxDrop = 0x347CCB0;
static constexpr uintptr_t IStageLayerManager_GetManorMatMaxDrop = 0x347CD88;
static constexpr uintptr_t IStageLayerManager_GetFountainUseMaxDrop = 0x347CE60;
static constexpr uintptr_t IStageLayerManager_GetFountainUpgradeMaxDrop = 0x347CF38;
static constexpr uintptr_t IStageLayerManager_GetEquipQuintessenceMaxDrop = 0x347CFA8;
static constexpr uintptr_t IStageLayerManager_GetChineseKnotMaxDrop = 0x347D080;
static constexpr uintptr_t IStageLayerManager_GetFirecrackerMaxDrop = 0x347D158;
static constexpr uintptr_t IStageLayerManager_GetPetLevelUpItemsMaxDrop = 0x347D230;
static constexpr uintptr_t IStageLayerManager_GetPetExchangeItemsMaxDrop = 0x347D308;
static constexpr uintptr_t IStageLayerManager_GetArtifactExchangeItemsMaxDrop = 0x347D3E0;
static constexpr uintptr_t IStageLayerManager_GetImprintLevelUpItemsMaxDrop = 0x347D4B8;
static constexpr uintptr_t IStageLayerManager_GetImprintExchangeItemsMaxDrop = 0x347D590;
static constexpr uintptr_t IStageLayerManager_GetImprintStoneItemsMaxDrop = 0x347D668;
static constexpr uintptr_t IStageLayerManager_GetPropMaxDropById = 0x347D740;
static constexpr uintptr_t IStageLayerManager_GetWingLevelUpItemsMaxDrop = 0x347D818;
static constexpr uintptr_t IStageLayerManager_GetAct5DonateItemsMaxDrop = 0x347D8F0;
static constexpr uintptr_t IStageLayerManager_GetNewPlay125BagCoinMaxDrop = 0x347D960;
static constexpr uintptr_t IStageLayerManager_GetShipUpgradeMaxDrop = 0x347D9D0;
static constexpr uintptr_t SailingBagBattleStageLayerManager_GetNewPlay125BagCoinMaxDrop = 0x347F380;
static constexpr uintptr_t SailingBagBattleStageLayerManager_GetCommonItemMaxDrop = 0x347F61C;
static constexpr uintptr_t SailingBagBattleStageLayerManager_GetShipUpgradeMaxDrop = 0x347FEA8;
static constexpr uintptr_t StageLevelManager_GetGoldDropPercent = 0x5BAA8A4;
static constexpr uintptr_t StageLevelManager_GetFreeGold = 0x5BAD2EC;
static constexpr uintptr_t StageLevelManager_GetEquipMaxDrop = 0x5BAAADC;
static constexpr uintptr_t StageLevelManager_GetMPMaxDrop = 0x5BAAB90;
static constexpr uintptr_t StageLevelManager_GetScrollMaxDrop = 0x5BAADE4;
static constexpr uintptr_t StageLevelManager_GetRuneStoneMaxDrop = 0x5BAB064;
static constexpr uintptr_t StageLevelManager_GetActivityPropMaxDrop = 0x5BAB2E4;
static constexpr uintptr_t StageLevelManager_GetCookieMaxDrop = 0x5BAB638;
static constexpr uintptr_t StageLevelManager_GetSoulStoneMaxDrop = 0x5BAB6F4;
static constexpr uintptr_t StageLevelManager_GetHonorStoneMaxDrop = 0x5BAB6FC;
static constexpr uintptr_t StageLevelManager_GetBoneMaxDrop = 0x5BAB7D4;
static constexpr uintptr_t StageLevelManager_GetHornMaxDrop = 0x5BAB8AC;
static constexpr uintptr_t StageLevelManager_GetStoneMaxDrop = 0x5BAC23C;
static constexpr uintptr_t StageLevelManager_GetBloodStoneMaxDrop = 0x5BAC2F0;
static constexpr uintptr_t StageLevelManager_GetFetterBadgeMaxDrop = 0x5BAC63C;
static constexpr uintptr_t StageLevelManager_GetAct4thItemsMaxDrop = 0x5BAC890;
static constexpr uintptr_t StageLevelManager_GetAct4thExchangeItemsMaxDrop = 0x5BACAE4;
static constexpr uintptr_t StageLevelManager_GetMagicStoneMaxDrop = 0x5BADB60;
static constexpr uintptr_t StageLevelManager_GetStarLightStoneMaxDrop = 0x5BADE98;
static constexpr uintptr_t StageLevelManager_GetFountainUseMaxDrop = 0x5BAE2EC;
static constexpr uintptr_t StageLevelManager_GetFountainUpgradeMaxDrop = 0x5BAE540;
static constexpr uintptr_t StageLevelManager_GetChineseKnotMaxDrop = 0x5BAE894;
static constexpr uintptr_t StageLevelManager_GetFirecrackerMaxDrop = 0x5BAEBE8;
static constexpr uintptr_t StageLevelManager_GetLoupeMaxDrop = 0x5BAED70;
static constexpr uintptr_t StageLevelManager_GetEquipQuintessenceMaxDrop = 0x5BAEF44;
static constexpr uintptr_t StageLevelManager_GetPetLevelUpItemsMaxDrop = 0x5BAF2D0;
static constexpr uintptr_t StageLevelManager_GetPetExchangeItemsMaxDrop = 0x5BAF608;
static constexpr uintptr_t StageLevelManager_GetArtifactExchangeItemsMaxDrop = 0x5BAF99C;
static constexpr uintptr_t StageLevelManager_GetImprintLevelUpItemsMaxDrop = 0x5BAFCD4;
static constexpr uintptr_t StageLevelManager_GetImprintExchangeItemsMaxDrop = 0x5BB000C;
static constexpr uintptr_t StageLevelManager_GetImprintStoneItemsMaxDrop = 0x5BB0344;
static constexpr uintptr_t StageLevelManager_GetWingLevelUpItemsMaxDrop = 0x5BB067C;
static constexpr uintptr_t StageLevelManager_GetAct5DonateItemsMaxDrop = 0x5BB09D0;
static constexpr uintptr_t StageLevelManager_GetPropMaxDropById = 0x5BB0E74;
static constexpr uintptr_t CampBattleStageLayerManager_GetEquipMaxDrop = 0x5B5EAA8;
static constexpr uintptr_t CampBattleStageLayerManager_GetStoneMaxDrop = 0x5B5EC70;
static constexpr uintptr_t CampBattleStageLayerManager_GetSkillStoneMaxDrop = 0x5B5EE38;

static constexpr uintptr_t DailyStageChapter_GetScrollMaxDrop = 0x50DD2B8;
static constexpr uintptr_t DailyStageChapter_GetAdventureCoinsMaxDrop = 0x50DD884;
static constexpr uintptr_t DailyStageChapter_GetLoupeMaxDrop = 0x50DDC38;
static constexpr uintptr_t DailyStageChapter_GetBoneMaxDrop = 0x50DDFD4;
static constexpr uintptr_t DailyStageChapter_GetHornMaxDrop = 0x50DE870;
static constexpr uintptr_t DailyStageChapter_GetRuneStoneMaxDrop = 0x50DE96C;
static constexpr uintptr_t DailyStageChapter_GetActivityPropMaxDrop = 0x50DEC18;
static constexpr uintptr_t DailyStageChapter_GetStoneMaxDrop = 0x50DED14;
static constexpr uintptr_t DailyStageChapter_GetCookieMaxDrop = 0x50DEE10;
static constexpr uintptr_t DailyStageChapter_GetSoulStoneMaxDrop = 0x50DEF0C;
static constexpr uintptr_t DailyStageChapter_GetHonorStoneMaxDrop = 0x50DF008;
static constexpr uintptr_t DailyStageChapter_GetEquipMaxDrop = 0x50DF4D4;
static constexpr uintptr_t DailyStageChapter_GetBloodStoneMaxDrop = 0x50DFE90;
static constexpr uintptr_t DailyStageChapter_GetFetterBadgeMaxDrop = 0x50E0224;
static constexpr uintptr_t DailyStageChapter_GetAct4thItemsMaxDrop = 0x50E04C0;
static constexpr uintptr_t DailyStageChapter_GetAct4thExchangeItemsMaxDrop = 0x50E075C;
static constexpr uintptr_t DailyStageChapter_GetWishCoinMaxDrop = 0x50E09F8;
static constexpr uintptr_t DailyStageChapter_GetMagicStoneMaxDrop = 0x50E0D78;
static constexpr uintptr_t DailyStageChapter_GetDragonCoinMaxDrop = 0x50E10F8;
static constexpr uintptr_t DailyStageChapter_GetModstoneMaxDrop = 0x50E1494;
static constexpr uintptr_t DailyStageChapter_GetManorMatMaxDrop = 0x50E1814;
static constexpr uintptr_t DailyStageChapter_GetFountainUseMaxDrop = 0x50E1CB0;
static constexpr uintptr_t DailyStageChapter_GetFountainUpgradeMaxDrop = 0x50E1F4C;
static constexpr uintptr_t DailyStageChapter_GetCommonItemMaxDrop = 0x50E23D8;
static constexpr uintptr_t DailyStageChapter_GetEquipQuintessenceMaxDrop = 0x50E26A0;
static constexpr uintptr_t DailyStageChapter_GetChineseKnotMaxDrop = 0x50E2A3C;
static constexpr uintptr_t DailyStageChapter_GetFirecrackerMaxDrop = 0x50E2DD8;
static constexpr uintptr_t DailyStageChapter_GetPetLevelUpItemsMaxDrop = 0x50E31AC;
static constexpr uintptr_t DailyStageChapter_GetPetExchangeItemsMaxDrop = 0x50E352C;
static constexpr uintptr_t DailyStageChapter_GetArtifactExchangeItemsMaxDrop = 0x50E3908;
static constexpr uintptr_t DailyStageChapter_GetImprintLevelUpItemsMaxDrop = 0x50E3C88;
static constexpr uintptr_t DailyStageChapter_GetImprintExchangeItemsMaxDrop = 0x50E4008;
static constexpr uintptr_t DailyStageChapter_GetImprintStoneItemsMaxDrop = 0x50E4388;
static constexpr uintptr_t DailyStageChapter_GetWingLevelUpItemsMaxDrop = 0x50E4708;
static constexpr uintptr_t DailyStageChapter_GetNewPlay125BagCoinMaxDrop = 0x50E49D0;
static constexpr uintptr_t DailyStageChapter_GetPropMaxDropById = 0x50E4D20;

static constexpr uintptr_t GameModeBase_GetGoldRatio = 0x5BF9F88;
static constexpr uintptr_t GameModeBase_GetDropDataGold = 0x5BFB464;
static constexpr uintptr_t GameModeCooperation_GetGoldRatio = 0x5BFF508;
static constexpr uintptr_t GameModeCooperation_GetDropDataGold = 0x5BFFB30;
static constexpr uintptr_t GameModeCooperationPVP_GetGoldRatio = 0x5C01B2C;
static constexpr uintptr_t GameModeCooperationPVP_GetDropDataGold = 0x5C02154;
static constexpr uintptr_t GameModeDaily_GetGoldRatio = 0x5C04134;
static constexpr uintptr_t GameModeDaily_GetDropDataGold = 0x5C05334;
static constexpr uintptr_t GameModeGold1_GetGoldRatio = 0x5C0B1F8;
static constexpr uintptr_t GameModeGold1_GetDropDataGold = 0x5C0BA04;
static constexpr uintptr_t GameModeLevel_GetGoldRatio = 0x5C0C764;
static constexpr uintptr_t GameModeLevel_GetDropDataGold = 0x5C0D140;
static constexpr uintptr_t GameModeMainChallenge_GetGoldRatio = 0x5C0E13C;
static constexpr uintptr_t GameModeMainChallenge_GetDropDataGold = 0x5C0E9F8;
static constexpr uintptr_t GameModeMeadowBattle_GetGoldRatio = 0x5C0EBE4;
static constexpr uintptr_t GameModeMeadowBattle_GetDropDataGold = 0x5C0ED20;
static constexpr uintptr_t GameModeTower_GetGoldRatio = 0x5C1112C;
static constexpr uintptr_t GameModeTower_GetDropDataGold = 0x5C11CA8;
static constexpr uintptr_t GameModeTryPlay_GetGoldRatio = 0x5C13FDC;
static constexpr uintptr_t GameModeTryPlay_GetDropDataGold = 0x5C14604;
static constexpr uintptr_t GameModeBase_GetAdventureCoinMaxDrop = 0x5BFA72C;
static constexpr uintptr_t GameModeBase_GetNewPlay125BagCoinMaxDrop = 0x5BFB2BC;
static constexpr uintptr_t GameModeCooperation_GetAdventureCoinMaxDrop = 0x5C0113C;
static constexpr uintptr_t GameModeCooperationPVP_GetAdventureCoinMaxDrop = 0x5C03148;
static constexpr uintptr_t GameModeDaily_GetAdventureCoinMaxDrop = 0x5C04734;
static constexpr uintptr_t GameModeGold1_GetAdventureCoinMaxDrop = 0x5C0B390;
static constexpr uintptr_t GameModeLevel_GetAdventureCoinMaxDrop = 0x5C0C9AC;
static constexpr uintptr_t GameModeMainChallenge_GetAdventureCoinMaxDrop = 0x5C0E3A4;
static constexpr uintptr_t GameModeSailingBagBattle_GetNewPlay125BagCoinMaxDrop = 0x5C0F364;
static constexpr uintptr_t GameModeTower_GetAdventureCoinMaxDrop = 0x5C113AC;

static constexpr uintptr_t TableTool_DailyChapter_get_AdventureCoinRateMax = 0x5620F10;
static constexpr uintptr_t TableTool_DailyChapter_get_BagCoinMax = 0x5623058;
static constexpr uintptr_t TableTool_PVEStage_get_GoldMax = 0x56E31EC;
static constexpr uintptr_t TableTool_PVEStage_get_HardGoldMax = 0x56E35C8;
static constexpr uintptr_t TableTool_ShipStage_get_BagCoinMax = 0x570E8E4;
static constexpr uintptr_t TableTool_SLGStage_get_GoldMax = 0x5733358;
static constexpr uintptr_t TableTool_SLGBaseLevel_get_GoldMax = 0x5734804;
static constexpr uintptr_t TableTool_TowerDefense_get_GoldMAX = 0x5865ACC;

static constexpr uintptr_t DropManager_GetRandomLevel = 0x5B60B00;
static constexpr uintptr_t DropManager_GetRandomGoldHitted = 0x5B6AAC0;
static constexpr uintptr_t DropManager_GetActivityProp = 0x5B65C9C;
static constexpr uintptr_t DropManager_GetStone = 0x5B63320;
static constexpr uintptr_t DropManager_GetBloodStone = 0x5B635D8;
static constexpr uintptr_t DropManager_GetRandomFetterBadge = 0x5B63980;
static constexpr uintptr_t DropManager_GetRandomSkillStone = 0x5B63C38;
static constexpr uintptr_t DropManager_GetWishCoin = 0x5B653B8;
static constexpr uintptr_t DropManager_GetModstone = 0x5B65724;
static constexpr uintptr_t DropManager_GetCommonItem = 0x5B659DC;
static constexpr uintptr_t DropManager_GetDropMat = 0x5B69F68;
static constexpr uintptr_t DropManager_GetRuneStone = 0x5B63EF0;
static constexpr uintptr_t DropManager_GetCookie = 0x5B641D8;
static constexpr uintptr_t DropManager_GetAdventureCoin = 0x5B65108;
static constexpr uintptr_t DropManager_GetLoupeDrops = 0x5B66034;
static constexpr uintptr_t DropManager_GetManorMat = 0x5B6A22C;
static constexpr uintptr_t DropManager_GetSoulStone = 0x5B64544;
static constexpr uintptr_t DropManager_GetBone = 0x5B66DD0;
static constexpr uintptr_t DropManager_GetHorn = 0x5B67140;
static constexpr uintptr_t DropManager_GetRandomEquipExp = 0x5B61470;
static constexpr uintptr_t DropManager_GetRandomMagicStone = 0x5B674B0;
static constexpr uintptr_t DropManager_GetRandomDragonCoin = 0x5B67778;
static constexpr uintptr_t DropManager_GetRandomStarLightStone = 0x5B67A40;
static constexpr uintptr_t DropManager_GetRandomEquip = 0x5B618B4;
static constexpr uintptr_t DropManager_GetDropIds = 0x5B647FC;
static constexpr uintptr_t DropManager_GetDropList = 0x5B6D514;
static constexpr uintptr_t DropManager_GetEquipQuintessence = 0x5B6A4F4;
static constexpr uintptr_t DropManager_GetNewPlay125BagCoin = 0x5B6A7CC;
static constexpr uintptr_t DropManager_getTotalCnt_WithCondition = 0x5B6DDD4;
static constexpr uintptr_t DropManager_getTotalCnt_WithEquipOne = 0x5B6DF94;
static constexpr uintptr_t DropManager_CanGetMoreDrop = 0x5B6AC84;
static constexpr uintptr_t DropManager_AppendDropCount = 0x5B6C7EC;
static constexpr uintptr_t LocalSave_BattleInBase_CanDropEquip = 0x5AC5A80;
static constexpr uintptr_t LocalSave_BattleIn_CanDropEquip = 0x5A4FA98;
static constexpr uintptr_t LocalSave_get_CanDropType501Equip = 0x5A5B448;
static constexpr uintptr_t LocalSave_get_CanDropType401Or402Equip = 0x5A5B03C;
static constexpr uintptr_t LocalSave_get_CanDropFirstEquip = 0x5A5B614;
}

static constexpr uintptr_t kIl2CppObjectHeaderSize = 0x10;
static constexpr int32_t kMoveProgressMaxSubsteps = 20;
static constexpr float kMoveProgressFractionEpsilon = 0.05f;

static uintptr_t g_off_entity_data_entity = 0;
static uintptr_t g_off_entity_data_fly_stone_count = 0;
static uintptr_t g_off_entity_data_fly_water_count = 0;
static uintptr_t g_off_entity_base_data = 0;
static uintptr_t g_off_entity_base_type = 0;
static uintptr_t g_off_entity_base_fly_water = 0;
static uintptr_t g_off_entity_base_fly_stone = 0;
static uintptr_t g_off_entity_base_hit_ctrl = 0;
static uintptr_t g_off_entity_base_move_layer_mask = 0;
static uintptr_t g_off_entity_hit_ctrl_entity = 0;
static uintptr_t g_off_bullet_transmit_entity = 0;
static uintptr_t g_off_bullet_transmit_weapon_data = 0;
static uintptr_t g_off_bullet_transmit_through_wall = 0;
static uintptr_t g_off_weapon_weapon_through_wall = 0;
static uintptr_t g_off_weapon_weapon_through_inside_wall = 0;
static uintptr_t g_off_bullet_base_entity = 0;
static uintptr_t g_off_bullet_base_weapon_data = 0;
static uintptr_t g_off_bullet_base_transmit = 0;
static uintptr_t g_off_move_control_entity = 0;
static uintptr_t g_off_move_control_moving = 0;
static uintptr_t g_off_move_control_move_direction = 0;
static uintptr_t g_off_adcallback_b_callback = 0;
static uintptr_t g_off_adcallback_b_opened = 0;
static uintptr_t g_off_base_driver_callback = 0;
static uintptr_t g_off_wrapped_adapter_callbacks = 0;
static uintptr_t g_off_obscured_vector3_key = 0;
static uintptr_t g_off_obscured_vector3_hidden = 0;
static uintptr_t g_off_obscured_vector3_inited = 0;
static uintptr_t g_off_obscured_vector3_fake = 0;
static uintptr_t g_off_obscured_vector3_fake_active = 0;

static constexpr int kEntityTypeHero = 1;
static constexpr int32_t kSkillWalkThroughWater = 2080;
static constexpr int32_t kSkillGreed = 1000040;
static constexpr int32_t kSkillSmart = 1000041;
static constexpr int32_t kAlwaysOnDamageValue = 2000000000;
static constexpr int32_t kAlwaysOnHealthValue = 2000000000;
static constexpr float kAlwaysOnAttackSpeedValue = 100.0f;
static constexpr int kIl2CppRuntimeMinSettleMs = 2000;
static constexpr int kIl2CppMetadataPollMs = 250;
static constexpr int kIl2CppMetadataTimeoutMs = 30000;

static volatile bool g_enable_headshot = true;
static volatile bool g_enable_godmode = true;
static volatile bool g_enable_damage_v1 = true;
static volatile bool g_enable_health = true;
static volatile bool g_enable_attack_speed = true;
static volatile bool g_enable_shoot_through_walls = true;
static volatile bool g_enable_walk_through_water = true;
static volatile bool g_enable_walk_through_walls = true;
static volatile bool g_enable_inject_greed_skill = true;
static volatile bool g_enable_inject_smart_skill = true;
static volatile bool g_enable_game_speed = true;
static volatile bool g_enable_move_speed = true;
static volatile bool g_skip_rewarded_ads = true;
static volatile bool g_install_gold_hooks = false;
static volatile bool g_gold_hooks_installed = false;
static volatile bool g_tiny_direct_patch = false;
static volatile bool g_gold_add_scale = false;
static volatile bool g_gold_get_fixed = false;
static volatile bool g_gold_get_scale = false;
static volatile bool g_gold_update_fixed = false;
static volatile bool g_gold_update_scale = false;
static volatile bool g_gold_formula_scale = false;
static volatile bool g_gold_static_scale = false;
static volatile bool g_gold_drop_scalar = false;
static volatile bool g_gold_drop_repeat = false;
static volatile bool g_gold_ratio_scale = false;
static volatile bool g_gold_list_scale = false;
static volatile bool g_gold_save_realtime = false;
static volatile bool g_material_drop_repeat = false;
static volatile bool g_max_drop_cap_patch = false;
static volatile float g_gold_multiplier = 2.0f;
static volatile float g_stage_gold_fixed = 0.0f;
static volatile int g_gold_drop_repeats = 1;
static volatile int g_material_drop_repeats = 2;
static volatile int g_max_drop_cap_value = 65535;
static volatile int g_max_gold_cap_value = 2000000000;
static volatile float g_game_speed_multiplier = 4.0f;
static volatile float g_move_speed_multiplier = 1.0f;

static constexpr int kRepeatCapMax = 50;
static uintptr_t g_il2cpp_base = 0;
static volatile bool g_il2cpp_metadata_ready = false;
static volatile bool g_startup_hooks_ready = false;
static volatile bool g_field_offsets_ready = false;
static volatile int g_il2cpp_metadata_wait_ms = 0;
static char g_last_config_path[256] = "none";

static volatile uint64_t g_config_loads = 0;
static volatile uint64_t g_status_writes = 0;
static volatile uint64_t g_hook_installed_count = 0;
static volatile uint64_t g_hook_skipped_tiny_count = 0;
static volatile uint64_t g_resolve_metadata_count = 0;
static volatile uint64_t g_resolve_rva_count = 0;
static volatile uint64_t g_resolve_fail_count = 0;
static volatile uint64_t g_field_resolve_metadata_count = 0;
static volatile uint64_t g_field_resolve_fail_count = 0;
static volatile uint64_t g_direct_patch_resolved_count = 0;
static volatile uint64_t g_direct_patch_write_count = 0;
static volatile uint64_t g_direct_patch_fail_count = 0;
static volatile uint64_t g_hit_add_gold = 0;
static volatile uint64_t g_hit_stage_balance = 0;
static volatile uint64_t g_hit_entity_formula = 0;
static volatile uint64_t g_hit_static_gold = 0;
static volatile uint64_t g_hit_drop_scalar = 0;
static volatile uint64_t g_hit_deadgood_startdrop = 0;
static volatile uint64_t g_hit_list_gold_scaled = 0;
static volatile uint64_t g_hit_list_material_scaled = 0;
static volatile uint64_t g_hit_drop_mutator_gold = 0;
static volatile uint64_t g_hit_drop_mutator_material = 0;
static volatile uint64_t g_hit_ratio = 0;
static volatile uint64_t g_hit_max_drop = 0;
static volatile uint64_t g_hit_total_counter_relax = 0;
static volatile uint64_t g_hit_always_damage = 0;
static volatile uint64_t g_hit_always_health = 0;
static volatile uint64_t g_hit_always_attack_speed = 0;
static volatile uint64_t g_hit_always_walls = 0;
static volatile uint64_t g_hit_always_inside_walls = 0;
static volatile uint64_t g_hit_runtime_walls_apply = 0;
static volatile uint64_t g_hit_runtime_walls_init = 0;
static volatile uint64_t g_hit_weapon_wall_field_apply = 0;
static volatile uint64_t g_hit_bullet_transmit_wall_get = 0;
static volatile uint64_t g_hit_bullet_hitwall_bypass = 0;
static volatile uint64_t g_hit_bullet_hitwall_internal_bypass = 0;
static volatile uint64_t g_hit_move_progress_get = 0;
static volatile uint64_t g_hit_move_progress_apply = 0;
static volatile uint64_t g_hit_move_progress_hero = 0;
static volatile uint64_t g_hit_move_progress_passthrough = 0;
static volatile uint64_t g_hit_move_progress_substeps = 0;
static volatile uint64_t g_hit_walk_water = 0;
static volatile uint64_t g_hit_walk_wall = 0;
static volatile uint64_t g_hit_walk_apply = 0;
static volatile uint64_t g_hit_walk_entitydata_apply = 0;
static volatile uint64_t g_hit_walk_mask_apply = 0;
static volatile uint64_t g_hit_walk_runtime_apply = 0;
static volatile uint64_t g_hit_walk_skill_inject = 0;
static volatile uint64_t g_hit_walk_check_pos = 0;
static volatile uint64_t g_hit_skill_inject_greed = 0;
static volatile uint64_t g_hit_skill_inject_smart = 0;
static volatile uint64_t g_hit_skill_inject_fail = 0;
static volatile uint64_t g_hit_skill_confirm_water = 0;
static volatile uint64_t g_hit_skill_confirm_greed = 0;
static volatile uint64_t g_hit_skill_confirm_smart = 0;
static volatile uint64_t g_hit_skill_confirm_fail = 0;
static volatile uint64_t g_hit_skill_confirm_unavailable = 0;
static volatile uint64_t g_hit_game_speed_get = 0;
static volatile uint64_t g_hit_game_speed_set = 0;
static volatile uint64_t g_hit_game_speed_apply = 0;
static volatile uint64_t g_hit_ad_skip_isloaded = 0;
static volatile uint64_t g_hit_ad_skip_show = 0;
static volatile uint64_t g_hit_ad_skip_high_ecpm = 0;
static volatile uint64_t g_hit_ad_skip_adapter = 0;
static volatile uint64_t g_hit_ad_skip_driver = 0;
static volatile uint64_t g_hit_ad_skip_reward = 0;
static volatile uint64_t g_hit_ad_skip_close = 0;
static volatile uint64_t g_hit_ad_skip_passthrough = 0;
static volatile float g_last_move_progress_speed = 0.0f;
static volatile float g_last_move_progress_scaled = 0.0f;
static volatile int32_t g_last_move_progress_steps = 0;
static volatile bool g_default_config_created = false;
static char g_last_resolve_error[192] = "none";
static char g_last_metadata_state[256] = "none";
static char g_last_field_resolve_state[256] = "none";

enum FoodTypeValue {
    FoodType_Gold = 102,
    FoodType_Exp = 103,
    FoodType_PureGold = 107,
    FoodType_TDStone = 108,
    FoodType_SLGTalent = 109,
    FoodType_SLGCampTalent = 110,
    FoodType_NewPlay126Stone = 113,
};

struct Il2CppObjectLite {
    void* klass;
    void* monitor;
};

struct BattleDropDataLite {
    void* klass;
    void* monitor;
    int32_t type;
    int32_t childtype;
    Il2CppObjectLite* data;
};

struct BattleDropDataArrayLite {
    void* klass;
    void* monitor;
    void* bounds;
    uintptr_t max_length;
    BattleDropDataLite* vector[1];
};

struct ListBattleDropDataLite {
    void* klass;
    void* monitor;
    BattleDropDataArrayLite* items;
    int32_t size;
    int32_t version;
    void* syncRoot;
};

struct Vector3Lite {
    float x;
    float y;
    float z;
};

struct ObscuredVector3Snapshot {
    int32_t key;
    int32_t hidden_x;
    int32_t hidden_y;
    int32_t hidden_z;
    uint8_t inited;
    Vector3Lite fake;
    uint8_t fake_active;
};

struct Patch8State {
    uintptr_t rva;
    uint32_t original0;
    uint32_t original1;
    bool saved;
    bool applied;
    uintptr_t target;
};

static Patch8State g_patch_dead_good_gold = {rva::DeadGoodMgr_GetGoldNum, 0, 0, false, false};
static Patch8State g_patch_istage_max_caps[] = {
    {rva::IStageLayerManager_GetEquipMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetMPMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetScrollMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetRuneStoneMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetAdventureCoinsMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetLoupeMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetStoneMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetBloodStoneMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetSkillStoneMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetFetterBadgeMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetCommonItemMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetAct4thItemsMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetAct4thExchangeItemsMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetWishCoinMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetActivityPropMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetCookieMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetSoulStoneMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetHonorStoneMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetBoneMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetHornMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetSoulPointMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetMagicStoneMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetDragonCoinMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetStarLightStoneMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetModstoneMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetManorMatMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetFountainUseMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetFountainUpgradeMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetEquipQuintessenceMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetChineseKnotMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetFirecrackerMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetPetLevelUpItemsMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetPetExchangeItemsMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetArtifactExchangeItemsMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetImprintLevelUpItemsMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetImprintExchangeItemsMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetImprintStoneItemsMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetPropMaxDropById, 0, 0, false, false},
    {rva::IStageLayerManager_GetWingLevelUpItemsMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetAct5DonateItemsMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetNewPlay125BagCoinMaxDrop, 0, 0, false, false},
    {rva::IStageLayerManager_GetShipUpgradeMaxDrop, 0, 0, false, false},
};
static Patch8State g_patch_stagelevel_max_caps[] = {
    {rva::StageLevelManager_GetEquipMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetMPMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetScrollMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetRuneStoneMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetActivityPropMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetCookieMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetSoulStoneMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetHonorStoneMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetBoneMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetHornMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetStoneMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetBloodStoneMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetFetterBadgeMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetAct4thItemsMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetAct4thExchangeItemsMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetMagicStoneMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetStarLightStoneMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetFountainUseMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetFountainUpgradeMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetChineseKnotMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetFirecrackerMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetLoupeMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetEquipQuintessenceMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetPetLevelUpItemsMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetPetExchangeItemsMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetArtifactExchangeItemsMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetImprintLevelUpItemsMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetImprintExchangeItemsMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetImprintStoneItemsMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetWingLevelUpItemsMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetAct5DonateItemsMaxDrop, 0, 0, false, false},
    {rva::StageLevelManager_GetPropMaxDropById, 0, 0, false, false},
};
static Patch8State g_patch_total_count_zero[] = {
    {rva::DropManager_getTotalCnt_WithCondition, 0, 0, false, false},
    {rva::DropManager_getTotalCnt_WithEquipOne, 0, 0, false, false},
};
static Patch8State g_patch_total_gate_true[] = {
    {rva::DropManager_CanGetMoreDrop, 0, 0, false, false},
    {rva::LocalSave_BattleInBase_CanDropEquip, 0, 0, false, false},
    {rva::LocalSave_BattleIn_CanDropEquip, 0, 0, false, false},
    {rva::LocalSave_get_CanDropType501Equip, 0, 0, false, false},
    {rva::LocalSave_get_CanDropType401Or402Equip, 0, 0, false, false},
    {rva::LocalSave_get_CanDropFirstEquip, 0, 0, false, false},
};
static Patch8State g_patch_append_drop_count = {rva::DropManager_AppendDropCount, 0, 0, false, false};
static Patch8State g_patch_meadow_gold_ratio = {rva::GameModeMeadowBattle_GetGoldRatio, 0, 0, false, false};
static Patch8State g_patch_meadow_drop_gold = {rva::GameModeMeadowBattle_GetDropDataGold, 0, 0, false, false};
static Patch8State g_patch_tryplay_gold_ratio = {rva::GameModeTryPlay_GetGoldRatio, 0, 0, false, false};
static Patch8State g_patch_tryplay_drop_gold = {rva::GameModeTryPlay_GetDropDataGold, 0, 0, false, false};

static uintptr_t get_base_address(const char *name) {
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f) return 0;
    char line[512];
    uintptr_t base = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, name)) {
            sscanf(line, "%lx", &base);
            break;
        }
    }
    fclose(f);
    return base;
}

static bool parse_bool_value(const char* value) {
    if (!value) return false;
    while (*value == ' ' || *value == '\t') value++;
    return !(strcmp(value, "0") == 0 ||
             strcasecmp(value, "false") == 0 ||
             strcasecmp(value, "off") == 0 ||
             strcasecmp(value, "no") == 0);
}

static int clamp_repeat(int value) {
    if (value < 1) return 1;
    if (value > kRepeatCapMax) return kRepeatCapMax;
    return value;
}

static int clamp_max_drop_cap(int value) {
    if (value < 1) return 1;
    if (value > 65535) return 65535;
    return value;
}

static int32_t clamp_max_gold_cap(long long value) {
    if (value < 1) return 1;
    if (value > 2147483647LL) return 2147483647;
    return static_cast<int32_t>(value);
}

static float clamp_multiplier(float value) {
    if (!isfinite(value) || value < 0.0f) return 1.0f;
    if (value > 100000.0f) return 100000.0f;
    return value;
}

static float clamp_game_speed(float value) {
    if (!isfinite(value) || value < 0.1f) return 1.0f;
    if (value > 20.0f) return 20.0f;
    return value;
}

static float clamp_move_speed(float value) {
    if (!isfinite(value) || value < 0.1f) return 1.0f;
    if (value > 20.0f) return 20.0f;
    return value;
}

static int32_t scale_int32(int32_t value, float multiplier) {
    if (value <= 0) return value;
    double scaled = static_cast<double>(value) * static_cast<double>(clamp_multiplier(multiplier));
    if (scaled > 2147483647.0) return 2147483647;
    if (scaled < 0.0) return value;
    return static_cast<int32_t>(scaled);
}

static float scale_float(float value, float multiplier) {
    if (!isfinite(value) || value <= 0.0f) return value;
    double scaled = static_cast<double>(value) * static_cast<double>(clamp_multiplier(multiplier));
    if (scaled > 2147483647.0) return 2147483647.0f;
    return static_cast<float>(scaled);
}

static void bump(volatile uint64_t& counter, uint64_t amount = 1) {
    counter += amount;
}

struct ModuleSegment {
    uintptr_t start;
    uintptr_t end;
    bool readable;
    bool executable;
};

struct HookSpec {
    uintptr_t dump_rva;
    const char* namespaze;
    const char* klass;
    const char* method;
    int arg_count;
    const char* first_param_type;
};

struct Il2CppApi {
    bool resolved;
    void* handle;
    void* (*domain_get)();
    void* (*thread_attach)(void*);
    void (*thread_detach)(void*);
    void** (*domain_get_assemblies)(void*, size_t*);
    void* (*domain_assembly_open)(void*, const char*);
    void* (*assembly_get_image)(void*);
    void* (*class_from_name)(void*, const char*, const char*);
    void* (*class_get_method_from_name)(void*, const char*, int);
    void* (*class_get_methods)(void*, void**);
    void* (*class_get_fields)(void*, void**);
    const char* (*class_get_name)(void*);
    const char* (*class_get_namespace)(void*);
    void* (*class_get_declaring_type)(void*);
    size_t (*image_get_class_count)(void*);
    void* (*image_get_class)(void*, size_t);
    const char* (*method_get_name)(void*);
    uint32_t (*method_get_param_count)(void*);
    void* (*method_get_param)(void*, uint32_t);
    char* (*type_get_name)(void*);
    void (*free)(void*);
    void* (*method_get_pointer)(void*);
    const char* (*field_get_name)(void*);
    size_t (*field_get_offset)(void*);
};

static Il2CppApi g_il2cpp_api = {};

static void set_last_resolve_error(const char* name, const char* reason) {
    snprintf(g_last_resolve_error, sizeof(g_last_resolve_error), "%s:%s",
             name ? name : "unknown", reason ? reason : "unknown");
}

static void set_last_metadata_state(const char* text) {
    if (!text) return;
    strncpy(g_last_metadata_state, text, sizeof(g_last_metadata_state) - 1);
    g_last_metadata_state[sizeof(g_last_metadata_state) - 1] = '\0';
}

static void set_last_field_resolve_state(const char* text) {
    if (!text) return;
    strncpy(g_last_field_resolve_state, text, sizeof(g_last_field_resolve_state) - 1);
    g_last_field_resolve_state[sizeof(g_last_field_resolve_state) - 1] = '\0';
}

static int read_module_segments(const char* module_name, ModuleSegment* segments, int max_segments) {
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return 0;
    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < max_segments) {
        if (!strstr(line, module_name)) continue;
        uintptr_t start = 0;
        uintptr_t end = 0;
        char perms[5] = {};
        if (sscanf(line, "%lx-%lx %4s", &start, &end, perms) != 3) continue;
        segments[count].start = start;
        segments[count].end = end;
        segments[count].readable = perms[0] == 'r';
        segments[count].executable = perms[2] == 'x';
        count++;
    }
    fclose(f);
    return count;
}

static bool find_module_path(const char* module_name, char* out, size_t out_size) {
    if (!module_name || !out || out_size == 0) return false;
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return false;
    char line[512];
    bool found = false;
    while (fgets(line, sizeof(line), f)) {
        if (!strstr(line, module_name)) continue;
        char path[384] = {};
        if (sscanf(line, "%*lx-%*lx %*4s %*s %*s %*s %383s", path) == 1 && path[0] == '/') {
            strncpy(out, path, out_size - 1);
            out[out_size - 1] = '\0';
            found = true;
            break;
        }
    }
    fclose(f);
    return found;
}

struct ExecAddressSearch {
    uintptr_t addr;
    bool found;
};

static int exec_address_callback(struct dl_phdr_info* info, size_t, void* data) {
    ExecAddressSearch* search = reinterpret_cast<ExecAddressSearch*>(data);
    if (!info || !search || search->found || !info->dlpi_name || !strstr(info->dlpi_name, "libil2cpp.so")) {
        return 0;
    }
    uintptr_t base = static_cast<uintptr_t>(info->dlpi_addr);
    uintptr_t lo = UINTPTR_MAX;
    for (int i = 0; i < info->dlpi_phnum; ++i) {
        const ElfW(Phdr)& phdr = info->dlpi_phdr[i];
        if (phdr.p_type != PT_LOAD) continue;
        uintptr_t start = base + static_cast<uintptr_t>(phdr.p_vaddr);
        if (start < lo) lo = start;
    }
    if (g_il2cpp_base && g_il2cpp_base != base && g_il2cpp_base != lo) return 0;

    for (int i = 0; i < info->dlpi_phnum; ++i) {
        const ElfW(Phdr)& phdr = info->dlpi_phdr[i];
        if (phdr.p_type != PT_LOAD || !(phdr.p_flags & PF_X)) continue;
        uintptr_t start = base + static_cast<uintptr_t>(phdr.p_vaddr);
        uintptr_t end = start + static_cast<uintptr_t>(phdr.p_memsz);
        if (search->addr >= start && search->addr < end) {
            search->found = true;
            return 1;
        }
    }
    return 0;
}

static bool address_in_libil2cpp_exec(uintptr_t addr) {
    ExecAddressSearch search = {addr, false};
    dl_iterate_phdr(exec_address_callback, &search);
    if (search.found) return true;

    ModuleSegment segments[16];
    int count = read_module_segments("libil2cpp.so", segments, 16);
    for (int i = 0; i < count; ++i) {
        if (segments[i].executable && addr >= segments[i].start && addr < segments[i].end) return true;
    }
    return false;
}

static uintptr_t normalize_dynamic_ptr(uintptr_t ptr, uintptr_t base, uintptr_t lo, uintptr_t hi) {
    if (ptr >= lo && ptr < hi) return ptr;
    uintptr_t adjusted = base + ptr;
    if (adjusted >= lo && adjusted < hi) return adjusted;
    return ptr;
}

static size_t gnu_hash_symbol_count(const uint32_t* gnu_hash) {
    if (!gnu_hash) return 0;
    uint32_t bucket_count = gnu_hash[0];
    uint32_t sym_offset = gnu_hash[1];
    uint32_t bloom_size = gnu_hash[2];
    if (bucket_count == 0) return 0;

    const uint32_t* buckets = gnu_hash + 4 + bloom_size * (sizeof(ElfW(Addr)) / sizeof(uint32_t));
    const uint32_t* chains = buckets + bucket_count;
    uint32_t max_symbol = sym_offset;
    for (uint32_t i = 0; i < bucket_count; ++i) {
        uint32_t symbol = buckets[i];
        if (symbol < sym_offset) continue;
        uint32_t chain_index = symbol - sym_offset;
        for (uint32_t guard = 0; guard < 200000; ++guard) {
            uint32_t hash = chains[chain_index++];
            symbol++;
            if (hash & 1u) break;
        }
        if (symbol > max_symbol) max_symbol = symbol;
    }
    return max_symbol;
}

struct LoadedSymbolSearch {
    const char* name;
    void* result;
};

static int loaded_symbol_callback(struct dl_phdr_info* info, size_t, void* data) {
    LoadedSymbolSearch* search = reinterpret_cast<LoadedSymbolSearch*>(data);
    if (!info || !search || search->result || !info->dlpi_name || !strstr(info->dlpi_name, "libil2cpp.so")) {
        return 0;
    }

    uintptr_t base = static_cast<uintptr_t>(info->dlpi_addr);
    uintptr_t lo = UINTPTR_MAX;
    uintptr_t hi = 0;
    const ElfW(Dyn)* dynamic = nullptr;
    for (int i = 0; i < info->dlpi_phnum; ++i) {
        const ElfW(Phdr)& phdr = info->dlpi_phdr[i];
        if (phdr.p_type == PT_LOAD) {
            uintptr_t start = base + static_cast<uintptr_t>(phdr.p_vaddr);
            uintptr_t end = start + static_cast<uintptr_t>(phdr.p_memsz);
            if (start < lo) lo = start;
            if (end > hi) hi = end;
        } else if (phdr.p_type == PT_DYNAMIC) {
            dynamic = reinterpret_cast<const ElfW(Dyn)*>(base + static_cast<uintptr_t>(phdr.p_vaddr));
        }
    }
    if (!dynamic || lo == UINTPTR_MAX || hi <= lo) return 0;
    if (g_il2cpp_base && g_il2cpp_base != base && g_il2cpp_base != lo) return 0;

    const ElfW(Sym)* symtab = nullptr;
    const char* strtab = nullptr;
    const uint32_t* sysv_hash = nullptr;
    const uint32_t* gnu_hash = nullptr;
    for (const ElfW(Dyn)* dyn = dynamic; dyn->d_tag != DT_NULL; ++dyn) {
        uintptr_t value = static_cast<uintptr_t>(dyn->d_un.d_ptr);
        switch (dyn->d_tag) {
            case DT_SYMTAB:
                symtab = reinterpret_cast<const ElfW(Sym)*>(normalize_dynamic_ptr(value, base, lo, hi));
                break;
            case DT_STRTAB:
                strtab = reinterpret_cast<const char*>(normalize_dynamic_ptr(value, base, lo, hi));
                break;
            case DT_HASH:
                sysv_hash = reinterpret_cast<const uint32_t*>(normalize_dynamic_ptr(value, base, lo, hi));
                break;
            case DT_GNU_HASH:
                gnu_hash = reinterpret_cast<const uint32_t*>(normalize_dynamic_ptr(value, base, lo, hi));
                break;
            default:
                break;
        }
    }
    if (!symtab || !strtab) return 0;

    size_t symbol_count = 0;
    if (sysv_hash) symbol_count = sysv_hash[1];
    if (symbol_count == 0 && gnu_hash) symbol_count = gnu_hash_symbol_count(gnu_hash);
    if (symbol_count == 0 || symbol_count > 250000) return 0;

    for (size_t i = 0; i < symbol_count; ++i) {
        if (!symtab[i].st_name || !symtab[i].st_value) continue;
        const char* symbol_name = strtab + symtab[i].st_name;
        if (strcmp(symbol_name, search->name) != 0) continue;
        search->result = reinterpret_cast<void*>(base + static_cast<uintptr_t>(symtab[i].st_value));
        return 1;
    }
    return 0;
}

static void* resolve_loaded_libil2cpp_symbol(const char* name) {
    LoadedSymbolSearch search = {name, nullptr};
    dl_iterate_phdr(loaded_symbol_callback, &search);
    return search.result;
}

static void* resolve_symbol(const char* name) {
    void* sym = resolve_loaded_libil2cpp_symbol(name);
    if (sym) return sym;
    sym = g_il2cpp_api.handle ? dlsym(g_il2cpp_api.handle, name) : nullptr;
    if (!sym) sym = dlsym(RTLD_DEFAULT, name);
    return sym;
}

static void append_text(char* out, size_t out_size, const char* text);

static bool resolve_il2cpp_api() {
    if (g_il2cpp_api.resolved && g_il2cpp_api.domain_assembly_open &&
        g_il2cpp_api.assembly_get_image && g_il2cpp_api.class_from_name &&
        g_il2cpp_api.class_get_method_from_name) {
        return true;
    }

    void* handle = dlopen("libil2cpp.so", RTLD_NOW | RTLD_NOLOAD);
    if (!handle) {
        char path[384] = {};
        if (find_module_path("libil2cpp.so", path, sizeof(path))) {
            handle = dlopen(path, RTLD_NOW | RTLD_NOLOAD);
        }
    }
    g_il2cpp_api.handle = handle;
    g_il2cpp_api.domain_get = reinterpret_cast<void* (*)()>(resolve_symbol("il2cpp_domain_get"));
    g_il2cpp_api.thread_attach = reinterpret_cast<void* (*)(void*)>(resolve_symbol("il2cpp_thread_attach"));
    g_il2cpp_api.thread_detach = reinterpret_cast<void (*)(void*)>(resolve_symbol("il2cpp_thread_detach"));
    g_il2cpp_api.domain_get_assemblies = reinterpret_cast<void** (*)(void*, size_t*)>(resolve_symbol("il2cpp_domain_get_assemblies"));
    g_il2cpp_api.domain_assembly_open = reinterpret_cast<void* (*)(void*, const char*)>(resolve_symbol("il2cpp_domain_assembly_open"));
    g_il2cpp_api.assembly_get_image = reinterpret_cast<void* (*)(void*)>(resolve_symbol("il2cpp_assembly_get_image"));
    g_il2cpp_api.class_from_name = reinterpret_cast<void* (*)(void*, const char*, const char*)>(resolve_symbol("il2cpp_class_from_name"));
    g_il2cpp_api.class_get_method_from_name = reinterpret_cast<void* (*)(void*, const char*, int)>(resolve_symbol("il2cpp_class_get_method_from_name"));
    g_il2cpp_api.class_get_methods = reinterpret_cast<void* (*)(void*, void**)>(resolve_symbol("il2cpp_class_get_methods"));
    g_il2cpp_api.class_get_fields = reinterpret_cast<void* (*)(void*, void**)>(resolve_symbol("il2cpp_class_get_fields"));
    g_il2cpp_api.class_get_name = reinterpret_cast<const char* (*)(void*)>(resolve_symbol("il2cpp_class_get_name"));
    g_il2cpp_api.class_get_namespace = reinterpret_cast<const char* (*)(void*)>(resolve_symbol("il2cpp_class_get_namespace"));
    g_il2cpp_api.class_get_declaring_type = reinterpret_cast<void* (*)(void*)>(resolve_symbol("il2cpp_class_get_declaring_type"));
    g_il2cpp_api.image_get_class_count = reinterpret_cast<size_t (*)(void*)>(resolve_symbol("il2cpp_image_get_class_count"));
    g_il2cpp_api.image_get_class = reinterpret_cast<void* (*)(void*, size_t)>(resolve_symbol("il2cpp_image_get_class"));
    g_il2cpp_api.method_get_name = reinterpret_cast<const char* (*)(void*)>(resolve_symbol("il2cpp_method_get_name"));
    g_il2cpp_api.method_get_param_count = reinterpret_cast<uint32_t (*)(void*)>(resolve_symbol("il2cpp_method_get_param_count"));
    g_il2cpp_api.method_get_param = reinterpret_cast<void* (*)(void*, uint32_t)>(resolve_symbol("il2cpp_method_get_param"));
    g_il2cpp_api.type_get_name = reinterpret_cast<char* (*)(void*)>(resolve_symbol("il2cpp_type_get_name"));
    g_il2cpp_api.free = reinterpret_cast<void (*)(void*)>(resolve_symbol("il2cpp_free"));
    g_il2cpp_api.method_get_pointer = reinterpret_cast<void* (*)(void*)>(resolve_symbol("il2cpp_method_get_pointer"));
    g_il2cpp_api.field_get_name = reinterpret_cast<const char* (*)(void*)>(resolve_symbol("il2cpp_field_get_name"));
    g_il2cpp_api.field_get_offset = reinterpret_cast<size_t (*)(void*)>(resolve_symbol("il2cpp_field_get_offset"));
    bool ok = g_il2cpp_api.domain_assembly_open &&
              g_il2cpp_api.assembly_get_image && g_il2cpp_api.class_from_name &&
              g_il2cpp_api.class_get_method_from_name;
    g_il2cpp_api.resolved = ok;
    if (!ok) {
        char state[256] = "api_missing:";
        if (!g_il2cpp_api.domain_assembly_open) append_text(state, sizeof(state), " domain_assembly_open");
        if (!g_il2cpp_api.assembly_get_image) append_text(state, sizeof(state), " assembly_get_image");
        if (!g_il2cpp_api.class_from_name) append_text(state, sizeof(state), " class_from_name");
        if (!g_il2cpp_api.class_get_method_from_name) append_text(state, sizeof(state), " class_get_method_from_name");
        set_last_metadata_state(state);
    }
    return ok;
}

static void append_text(char* out, size_t out_size, const char* text) {
    if (!out || !out_size || !text) return;
    size_t used = strlen(out);
    if (used >= out_size - 1) return;
    strncat(out, text, out_size - used - 1);
}

static void build_class_name_chain(void* klass, char* out, size_t out_size) {
    if (!klass || !out || !out_size || !g_il2cpp_api.class_get_name) return;
    if (g_il2cpp_api.class_get_declaring_type) {
        void* parent = g_il2cpp_api.class_get_declaring_type(klass);
        if (parent) {
            build_class_name_chain(parent, out, out_size);
            append_text(out, out_size, ".");
        }
    }
    append_text(out, out_size, g_il2cpp_api.class_get_name(klass));
}

static bool class_matches(void* klass, const HookSpec& spec) {
    if (!klass || !spec.klass || !g_il2cpp_api.class_get_name || !g_il2cpp_api.class_get_namespace) return false;
    const char* ns = g_il2cpp_api.class_get_namespace(klass);
    const char* name = g_il2cpp_api.class_get_name(klass);
    if (!ns) ns = "";
    if (!name) name = "";
    if (spec.namespaze && spec.namespaze[0]) {
        return strcmp(ns, spec.namespaze) == 0 && strcmp(name, spec.klass) == 0;
    }
    if (strcmp(ns, "") == 0 && strcmp(name, spec.klass) == 0) return true;
    char full[192] = {};
    if (ns[0]) {
        append_text(full, sizeof(full), ns);
        append_text(full, sizeof(full), ".");
    }
    build_class_name_chain(klass, full + strlen(full), sizeof(full) - strlen(full));
    return strcmp(full, spec.klass) == 0;
}

static void* resolve_class_in_image(void* image, const HookSpec& spec) {
    if (!image) return nullptr;
    void* klass = g_il2cpp_api.class_from_name(image, spec.namespaze ? spec.namespaze : "", spec.klass);
    if (klass) return klass;
    if (!g_il2cpp_api.image_get_class_count || !g_il2cpp_api.image_get_class) return nullptr;
    size_t count = g_il2cpp_api.image_get_class_count(image);
    for (size_t i = 0; i < count; ++i) {
        klass = g_il2cpp_api.image_get_class(image, i);
        if (class_matches(klass, spec)) return klass;
    }
    return nullptr;
}

static bool method_param_matches(void* method, const HookSpec& spec) {
    if (!spec.first_param_type || !spec.first_param_type[0]) return true;
    if (!g_il2cpp_api.method_get_param || !g_il2cpp_api.method_get_param_count || !g_il2cpp_api.type_get_name) return false;

    uint32_t param_index = 0;
    const char* needle = spec.first_param_type;
    const char* p = spec.first_param_type;
    uint32_t parsed_index = 0;
    while (*p >= '0' && *p <= '9') {
        parsed_index = parsed_index * 10u + static_cast<uint32_t>(*p - '0');
        p++;
    }
    if (p != spec.first_param_type && *p == ':') {
        param_index = parsed_index;
        needle = p + 1;
    }
    if (!needle[0] || param_index >= g_il2cpp_api.method_get_param_count(method)) return false;

    void* type = g_il2cpp_api.method_get_param(method, param_index);
    if (!type) return false;
    char* type_name = g_il2cpp_api.type_get_name(type);
    bool ok = type_name && strstr(type_name, needle);
    if (type_name && g_il2cpp_api.free) g_il2cpp_api.free(type_name);
    return ok;
}

static void* resolve_method_in_class(void* klass, const HookSpec& spec) {
    if (!klass) return nullptr;
    if (!spec.first_param_type) {
        return g_il2cpp_api.class_get_method_from_name(klass, spec.method, spec.arg_count);
    }
    if (!g_il2cpp_api.class_get_methods || !g_il2cpp_api.method_get_name || !g_il2cpp_api.method_get_param_count) {
        return nullptr;
    }
    void* iter = nullptr;
    void* method = nullptr;
    while ((method = g_il2cpp_api.class_get_methods(klass, &iter)) != nullptr) {
        const char* method_name = g_il2cpp_api.method_get_name(method);
        if (!method_name || strcmp(method_name, spec.method) != 0) continue;
        if (static_cast<int>(g_il2cpp_api.method_get_param_count(method)) != spec.arg_count) continue;
        if (method_param_matches(method, spec)) return method;
    }
    return nullptr;
}

static uintptr_t pointer_from_method(void* method) {
    if (!method) return 0;
    uintptr_t target = 0;
    if (g_il2cpp_api.method_get_pointer) {
        target = reinterpret_cast<uintptr_t>(g_il2cpp_api.method_get_pointer(method));
        if (target && address_in_libil2cpp_exec(target)) return target;
    }
    target = *reinterpret_cast<uintptr_t*>(method);
    return target && address_in_libil2cpp_exec(target) ? target : 0;
}

static uintptr_t resolve_method_pointer_in_image(void* image, const HookSpec& spec,
                                                 int* class_hits, int* method_hits,
                                                 uintptr_t* raw0, uintptr_t* raw1, uintptr_t* raw2) {
    if (!image) return 0;
    void* klass = resolve_class_in_image(image, spec);
    if (klass && class_hits) (*class_hits)++;
    void* method = resolve_method_in_class(klass, spec);
    if (method) {
        if (method_hits) (*method_hits)++;
        uintptr_t* raw = reinterpret_cast<uintptr_t*>(method);
        if (raw0) *raw0 = raw[0];
        if (raw1) *raw1 = raw[1];
        if (raw2) *raw2 = raw[2];
    }
    return pointer_from_method(method);
}

static uintptr_t resolve_by_metadata(const HookSpec& spec) {
    if (!resolve_il2cpp_api()) {
        set_last_metadata_state("api_missing");
        return 0;
    }
    static const char* kAssemblyNames[] = {
        "Assembly-CSharp",
        "Assembly-CSharp.dll",
        "UnityEngine.CoreModule",
        "UnityEngine.CoreModule.dll",
        "mscorlib",
        "mscorlib.dll",
        nullptr
    };
    int opened_assemblies = 0;
    int class_hits = 0;
    int method_hits = 0;
    uintptr_t raw0 = 0;
    uintptr_t raw1 = 0;
    uintptr_t raw2 = 0;
    if (g_il2cpp_api.domain_get_assemblies) {
        size_t assembly_count = 0;
        void** assemblies = g_il2cpp_api.domain_get_assemblies(nullptr, &assembly_count);
        if (assemblies && assembly_count > 0 && assembly_count < 1024) {
            for (size_t i = 0; i < assembly_count; ++i) {
                if (!assemblies[i]) continue;
                opened_assemblies++;
                void* image = g_il2cpp_api.assembly_get_image(assemblies[i]);
                uintptr_t pointer = resolve_method_pointer_in_image(image, spec, &class_hits, &method_hits,
                                                                     &raw0, &raw1, &raw2);
                if (pointer) {
                    char state[256];
                    snprintf(state, sizeof(state), "ok enum=%zu asm=%d class=%d method=%d target=0x%lx %s.%s.%s",
                             assembly_count, opened_assemblies, class_hits, method_hits,
                             static_cast<unsigned long>(pointer),
                             spec.namespaze ? spec.namespaze : "", spec.klass ? spec.klass : "", spec.method ? spec.method : "");
                    set_last_metadata_state(state);
                    return pointer;
                }
            }
        }
    }
    for (int i = 0; kAssemblyNames[i] != nullptr; ++i) {
        void* assembly = g_il2cpp_api.domain_assembly_open(nullptr, kAssemblyNames[i]);
        if (!assembly) continue;
        opened_assemblies++;
        void* image = g_il2cpp_api.assembly_get_image(assembly);
        uintptr_t pointer = resolve_method_pointer_in_image(image, spec, &class_hits, &method_hits,
                                                            &raw0, &raw1, &raw2);
        if (pointer) {
            char state[256];
            snprintf(state, sizeof(state), "ok open asm=%d class=%d method=%d target=0x%lx %s.%s.%s",
                     opened_assemblies, class_hits, method_hits, static_cast<unsigned long>(pointer),
                     spec.namespaze ? spec.namespaze : "", spec.klass ? spec.klass : "", spec.method ? spec.method : "");
            set_last_metadata_state(state);
            return pointer;
        }
    }
    char state[256];
    snprintf(state, sizeof(state), "fail asm=%d class=%d method=%d raw=0x%lx/0x%lx/0x%lx %s.%s.%s/%d",
             opened_assemblies, class_hits, method_hits,
             static_cast<unsigned long>(raw0), static_cast<unsigned long>(raw1), static_cast<unsigned long>(raw2),
             spec.namespaze ? spec.namespaze : "", spec.klass ? spec.klass : "", spec.method ? spec.method : "", spec.arg_count);
    set_last_metadata_state(state);
    return 0;
}

static void* resolve_class_by_metadata_name(const char* namespaze, const char* klass) {
    if (!resolve_il2cpp_api() || !klass) return nullptr;
    static const char* kAssemblyNames[] = {
        "Assembly-CSharp",
        "Assembly-CSharp.dll",
        "UnityEngine.CoreModule",
        "UnityEngine.CoreModule.dll",
        "mscorlib",
        "mscorlib.dll",
        nullptr
    };
    HookSpec spec = {0, namespaze ? namespaze : "", klass, nullptr, 0, nullptr};
    if (g_il2cpp_api.domain_get_assemblies) {
        size_t assembly_count = 0;
        void** assemblies = g_il2cpp_api.domain_get_assemblies(nullptr, &assembly_count);
        if (assemblies && assembly_count > 0 && assembly_count < 1024) {
            for (size_t i = 0; i < assembly_count; ++i) {
                if (!assemblies[i]) continue;
                void* image = g_il2cpp_api.assembly_get_image(assemblies[i]);
                void* found = resolve_class_in_image(image, spec);
                if (found) return found;
            }
        }
    }
    for (int i = 0; kAssemblyNames[i] != nullptr; ++i) {
        void* assembly = g_il2cpp_api.domain_assembly_open(nullptr, kAssemblyNames[i]);
        if (!assembly) continue;
        void* image = g_il2cpp_api.assembly_get_image(assembly);
        void* found = resolve_class_in_image(image, spec);
        if (found) return found;
    }
    return nullptr;
}

static bool resolve_field_offset_by_metadata(const char* namespaze,
                                             const char* klass,
                                             const char* field_name,
                                             uintptr_t* out) {
    if (!out) return false;
    *out = 0;
    if (!resolve_il2cpp_api() || !g_il2cpp_api.class_get_fields ||
        !g_il2cpp_api.field_get_name || !g_il2cpp_api.field_get_offset) {
        bump(g_field_resolve_fail_count);
        set_last_field_resolve_state("field_api_missing");
        return false;
    }

    void* klass_ptr = resolve_class_by_metadata_name(namespaze, klass);
    if (!klass_ptr) {
        bump(g_field_resolve_fail_count);
        char state[256];
        snprintf(state, sizeof(state), "class_missing %s.%s.%s",
                 namespaze ? namespaze : "", klass ? klass : "", field_name ? field_name : "");
        set_last_field_resolve_state(state);
        return false;
    }

    void* iter = nullptr;
    void* field = nullptr;
    while ((field = g_il2cpp_api.class_get_fields(klass_ptr, &iter)) != nullptr) {
        const char* name = g_il2cpp_api.field_get_name(field);
        if (!name || !field_name || strcmp(name, field_name) != 0) continue;
        size_t offset = g_il2cpp_api.field_get_offset(field);
        if (offset >= 0x100000) {
            bump(g_field_resolve_fail_count);
            char state[256];
            snprintf(state, sizeof(state), "offset_invalid %s.%s.%s offset=0x%lx",
                     namespaze ? namespaze : "", klass ? klass : "", field_name,
                     static_cast<unsigned long>(offset));
            set_last_field_resolve_state(state);
            return false;
        }
        *out = static_cast<uintptr_t>(offset);
        bump(g_field_resolve_metadata_count);
        return true;
    }

    bump(g_field_resolve_fail_count);
    char state[256];
    snprintf(state, sizeof(state), "field_missing %s.%s.%s",
             namespaze ? namespaze : "", klass ? klass : "", field_name ? field_name : "");
    set_last_field_resolve_state(state);
    return false;
}

static bool resolve_value_type_field_offset_by_metadata(const char* namespaze,
                                                        const char* klass,
                                                        const char* field_name,
                                                        uintptr_t* out) {
    bool ok = resolve_field_offset_by_metadata(namespaze, klass, field_name, out);
    if (!ok || !out) return ok;
    if (*out >= kIl2CppObjectHeaderSize) {
        *out -= kIl2CppObjectHeaderSize;
        return true;
    }

    *out = 0;
    bump(g_field_resolve_fail_count);
    char state[256];
    snprintf(state, sizeof(state), "value_type_offset_invalid %s.%s.%s",
             namespaze ? namespaze : "", klass ? klass : "", field_name ? field_name : "");
    set_last_field_resolve_state(state);
    return false;
}

static void resolve_runtime_field_offsets() {
    g_field_offsets_ready = false;
    int requested = 0;
    int resolved = 0;
#define RESOLVE_FIELD_OFFSET(storage, ns, klass, field) \
    do { \
        ++requested; \
        if (resolve_field_offset_by_metadata((ns), (klass), (field), &(storage))) { \
            ++resolved; \
        } \
    } while (0)
#define RESOLVE_VALUE_FIELD_OFFSET(storage, ns, klass, field) \
    do { \
        ++requested; \
        if (resolve_value_type_field_offset_by_metadata((ns), (klass), (field), &(storage))) { \
            ++resolved; \
        } \
    } while (0)

    RESOLVE_FIELD_OFFSET(g_off_entity_data_entity, "", "EntityData", "m_Entity");
    RESOLVE_FIELD_OFFSET(g_off_entity_data_fly_stone_count, "", "EntityData", "mFlyStoneCount");
    RESOLVE_FIELD_OFFSET(g_off_entity_data_fly_water_count, "", "EntityData", "mFlyWaterCount");
    RESOLVE_FIELD_OFFSET(g_off_entity_base_data, "", "EntityBase", "m_EntityData");
    RESOLVE_FIELD_OFFSET(g_off_entity_base_type, "", "EntityBase", "m_Type");
    RESOLVE_FIELD_OFFSET(g_off_entity_base_fly_water, "", "EntityBase", "bFlyWater");
    RESOLVE_FIELD_OFFSET(g_off_entity_base_fly_stone, "", "EntityBase", "bFlyStone");
    RESOLVE_FIELD_OFFSET(g_off_entity_base_hit_ctrl, "", "EntityBase", "mHitCtrl");
    RESOLVE_FIELD_OFFSET(g_off_entity_base_move_layer_mask, "", "EntityBase", "move_layermask");
    RESOLVE_FIELD_OFFSET(g_off_entity_hit_ctrl_entity, "", "EntityHitCtrl", "m_Entity");
    RESOLVE_FIELD_OFFSET(g_off_bullet_transmit_entity, "", "BulletTransmit", "m_Entity");
    RESOLVE_FIELD_OFFSET(g_off_bullet_transmit_weapon_data, "", "BulletTransmit", "weapondata");
    RESOLVE_FIELD_OFFSET(g_off_bullet_transmit_through_wall, "", "BulletTransmit", "<ThroughWall>k__BackingField");
    RESOLVE_FIELD_OFFSET(g_off_weapon_weapon_through_wall, "TableTool", "Weapon_weapon", "bThroughWallp");
    RESOLVE_FIELD_OFFSET(g_off_weapon_weapon_through_inside_wall, "TableTool", "Weapon_weapon", "bThroughInsideWallp");
    RESOLVE_FIELD_OFFSET(g_off_bullet_base_entity, "", "BulletBase", "<m_Entity>k__BackingField");
    RESOLVE_FIELD_OFFSET(g_off_bullet_base_weapon_data, "", "BulletBase", "m_Data");
    RESOLVE_FIELD_OFFSET(g_off_bullet_base_transmit, "", "BulletBase", "mBulletTransmit");
    RESOLVE_FIELD_OFFSET(g_off_move_control_entity, "", "MoveControl", "m_Entity");
    RESOLVE_FIELD_OFFSET(g_off_move_control_moving, "", "MoveControl", "bMoveing");
    RESOLVE_FIELD_OFFSET(g_off_move_control_move_direction, "", "MoveControl", "MoveDirection");
    RESOLVE_FIELD_OFFSET(g_off_adcallback_b_callback, "", "AdCallbackControl", "bCallback");
    RESOLVE_FIELD_OFFSET(g_off_adcallback_b_opened, "", "AdCallbackControl", "bOpened");
    RESOLVE_FIELD_OFFSET(g_off_base_driver_callback, "", "AdsRequestHelper.BaseDriver", "callback");
    RESOLVE_FIELD_OFFSET(g_off_wrapped_adapter_callbacks, "", "AdsRequestHelper.WrappedAdapter", "callbacks");
    RESOLVE_VALUE_FIELD_OFFSET(g_off_obscured_vector3_key, "CodeStage.AntiCheat.ObscuredTypes", "ObscuredVector3", "currentCryptoKey");
    RESOLVE_VALUE_FIELD_OFFSET(g_off_obscured_vector3_hidden, "CodeStage.AntiCheat.ObscuredTypes", "ObscuredVector3", "hiddenValue");
    RESOLVE_VALUE_FIELD_OFFSET(g_off_obscured_vector3_inited, "CodeStage.AntiCheat.ObscuredTypes", "ObscuredVector3", "inited");
    RESOLVE_VALUE_FIELD_OFFSET(g_off_obscured_vector3_fake, "CodeStage.AntiCheat.ObscuredTypes", "ObscuredVector3", "fakeValue");
    RESOLVE_VALUE_FIELD_OFFSET(g_off_obscured_vector3_fake_active, "CodeStage.AntiCheat.ObscuredTypes", "ObscuredVector3", "fakeValueActive");

#undef RESOLVE_VALUE_FIELD_OFFSET
#undef RESOLVE_FIELD_OFFSET

    g_field_offsets_ready = (resolved == requested);
    char state[256];
    snprintf(state, sizeof(state), "resolved=%d/%d ready=%d bullet_through=0x%lx move_moving=0x%lx move_dir=0x%lx ad_cb=0x%lx base_cb=0x%lx adapter_cb=0x%lx entity_type=0x%lx",
             resolved, requested, g_field_offsets_ready ? 1 : 0,
             static_cast<unsigned long>(g_off_bullet_transmit_through_wall),
             static_cast<unsigned long>(g_off_move_control_moving),
             static_cast<unsigned long>(g_off_move_control_move_direction),
             static_cast<unsigned long>(g_off_adcallback_b_callback),
             static_cast<unsigned long>(g_off_base_driver_callback),
             static_cast<unsigned long>(g_off_wrapped_adapter_callbacks),
             static_cast<unsigned long>(g_off_entity_base_type));
    set_last_field_resolve_state(state);
}

static const HookSpec kHookSpecs[] = {
    {rva::GameModeBase_GetGoldRatio, "", "GameModeBase", "GetGoldRatio", 0, nullptr},
    {rva::GameModeBase_GetDropDataGold, "", "GameModeBase", "GetDropDataGold", 1, nullptr},
    {rva::GameModeCooperation_GetGoldRatio, "", "GameModeCooperation", "GetGoldRatio", 0, nullptr},
    {rva::GameModeCooperation_GetDropDataGold, "", "GameModeCooperation", "GetDropDataGold", 1, nullptr},
    {rva::GameModeCooperationPVP_GetGoldRatio, "", "GameModeCooperationPVP", "GetGoldRatio", 0, nullptr},
    {rva::GameModeCooperationPVP_GetDropDataGold, "", "GameModeCooperationPVP", "GetDropDataGold", 1, nullptr},
    {rva::GameModeDaily_GetGoldRatio, "", "GameModeDaily", "GetGoldRatio", 0, nullptr},
    {rva::GameModeDaily_GetDropDataGold, "", "GameModeDaily", "GetDropDataGold", 1, nullptr},
    {rva::GameModeGold1_GetGoldRatio, "", "GameModeGold1", "GetGoldRatio", 0, nullptr},
    {rva::GameModeGold1_GetDropDataGold, "", "GameModeGold1", "GetDropDataGold", 1, nullptr},
    {rva::GameModeLevel_GetGoldRatio, "", "GameModeLevel", "GetGoldRatio", 0, nullptr},
    {rva::GameModeLevel_GetDropDataGold, "", "GameModeLevel", "GetDropDataGold", 1, nullptr},
    {rva::GameModeMainChallenge_GetGoldRatio, "", "GameModeMainChallenge", "GetGoldRatio", 0, nullptr},
    {rva::GameModeMainChallenge_GetDropDataGold, "", "GameModeMainChallenge", "GetDropDataGold", 1, nullptr},
    {rva::GameModeMeadowBattle_GetGoldRatio, "", "GameModeMeadowBattle", "GetGoldRatio", 0, nullptr},
    {rva::GameModeMeadowBattle_GetDropDataGold, "", "GameModeMeadowBattle", "GetDropDataGold", 1, nullptr},
    {rva::GameModeTower_GetGoldRatio, "", "GameModeTower", "GetGoldRatio", 0, nullptr},
    {rva::GameModeTower_GetDropDataGold, "", "GameModeTower", "GetDropDataGold", 1, nullptr},
    {rva::GameModeTryPlay_GetGoldRatio, "", "GameModeTryPlay", "GetGoldRatio", 0, nullptr},
    {rva::GameModeTryPlay_GetDropDataGold, "", "GameModeTryPlay", "GetDropDataGold", 1, nullptr},
    {rva::DropManager_GetRandomLevel, "", "DropManager", "GetRandomLevel", 4, nullptr},
    {rva::DropManager_GetRandomGoldHitted, "", "DropManager", "GetRandomGoldHitted", 2, nullptr},
    {rva::DropManager_GetActivityProp, "", "DropManager", "GetActivityProp", 2, nullptr},
    {rva::DropManager_GetStone, "", "DropManager", "GetStone", 2, nullptr},
    {rva::DropManager_GetBloodStone, "", "DropManager", "GetBloodStone", 2, nullptr},
    {rva::DropManager_GetRandomFetterBadge, "", "DropManager", "GetRandomFetterBadge", 2, nullptr},
    {rva::DropManager_GetRandomSkillStone, "", "DropManager", "GetRandomSkillStone", 2, nullptr},
    {rva::DropManager_GetWishCoin, "", "DropManager", "GetWishCoin", 2, nullptr},
    {rva::DropManager_GetModstone, "", "DropManager", "GetModstone", 2, nullptr},
    {rva::DropManager_GetCommonItem, "", "DropManager", "GetCommonItem", 2, nullptr},
    {rva::DropManager_GetDropMat, "", "DropManager", "GetDropMat", 3, nullptr},
    {rva::DropManager_GetRuneStone, "", "DropManager", "GetRuneStone", 2, nullptr},
    {rva::DropManager_GetCookie, "", "DropManager", "GetCookie", 2, nullptr},
    {rva::DropManager_GetAdventureCoin, "", "DropManager", "GetAdventureCoin", 2, nullptr},
    {rva::DropManager_GetLoupeDrops, "", "DropManager", "GetLoupeDrops", 2, nullptr},
    {rva::DropManager_GetManorMat, "", "DropManager", "GetManorMat", 2, nullptr},
    {rva::DropManager_GetSoulStone, "", "DropManager", "GetSoulStone", 2, nullptr},
    {rva::DropManager_GetBone, "", "DropManager", "GetBone", 2, nullptr},
    {rva::DropManager_GetHorn, "", "DropManager", "GetHorn", 2, nullptr},
    {rva::DropManager_GetRandomEquipExp, "", "DropManager", "GetRandomEquipExp", 2, nullptr},
    {rva::DropManager_GetRandomMagicStone, "", "DropManager", "GetRandomMagicStone", 2, nullptr},
    {rva::DropManager_GetRandomDragonCoin, "", "DropManager", "GetRandomDragonCoin", 2, nullptr},
    {rva::DropManager_GetRandomStarLightStone, "", "DropManager", "GetRandomStarLightStone", 2, nullptr},
    {rva::DropManager_GetRandomEquip, "", "DropManager", "GetRandomEquip", 4, nullptr},
    {rva::DropManager_GetDropIds, "", "DropManager", "GetDropIds", 2, nullptr},
    {rva::DropManager_GetEquipQuintessence, "", "DropManager", "GetEquipQuintessence", 2, nullptr},
    {rva::DropManager_GetNewPlay125BagCoin, "", "DropManager", "GetNewPlay125BagCoin", 2, nullptr},
    {rva::DropManager_getTotalCnt_WithCondition, "", "DropManager", "getTotalCnt", 2, "1:System.Func"},
    {rva::DropManager_getTotalCnt_WithEquipOne, "", "DropManager", "getTotalCnt", 2, "1:EquipOne"},
    {rva::DropManager_CanGetMoreDrop, "", "DropManager", "CanGetMoreDrop", 1, nullptr},
    {rva::DropManager_AppendDropCount, "", "DropManager", "AppendDropCount", 1, nullptr},
    {rva::DeadGoodMgr_GetGoldNum, "", "DeadGoodMgr", "GetGoldNum", 0, nullptr},
    {rva::LocalSave_BattleInBase_CanDropEquip, "", "LocalSave.BattleInBase", "CanDropEquip", 1, nullptr},
    {rva::LocalSave_BattleIn_CanDropEquip, "", "LocalSave", "BattleIn_CanDropEquip", 1, nullptr},
    {rva::LocalSave_get_CanDropType501Equip, "", "LocalSave", "get_CanDropType501Equip", 0, nullptr},
    {rva::LocalSave_get_CanDropType401Or402Equip, "", "LocalSave", "get_CanDropType401Or402Equip", 0, nullptr},
    {rva::LocalSave_get_CanDropFirstEquip, "", "LocalSave", "get_CanDropFirstEquip", 0, nullptr},
    {rva::IStageLayerManager_GetEquipMaxDrop, "", "IStageLayerManager", "GetEquipMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetMPMaxDrop, "", "IStageLayerManager", "GetMPMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetScrollMaxDrop, "", "IStageLayerManager", "GetScrollMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetRuneStoneMaxDrop, "", "IStageLayerManager", "GetRuneStoneMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetAdventureCoinsMaxDrop, "", "IStageLayerManager", "GetAdventureCoinsMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetLoupeMaxDrop, "", "IStageLayerManager", "GetLoupeMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetStoneMaxDrop, "", "IStageLayerManager", "GetStoneMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetBloodStoneMaxDrop, "", "IStageLayerManager", "GetBloodStoneMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetSkillStoneMaxDrop, "", "IStageLayerManager", "GetSkillStoneMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetFetterBadgeMaxDrop, "", "IStageLayerManager", "GetFetterBadgeMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetCommonItemMaxDrop, "", "IStageLayerManager", "GetCommonItemMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetAct4thItemsMaxDrop, "", "IStageLayerManager", "GetAct4thItemsMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetAct4thExchangeItemsMaxDrop, "", "IStageLayerManager", "GetAct4thExchangeItemsMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetWishCoinMaxDrop, "", "IStageLayerManager", "GetWishCoinMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetActivityPropMaxDrop, "", "IStageLayerManager", "GetActivityPropMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetCookieMaxDrop, "", "IStageLayerManager", "GetCookieMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetSoulStoneMaxDrop, "", "IStageLayerManager", "GetSoulStoneMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetHonorStoneMaxDrop, "", "IStageLayerManager", "GetHonorStoneMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetBoneMaxDrop, "", "IStageLayerManager", "GetBoneMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetHornMaxDrop, "", "IStageLayerManager", "GetHornMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetSoulPointMaxDrop, "", "IStageLayerManager", "GetSoulPointMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetMagicStoneMaxDrop, "", "IStageLayerManager", "GetMagicStoneMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetDragonCoinMaxDrop, "", "IStageLayerManager", "GetDragonCoinMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetStarLightStoneMaxDrop, "", "IStageLayerManager", "GetStarLightStoneMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetModstoneMaxDrop, "", "IStageLayerManager", "GetModstoneMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetManorMatMaxDrop, "", "IStageLayerManager", "GetManorMatMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetFountainUseMaxDrop, "", "IStageLayerManager", "GetFountainUseMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetFountainUpgradeMaxDrop, "", "IStageLayerManager", "GetFountainUpgradeMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetEquipQuintessenceMaxDrop, "", "IStageLayerManager", "GetEquipQuintessenceMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetChineseKnotMaxDrop, "", "IStageLayerManager", "GetChineseKnotMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetFirecrackerMaxDrop, "", "IStageLayerManager", "GetFirecrackerMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetPetLevelUpItemsMaxDrop, "", "IStageLayerManager", "GetPetLevelUpItemsMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetPetExchangeItemsMaxDrop, "", "IStageLayerManager", "GetPetExchangeItemsMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetArtifactExchangeItemsMaxDrop, "", "IStageLayerManager", "GetArtifactExchangeItemsMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetImprintLevelUpItemsMaxDrop, "", "IStageLayerManager", "GetImprintLevelUpItemsMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetImprintExchangeItemsMaxDrop, "", "IStageLayerManager", "GetImprintExchangeItemsMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetImprintStoneItemsMaxDrop, "", "IStageLayerManager", "GetImprintStoneItemsMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetPropMaxDropById, "", "IStageLayerManager", "GetPropMaxDropById", 1, nullptr},
    {rva::IStageLayerManager_GetWingLevelUpItemsMaxDrop, "", "IStageLayerManager", "GetWingLevelUpItemsMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetAct5DonateItemsMaxDrop, "", "IStageLayerManager", "GetAct5DonateItemsMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetNewPlay125BagCoinMaxDrop, "", "IStageLayerManager", "GetNewPlay125BagCoinMaxDrop", 0, nullptr},
    {rva::IStageLayerManager_GetShipUpgradeMaxDrop, "", "IStageLayerManager", "GetShipUpgradeMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetEquipMaxDrop, "", "StageLevelManager", "GetEquipMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetMPMaxDrop, "", "StageLevelManager", "GetMPMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetScrollMaxDrop, "", "StageLevelManager", "GetScrollMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetRuneStoneMaxDrop, "", "StageLevelManager", "GetRuneStoneMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetActivityPropMaxDrop, "", "StageLevelManager", "GetActivityPropMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetCookieMaxDrop, "", "StageLevelManager", "GetCookieMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetSoulStoneMaxDrop, "", "StageLevelManager", "GetSoulStoneMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetHonorStoneMaxDrop, "", "StageLevelManager", "GetHonorStoneMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetBoneMaxDrop, "", "StageLevelManager", "GetBoneMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetHornMaxDrop, "", "StageLevelManager", "GetHornMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetStoneMaxDrop, "", "StageLevelManager", "GetStoneMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetBloodStoneMaxDrop, "", "StageLevelManager", "GetBloodStoneMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetFetterBadgeMaxDrop, "", "StageLevelManager", "GetFetterBadgeMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetAct4thItemsMaxDrop, "", "StageLevelManager", "GetAct4thItemsMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetAct4thExchangeItemsMaxDrop, "", "StageLevelManager", "GetAct4thExchangeItemsMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetMagicStoneMaxDrop, "", "StageLevelManager", "GetMagicStoneMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetStarLightStoneMaxDrop, "", "StageLevelManager", "GetStarLightStoneMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetFountainUseMaxDrop, "", "StageLevelManager", "GetFountainUseMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetFountainUpgradeMaxDrop, "", "StageLevelManager", "GetFountainUpgradeMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetChineseKnotMaxDrop, "", "StageLevelManager", "GetChineseKnotMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetFirecrackerMaxDrop, "", "StageLevelManager", "GetFirecrackerMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetLoupeMaxDrop, "", "StageLevelManager", "GetLoupeMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetEquipQuintessenceMaxDrop, "", "StageLevelManager", "GetEquipQuintessenceMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetPetLevelUpItemsMaxDrop, "", "StageLevelManager", "GetPetLevelUpItemsMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetPetExchangeItemsMaxDrop, "", "StageLevelManager", "GetPetExchangeItemsMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetArtifactExchangeItemsMaxDrop, "", "StageLevelManager", "GetArtifactExchangeItemsMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetImprintLevelUpItemsMaxDrop, "", "StageLevelManager", "GetImprintLevelUpItemsMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetImprintExchangeItemsMaxDrop, "", "StageLevelManager", "GetImprintExchangeItemsMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetImprintStoneItemsMaxDrop, "", "StageLevelManager", "GetImprintStoneItemsMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetWingLevelUpItemsMaxDrop, "", "StageLevelManager", "GetWingLevelUpItemsMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetAct5DonateItemsMaxDrop, "", "StageLevelManager", "GetAct5DonateItemsMaxDrop", 0, nullptr},
    {rva::StageLevelManager_GetPropMaxDropById, "", "StageLevelManager", "GetPropMaxDropById", 1, nullptr},
    {rva::SailingBagBattleStageLayerManager_GetNewPlay125BagCoinMaxDrop, "", "SailingBagBattleStageLayerManager", "GetNewPlay125BagCoinMaxDrop", 0, nullptr},
    {rva::GameModeBase_GetAdventureCoinMaxDrop, "", "GameModeBase", "GetAdventureCoinMaxDrop", 0, nullptr},
    {rva::GameModeBase_GetNewPlay125BagCoinMaxDrop, "", "GameModeBase", "GetNewPlay125BagCoinMaxDrop", 0, nullptr},
    {rva::GameModeCooperation_GetAdventureCoinMaxDrop, "", "GameModeCooperation", "GetAdventureCoinMaxDrop", 0, nullptr},
    {rva::GameModeCooperationPVP_GetAdventureCoinMaxDrop, "", "GameModeCooperationPVP", "GetAdventureCoinMaxDrop", 0, nullptr},
    {rva::GameModeDaily_GetAdventureCoinMaxDrop, "", "GameModeDaily", "GetAdventureCoinMaxDrop", 0, nullptr},
    {rva::GameModeGold1_GetAdventureCoinMaxDrop, "", "GameModeGold1", "GetAdventureCoinMaxDrop", 0, nullptr},
    {rva::GameModeLevel_GetAdventureCoinMaxDrop, "", "GameModeLevel", "GetAdventureCoinMaxDrop", 0, nullptr},
    {rva::GameModeMainChallenge_GetAdventureCoinMaxDrop, "", "GameModeMainChallenge", "GetAdventureCoinMaxDrop", 0, nullptr},
    {rva::GameModeSailingBagBattle_GetNewPlay125BagCoinMaxDrop, "", "GameModeSailingBagBattle", "GetNewPlay125BagCoinMaxDrop", 0, nullptr},
    {rva::GameModeTower_GetAdventureCoinMaxDrop, "", "GameModeTower", "GetAdventureCoinMaxDrop", 0, nullptr},
    {rva::TableTool_DailyChapter_get_AdventureCoinRateMax, "TableTool", "Daily_DailyChapter", "get_AdventureCoinRateMax", 0, nullptr},
    {rva::TableTool_DailyChapter_get_BagCoinMax, "TableTool", "Daily_DailyChapter", "get_BagCoinMax", 0, nullptr},
    {rva::TableTool_PVEStage_get_GoldMax, "TableTool", "PVEStage_stagechapter", "get_GoldMax", 0, nullptr},
    {rva::TableTool_PVEStage_get_HardGoldMax, "TableTool", "PVEStage_stagechapter", "get_Hard_GoldMax", 0, nullptr},
    {rva::TableTool_ShipStage_get_BagCoinMax, "TableTool", "ShipStage_BagDifficulty", "get_BagCoinMax", 0, nullptr},
    {rva::TableTool_SLGStage_get_GoldMax, "TableTool", "SLGStage_stagechapter", "get_GoldMax", 0, nullptr},
    {rva::TableTool_SLGBaseLevel_get_GoldMax, "TableTool", "SLG_BaseLevel", "get_GoldMax", 0, nullptr},
    {rva::TableTool_TowerDefense_get_GoldMAX, "TableTool", "Tower_Defense_TDlevel", "get_GoldMAX", 0, nullptr},
    {rva::SailingBagBattleStageLayerManager_GetCommonItemMaxDrop, "", "SailingBagBattleStageLayerManager", "GetCommonItemMaxDrop", 0, nullptr},
    {rva::SailingBagBattleStageLayerManager_GetShipUpgradeMaxDrop, "", "SailingBagBattleStageLayerManager", "GetShipUpgradeMaxDrop", 0, nullptr},
    {rva::CampBattleStageLayerManager_GetEquipMaxDrop, "", "CampBattleStageLayerManager", "GetEquipMaxDrop", 0, nullptr},
    {rva::CampBattleStageLayerManager_GetStoneMaxDrop, "", "CampBattleStageLayerManager", "GetStoneMaxDrop", 0, nullptr},
    {rva::CampBattleStageLayerManager_GetSkillStoneMaxDrop, "", "CampBattleStageLayerManager", "GetSkillStoneMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetScrollMaxDrop, "", "DailyActivity.DailyStageChapter", "GetScrollMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetAdventureCoinsMaxDrop, "", "DailyActivity.DailyStageChapter", "GetAdventureCoinsMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetLoupeMaxDrop, "", "DailyActivity.DailyStageChapter", "GetLoupeMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetBoneMaxDrop, "", "DailyActivity.DailyStageChapter", "GetBoneMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetHornMaxDrop, "", "DailyActivity.DailyStageChapter", "GetHornMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetRuneStoneMaxDrop, "", "DailyActivity.DailyStageChapter", "GetRuneStoneMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetActivityPropMaxDrop, "", "DailyActivity.DailyStageChapter", "GetActivityPropMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetStoneMaxDrop, "", "DailyActivity.DailyStageChapter", "GetStoneMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetCookieMaxDrop, "", "DailyActivity.DailyStageChapter", "GetCookieMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetSoulStoneMaxDrop, "", "DailyActivity.DailyStageChapter", "GetSoulStoneMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetHonorStoneMaxDrop, "", "DailyActivity.DailyStageChapter", "GetHonorStoneMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetEquipMaxDrop, "", "DailyActivity.DailyStageChapter", "GetEquipMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetBloodStoneMaxDrop, "", "DailyActivity.DailyStageChapter", "GetBloodStoneMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetFetterBadgeMaxDrop, "", "DailyActivity.DailyStageChapter", "GetFetterBadgeMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetAct4thItemsMaxDrop, "", "DailyActivity.DailyStageChapter", "GetAct4thItemsMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetAct4thExchangeItemsMaxDrop, "", "DailyActivity.DailyStageChapter", "GetAct4thExchangeItemsMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetWishCoinMaxDrop, "", "DailyActivity.DailyStageChapter", "GetWishCoinMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetMagicStoneMaxDrop, "", "DailyActivity.DailyStageChapter", "GetMagicStoneMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetDragonCoinMaxDrop, "", "DailyActivity.DailyStageChapter", "GetDragonCoinMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetModstoneMaxDrop, "", "DailyActivity.DailyStageChapter", "GetModstoneMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetManorMatMaxDrop, "", "DailyActivity.DailyStageChapter", "GetManorMatMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetFountainUseMaxDrop, "", "DailyActivity.DailyStageChapter", "GetFountainUseMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetFountainUpgradeMaxDrop, "", "DailyActivity.DailyStageChapter", "GetFountainUpgradeMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetCommonItemMaxDrop, "", "DailyActivity.DailyStageChapter", "GetCommonItemMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetEquipQuintessenceMaxDrop, "", "DailyActivity.DailyStageChapter", "GetEquipQuintessenceMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetChineseKnotMaxDrop, "", "DailyActivity.DailyStageChapter", "GetChineseKnotMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetFirecrackerMaxDrop, "", "DailyActivity.DailyStageChapter", "GetFirecrackerMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetPetLevelUpItemsMaxDrop, "", "DailyActivity.DailyStageChapter", "GetPetLevelUpItemsMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetPetExchangeItemsMaxDrop, "", "DailyActivity.DailyStageChapter", "GetPetExchangeItemsMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetArtifactExchangeItemsMaxDrop, "", "DailyActivity.DailyStageChapter", "GetArtifactExchangeItemsMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetImprintLevelUpItemsMaxDrop, "", "DailyActivity.DailyStageChapter", "GetImprintLevelUpItemsMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetImprintExchangeItemsMaxDrop, "", "DailyActivity.DailyStageChapter", "GetImprintExchangeItemsMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetImprintStoneItemsMaxDrop, "", "DailyActivity.DailyStageChapter", "GetImprintStoneItemsMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetWingLevelUpItemsMaxDrop, "", "DailyActivity.DailyStageChapter", "GetWingLevelUpItemsMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetNewPlay125BagCoinMaxDrop, "", "DailyActivity.DailyStageChapter", "GetNewPlay125BagCoinMaxDrop", 0, nullptr},
    {rva::DailyStageChapter_GetPropMaxDropById, "", "DailyActivity.DailyStageChapter", "GetPropMaxDropById", 1, nullptr},
    {rva::BattleModuleData_AddGold_Float, "", "BattleModuleData", "AddGold", 1, "System.Single"},
    {rva::BattleModuleData_AddGold_Int, "", "BattleModuleData", "AddGold", 1, "System.Int32"},
    {rva::BattleModuleData_GetGold, "", "BattleModuleData", "GetGold", 0, nullptr},
    {rva::LocalSave_BattleIn_GetGold, "", "LocalSave", "BattleIn_GetGold", 0, nullptr},
    {rva::LocalSave_BattleIn_UpdateGold, "", "LocalSave", "BattleIn_UpdateGold", 1, nullptr},
    {rva::EntityData_getGold, "", "EntityData", "getGold", 1, nullptr},
    {rva::GameConfig_GetCoin1Wave, "", "GameConfig", "GetCoin1Wave", 0, nullptr},
    {rva::GameConfig_GetBoxDropGold, "", "GameConfig", "GetBoxDropGold", 0, nullptr},
    {rva::GameConfig_GetBoxChooseGold, "", "GameConfig", "GetBoxChooseGold", 1, nullptr},
    {rva::Drop_DropModel_GetGoldDropPercent, "TableTool", "Drop_DropModel", "GetGoldDropPercent", 0, nullptr},
    {rva::Drop_DropModel_GetDropGold, "TableTool", "Drop_DropModel", "GetDropGold", 1, nullptr},
    {rva::DeadGoodMgr_StartDrop, "", "DeadGoodMgr", "StartDrop", 4, nullptr},
    {rva::DropManager_GetDropList, "", "DropManager", "GetDropList", 0, nullptr},
    {rva::DropGold_OnGetHittedList, "", "DropGold", "OnGetHittedList", 1, nullptr},
    {rva::DropGold_OnGetDropDead, "", "DropGold", "OnGetDropDead", 0, nullptr},
    {rva::GameLogic_GetPureGoldList, "", "GameLogic", "GetPureGoldList", 1, nullptr},
    {rva::GameLogic_CanSaveGoldInRealTime, "", "GameLogic", "CanSaveGoldInRealTime", 0, nullptr},
    {rva::StageLevelManager_GetGoldDropPercent, "", "StageLevelManager", "GetGoldDropPercent", 1, nullptr},
    {rva::StageLevelManager_GetFreeGold, "", "StageLevelManager", "GetFreeGold", 0, nullptr},
    {rva::EntityData_GetHeadShot, "", "EntityData", "GetHeadShot", 2, nullptr},
    {rva::EntityData_GetMiss, "", "EntityData", "GetMiss", 1, nullptr},
    {rva::TableTool_PlayerCharacter_UpgradeModel_GetATKBase, "TableTool", "PlayerCharacter_UpgradeModel", "GetATKBase", 1, nullptr},
    {rva::TableTool_PlayerCharacter_UpgradeModel_GetHPMaxBase, "TableTool", "PlayerCharacter_UpgradeModel", "GetHPMaxBase", 1, nullptr},
    {rva::TableTool_Weapon_weapon_get_Speed, "TableTool", "Weapon_weapon", "get_Speed", 0, nullptr},
    {rva::TableTool_Weapon_weapon_get_AttackSpeed, "TableTool", "Weapon_weapon", "get_AttackSpeed", 0, nullptr},
    {rva::TableTool_Weapon_weapon_get_bThroughWall, "TableTool", "Weapon_weapon", "get_bThroughWall", 0, nullptr},
    {rva::TableTool_Weapon_weapon_get_bThroughInsideWall, "TableTool", "Weapon_weapon", "get_bThroughInsideWall", 0, nullptr},
    {rva::BulletTransmit_Init_Simple, "", "BulletTransmit", "Init", 4, "EntityBase"},
    {rva::BulletTransmit_Init_Full, "", "BulletTransmit", "Init", 8, "EntityBase"},
    {rva::BulletTransmit_get_ThroughWall, "", "BulletTransmit", "get_ThroughWall", 0, nullptr},
    {rva::BulletBase_HitWall, "", "BulletBase", "HitWall", 1, "UnityEngine.Collider"},
    {rva::BulletBase_TriggerEnter1_HitWallInternal, "", "BulletBase", "<TriggerEnter1>g__HitWallInternal|337_0", 1, nullptr},
    {rva::EntityBase_SetFlyWater, "", "EntityBase", "SetFlyWater", 1, nullptr},
    {rva::EntityBase_GetFlyWater, "", "EntityBase", "GetFlyWater", 0, nullptr},
    {rva::EntityBase_SetFlyStone, "", "EntityBase", "SetFlyStone", 1, nullptr},
    {rva::EntityBase_GetOnCalCanMove, "", "EntityBase", "get_OnCalCanMove", 0, nullptr},
    {rva::EntityBase_SetCollider, "", "EntityBase", "SetCollider", 1, nullptr},
    {rva::EntityBase_SetFlyAll, "", "EntityBase", "SetFlyAll", 1, nullptr},
    {rva::EntityBase_CheckPos, "", "EntityBase", "check_pos", 1, "UnityEngine.Vector3"},
    {rva::EntityBase_SelfMoveBy, "", "EntityBase", "SelfMoveBy", 1, "UnityEngine.Vector3"},
    {rva::EntityBase_AddSkill, "", "EntityBase", "AddSkill", 1, "System.Int32"},
    {rva::EntityBase_ContainsSkill, "", "EntityBase", "ContainsSkill", 1, "System.Int32"},
    {rva::EntityBase_AddInitSkills, "", "EntityBase", "AddInitSkills", 0, nullptr},
    {rva::EntityHitCtrl_SetFlyOne, "", "EntityHitCtrl", "SetFlyOne", 2, nullptr},
    {rva::MoveControl_UpdateProgress, "", "MoveControl", "UpdateProgress", 0, nullptr},
    {rva::AdCallbackControl_IsLoaded, "", "AdCallbackControl", "IsLoaded", 1, nullptr},
    {rva::AdCallbackControl_Show, "", "AdCallbackControl", "Show", 1, nullptr},
    {rva::AdCallbackControl_onClose, "", "AdCallbackControl", "onClose", 2, nullptr},
    {rva::AdCallbackControl_onReward, "", "AdCallbackControl", "onReward", 2, nullptr},
    {rva::AdsRequestHelper_ALMaxRewardedDriver_isLoaded, "", "AdsRequestHelper.ALMaxRewardedDriver", "isLoaded", 0, nullptr},
    {rva::AdsRequestHelper_ALMaxRewardedDriver_Show, "", "AdsRequestHelper.ALMaxRewardedDriver", "Show", 0, nullptr},
    {rva::AdsRequestHelper_WrappedDriver_onClose, "", "AdsRequestHelper.WrappedDriver", "onClose", 2, nullptr},
    {rva::AdsRequestHelper_WrappedDriver_onReward, "", "AdsRequestHelper.WrappedDriver", "onReward", 2, nullptr},
    {rva::AdsRequestHelper_CombinedDriver_onClose, "", "AdsRequestHelper.CombinedDriver", "onClose", 2, nullptr},
    {rva::AdsRequestHelper_CombinedDriver_onReward, "", "AdsRequestHelper.CombinedDriver", "onReward", 2, nullptr},
    {rva::AdsRequestHelper_CallbackRouter_onClose, "", "AdsRequestHelper.CallbackRouter", "onClose", 2, nullptr},
    {rva::AdsRequestHelper_CallbackRouter_onReward, "", "AdsRequestHelper.CallbackRouter", "onReward", 2, nullptr},
    {rva::AdsRequestHelper_WrappedAdapter_isLoaded, "", "AdsRequestHelper.WrappedAdapter", "isLoaded", 0, nullptr},
    {rva::AdsRequestHelper_WrappedAdapter_Show, "", "AdsRequestHelper.WrappedAdapter", "Show", 0, nullptr},
    {rva::AdsRequestHelper_WrappedAdapter_Show_Callback, "", "AdsRequestHelper.WrappedAdapter", "Show", 1, nullptr},
    {rva::AdsRequestHelper_WrappedAdapter_Show_Callback_Source, "", "AdsRequestHelper.WrappedAdapter", "Show", 2, nullptr},
    {rva::AdsRequestHelper_rewarded_high_eCPM_isLoaded, "", "AdsRequestHelper", "rewarded_high_eCPM_isLoaded", 0, nullptr},
    {rva::AdsRequestHelper_rewarded_high_eCPM_show, "", "AdsRequestHelper", "rewarded_high_eCPM_show", 2, nullptr},
    {rva::UnityEngine_Time_get_deltaTime, "UnityEngine", "Time", "get_deltaTime", 0, nullptr},
    {rva::UnityEngine_Time_get_timeScale, "UnityEngine", "Time", "get_timeScale", 0, nullptr},
    {rva::UnityEngine_Time_set_timeScale, "UnityEngine", "Time", "set_timeScale", 1, nullptr},
};

static const HookSpec* find_hook_spec(uintptr_t rva_value) {
    for (size_t i = 0; i < sizeof(kHookSpecs) / sizeof(kHookSpecs[0]); ++i) {
        if (kHookSpecs[i].dump_rva == rva_value) return &kHookSpecs[i];
    }
    return nullptr;
}

static uintptr_t resolve_hook_target(uintptr_t base, uintptr_t rva_value, const char* log_name, const char** strategy) {
    (void)base;
    const HookSpec* spec = find_hook_spec(rva_value);
    if (spec) {
        uintptr_t target = resolve_by_metadata(*spec);
        if (target) {
            *strategy = "metadata";
            bump(g_resolve_metadata_count);
            return target;
        }
    }
    bump(g_resolve_fail_count);
    set_last_resolve_error(log_name, spec ? "metadata_unresolved" : "missing_spec");
    *strategy = "fail";
    return 0;
}

static bool wait_for_il2cpp_metadata_ready() {
    const HookSpec* spec = find_hook_spec(rva::EntityData_GetHeadShot);
    if (!spec) return false;

    usleep(kIl2CppRuntimeMinSettleMs * 1000);
    for (int elapsed = kIl2CppRuntimeMinSettleMs;
         elapsed <= kIl2CppMetadataTimeoutMs;
         elapsed += kIl2CppMetadataPollMs) {
        if (resolve_by_metadata(*spec)) {
            g_il2cpp_metadata_wait_ms = elapsed;
            return true;
        }
        usleep(kIl2CppMetadataPollMs * 1000);
    }
    g_il2cpp_metadata_wait_ms = kIl2CppMetadataTimeoutMs;
    set_last_resolve_error("metadata_probe", "timeout");
    return false;
}

static bool total_counter_relax_enabled() {
    return false;
}

static void relax_drop_manager_total_counters(void* thiz) {
    (void)thiz;
}

static void set_last_config_path(const char* path) {
    if (!path) return;
    strncpy(g_last_config_path, path, sizeof(g_last_config_path) - 1);
    g_last_config_path[sizeof(g_last_config_path) - 1] = '\0';
}

static uint32_t mov_w0_imm16_word(int32_t value) {
    if (value < 0) value = 0;
    if (value > 65535) value = 65535;
    return 0x52800000u | (static_cast<uint32_t>(value) << 5);
}

static uint32_t fmov_s0_nearest_word(float value) {
    if (!isfinite(value)) value = 1.0f;
    if (value <= 0.5f) return 0x1E2C1000u;   // fmov s0, #0.5
    if (value <= 1.0f) return 0x1E2E1000u;   // fmov s0, #1.0
    if (value <= 2.0f) return 0x1E201000u;   // fmov s0, #2.0
    if (value <= 3.0f) return 0x1E211000u;   // fmov s0, #3.0
    if (value <= 4.0f) return 0x1E221000u;   // fmov s0, #4.0
    if (value <= 5.0f) return 0x1E229000u;   // fmov s0, #5.0
    if (value <= 6.0f) return 0x1E231000u;   // fmov s0, #6.0
    if (value <= 8.0f) return 0x1E241000u;   // fmov s0, #8.0
    if (value <= 10.0f) return 0x1E249000u;  // fmov s0, #10.0
    if (value <= 12.0f) return 0x1E251000u;  // fmov s0, #12.0
    return 0x1E261000u;                       // fmov s0, #16.0
}

static bool write_code_pair(uintptr_t addr, uint32_t word0, uint32_t word1) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) return false;
    uintptr_t page = addr & ~(static_cast<uintptr_t>(page_size) - 1u);
    if (mprotect(reinterpret_cast<void*>(page), static_cast<size_t>(page_size), PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return false;
    }
    uint32_t* code = reinterpret_cast<uint32_t*>(addr);
    code[0] = word0;
    code[1] = word1;
    __builtin___clear_cache(reinterpret_cast<char*>(addr), reinterpret_cast<char*>(addr + 8));
    mprotect(reinterpret_cast<void*>(page), static_cast<size_t>(page_size), PROT_READ | PROT_EXEC);
    return true;
}

static void mkdirs_for_file_path(const char* path) {
    if (!path || !path[0]) return;
    char buf[256];
    strncpy(buf, path, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char* last_slash = strrchr(buf, '/');
    if (!last_slash) return;
    *last_slash = '\0';

    for (char* p = buf + 1; *p; ++p) {
        if (*p == '/') {
            *p = '\0';
            mkdir(buf, 0777);
            *p = '/';
        }
    }
    mkdir(buf, 0777);
}

static bool write_default_config_file(const char* path) {
    if (!path || !path[0]) return false;
    mkdirs_for_file_path(path);
    FILE* f = fopen(path, "w");
    if (!f) return false;

    static const char* body =
        "# Archero LSPosed config generated by native module\n"
        "# Boolean options use 0 for OFF and 1 for ON.\n"
        "# Default profile: always-on gameplay assists enabled; optional gold/drop hooks disabled.\n"
        "headshot=1\n"
        "godmode=1\n"
        "damage_v1=1\n"
        "health=1\n"
        "attack_speed=1\n"
        "shoot_through_walls=1\n"
        "walk_through_water=1\n"
        "walk_through_walls=1\n"
        "inject_greed_skill=1\n"
        "inject_smart_skill=1\n"
        "game_speed=1\n"
        "game_speed_multiplier=4\n"
        "move_speed=1\n"
        "move_speed_multiplier=1\n"
        "skip_rewarded_ads=1\n"
        "install_gold_hooks=0\n"
        "tiny_direct_patch=0\n"
        "gold_add_scale=0\n"
        "gold_get_fixed=0\n"
        "gold_get_scale=0\n"
        "gold_update_fixed=0\n"
        "gold_update_scale=0\n"
        "gold_formula_scale=0\n"
        "gold_static_scale=0\n"
        "gold_drop_scalar=0\n"
        "gold_drop_repeat=0\n"
        "gold_ratio_scale=0\n"
        "gold_list_scale=0\n"
        "gold_save_realtime=0\n"
        "material_drop_repeat=0\n"
        "max_drop_cap_patch=0\n"
        "gold_multiplier=2\n"
        "stage_gold_fixed=0\n"
        "gold_drop_repeats=1\n"
        "material_drop_repeats=2\n"
        "max_drop_cap_value=65535\n"
        "max_gold_cap_value=2000000000\n";

    fputs(body, f);
    fclose(f);
    chmod(path, 0666);
    g_default_config_created = true;
    return true;
}

static uintptr_t resolve_direct_patch_target(uintptr_t base, Patch8State& patch, const char* name) {
    if (!base || !patch.rva) return 0;
    if (patch.target) return patch.target;
    const char* strategy = "unknown";
    uintptr_t target = resolve_hook_target(base, patch.rva, name, &strategy);
    if (!target || !address_in_libil2cpp_exec(target)) {
        bump(g_direct_patch_fail_count);
        set_last_resolve_error(name, "direct_patch_unresolved");
        LOGD("Unable to resolve direct patch %s", name ? name : "unknown");
        return 0;
    }
    patch.target = target;
    bump(g_direct_patch_resolved_count);
    LOGD("Resolved direct patch %s at 0x%lx via %s", name ? name : "unknown",
         static_cast<unsigned long>(target), strategy);
    return target;
}

static void set_direct_patch8(uintptr_t base, Patch8State& patch, bool enable, uint32_t word0, uint32_t word1, const char* name) {
    if (!base || !patch.rva) return;
    if (!enable && !patch.applied) return;
    uintptr_t addr = resolve_direct_patch_target(base, patch, name);
    if (!addr) return;
    uint32_t* code = reinterpret_cast<uint32_t*>(addr);
    if (!patch.saved) {
        patch.original0 = code[0];
        patch.original1 = code[1];
        patch.saved = true;
    }
    if (enable) {
        if (!patch.applied || code[0] != word0 || code[1] != word1) {
            if (write_code_pair(addr, word0, word1)) {
                patch.applied = true;
                bump(g_direct_patch_write_count);
            }
        }
    } else if (patch.applied) {
        if (write_code_pair(addr, patch.original0, patch.original1)) {
            patch.applied = false;
            bump(g_direct_patch_write_count);
        }
    }
}

#define SET_DIRECT_PATCH8(base, patch, enable, word0, word1) \
    set_direct_patch8((base), (patch), (enable), (word0), (word1), #patch)

static void update_tiny_direct_patches(uintptr_t base) {
    if (!base) return;
    const uint32_t ret = 0xD65F03C0u;
    const uint32_t nop = 0xD503201Fu;
    int32_t fixed_gold = static_cast<int32_t>(clamp_multiplier(g_stage_gold_fixed));
    int32_t max_drop_cap = clamp_max_drop_cap(g_max_drop_cap_value);
    uint32_t ratio = fmov_s0_nearest_word(g_gold_multiplier);

    SET_DIRECT_PATCH8(base, g_patch_dead_good_gold, g_tiny_direct_patch, mov_w0_imm16_word(fixed_gold), ret);
    for (size_t i = 0; i < sizeof(g_patch_istage_max_caps) / sizeof(g_patch_istage_max_caps[0]); ++i) {
        bool coin_cap = g_patch_istage_max_caps[i].rva == rva::IStageLayerManager_GetAdventureCoinsMaxDrop ||
                        g_patch_istage_max_caps[i].rva == rva::IStageLayerManager_GetNewPlay125BagCoinMaxDrop;
        if (coin_cap) {
            // These interface stubs are only 8 bytes, so they cannot safely return
            // a full 32-bit gold cap with MOV/MOVK/RET. Concrete gold-cap hooks
            // below carry max_gold_cap_value instead of downgrading coins to 65535.
            SET_DIRECT_PATCH8(base, g_patch_istage_max_caps[i], false, 0, 0);
            continue;
        }
        SET_DIRECT_PATCH8(base, g_patch_istage_max_caps[i], g_max_drop_cap_patch,
                          mov_w0_imm16_word(max_drop_cap), ret);
    }
    for (size_t i = 0; i < sizeof(g_patch_stagelevel_max_caps) / sizeof(g_patch_stagelevel_max_caps[0]); ++i) {
        SET_DIRECT_PATCH8(base, g_patch_stagelevel_max_caps[i], g_max_drop_cap_patch,
                          mov_w0_imm16_word(max_drop_cap), ret);
    }
    for (size_t i = 0; i < sizeof(g_patch_total_count_zero) / sizeof(g_patch_total_count_zero[0]); ++i) {
        SET_DIRECT_PATCH8(base, g_patch_total_count_zero[i], g_max_drop_cap_patch,
                          mov_w0_imm16_word(0), ret);
    }
    for (size_t i = 0; i < sizeof(g_patch_total_gate_true) / sizeof(g_patch_total_gate_true[0]); ++i) {
        SET_DIRECT_PATCH8(base, g_patch_total_gate_true[i], g_max_drop_cap_patch,
                          mov_w0_imm16_word(1), ret);
    }
    SET_DIRECT_PATCH8(base, g_patch_append_drop_count, g_max_drop_cap_patch, ret, nop);
    SET_DIRECT_PATCH8(base, g_patch_meadow_gold_ratio, g_tiny_direct_patch, ratio, ret);
    SET_DIRECT_PATCH8(base, g_patch_meadow_drop_gold, g_tiny_direct_patch, mov_w0_imm16_word(fixed_gold), ret);
    SET_DIRECT_PATCH8(base, g_patch_tryplay_gold_ratio, g_tiny_direct_patch, ratio, ret);
    SET_DIRECT_PATCH8(base, g_patch_tryplay_drop_gold, g_tiny_direct_patch, mov_w0_imm16_word(fixed_gold), ret);
}

static void set_config_value(const char* key, const char* value) {
    if (!key || !value) return;
    if (strcmp(key, "headshot") == 0) g_enable_headshot = parse_bool_value(value);
    else if (strcmp(key, "godmode") == 0) g_enable_godmode = parse_bool_value(value);
    else if (strcmp(key, "damage_v1") == 0) g_enable_damage_v1 = parse_bool_value(value);
    else if (strcmp(key, "health") == 0) g_enable_health = parse_bool_value(value);
    else if (strcmp(key, "attack_speed") == 0) g_enable_attack_speed = parse_bool_value(value);
    else if (strcmp(key, "shoot_through_walls") == 0) g_enable_shoot_through_walls = parse_bool_value(value);
    else if (strcmp(key, "walk_through_water") == 0) g_enable_walk_through_water = parse_bool_value(value);
    else if (strcmp(key, "walk_through_walls") == 0) g_enable_walk_through_walls = parse_bool_value(value);
    else if (strcmp(key, "inject_greed_skill") == 0) g_enable_inject_greed_skill = parse_bool_value(value);
    else if (strcmp(key, "inject_smart_skill") == 0) g_enable_inject_smart_skill = parse_bool_value(value);
    else if (strcmp(key, "game_speed") == 0) g_enable_game_speed = parse_bool_value(value);
    else if (strcmp(key, "move_speed") == 0) g_enable_move_speed = parse_bool_value(value);
    else if (strcmp(key, "skip_rewarded_ads") == 0) g_skip_rewarded_ads = parse_bool_value(value);
    else if (strcmp(key, "install_gold_hooks") == 0) g_install_gold_hooks = parse_bool_value(value);
    else if (strcmp(key, "tiny_direct_patch") == 0) g_tiny_direct_patch = parse_bool_value(value);
    else if (strcmp(key, "gold_add_scale") == 0) g_gold_add_scale = parse_bool_value(value);
    else if (strcmp(key, "gold_get_fixed") == 0) g_gold_get_fixed = parse_bool_value(value);
    else if (strcmp(key, "gold_get_scale") == 0) g_gold_get_scale = parse_bool_value(value);
    else if (strcmp(key, "gold_update_fixed") == 0) g_gold_update_fixed = parse_bool_value(value);
    else if (strcmp(key, "gold_update_scale") == 0) g_gold_update_scale = parse_bool_value(value);
    else if (strcmp(key, "gold_formula_scale") == 0) g_gold_formula_scale = parse_bool_value(value);
    else if (strcmp(key, "gold_static_scale") == 0) g_gold_static_scale = parse_bool_value(value);
    else if (strcmp(key, "gold_drop_scalar") == 0) g_gold_drop_scalar = parse_bool_value(value);
    else if (strcmp(key, "gold_drop_repeat") == 0) g_gold_drop_repeat = parse_bool_value(value);
    else if (strcmp(key, "gold_ratio_scale") == 0) g_gold_ratio_scale = parse_bool_value(value);
    else if (strcmp(key, "gold_list_scale") == 0) g_gold_list_scale = parse_bool_value(value);
    else if (strcmp(key, "gold_save_realtime") == 0) g_gold_save_realtime = parse_bool_value(value);
    else if (strcmp(key, "material_drop_repeat") == 0) g_material_drop_repeat = parse_bool_value(value);
    else if (strcmp(key, "max_drop_cap_patch") == 0) g_max_drop_cap_patch = parse_bool_value(value);
    else if (strcmp(key, "gold_multiplier") == 0) g_gold_multiplier = clamp_multiplier(strtof(value, nullptr));
    else if (strcmp(key, "stage_gold_fixed") == 0) g_stage_gold_fixed = clamp_multiplier(strtof(value, nullptr));
    else if (strcmp(key, "gold_drop_repeats") == 0) g_gold_drop_repeats = clamp_repeat(static_cast<int>(strtol(value, nullptr, 10)));
    else if (strcmp(key, "material_drop_repeats") == 0) g_material_drop_repeats = clamp_repeat(static_cast<int>(strtol(value, nullptr, 10)));
    else if (strcmp(key, "max_drop_cap_value") == 0) g_max_drop_cap_value = clamp_max_drop_cap(static_cast<int>(strtol(value, nullptr, 10)));
    else if (strcmp(key, "max_gold_cap_value") == 0) g_max_gold_cap_value = clamp_max_gold_cap(strtoll(value, nullptr, 10));
    else if (strcmp(key, "game_speed_multiplier") == 0) g_game_speed_multiplier = clamp_game_speed(strtof(value, nullptr));
    else if (strcmp(key, "move_speed_multiplier") == 0) g_move_speed_multiplier = clamp_move_speed(strtof(value, nullptr));
}

static bool load_config_file_path(const char* path) {
    struct stat st = {};
    if (stat(path, &st) != 0) return false;
    static bool loaded_once = false;
    static char loaded_path[256] = {};
    static time_t loaded_mtime = 0;
    static long loaded_mtime_nsec = 0;
    static time_t loaded_ctime = 0;
    static long loaded_ctime_nsec = 0;
    static off_t loaded_size = -1;
    if (loaded_once && strcmp(path, loaded_path) == 0 &&
        st.st_mtime == loaded_mtime && st.st_mtim.tv_nsec == loaded_mtime_nsec &&
        st.st_ctime == loaded_ctime && st.st_ctim.tv_nsec == loaded_ctime_nsec &&
        st.st_size == loaded_size) {
        return true;
    }

    FILE* f = fopen(path, "r");
    if (!f) return false;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '\n' || *p == '#') continue;
        char* eq = strchr(p, '=');
        if (!eq) continue;
        *eq = '\0';
        char* key = p;
        char* value = eq + 1;
        char* end = key + strlen(key);
        while (end > key && (end[-1] == ' ' || end[-1] == '\t')) *--end = '\0';
        end = value + strlen(value);
        while (end > value && (end[-1] == '\n' || end[-1] == '\r' || end[-1] == ' ' || end[-1] == '\t')) *--end = '\0';
        set_config_value(key, value);
    }
    fclose(f);
    set_last_config_path(path);
    strncpy(loaded_path, path, sizeof(loaded_path) - 1);
    loaded_path[sizeof(loaded_path) - 1] = '\0';
    loaded_mtime = st.st_mtime;
    loaded_mtime_nsec = st.st_mtim.tv_nsec;
    loaded_ctime = st.st_ctime;
    loaded_ctime_nsec = st.st_ctim.tv_nsec;
    loaded_size = st.st_size;
    loaded_once = true;
    bump(g_config_loads);
    return true;
}

static void load_config_file_once() {
    static const char* paths[] = {
        "/storage/emulated/0/Android/data/com.habby.archero/files/archero_mod_config.txt",
        nullptr
    };

    for (int i = 0; paths[i] != nullptr; ++i) {
        // ASSUMPTION: app-owned config is the authoritative source. Stop at
        // the first readable file so stale lower-priority mirrors cannot
        // re-enable risky hook families.
        if (load_config_file_path(paths[i])) return;
    }
    if (write_default_config_file(paths[0]) && load_config_file_path(paths[0])) return;
    set_last_config_path("none");
}

static void install_gold_hooks_once(uintptr_t base);

static void write_status_file_once() {
    FILE* f = fopen("/storage/emulated/0/Android/data/com.habby.archero/files/archero_mod_status.txt", "w");
    if (!f) {
        f = fopen("/data/data/com.habby.archero/files/archero_mod_status.txt", "w");
    }
    if (!f) return;

    fprintf(f, "version=archero_mod_status_v1\n");
    fprintf(f, "pid=%d\n", getpid());
    fprintf(f, "il2cpp_base=0x%lx\n", static_cast<unsigned long>(g_il2cpp_base));
    fprintf(f, "il2cpp_metadata_ready=%d\n", g_il2cpp_metadata_ready ? 1 : 0);
    fprintf(f, "startup_hooks_ready=%d\n", g_startup_hooks_ready ? 1 : 0);
    fprintf(f, "il2cpp_metadata_wait_ms=%d\n", g_il2cpp_metadata_wait_ms);
    fprintf(f, "il2cpp_runtime_min_settle_ms=%d\n", kIl2CppRuntimeMinSettleMs);
    fprintf(f, "il2cpp_metadata_poll_ms=%d\n", kIl2CppMetadataPollMs);
    fprintf(f, "il2cpp_metadata_timeout_ms=%d\n", kIl2CppMetadataTimeoutMs);
    fprintf(f, "last_config_path=%s\n", g_last_config_path);
    fprintf(f, "default_config_created=%d\n", g_default_config_created ? 1 : 0);
    fprintf(f, "config_loads=%llu\n", static_cast<unsigned long long>(g_config_loads));
    fprintf(f, "status_writes=%llu\n", static_cast<unsigned long long>(g_status_writes));
    fprintf(f, "headshot=%d\n", g_enable_headshot ? 1 : 0);
    fprintf(f, "godmode=%d\n", g_enable_godmode ? 1 : 0);
    fprintf(f, "damage_v1=%d\n", g_enable_damage_v1 ? 1 : 0);
    fprintf(f, "health=%d\n", g_enable_health ? 1 : 0);
    fprintf(f, "attack_speed=%d\n", g_enable_attack_speed ? 1 : 0);
    fprintf(f, "attack_speed_value=%f\n", static_cast<double>(kAlwaysOnAttackSpeedValue));
    fprintf(f, "shoot_through_walls=%d\n", g_enable_shoot_through_walls ? 1 : 0);
    fprintf(f, "walk_through_water=%d\n", g_enable_walk_through_water ? 1 : 0);
    fprintf(f, "walk_through_walls=%d\n", g_enable_walk_through_walls ? 1 : 0);
    fprintf(f, "inject_greed_skill=%d\n", g_enable_inject_greed_skill ? 1 : 0);
    fprintf(f, "inject_smart_skill=%d\n", g_enable_inject_smart_skill ? 1 : 0);
    fprintf(f, "game_speed=%d\n", g_enable_game_speed ? 1 : 0);
    fprintf(f, "game_speed_multiplier=%f\n", static_cast<double>(g_game_speed_multiplier));
    fprintf(f, "move_speed=%d\n", g_enable_move_speed ? 1 : 0);
    fprintf(f, "move_speed_multiplier=%f\n", static_cast<double>(g_move_speed_multiplier));
    fprintf(f, "skip_rewarded_ads=%d\n", g_skip_rewarded_ads ? 1 : 0);
    fprintf(f, "install_gold_hooks=%d\n", g_install_gold_hooks ? 1 : 0);
    fprintf(f, "gold_hooks_installed=%d\n", g_gold_hooks_installed ? 1 : 0);
    fprintf(f, "hook_installed_count=%llu\n", static_cast<unsigned long long>(g_hook_installed_count));
    fprintf(f, "hook_skipped_tiny_count=%llu\n", static_cast<unsigned long long>(g_hook_skipped_tiny_count));
    fprintf(f, "resolver.metadata=%llu\n", static_cast<unsigned long long>(g_resolve_metadata_count));
    fprintf(f, "resolver.aob=0\n");
    fprintf(f, "resolver.xref=0\n");
    fprintf(f, "resolver.rva=%llu\n", static_cast<unsigned long long>(g_resolve_rva_count));
    fprintf(f, "resolver.fail=%llu\n", static_cast<unsigned long long>(g_resolve_fail_count));
    fprintf(f, "resolver.last_error=%s\n", g_last_resolve_error);
    fprintf(f, "resolver.metadata_state=%s\n", g_last_metadata_state);
    fprintf(f, "field_resolver.metadata=%llu\n", static_cast<unsigned long long>(g_field_resolve_metadata_count));
    fprintf(f, "field_resolver.fallback=0\n");
    fprintf(f, "field_resolver.fail=%llu\n", static_cast<unsigned long long>(g_field_resolve_fail_count));
    fprintf(f, "field_resolver.state=%s\n", g_last_field_resolve_state);
    fprintf(f, "field_offsets_ready=%d\n", g_field_offsets_ready ? 1 : 0);
    fprintf(f, "field_offsets.entity_type=0x%lx\n", static_cast<unsigned long>(g_off_entity_base_type));
    fprintf(f, "field_offsets.bullet_through_wall=0x%lx\n", static_cast<unsigned long>(g_off_bullet_transmit_through_wall));
    fprintf(f, "field_offsets.move_moving=0x%lx\n", static_cast<unsigned long>(g_off_move_control_moving));
    fprintf(f, "field_offsets.move_direction=0x%lx\n", static_cast<unsigned long>(g_off_move_control_move_direction));
    fprintf(f, "field_offsets.adcallback_b_callback=0x%lx\n", static_cast<unsigned long>(g_off_adcallback_b_callback));
    fprintf(f, "field_offsets.adcallback_b_opened=0x%lx\n", static_cast<unsigned long>(g_off_adcallback_b_opened));
    fprintf(f, "field_offsets.base_driver_callback=0x%lx\n", static_cast<unsigned long>(g_off_base_driver_callback));
    fprintf(f, "field_offsets.wrapped_adapter_callbacks=0x%lx\n", static_cast<unsigned long>(g_off_wrapped_adapter_callbacks));
    fprintf(f, "field_offsets.obscured_vector3=key:0x%lx hidden:0x%lx inited:0x%lx fake:0x%lx active:0x%lx\n",
            static_cast<unsigned long>(g_off_obscured_vector3_key),
            static_cast<unsigned long>(g_off_obscured_vector3_hidden),
            static_cast<unsigned long>(g_off_obscured_vector3_inited),
            static_cast<unsigned long>(g_off_obscured_vector3_fake),
            static_cast<unsigned long>(g_off_obscured_vector3_fake_active));
    fprintf(f, "direct_patch.resolved=%llu\n", static_cast<unsigned long long>(g_direct_patch_resolved_count));
    fprintf(f, "direct_patch.writes=%llu\n", static_cast<unsigned long long>(g_direct_patch_write_count));
    fprintf(f, "direct_patch.fail=%llu\n", static_cast<unsigned long long>(g_direct_patch_fail_count));
    fprintf(f, "tiny_direct_patch=%d\n", g_tiny_direct_patch ? 1 : 0);
    fprintf(f, "gold_multiplier=%f\n", static_cast<double>(g_gold_multiplier));
    fprintf(f, "stage_gold_fixed=%f\n", static_cast<double>(g_stage_gold_fixed));
    fprintf(f, "gold_drop_repeats=%d\n", g_gold_drop_repeats);
    fprintf(f, "material_drop_repeats=%d\n", g_material_drop_repeats);
    fprintf(f, "repeat_cap_max=%d\n", kRepeatCapMax);
    fprintf(f, "max_drop_cap_patch=%d\n", g_max_drop_cap_patch ? 1 : 0);
    fprintf(f, "max_drop_cap_value=%d\n", clamp_max_drop_cap(g_max_drop_cap_value));
    fprintf(f, "max_gold_cap_value=%d\n", clamp_max_gold_cap(g_max_gold_cap_value));
    fprintf(f, "chapter_drop_total_counter_bypass=%d\n", total_counter_relax_enabled() ? 1 : 0);
    fprintf(f, "drop_manager_total_counter_offsets=disabled_metadata_only\n");
    fprintf(f, "stagelevel_direct_cap_count=%zu\n",
            sizeof(g_patch_stagelevel_max_caps) / sizeof(g_patch_stagelevel_max_caps[0]));
    fprintf(f, "istage_direct_gold_cap=skipped_full_int32_requires_concrete_hook\n");
    fprintf(f, "flags.add=%d get_fixed=%d get_scale=%d update_fixed=%d update_scale=%d formula=%d static=%d drop_scalar=%d drop_repeat=%d ratio=%d list=%d save=%d mat=%d cap=%d\n",
            g_gold_add_scale ? 1 : 0, g_gold_get_fixed ? 1 : 0, g_gold_get_scale ? 1 : 0,
            g_gold_update_fixed ? 1 : 0, g_gold_update_scale ? 1 : 0, g_gold_formula_scale ? 1 : 0,
            g_gold_static_scale ? 1 : 0, g_gold_drop_scalar ? 1 : 0, g_gold_drop_repeat ? 1 : 0,
            g_gold_ratio_scale ? 1 : 0, g_gold_list_scale ? 1 : 0, g_gold_save_realtime ? 1 : 0,
            g_material_drop_repeat ? 1 : 0, g_max_drop_cap_patch ? 1 : 0);
    fprintf(f, "hits.add_gold=%llu\n", static_cast<unsigned long long>(g_hit_add_gold));
    fprintf(f, "hits.stage_balance=%llu\n", static_cast<unsigned long long>(g_hit_stage_balance));
    fprintf(f, "hits.entity_formula=%llu\n", static_cast<unsigned long long>(g_hit_entity_formula));
    fprintf(f, "hits.static_gold=%llu\n", static_cast<unsigned long long>(g_hit_static_gold));
    fprintf(f, "hits.drop_scalar=%llu\n", static_cast<unsigned long long>(g_hit_drop_scalar));
    fprintf(f, "hits.deadgood_startdrop=%llu\n", static_cast<unsigned long long>(g_hit_deadgood_startdrop));
    fprintf(f, "hits.list_gold_scaled=%llu\n", static_cast<unsigned long long>(g_hit_list_gold_scaled));
    fprintf(f, "hits.list_material_scaled=%llu\n", static_cast<unsigned long long>(g_hit_list_material_scaled));
    fprintf(f, "hits.drop_mutator_gold=%llu\n", static_cast<unsigned long long>(g_hit_drop_mutator_gold));
    fprintf(f, "hits.drop_mutator_material=%llu\n", static_cast<unsigned long long>(g_hit_drop_mutator_material));
    fprintf(f, "hits.ratio=%llu\n", static_cast<unsigned long long>(g_hit_ratio));
    fprintf(f, "hits.max_drop=%llu\n", static_cast<unsigned long long>(g_hit_max_drop));
    fprintf(f, "hits.total_counter_relax=%llu\n", static_cast<unsigned long long>(g_hit_total_counter_relax));
    fprintf(f, "hits.always_damage=%llu\n", static_cast<unsigned long long>(g_hit_always_damage));
    fprintf(f, "hits.always_health=%llu\n", static_cast<unsigned long long>(g_hit_always_health));
    fprintf(f, "hits.always_attack_speed=%llu\n", static_cast<unsigned long long>(g_hit_always_attack_speed));
    fprintf(f, "hits.always_walls=%llu\n", static_cast<unsigned long long>(g_hit_always_walls));
    fprintf(f, "hits.always_inside_walls=%llu\n", static_cast<unsigned long long>(g_hit_always_inside_walls));
    fprintf(f, "hits.runtime_walls_apply=%llu\n", static_cast<unsigned long long>(g_hit_runtime_walls_apply));
    fprintf(f, "hits.runtime_walls_init=%llu\n", static_cast<unsigned long long>(g_hit_runtime_walls_init));
    fprintf(f, "hits.weapon_wall_field_apply=%llu\n", static_cast<unsigned long long>(g_hit_weapon_wall_field_apply));
    fprintf(f, "hits.bullet_transmit_wall_get=%llu\n", static_cast<unsigned long long>(g_hit_bullet_transmit_wall_get));
    fprintf(f, "hits.bullet_hitwall_bypass=%llu\n", static_cast<unsigned long long>(g_hit_bullet_hitwall_bypass));
    fprintf(f, "hits.bullet_hitwall_internal_bypass=%llu\n", static_cast<unsigned long long>(g_hit_bullet_hitwall_internal_bypass));
    fprintf(f, "hits.move_progress_get=%llu\n", static_cast<unsigned long long>(g_hit_move_progress_get));
    fprintf(f, "hits.move_progress_apply=%llu\n", static_cast<unsigned long long>(g_hit_move_progress_apply));
    fprintf(f, "hits.move_progress_hero=%llu\n", static_cast<unsigned long long>(g_hit_move_progress_hero));
    fprintf(f, "hits.move_progress_passthrough=%llu\n", static_cast<unsigned long long>(g_hit_move_progress_passthrough));
    fprintf(f, "hits.move_progress_substeps=%llu\n", static_cast<unsigned long long>(g_hit_move_progress_substeps));
    fprintf(f, "move_progress_last_speed=%f\n", static_cast<double>(g_last_move_progress_speed));
    fprintf(f, "move_progress_last_scaled=%f\n", static_cast<double>(g_last_move_progress_scaled));
    fprintf(f, "move_progress_last_steps=%d\n", static_cast<int>(g_last_move_progress_steps));
    fprintf(f, "hits.walk_water=%llu\n", static_cast<unsigned long long>(g_hit_walk_water));
    fprintf(f, "hits.walk_wall=%llu\n", static_cast<unsigned long long>(g_hit_walk_wall));
    fprintf(f, "hits.walk_apply=%llu\n", static_cast<unsigned long long>(g_hit_walk_apply));
    fprintf(f, "hits.walk_entitydata_apply=%llu\n", static_cast<unsigned long long>(g_hit_walk_entitydata_apply));
    fprintf(f, "hits.walk_mask_apply=%llu\n", static_cast<unsigned long long>(g_hit_walk_mask_apply));
    fprintf(f, "hits.walk_runtime_apply=%llu\n", static_cast<unsigned long long>(g_hit_walk_runtime_apply));
    fprintf(f, "hits.walk_skill_inject=%llu\n", static_cast<unsigned long long>(g_hit_walk_skill_inject));
    fprintf(f, "hits.walk_check_pos=%llu\n", static_cast<unsigned long long>(g_hit_walk_check_pos));
    fprintf(f, "hits.skill_inject_greed=%llu\n", static_cast<unsigned long long>(g_hit_skill_inject_greed));
    fprintf(f, "hits.skill_inject_smart=%llu\n", static_cast<unsigned long long>(g_hit_skill_inject_smart));
    fprintf(f, "hits.skill_inject_fail=%llu\n", static_cast<unsigned long long>(g_hit_skill_inject_fail));
    fprintf(f, "hits.skill_confirm_water=%llu\n", static_cast<unsigned long long>(g_hit_skill_confirm_water));
    fprintf(f, "hits.skill_confirm_greed=%llu\n", static_cast<unsigned long long>(g_hit_skill_confirm_greed));
    fprintf(f, "hits.skill_confirm_smart=%llu\n", static_cast<unsigned long long>(g_hit_skill_confirm_smart));
    fprintf(f, "hits.skill_confirm_fail=%llu\n", static_cast<unsigned long long>(g_hit_skill_confirm_fail));
    fprintf(f, "hits.skill_confirm_unavailable=%llu\n", static_cast<unsigned long long>(g_hit_skill_confirm_unavailable));
    fprintf(f, "hits.game_speed_get=%llu\n", static_cast<unsigned long long>(g_hit_game_speed_get));
    fprintf(f, "hits.game_speed_set=%llu\n", static_cast<unsigned long long>(g_hit_game_speed_set));
    fprintf(f, "hits.game_speed_apply=%llu\n", static_cast<unsigned long long>(g_hit_game_speed_apply));
    fprintf(f, "hits.ad_skip_isloaded=%llu\n", static_cast<unsigned long long>(g_hit_ad_skip_isloaded));
    fprintf(f, "hits.ad_skip_show=%llu\n", static_cast<unsigned long long>(g_hit_ad_skip_show));
    fprintf(f, "hits.ad_skip_high_ecpm=%llu\n", static_cast<unsigned long long>(g_hit_ad_skip_high_ecpm));
    fprintf(f, "hits.ad_skip_adapter=%llu\n", static_cast<unsigned long long>(g_hit_ad_skip_adapter));
    fprintf(f, "hits.ad_skip_driver=%llu\n", static_cast<unsigned long long>(g_hit_ad_skip_driver));
    fprintf(f, "hits.ad_skip_reward=%llu\n", static_cast<unsigned long long>(g_hit_ad_skip_reward));
    fprintf(f, "hits.ad_skip_close=%llu\n", static_cast<unsigned long long>(g_hit_ad_skip_close));
    fprintf(f, "hits.ad_skip_passthrough=%llu\n", static_cast<unsigned long long>(g_hit_ad_skip_passthrough));
    fclose(f);
    bump(g_status_writes);
}

static void* config_thread(void*) {
    while (true) {
        load_config_file_once();
        if (g_install_gold_hooks && g_il2cpp_base != 0 && g_startup_hooks_ready && !g_gold_hooks_installed) {
            install_gold_hooks_once(g_il2cpp_base);
        }
        if (g_il2cpp_base != 0 && g_startup_hooks_ready) {
            update_tiny_direct_patches(g_il2cpp_base);
        }
        write_status_file_once();
        sleep(2);
    }
    return nullptr;
}

static int get_entity_type_from_entity_base(void* entity_base);

static int get_entity_type_from_entity_data(void* thiz) {
    if (!g_field_offsets_ready || !thiz) return -1;
    void* entity_base = *reinterpret_cast<void**>(
        reinterpret_cast<uintptr_t>(thiz) + g_off_entity_data_entity
    );
    if (!entity_base) return -1;
    return get_entity_type_from_entity_base(entity_base);
}

static int get_entity_type_from_entity_base(void* entity_base) {
    if (!g_field_offsets_ready || !entity_base) return -1;
    return *reinterpret_cast<int*>(
        reinterpret_cast<uintptr_t>(entity_base) + g_off_entity_base_type
    );
}

static void* get_entity_base_from_hit_ctrl(void* hit_ctrl) {
    if (!g_field_offsets_ready || !hit_ctrl) return nullptr;
    return *reinterpret_cast<void**>(
        reinterpret_cast<uintptr_t>(hit_ctrl) + g_off_entity_hit_ctrl_entity
    );
}

static void* get_entity_data_from_entity_base(void* entity_base) {
    if (!g_field_offsets_ready || !entity_base) return nullptr;
    return *reinterpret_cast<void**>(
        reinterpret_cast<uintptr_t>(entity_base) + g_off_entity_base_data
    );
}

static bool is_hero_entity_base(void* entity_base) {
    return get_entity_type_from_entity_base(entity_base) == kEntityTypeHero;
}

static void* get_entity_base_from_bullet_transmit(void* transmit) {
    if (!g_field_offsets_ready || !transmit) return nullptr;
    return *reinterpret_cast<void**>(
        reinterpret_cast<uintptr_t>(transmit) + g_off_bullet_transmit_entity
    );
}

static void* get_weapon_data_from_bullet_transmit(void* transmit) {
    if (!g_field_offsets_ready || !transmit) return nullptr;
    return *reinterpret_cast<void**>(
        reinterpret_cast<uintptr_t>(transmit) + g_off_bullet_transmit_weapon_data
    );
}

static void* get_entity_base_from_bullet_base(void* bullet_base) {
    if (!g_field_offsets_ready || !bullet_base) return nullptr;
    return *reinterpret_cast<void**>(
        reinterpret_cast<uintptr_t>(bullet_base) + g_off_bullet_base_entity
    );
}

static void* get_weapon_data_from_bullet_base(void* bullet_base) {
    if (!g_field_offsets_ready || !bullet_base) return nullptr;
    return *reinterpret_cast<void**>(
        reinterpret_cast<uintptr_t>(bullet_base) + g_off_bullet_base_weapon_data
    );
}

static void* get_bullet_transmit_from_bullet_base(void* bullet_base) {
    if (!g_field_offsets_ready || !bullet_base) return nullptr;
    return *reinterpret_cast<void**>(
        reinterpret_cast<uintptr_t>(bullet_base) + g_off_bullet_base_transmit
    );
}

static void* get_entity_base_from_move_control(void* move_control) {
    if (!g_field_offsets_ready || !move_control) return nullptr;
    return *reinterpret_cast<void**>(
        reinterpret_cast<uintptr_t>(move_control) + g_off_move_control_entity
    );
}

static void force_bullet_transmit_through_wall(void* transmit) {
    if (!g_field_offsets_ready || !transmit) return;
    *reinterpret_cast<volatile uint8_t*>(
        reinterpret_cast<uintptr_t>(transmit) + g_off_bullet_transmit_through_wall
    ) = 1;
}

static void force_weapon_data_through_wall(void* weapon_data) {
    if (!g_field_offsets_ready || !weapon_data) return;
    uintptr_t base = reinterpret_cast<uintptr_t>(weapon_data);
    *reinterpret_cast<volatile uint8_t*>(base + g_off_weapon_weapon_through_wall) = 1;
    *reinterpret_cast<volatile uint8_t*>(base + g_off_weapon_weapon_through_inside_wall) = 1;
    bump(g_hit_weapon_wall_field_apply);
}

struct Il2CppStringLite {
    void* klass;
    void* monitor;
    int32_t length;
    uint16_t chars[1];
};

static bool il2cpp_string_equals_ascii(void* value, const char* ascii) {
    if (!value || !ascii) return false;
    Il2CppStringLite* str = reinterpret_cast<Il2CppStringLite*>(value);
    int32_t len = str->length;
    if (len < 0 || len > 64) return false;
    for (int32_t i = 0; i < len; ++i) {
        unsigned char expected = static_cast<unsigned char>(ascii[i]);
        if (expected == 0 || str->chars[i] != expected) return false;
    }
    return ascii[len] == '\0';
}

static bool should_force_fly_layer(void* layer) {
    if (g_enable_walk_through_water && il2cpp_string_equals_ascii(layer, "Entity2Water")) {
        return true;
    }
    if (!g_enable_walk_through_walls) return false;
    return il2cpp_string_equals_ascii(layer, "Entity2Stone") ||
           il2cpp_string_equals_ascii(layer, "Entity2DragonStone");
}

static void set_hero_traversal_flags_direct(void* entity_base) {
    if (!g_field_offsets_ready || !entity_base || !is_hero_entity_base(entity_base)) return;
    uintptr_t base = reinterpret_cast<uintptr_t>(entity_base);
    if (g_enable_walk_through_water) {
        *reinterpret_cast<volatile uint8_t*>(base + g_off_entity_base_fly_water) = 1;
    }
    if (g_enable_walk_through_walls) {
        *reinterpret_cast<volatile uint8_t*>(base + g_off_entity_base_fly_stone) = 1;
    }
    void* entity_data = get_entity_data_from_entity_base(entity_base);
    if (entity_data) {
        uintptr_t data = reinterpret_cast<uintptr_t>(entity_data);
        if (g_enable_walk_through_water) {
            volatile int32_t* count = reinterpret_cast<volatile int32_t*>(
                data + g_off_entity_data_fly_water_count
            );
            if (*count <= 0) {
                *count = 1;
                bump(g_hit_walk_entitydata_apply);
            }
        }
        if (g_enable_walk_through_walls) {
            volatile int32_t* count = reinterpret_cast<volatile int32_t*>(
                data + g_off_entity_data_fly_stone_count
            );
            if (*count <= 0) {
                *count = 1;
                bump(g_hit_walk_entitydata_apply);
            }
        }
    }
    bump(g_hit_walk_apply);
}

using GetHeadShotFn = bool (*)(void* thiz, void* source, void* data, void* method);
using GetMissFn = bool (*)(void* thiz, void* otherhs, void* method);
using UpgradeBaseIntFn = int32_t (*)(void* thiz, int32_t charid, void* method);
using WeaponFloatGetterFn = float (*)(void* thiz, void* method);
using WeaponBoolGetterFn = bool (*)(void* thiz, void* method);
using BulletTransmitInitSimpleFn = void* (*)(void* thiz, void* entity, int32_t bullet_id, bool clear, float final_hit_ratio, void* method);
using BulletTransmitInitFullFn = void* (*)(void* thiz, void* entity, int32_t bullet_id, float attack_ratio, float back_ratio, int32_t through_enemy, float through_ratio, bool clear, float final_hit_ratio, void* method);
using BulletTransmitBoolGetterFn = bool (*)(void* thiz, void* method);
using BulletBaseWallFn = void (*)(void* thiz, void* collider_or_state, void* method);
using EntityBoolGetterFn = bool (*)(void* thiz, void* method);
using EntitySetBoolFn = void (*)(void* thiz, bool value, void* method);
using EntityVoidFn = void (*)(void* thiz, void* method);
using EntityAddSkillFn = void (*)(void* thiz, int32_t skill_id, void* method);
using EntityContainsSkillFn = bool (*)(void* thiz, int32_t skill_id, void* method);
using EntityVector3Fn = Vector3Lite (*)(void* thiz, Vector3Lite value, void* method);
using EntitySelfMoveByFn = void (*)(void* thiz, Vector3Lite pos, void* method);
using EntitySetFlyOneFn = void (*)(void* thiz, void* layer, bool value, void* method);
using MoveControlVoidFn = void (*)(void* thiz, void* method);
using TimeDeltaGetterFn = float (*)(void* method);
using TimeScaleGetterFn = float (*)(void* method);
using TimeScaleSetterFn = void (*)(float value, void* method);
using AdCallbackIsLoadedFn = bool (*)(void* thiz, int32_t adapter_id, void* method);
using AdCallbackShowFn = void (*)(void* thiz, int32_t adapter_id, void* method);
using AdCallbackEventFn = void (*)(void* thiz, void* sender, void* network_name, void* method);
using DriverBoolFn = bool (*)(void* thiz, void* method);
using AdapterShowNoArgFn = bool (*)(void* thiz, void* method);
using AdapterShowCallbackFn = bool (*)(void* thiz, void* callback, void* method);
using AdapterShowCallbackSourceFn = bool (*)(void* thiz, void* callback, int32_t source, void* method);
using RewardedHighEcpmIsLoadedFn = bool (*)(void* method);
using RewardedHighEcpmShowFn = void (*)(void* callback, int32_t source, void* method);
static GetHeadShotFn g_orig_get_headshot = nullptr;
static GetMissFn g_orig_get_miss = nullptr;
static UpgradeBaseIntFn g_orig_get_atk_base = nullptr;
static UpgradeBaseIntFn g_orig_get_hp_base = nullptr;
static WeaponFloatGetterFn g_orig_weapon_get_speed = nullptr;
static WeaponFloatGetterFn g_orig_weapon_get_attack_speed = nullptr;
static WeaponBoolGetterFn g_orig_weapon_get_through_wall = nullptr;
static WeaponBoolGetterFn g_orig_weapon_get_through_inside_wall = nullptr;
static BulletTransmitInitSimpleFn g_orig_bullet_transmit_init_simple = nullptr;
static BulletTransmitInitFullFn g_orig_bullet_transmit_init_full = nullptr;
static BulletTransmitBoolGetterFn g_orig_bullet_transmit_get_through_wall = nullptr;
static BulletBaseWallFn g_orig_bulletbase_hit_wall = nullptr;
static BulletBaseWallFn g_orig_bulletbase_hitwall_internal = nullptr;
static EntitySetBoolFn g_orig_entitybase_set_fly_water = nullptr;
static EntityBoolGetterFn g_orig_entitybase_get_fly_water = nullptr;
static EntitySetBoolFn g_orig_entitybase_set_fly_stone = nullptr;
static EntityBoolGetterFn g_orig_entitybase_get_on_cal_can_move = nullptr;
static EntitySetBoolFn g_orig_entitybase_set_collider = nullptr;
static EntitySetBoolFn g_orig_entitybase_set_fly_all = nullptr;
static EntityVector3Fn g_orig_entitybase_check_pos = nullptr;
static EntityVoidFn g_orig_entitybase_add_init_skills = nullptr;
static EntityAddSkillFn g_entitybase_add_skill = nullptr;
static EntityContainsSkillFn g_entitybase_contains_skill = nullptr;
static EntitySelfMoveByFn g_entitybase_self_move_by = nullptr;
static EntitySetFlyOneFn g_orig_entityhitctrl_set_fly_one = nullptr;
static MoveControlVoidFn g_orig_movecontrol_update_progress = nullptr;
static TimeDeltaGetterFn g_time_get_delta_time = nullptr;
static TimeScaleGetterFn g_orig_time_get_scale = nullptr;
static TimeScaleSetterFn g_orig_time_set_scale = nullptr;
static AdCallbackIsLoadedFn g_orig_adcallback_is_loaded = nullptr;
static AdCallbackShowFn g_orig_adcallback_show = nullptr;
static AdCallbackEventFn g_adcallback_on_reward = nullptr;
static AdCallbackEventFn g_adcallback_on_close = nullptr;
static AdCallbackEventFn g_wrappeddriver_on_reward = nullptr;
static AdCallbackEventFn g_wrappeddriver_on_close = nullptr;
static AdCallbackEventFn g_combineddriver_on_reward = nullptr;
static AdCallbackEventFn g_combineddriver_on_close = nullptr;
static AdCallbackEventFn g_callbackrouter_on_reward = nullptr;
static AdCallbackEventFn g_callbackrouter_on_close = nullptr;
static DriverBoolFn g_orig_almax_rewarded_is_loaded = nullptr;
static DriverBoolFn g_orig_almax_rewarded_show = nullptr;
static DriverBoolFn g_orig_wrapped_adapter_is_loaded = nullptr;
static AdapterShowNoArgFn g_orig_wrapped_adapter_show = nullptr;
static AdapterShowCallbackFn g_orig_wrapped_adapter_show_callback = nullptr;
static AdapterShowCallbackSourceFn g_orig_wrapped_adapter_show_callback_source = nullptr;
static RewardedHighEcpmIsLoadedFn g_orig_rewarded_high_ecpm_is_loaded = nullptr;
static RewardedHighEcpmShowFn g_orig_rewarded_high_ecpm_show = nullptr;
static void* g_adcallback_control_class = nullptr;
static void* g_wrappeddriver_class = nullptr;
static void* g_combineddriver_class = nullptr;
static void* g_callbackrouter_class = nullptr;
static uintptr_t g_last_traversal_entity = 0;
static uintptr_t g_last_water_skill_inject_entity = 0;
static uintptr_t g_last_greed_skill_inject_entity = 0;
static uintptr_t g_last_smart_skill_inject_entity = 0;
static __thread bool g_applying_traversal_runtime = false;
static __thread bool g_completing_rewarded_ad = false;

static bool hero_traversal_needs_native_sync(void* entity_base, bool first_seen) {
    if (!g_field_offsets_ready || !entity_base || !is_hero_entity_base(entity_base)) return false;
    uintptr_t base = reinterpret_cast<uintptr_t>(entity_base);
    bool needs_sync = first_seen;
    if (g_enable_walk_through_water) {
        needs_sync = needs_sync ||
            (*reinterpret_cast<volatile uint8_t*>(base + g_off_entity_base_fly_water) == 0);
    }
    if (g_enable_walk_through_walls) {
        needs_sync = needs_sync ||
            (*reinterpret_cast<volatile uint8_t*>(base + g_off_entity_base_fly_stone) == 0);
    }
    void* entity_data = get_entity_data_from_entity_base(entity_base);
    if (entity_data) {
        uintptr_t data = reinterpret_cast<uintptr_t>(entity_data);
        if (g_enable_walk_through_water) {
            needs_sync = needs_sync ||
                (*reinterpret_cast<volatile int32_t*>(data + g_off_entity_data_fly_water_count) <= 0);
        }
        if (g_enable_walk_through_walls) {
            needs_sync = needs_sync ||
                (*reinterpret_cast<volatile int32_t*>(data + g_off_entity_data_fly_stone_count) <= 0);
        }
    }
    return needs_sync;
}

static bool hero_contains_skill(void* entity_base, int32_t skill_id) {
    if (!g_entitybase_contains_skill || !entity_base) return false;
    return g_entitybase_contains_skill(entity_base, skill_id, nullptr);
}

static void inject_hero_battle_skill(void* entity_base,
                                     int32_t skill_id,
                                     const char* skill_name,
                                     bool enabled,
                                     uintptr_t* last_entity,
                                     volatile uint64_t* inject_counter,
                                     volatile uint64_t* confirm_counter,
                                     bool confirm_required,
                                     bool force) {
    if (!enabled || !entity_base || !is_hero_entity_base(entity_base)) {
        return;
    }
    if (!g_entitybase_add_skill) {
        bump(g_hit_skill_inject_fail);
        return;
    }
    uintptr_t base = reinterpret_cast<uintptr_t>(entity_base);
    if (!force && last_entity && *last_entity == base) return;
    if (last_entity) *last_entity = base;

    bool already_present = hero_contains_skill(entity_base, skill_id);
    if (!already_present) {
        g_entitybase_add_skill(entity_base, skill_id, nullptr);
        bump(*inject_counter);
    }

    if (g_entitybase_contains_skill) {
        bool confirmed = hero_contains_skill(entity_base, skill_id);
        if (confirmed) {
            bump(*confirm_counter);
        } else if (confirm_required) {
            bump(g_hit_skill_confirm_fail);
        }
        LOGD("Battle skill %s id=%d hero=%p force=%d already=%d confirmed=%d",
             skill_name ? skill_name : "unknown", skill_id, entity_base,
             force ? 1 : 0, already_present ? 1 : 0, confirmed ? 1 : 0);
    } else {
        if (confirm_required) bump(g_hit_skill_confirm_unavailable);
        LOGD("Battle skill %s id=%d hero=%p force=%d already=%d confirm=unavailable",
             skill_name ? skill_name : "unknown", skill_id, entity_base,
             force ? 1 : 0, already_present ? 1 : 0);
    }
}

static void inject_hero_battle_skills(void* entity_base, bool force) {
    inject_hero_battle_skill(entity_base, kSkillWalkThroughWater, "walk_through_water",
                             g_enable_walk_through_water,
                             &g_last_water_skill_inject_entity,
                             &g_hit_walk_skill_inject,
                             &g_hit_skill_confirm_water,
                             false,
                             force);
    inject_hero_battle_skill(entity_base, kSkillGreed, "greed",
                             g_enable_inject_greed_skill,
                             &g_last_greed_skill_inject_entity,
                             &g_hit_skill_inject_greed,
                             &g_hit_skill_confirm_greed,
                             true,
                             force);
    inject_hero_battle_skill(entity_base, kSkillSmart, "smart",
                             g_enable_inject_smart_skill,
                             &g_last_smart_skill_inject_entity,
                             &g_hit_skill_inject_smart,
                             &g_hit_skill_confirm_smart,
                             true,
                             force);
}

static void apply_hero_traversal_runtime(void* entity_base) {
    if (!entity_base || !is_hero_entity_base(entity_base) ||
        !(g_enable_walk_through_water || g_enable_walk_through_walls)) {
        return;
    }
    bool first_seen = g_last_traversal_entity != reinterpret_cast<uintptr_t>(entity_base);
    bool needs_native_sync = hero_traversal_needs_native_sync(entity_base, first_seen);
    inject_hero_battle_skills(entity_base, false);
    set_hero_traversal_flags_direct(entity_base);
    if (!needs_native_sync || g_applying_traversal_runtime) return;

    g_last_traversal_entity = reinterpret_cast<uintptr_t>(entity_base);
    g_applying_traversal_runtime = true;
    if (g_enable_walk_through_water && g_orig_entitybase_set_fly_water) {
        g_orig_entitybase_set_fly_water(entity_base, true, nullptr);
    }
    if (g_enable_walk_through_walls) {
        if (g_orig_entitybase_set_fly_stone) {
            g_orig_entitybase_set_fly_stone(entity_base, true, nullptr);
        }
    }
    set_hero_traversal_flags_direct(entity_base);
    bump(g_hit_walk_runtime_apply);
    g_applying_traversal_runtime = false;
}

static void apply_hero_bullet_runtime_wall(void* transmit, void* entity_hint, bool from_init) {
    if (!g_enable_shoot_through_walls || !transmit) return;
    void* entity_base = entity_hint ? entity_hint : get_entity_base_from_bullet_transmit(transmit);
    if (!is_hero_entity_base(entity_base)) return;
    force_weapon_data_through_wall(get_weapon_data_from_bullet_transmit(transmit));
    force_bullet_transmit_through_wall(transmit);
    if (from_init) bump(g_hit_runtime_walls_init);
    else bump(g_hit_runtime_walls_apply);
}

static bool is_hero_bullet_base(void* bullet_base) {
    if (!g_enable_shoot_through_walls || !bullet_base) return false;
    void* entity_base = get_entity_base_from_bullet_base(bullet_base);
    if (!entity_base) {
        entity_base = get_entity_base_from_bullet_transmit(get_bullet_transmit_from_bullet_base(bullet_base));
    }
    return is_hero_entity_base(entity_base);
}

static void apply_hero_bulletbase_runtime_wall(void* bullet_base) {
    if (!is_hero_bullet_base(bullet_base)) return;
    force_weapon_data_through_wall(get_weapon_data_from_bullet_base(bullet_base));
    void* transmit = get_bullet_transmit_from_bullet_base(bullet_base);
    if (transmit) {
        force_weapon_data_through_wall(get_weapon_data_from_bullet_transmit(transmit));
        force_bullet_transmit_through_wall(transmit);
        bump(g_hit_runtime_walls_apply);
    }
}

static bool hk_get_headshot(void* thiz, void* source, void* data, void* method) {
    if (!g_orig_get_headshot) return true;
    if (!g_enable_headshot) return g_orig_get_headshot(thiz, source, data, method);
    int entity_type = get_entity_type_from_entity_data(thiz);
    if (entity_type < 0) return g_orig_get_headshot(thiz, source, data, method);
    if (entity_type != kEntityTypeHero) return true;
    return g_orig_get_headshot(thiz, source, data, method);
}

static bool hk_get_miss(void* thiz, void* otherhs, void* method) {
    if (!g_orig_get_miss) return true;
    if (!g_enable_godmode) return g_orig_get_miss(thiz, otherhs, method);
    int entity_type = get_entity_type_from_entity_data(thiz);
    if (entity_type < 0) return g_orig_get_miss(thiz, otherhs, method);
    if (entity_type == kEntityTypeHero) return true;
    return g_orig_get_miss(thiz, otherhs, method);
}

static int32_t hk_get_atk_base(void* thiz, int32_t charid, void* method) {
    bump(g_hit_always_damage);
    if (!g_enable_damage_v1) {
        return g_orig_get_atk_base ? g_orig_get_atk_base(thiz, charid, method) : 0;
    }
    return kAlwaysOnDamageValue;
}

static int32_t hk_get_hp_base(void* thiz, int32_t charid, void* method) {
    bump(g_hit_always_health);
    if (!g_enable_health) {
        return g_orig_get_hp_base ? g_orig_get_hp_base(thiz, charid, method) : 0;
    }
    return kAlwaysOnHealthValue;
}

static float hk_weapon_get_speed(void* thiz, void* method) {
    bump(g_hit_always_attack_speed);
    if (!g_enable_attack_speed) {
        return g_orig_weapon_get_speed ? g_orig_weapon_get_speed(thiz, method) : 0.0f;
    }
    return kAlwaysOnAttackSpeedValue;
}

static float hk_weapon_get_attack_speed(void* thiz, void* method) {
    bump(g_hit_always_attack_speed);
    if (!g_enable_attack_speed) {
        return g_orig_weapon_get_attack_speed ? g_orig_weapon_get_attack_speed(thiz, method) : 0.0f;
    }
    return kAlwaysOnAttackSpeedValue;
}

static bool hk_weapon_get_through_wall(void* thiz, void* method) {
    bump(g_hit_always_walls);
    bool original = g_orig_weapon_get_through_wall ? g_orig_weapon_get_through_wall(thiz, method) : false;
    if (!g_enable_shoot_through_walls) {
        return original;
    }
    force_weapon_data_through_wall(thiz);
    return true;
}

static bool hk_weapon_get_through_inside_wall(void* thiz, void* method) {
    bump(g_hit_always_inside_walls);
    bool original = g_orig_weapon_get_through_inside_wall
        ? g_orig_weapon_get_through_inside_wall(thiz, method)
        : false;
    if (!g_enable_shoot_through_walls) {
        return original;
    }
    force_weapon_data_through_wall(thiz);
    return true;
}

static bool hk_bullet_transmit_get_through_wall(void* thiz, void* method) {
    bump(g_hit_bullet_transmit_wall_get);
    bool original = g_orig_bullet_transmit_get_through_wall
        ? g_orig_bullet_transmit_get_through_wall(thiz, method)
        : false;
    if (!g_enable_shoot_through_walls) return original;
    force_bullet_transmit_through_wall(thiz);
    return true;
}

static void* hk_bullet_transmit_init_simple(void* thiz, void* entity, int32_t bullet_id, bool clear, float final_hit_ratio, void* method) {
    void* result = g_orig_bullet_transmit_init_simple
        ? g_orig_bullet_transmit_init_simple(thiz, entity, bullet_id, clear, final_hit_ratio, method)
        : thiz;
    apply_hero_bullet_runtime_wall(result, entity, true);
    return result;
}

static void* hk_bullet_transmit_init_full(void* thiz, void* entity, int32_t bullet_id, float attack_ratio, float back_ratio, int32_t through_enemy, float through_ratio, bool clear, float final_hit_ratio, void* method) {
    void* result = g_orig_bullet_transmit_init_full
        ? g_orig_bullet_transmit_init_full(thiz, entity, bullet_id, attack_ratio, back_ratio, through_enemy, through_ratio, clear, final_hit_ratio, method)
        : thiz;
    apply_hero_bullet_runtime_wall(result, entity, true);
    return result;
}

static void hk_bulletbase_hit_wall(void* thiz, void* collider, void* method) {
    apply_hero_bulletbase_runtime_wall(thiz);
    if (is_hero_bullet_base(thiz)) {
        bump(g_hit_bullet_hitwall_bypass);
        return;
    }
    if (g_orig_bulletbase_hit_wall) g_orig_bulletbase_hit_wall(thiz, collider, method);
}

static void hk_bulletbase_hitwall_internal(void* thiz, void* state, void* method) {
    apply_hero_bulletbase_runtime_wall(thiz);
    if (is_hero_bullet_base(thiz)) {
        bump(g_hit_bullet_hitwall_internal_bypass);
        return;
    }
    if (g_orig_bulletbase_hitwall_internal) g_orig_bulletbase_hitwall_internal(thiz, state, method);
}

static void hk_entitybase_set_fly_water(void* thiz, bool value, void* method) {
    bump(g_hit_walk_water);
    if (g_enable_walk_through_water && is_hero_entity_base(thiz)) value = true;
    if (g_orig_entitybase_set_fly_water) g_orig_entitybase_set_fly_water(thiz, value, method);
    set_hero_traversal_flags_direct(thiz);
}

static bool hk_entitybase_get_fly_water(void* thiz, void* method) {
    bump(g_hit_walk_water);
    if (g_enable_walk_through_water && is_hero_entity_base(thiz)) {
        set_hero_traversal_flags_direct(thiz);
        return true;
    }
    return g_orig_entitybase_get_fly_water ? g_orig_entitybase_get_fly_water(thiz, method) : false;
}

static void hk_entitybase_set_fly_stone(void* thiz, bool value, void* method) {
    bump(g_hit_walk_wall);
    if (g_enable_walk_through_walls && is_hero_entity_base(thiz)) value = true;
    if (g_orig_entitybase_set_fly_stone) g_orig_entitybase_set_fly_stone(thiz, value, method);
    set_hero_traversal_flags_direct(thiz);
}

static bool hk_entitybase_get_on_cal_can_move(void* thiz, void* method) {
    bool can_move = g_orig_entitybase_get_on_cal_can_move
        ? g_orig_entitybase_get_on_cal_can_move(thiz, method)
        : true;
    if ((g_enable_walk_through_water || g_enable_walk_through_walls) && is_hero_entity_base(thiz)) {
        apply_hero_traversal_runtime(thiz);
        return true;
    }
    return can_move;
}

static void hk_entitybase_set_collider(void* thiz, bool value, void* method) {
    if (g_orig_entitybase_set_collider) g_orig_entitybase_set_collider(thiz, value, method);
    if (!value || !is_hero_entity_base(thiz)) return;
    apply_hero_traversal_runtime(thiz);
}

static void hk_entitybase_set_fly_all(void* thiz, bool value, void* method) {
    bool hero = is_hero_entity_base(thiz);
    bool forced = hero && (g_enable_walk_through_water || g_enable_walk_through_walls);
    if (g_orig_entitybase_set_fly_all) g_orig_entitybase_set_fly_all(thiz, value, method);
    if (forced) apply_hero_traversal_runtime(thiz);
}

static Vector3Lite hk_entitybase_check_pos(void* thiz, Vector3Lite pos, void* method) {
    if ((g_enable_walk_through_water || g_enable_walk_through_walls) && is_hero_entity_base(thiz)) {
        apply_hero_traversal_runtime(thiz);
        bump(g_hit_walk_check_pos);
    }
    return g_orig_entitybase_check_pos ? g_orig_entitybase_check_pos(thiz, pos, method) : pos;
}

static void hk_entitybase_add_init_skills(void* thiz, void* method) {
    if (g_orig_entitybase_add_init_skills) g_orig_entitybase_add_init_skills(thiz, method);
    inject_hero_battle_skills(thiz, true);
    apply_hero_traversal_runtime(thiz);
}

static void hk_entityhitctrl_set_fly_one(void* thiz, void* layer, bool value, void* method) {
    void* entity_base = get_entity_base_from_hit_ctrl(thiz);
    bool forced = is_hero_entity_base(entity_base) && should_force_fly_layer(layer);
    if (forced) value = true;
    if (g_orig_entityhitctrl_set_fly_one) g_orig_entityhitctrl_set_fly_one(thiz, layer, value, method);
    if (forced) set_hero_traversal_flags_direct(entity_base);
}

static float float_from_bits(uint32_t bits) {
    float value = 0.0f;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

static uint32_t bits_from_float(float value) {
    uint32_t bits = 0;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static Vector3Lite decrypt_obscured_vector3(uintptr_t obscured) {
    uint32_t key = static_cast<uint32_t>(
        *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_key)
    );
    uint32_t hidden_x = static_cast<uint32_t>(
        *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_hidden)
    );
    uint32_t hidden_y = static_cast<uint32_t>(
        *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_hidden + sizeof(int32_t))
    );
    uint32_t hidden_z = static_cast<uint32_t>(
        *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_hidden + sizeof(int32_t) * 2)
    );
    return {
        float_from_bits(hidden_x ^ key),
        float_from_bits(hidden_y ^ key),
        float_from_bits(hidden_z ^ key),
    };
}

static void encrypt_obscured_vector3(uintptr_t obscured, const Vector3Lite& value) {
    uint32_t key = static_cast<uint32_t>(
        *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_key)
    );
    *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_hidden) =
        static_cast<int32_t>(bits_from_float(value.x) ^ key);
    *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_hidden + sizeof(int32_t)) =
        static_cast<int32_t>(bits_from_float(value.y) ^ key);
    *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_hidden + sizeof(int32_t) * 2) =
        static_cast<int32_t>(bits_from_float(value.z) ^ key);

    volatile uint8_t* fake_active = reinterpret_cast<volatile uint8_t*>(
        obscured + g_off_obscured_vector3_fake_active
    );
    if (*fake_active) {
        *reinterpret_cast<Vector3Lite*>(obscured + g_off_obscured_vector3_fake) = value;
    }
}

static void save_obscured_vector3(uintptr_t obscured, ObscuredVector3Snapshot* snapshot) {
    snapshot->key = *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_key);
    snapshot->hidden_x = *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_hidden);
    snapshot->hidden_y = *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_hidden + sizeof(int32_t));
    snapshot->hidden_z = *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_hidden + sizeof(int32_t) * 2);
    snapshot->inited = *reinterpret_cast<volatile uint8_t*>(obscured + g_off_obscured_vector3_inited);
    snapshot->fake = *reinterpret_cast<Vector3Lite*>(obscured + g_off_obscured_vector3_fake);
    snapshot->fake_active = *reinterpret_cast<volatile uint8_t*>(obscured + g_off_obscured_vector3_fake_active);
}

static void restore_obscured_vector3(uintptr_t obscured, const ObscuredVector3Snapshot& snapshot) {
    *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_key) = snapshot.key;
    *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_hidden) = snapshot.hidden_x;
    *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_hidden + sizeof(int32_t)) = snapshot.hidden_y;
    *reinterpret_cast<volatile int32_t*>(obscured + g_off_obscured_vector3_hidden + sizeof(int32_t) * 2) = snapshot.hidden_z;
    *reinterpret_cast<volatile uint8_t*>(obscured + g_off_obscured_vector3_inited) = snapshot.inited;
    *reinterpret_cast<Vector3Lite*>(obscured + g_off_obscured_vector3_fake) = snapshot.fake;
    *reinterpret_cast<volatile uint8_t*>(obscured + g_off_obscured_vector3_fake_active) = snapshot.fake_active;
}

static bool read_move_progress_direction(void* move_control,
                                         ObscuredVector3Snapshot* snapshot,
                                         Vector3Lite* direction,
                                         float* magnitude) {
    if (!g_field_offsets_ready || !move_control || !snapshot || !direction || !magnitude) return false;
    uintptr_t obscured = reinterpret_cast<uintptr_t>(move_control) + g_off_move_control_move_direction;
    save_obscured_vector3(obscured, snapshot);

    *direction = decrypt_obscured_vector3(obscured);
    *magnitude = sqrtf(direction->x * direction->x + direction->y * direction->y + direction->z * direction->z);
    g_last_move_progress_speed = *magnitude;
    if (!isfinite(*magnitude) || *magnitude <= 0.001f ||
        !isfinite(direction->x) || !isfinite(direction->y) || !isfinite(direction->z)) {
        g_last_move_progress_scaled = *magnitude;
        return false;
    }
    return true;
}

static bool write_move_progress_direction_scaled(void* move_control,
                                                 const Vector3Lite& direction,
                                                 float scale) {
    if (!g_field_offsets_ready || !move_control || scale <= 0.0f || !isfinite(scale)) return false;
    Vector3Lite scaled = {
        static_cast<float>(static_cast<double>(direction.x) * static_cast<double>(scale)),
        static_cast<float>(static_cast<double>(direction.y) * static_cast<double>(scale)),
        static_cast<float>(static_cast<double>(direction.z) * static_cast<double>(scale)),
    };
    if (!isfinite(scaled.x) || !isfinite(scaled.y) || !isfinite(scaled.z)) {
        return false;
    }

    uintptr_t obscured = reinterpret_cast<uintptr_t>(move_control) + g_off_move_control_move_direction;
    encrypt_obscured_vector3(obscured, scaled);
    return true;
}

static bool move_control_is_moving(void* move_control) {
    if (!g_field_offsets_ready || !move_control) return false;
    return *reinterpret_cast<volatile uint8_t*>(
        reinterpret_cast<uintptr_t>(move_control) + g_off_move_control_moving
    ) != 0;
}

static bool apply_extra_self_move(void* move_control, void* entity_base, float extra_multiplier) {
    if (!move_control || !entity_base || !g_entitybase_self_move_by || !g_time_get_delta_time) {
        return false;
    }
    if (extra_multiplier <= kMoveProgressFractionEpsilon || !isfinite(extra_multiplier)) return false;
    if (!move_control_is_moving(move_control)) return false;

    ObscuredVector3Snapshot snapshot{};
    Vector3Lite direction{};
    float magnitude = 0.0f;
    if (!read_move_progress_direction(move_control, &snapshot, &direction, &magnitude)) return false;

    float delta_time = g_time_get_delta_time(nullptr);
    if (!isfinite(delta_time) || delta_time <= 0.0f || delta_time > 1.0f) return false;

    g_last_move_progress_scaled = magnitude * (extra_multiplier + 1.0f);
    int32_t whole_steps = static_cast<int32_t>(floorf(extra_multiplier));
    if (whole_steps < 0) whole_steps = 0;
    if (whole_steps > kMoveProgressMaxSubsteps) whole_steps = kMoveProgressMaxSubsteps;
    float fractional_step = extra_multiplier - static_cast<float>(whole_steps);
    if (fractional_step < kMoveProgressFractionEpsilon) fractional_step = 0.0f;

    int32_t applied_steps = 0;
    for (int32_t i = 0; i < whole_steps; ++i) {
        Vector3Lite delta = {
            static_cast<float>(static_cast<double>(direction.x) * static_cast<double>(delta_time)),
            static_cast<float>(static_cast<double>(direction.y) * static_cast<double>(delta_time)),
            static_cast<float>(static_cast<double>(direction.z) * static_cast<double>(delta_time)),
        };
        if (!isfinite(delta.x) || !isfinite(delta.y) || !isfinite(delta.z)) break;
        g_entitybase_self_move_by(entity_base, delta, nullptr);
        ++applied_steps;
    }
    if (fractional_step > 0.0f) {
        Vector3Lite delta = {
            static_cast<float>(static_cast<double>(direction.x) * static_cast<double>(delta_time) * static_cast<double>(fractional_step)),
            static_cast<float>(static_cast<double>(direction.y) * static_cast<double>(delta_time) * static_cast<double>(fractional_step)),
            static_cast<float>(static_cast<double>(direction.z) * static_cast<double>(delta_time) * static_cast<double>(fractional_step)),
        };
        if (isfinite(delta.x) && isfinite(delta.y) && isfinite(delta.z)) {
            g_entitybase_self_move_by(entity_base, delta, nullptr);
            ++applied_steps;
        }
    }

    g_last_move_progress_steps = 1 + applied_steps;
    if (applied_steps > 0) {
        bump(g_hit_move_progress_substeps, static_cast<uint64_t>(applied_steps));
    }
    return applied_steps > 0;
}

static void hk_movecontrol_update_progress(void* thiz, void* method) {
    bump(g_hit_move_progress_get);
    if (!g_orig_movecontrol_update_progress) return;

    void* entity_base = get_entity_base_from_move_control(thiz);
    bool hero = is_hero_entity_base(entity_base);
    if (hero) bump(g_hit_move_progress_hero);

    const float multiplier = clamp_move_speed(g_move_speed_multiplier);
    if (!g_enable_move_speed || !hero || multiplier == 1.0f) {
        bump(g_hit_move_progress_passthrough);
        g_orig_movecontrol_update_progress(thiz, method);
        return;
    }

    if (multiplier > 1.0f) {
        g_orig_movecontrol_update_progress(thiz, method);
        void* refreshed_entity_base = get_entity_base_from_move_control(thiz);
        if (!is_hero_entity_base(refreshed_entity_base)) return;
        if (apply_extra_self_move(thiz, refreshed_entity_base, multiplier - 1.0f)) {
            bump(g_hit_move_progress_apply);
        } else {
            bump(g_hit_move_progress_passthrough);
            g_last_move_progress_steps = 1;
        }
        return;
    }

    ObscuredVector3Snapshot snapshot{};
    Vector3Lite direction{};
    float magnitude = 0.0f;
    bool readable = read_move_progress_direction(thiz, &snapshot, &direction, &magnitude);
    if (!readable) {
        bump(g_hit_move_progress_passthrough);
        g_orig_movecontrol_update_progress(thiz, method);
        return;
    }

    g_last_move_progress_scaled = magnitude * multiplier;
    bump(g_hit_move_progress_apply);
    int32_t applied_steps = 0;
    if (write_move_progress_direction_scaled(thiz, direction, multiplier)) {
        g_orig_movecontrol_update_progress(thiz, method);
        applied_steps = 1;
    }
    g_last_move_progress_steps = applied_steps;
    if (applied_steps > 0) {
        bump(g_hit_move_progress_substeps, static_cast<uint64_t>(applied_steps));
    }
    if (applied_steps == 0) {
        g_orig_movecontrol_update_progress(thiz, method);
    }
    restore_obscured_vector3(
        reinterpret_cast<uintptr_t>(thiz) + g_off_move_control_move_direction,
        snapshot
    );
}

static float hk_time_get_scale(void* method) {
    bump(g_hit_game_speed_get);
    if (!g_enable_game_speed) {
        (void)method;
        return 1.0f;
    }
    return clamp_game_speed(g_game_speed_multiplier);
}

static void hk_time_set_scale(float value, void* method) {
    bump(g_hit_game_speed_set);
    if (!g_enable_game_speed) {
        (void)value;
        (void)method;
        return;
    }
    if (!g_orig_time_set_scale) return;
    bump(g_hit_game_speed_apply);
    g_orig_time_set_scale(clamp_game_speed(g_game_speed_multiplier), method);
}

static bool is_adcallback_control_instance(void* callback) {
    if (!callback || !g_adcallback_control_class) return false;
    return *reinterpret_cast<void**>(callback) == g_adcallback_control_class;
}

static bool is_instance_of_class(void* obj, void* klass) {
    if (!obj || !klass) return false;
    return *reinterpret_cast<void**>(obj) == klass;
}

static bool can_complete_rewarded_ad() {
    return (g_adcallback_on_reward && g_adcallback_on_close) ||
           (g_wrappeddriver_on_reward && g_wrappeddriver_on_close) ||
           (g_combineddriver_on_reward && g_combineddriver_on_close) ||
           (g_callbackrouter_on_reward && g_callbackrouter_on_close);
}

static void complete_rewarded_ad(void* ad_callback_control) {
    if (!ad_callback_control) return;
    if (!g_field_offsets_ready) return;
    if (g_completing_rewarded_ad) return;
    g_completing_rewarded_ad = true;
    uintptr_t base = reinterpret_cast<uintptr_t>(ad_callback_control);
    *reinterpret_cast<volatile uint8_t*>(base + g_off_adcallback_b_callback) = 0;
    *reinterpret_cast<volatile uint8_t*>(base + g_off_adcallback_b_opened) = 1;

    if (g_adcallback_on_reward) {
        g_adcallback_on_reward(ad_callback_control, nullptr, nullptr, nullptr);
        bump(g_hit_ad_skip_reward);
    }

    *reinterpret_cast<volatile uint8_t*>(base + g_off_adcallback_b_opened) = 1;
    if (g_adcallback_on_close) {
        g_adcallback_on_close(ad_callback_control, nullptr, nullptr, nullptr);
        bump(g_hit_ad_skip_close);
    }
    g_completing_rewarded_ad = false;
}

static bool complete_ads_callback(void* callback, void* sender) {
    if (!callback || !can_complete_rewarded_ad()) return false;
    if (is_adcallback_control_instance(callback)) {
        complete_rewarded_ad(callback);
        return true;
    }

    AdCallbackEventFn on_reward = nullptr;
    AdCallbackEventFn on_close = nullptr;
    if (is_instance_of_class(callback, g_wrappeddriver_class)) {
        on_reward = g_wrappeddriver_on_reward;
        on_close = g_wrappeddriver_on_close;
    } else if (is_instance_of_class(callback, g_combineddriver_class)) {
        on_reward = g_combineddriver_on_reward;
        on_close = g_combineddriver_on_close;
    } else if (is_instance_of_class(callback, g_callbackrouter_class)) {
        on_reward = g_callbackrouter_on_reward;
        on_close = g_callbackrouter_on_close;
    }
    if (!on_reward || !on_close) return false;

    on_reward(callback, sender, nullptr, nullptr);
    bump(g_hit_ad_skip_reward);
    on_close(callback, sender, nullptr, nullptr);
    bump(g_hit_ad_skip_close);
    return true;
}

static void* read_pointer_field(void* obj, uintptr_t offset) {
    if (!obj || !g_field_offsets_ready || offset == 0) return nullptr;
    return *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(obj) + offset);
}

static bool hk_adcallback_is_loaded(void* thiz, int32_t adapter_id, void* method) {
    if (g_skip_rewarded_ads && can_complete_rewarded_ad()) {
        (void)thiz;
        (void)adapter_id;
        (void)method;
        bump(g_hit_ad_skip_isloaded);
        return true;
    }
    return g_orig_adcallback_is_loaded ? g_orig_adcallback_is_loaded(thiz, adapter_id, method) : false;
}

static void hk_adcallback_show(void* thiz, int32_t adapter_id, void* method) {
    if (g_skip_rewarded_ads && thiz && can_complete_rewarded_ad()) {
        (void)adapter_id;
        (void)method;
        bump(g_hit_ad_skip_show);
        complete_rewarded_ad(thiz);
        return;
    }
    bump(g_hit_ad_skip_passthrough);
    if (g_orig_adcallback_show) g_orig_adcallback_show(thiz, adapter_id, method);
}

static bool hk_rewarded_high_ecpm_is_loaded(void* method) {
    if (g_skip_rewarded_ads && can_complete_rewarded_ad()) {
        (void)method;
        bump(g_hit_ad_skip_isloaded);
        return true;
    }
    return g_orig_rewarded_high_ecpm_is_loaded ? g_orig_rewarded_high_ecpm_is_loaded(method) : false;
}

static void hk_rewarded_high_ecpm_show(void* callback, int32_t source, void* method) {
    if (g_skip_rewarded_ads && complete_ads_callback(callback, nullptr)) {
        (void)source;
        (void)method;
        bump(g_hit_ad_skip_high_ecpm);
        return;
    }
    bump(g_hit_ad_skip_passthrough);
    if (g_orig_rewarded_high_ecpm_show) g_orig_rewarded_high_ecpm_show(callback, source, method);
}

static bool hk_almax_rewarded_is_loaded(void* thiz, void* method) {
    if (g_skip_rewarded_ads && can_complete_rewarded_ad()) {
        (void)thiz;
        (void)method;
        bump(g_hit_ad_skip_isloaded);
        return true;
    }
    return g_orig_almax_rewarded_is_loaded ? g_orig_almax_rewarded_is_loaded(thiz, method) : false;
}

static bool hk_almax_rewarded_show(void* thiz, void* method) {
    if (g_skip_rewarded_ads && complete_ads_callback(read_pointer_field(thiz, g_off_base_driver_callback), thiz)) {
        (void)method;
        bump(g_hit_ad_skip_driver);
        return true;
    }
    bump(g_hit_ad_skip_passthrough);
    return g_orig_almax_rewarded_show ? g_orig_almax_rewarded_show(thiz, method) : false;
}

static bool hk_wrapped_adapter_is_loaded(void* thiz, void* method) {
    if (g_skip_rewarded_ads && can_complete_rewarded_ad()) {
        (void)thiz;
        (void)method;
        bump(g_hit_ad_skip_isloaded);
        return true;
    }
    return g_orig_wrapped_adapter_is_loaded ? g_orig_wrapped_adapter_is_loaded(thiz, method) : false;
}

static bool hk_wrapped_adapter_show(void* thiz, void* method) {
    if (g_skip_rewarded_ads && complete_ads_callback(read_pointer_field(thiz, g_off_wrapped_adapter_callbacks), thiz)) {
        (void)method;
        bump(g_hit_ad_skip_adapter);
        return true;
    }
    bump(g_hit_ad_skip_passthrough);
    return g_orig_wrapped_adapter_show ? g_orig_wrapped_adapter_show(thiz, method) : false;
}

static bool hk_wrapped_adapter_show_callback(void* thiz, void* callback, void* method) {
    if (g_skip_rewarded_ads && complete_ads_callback(callback, thiz)) {
        (void)method;
        bump(g_hit_ad_skip_adapter);
        return true;
    }
    bump(g_hit_ad_skip_passthrough);
    return g_orig_wrapped_adapter_show_callback ? g_orig_wrapped_adapter_show_callback(thiz, callback, method) : false;
}

static bool hk_wrapped_adapter_show_callback_source(void* thiz, void* callback, int32_t source, void* method) {
    if (g_skip_rewarded_ads && complete_ads_callback(callback, thiz)) {
        (void)source;
        (void)method;
        bump(g_hit_ad_skip_adapter);
        return true;
    }
    bump(g_hit_ad_skip_passthrough);
    return g_orig_wrapped_adapter_show_callback_source ? g_orig_wrapped_adapter_show_callback_source(thiz, callback, source, method) : false;
}

static bool is_gold_drop_type(int32_t type) {
    return type == FoodType_Gold || type == FoodType_PureGold;
}

static bool is_supported_scalar_drop_type(int32_t type) {
    return type == FoodType_Gold || type == FoodType_PureGold || type == FoodType_Exp ||
           type == FoodType_TDStone || type == FoodType_SLGTalent ||
           type == FoodType_SLGCampTalent || type == FoodType_NewPlay126Stone;
}

static bool is_supported_material_drop_type(int32_t type) {
    return is_supported_scalar_drop_type(type) && !is_gold_drop_type(type);
}

static void scale_boxed_i32(Il2CppObjectLite* boxed, float multiplier) {
    if (!boxed) return;
    int32_t* value = reinterpret_cast<int32_t*>(
        reinterpret_cast<uintptr_t>(boxed) + sizeof(Il2CppObjectLite)
    );
    *value = scale_int32(*value, multiplier);
}

static int32_t scale_drop_list(ListBattleDropDataLite* list, bool only_gold) {
    if (!list || !list->items || list->size <= 0) return 0;
    if (list->items->max_length < static_cast<uintptr_t>(list->size)) return 0;
    if (list->size > 512) return 0;
    int32_t scaled = 0;
    for (int32_t i = 0; i < list->size; ++i) {
        BattleDropDataLite* item = list->items->vector[i];
        if (!item || !item->data) continue;
        if (only_gold) {
            if (is_gold_drop_type(item->type)) {
                scale_boxed_i32(item->data, g_gold_multiplier);
                scaled++;
            }
        } else {
            if (is_supported_material_drop_type(item->type)) {
                scale_boxed_i32(item->data, g_gold_multiplier);
                scaled++;
            }
        }
    }
    return scaled;
}

using AddGoldFloatFn = void (*)(void*, float, void*);
using AddGoldIntFn = void (*)(void*, int32_t, void*);
using InstanceFloatFn = float (*)(void*, void*);
using InstanceIntFn = int32_t (*)(void*, void*);
using InstanceBoolFn = bool (*)(void*);
using StaticFloatFn = float (*)(void*);
using StaticIntFn = int32_t (*)(void*);
using StaticBoolFn = bool (*)(void*);
using StaticIntArgFn = int32_t (*)(int32_t, void*);
using UpdateGoldFn = void (*)(void*, float, void*);
using EntityGetGoldFn = float (*)(void*, int64_t, void*);
using FloatIntArgFn = float (*)(void*, int32_t, void*);
using IntObjArgFn = int32_t (*)(void*, void*, void*);
using ListReturnFn = ListBattleDropDataLite* (*)(void*, void*);
using ListReturnLongArgFn = ListBattleDropDataLite* (*)(void*, int64_t, void*);
using StaticListIntArgFn = ListBattleDropDataLite* (*)(int32_t, void*);
using DeadGoodStartDropFn = void (*)(void*, Vector3Lite, ListBattleDropDataLite*, int32_t, void*, void*);

static AddGoldFloatFn g_orig_add_gold_float = nullptr;
static AddGoldIntFn g_orig_add_gold_int = nullptr;
static InstanceFloatFn g_orig_battle_get_gold = nullptr;
static UpdateGoldFn g_orig_battlein_update_gold = nullptr;
static InstanceFloatFn g_orig_battlein_get_gold = nullptr;
static EntityGetGoldFn g_orig_entity_get_gold = nullptr;
static StaticFloatFn g_orig_get_coin_1_wave = nullptr;
static StaticIntFn g_orig_get_box_drop_gold = nullptr;
static StaticIntArgFn g_orig_get_box_choose_gold = nullptr;
static InstanceFloatFn g_orig_drop_gold_percent = nullptr;
static IntObjArgFn g_orig_drop_model_get_drop_gold = nullptr;
static StaticBoolFn g_orig_can_save_gold_realtime = nullptr;
static DeadGoodStartDropFn g_orig_dead_good_start_drop = nullptr;
static ListReturnLongArgFn g_orig_drop_gold_hitted_list = nullptr;
static ListReturnFn g_orig_drop_gold_dead_list = nullptr;
static StaticListIntArgFn g_orig_get_pure_gold_list = nullptr;
static ListReturnFn g_orig_drop_manager_get_drop_list = nullptr;

static void hk_add_gold_float(void* thiz, float value, void* method) {
    bump(g_hit_add_gold);
    if (!g_orig_add_gold_float) return;
    if (g_gold_add_scale) value = scale_float(value, g_gold_multiplier);
    g_orig_add_gold_float(thiz, value, method);
}

static void hk_add_gold_int(void* thiz, int32_t value, void* method) {
    bump(g_hit_add_gold);
    if (!g_orig_add_gold_int) return;
    if (g_gold_add_scale) value = scale_int32(value, g_gold_multiplier);
    g_orig_add_gold_int(thiz, value, method);
}

static float adjust_gold_get_value(float value) {
    if (g_gold_get_fixed) return g_stage_gold_fixed;
    if (g_gold_get_scale) return scale_float(value, g_gold_multiplier);
    return value;
}

static float hk_battle_get_gold(void* thiz, void* method) {
    bump(g_hit_stage_balance);
    float value = g_orig_battle_get_gold ? g_orig_battle_get_gold(thiz, method) : 0.0f;
    return adjust_gold_get_value(value);
}

static void hk_battlein_update_gold(void* thiz, float gold, void* method) {
    bump(g_hit_stage_balance);
    if (!g_orig_battlein_update_gold) return;
    if (g_gold_update_fixed) gold = g_stage_gold_fixed;
    else if (g_gold_update_scale) gold = scale_float(gold, g_gold_multiplier);
    g_orig_battlein_update_gold(thiz, gold, method);
}

static float hk_battlein_get_gold(void* thiz, void* method) {
    bump(g_hit_stage_balance);
    float value = g_orig_battlein_get_gold ? g_orig_battlein_get_gold(thiz, method) : 0.0f;
    return adjust_gold_get_value(value);
}

static float hk_entity_get_gold(void* thiz, int64_t value, void* method) {
    bump(g_hit_entity_formula);
    float result = g_orig_entity_get_gold ? g_orig_entity_get_gold(thiz, value, method) : 0.0f;
    return g_gold_formula_scale ? scale_float(result, g_gold_multiplier) : result;
}

static float hk_get_coin_1_wave(void* method) {
    bump(g_hit_static_gold);
    float result = g_orig_get_coin_1_wave ? g_orig_get_coin_1_wave(method) : 0.0f;
    return g_gold_static_scale ? scale_float(result, g_gold_multiplier) : result;
}

static int32_t hk_get_box_drop_gold(void* method) {
    bump(g_hit_static_gold);
    int32_t result = g_orig_get_box_drop_gold ? g_orig_get_box_drop_gold(method) : 0;
    return g_gold_static_scale ? scale_int32(result, g_gold_multiplier) : result;
}

static int32_t hk_get_box_choose_gold(int32_t type, void* method) {
    bump(g_hit_static_gold);
    int32_t result = g_orig_get_box_choose_gold ? g_orig_get_box_choose_gold(type, method) : 0;
    return g_gold_static_scale ? scale_int32(result, g_gold_multiplier) : result;
}

static float hk_drop_gold_percent(void* thiz, void* method) {
    bump(g_hit_drop_scalar);
    float result = g_orig_drop_gold_percent ? g_orig_drop_gold_percent(thiz, method) : 0.0f;
    return g_gold_drop_scalar ? scale_float(result, g_gold_multiplier) : result;
}

static int32_t hk_drop_model_get_drop_gold(void* thiz, void* list, void* method) {
    bump(g_hit_drop_scalar);
    int32_t result = g_orig_drop_model_get_drop_gold ? g_orig_drop_model_get_drop_gold(thiz, list, method) : 0;
    return g_gold_drop_scalar ? scale_int32(result, g_gold_multiplier) : result;
}

static bool hk_can_save_gold_realtime(void* method) {
    if (g_gold_save_realtime) return true;
    return g_orig_can_save_gold_realtime ? g_orig_can_save_gold_realtime(method) : false;
}

static void hk_dead_good_start_drop(void* thiz, Vector3Lite pos, ListBattleDropDataLite* goodslist, int32_t radius, void* mapGoodsDrop, void* method) {
    bump(g_hit_deadgood_startdrop);
    if (g_gold_list_scale) bump(g_hit_list_gold_scaled, scale_drop_list(goodslist, true));
    if (g_material_drop_repeat) bump(g_hit_list_material_scaled, scale_drop_list(goodslist, false));
    if (g_orig_dead_good_start_drop) {
        g_orig_dead_good_start_drop(thiz, pos, goodslist, radius, mapGoodsDrop, method);
    }
}

static ListBattleDropDataLite* hk_drop_gold_hitted_list(void* thiz, int64_t hit, void* method) {
    ListBattleDropDataLite* result = g_orig_drop_gold_hitted_list ? g_orig_drop_gold_hitted_list(thiz, hit, method) : nullptr;
    if (g_gold_list_scale) bump(g_hit_list_gold_scaled, scale_drop_list(result, true));
    return result;
}

static ListBattleDropDataLite* hk_drop_gold_dead_list(void* thiz, void* method) {
    ListBattleDropDataLite* result = g_orig_drop_gold_dead_list ? g_orig_drop_gold_dead_list(thiz, method) : nullptr;
    if (g_gold_list_scale) bump(g_hit_list_gold_scaled, scale_drop_list(result, true));
    return result;
}

static ListBattleDropDataLite* hk_get_pure_gold_list(int32_t pureGold, void* method) {
    ListBattleDropDataLite* result = g_orig_get_pure_gold_list ? g_orig_get_pure_gold_list(pureGold, method) : nullptr;
    if (g_gold_list_scale) bump(g_hit_list_gold_scaled, scale_drop_list(result, true));
    return result;
}

static ListBattleDropDataLite* hk_drop_manager_get_drop_list(void* thiz, void* method) {
    relax_drop_manager_total_counters(thiz);
    ListBattleDropDataLite* result = g_orig_drop_manager_get_drop_list ? g_orig_drop_manager_get_drop_list(thiz, method) : nullptr;
    relax_drop_manager_total_counters(thiz);
    if (g_gold_list_scale) bump(g_hit_list_gold_scaled, scale_drop_list(result, true));
    if (g_material_drop_repeat) bump(g_hit_list_material_scaled, scale_drop_list(result, false));
    return result;
}

using GoldRatioFn = float (*)(void*, void*);
using DropDataGoldFn = int32_t (*)(void*, void*, void*);

#define DEFINE_GOLD_RATIO_HOOK(symbol) \
static GoldRatioFn g_orig_##symbol = nullptr; \
static float hk_##symbol(void* thiz, void* method) { \
    bump(g_hit_ratio); \
    float result = g_orig_##symbol ? g_orig_##symbol(thiz, method) : 0.0f; \
    return g_gold_ratio_scale ? scale_float(result, g_gold_multiplier) : result; \
}

#define DEFINE_DROP_DATA_GOLD_HOOK(symbol) \
static DropDataGoldFn g_orig_##symbol = nullptr; \
static int32_t hk_##symbol(void* thiz, void* data, void* method) { \
    bump(g_hit_ratio); \
    int32_t result = g_orig_##symbol ? g_orig_##symbol(thiz, data, method) : 0; \
    return g_gold_ratio_scale ? scale_int32(result, g_gold_multiplier) : result; \
}

DEFINE_GOLD_RATIO_HOOK(game_mode_base_gold_ratio)
DEFINE_DROP_DATA_GOLD_HOOK(game_mode_base_drop_gold)
DEFINE_GOLD_RATIO_HOOK(game_mode_coop_gold_ratio)
DEFINE_DROP_DATA_GOLD_HOOK(game_mode_coop_drop_gold)
DEFINE_GOLD_RATIO_HOOK(game_mode_coop_pvp_gold_ratio)
DEFINE_DROP_DATA_GOLD_HOOK(game_mode_coop_pvp_drop_gold)
DEFINE_GOLD_RATIO_HOOK(game_mode_daily_gold_ratio)
DEFINE_DROP_DATA_GOLD_HOOK(game_mode_daily_drop_gold)
DEFINE_GOLD_RATIO_HOOK(game_mode_gold1_gold_ratio)
DEFINE_DROP_DATA_GOLD_HOOK(game_mode_gold1_drop_gold)
DEFINE_GOLD_RATIO_HOOK(game_mode_level_gold_ratio)
DEFINE_DROP_DATA_GOLD_HOOK(game_mode_level_drop_gold)
DEFINE_GOLD_RATIO_HOOK(game_mode_main_challenge_gold_ratio)
DEFINE_DROP_DATA_GOLD_HOOK(game_mode_main_challenge_drop_gold)
DEFINE_GOLD_RATIO_HOOK(game_mode_meadow_gold_ratio)
DEFINE_DROP_DATA_GOLD_HOOK(game_mode_meadow_drop_gold)
DEFINE_GOLD_RATIO_HOOK(game_mode_tower_gold_ratio)
DEFINE_DROP_DATA_GOLD_HOOK(game_mode_tower_drop_gold)
DEFINE_GOLD_RATIO_HOOK(game_mode_tryplay_gold_ratio)
DEFINE_DROP_DATA_GOLD_HOOK(game_mode_tryplay_drop_gold)

using LayerGoldPercentFn = float (*)(void*, int32_t, void*);
using MaxDropFn = int32_t (*)(void*, void*);
using MaxDropIdFn = int32_t (*)(void*, int32_t, void*);

static LayerGoldPercentFn g_orig_stage_level_gold_percent = nullptr;
static MaxDropFn g_orig_stage_level_free_gold = nullptr;
static MaxDropFn g_orig_bag_coin_max_drop = nullptr;

static float hk_stage_level_gold_percent(void* thiz, int32_t layer, void* method) {
    bump(g_hit_ratio);
    float result = g_orig_stage_level_gold_percent ? g_orig_stage_level_gold_percent(thiz, layer, method) : 0.0f;
    return g_gold_ratio_scale ? scale_float(result, g_gold_multiplier) : result;
}

static int32_t hk_stage_level_free_gold(void* thiz, void* method) {
    bump(g_hit_ratio);
    int32_t result = g_orig_stage_level_free_gold ? g_orig_stage_level_free_gold(thiz, method) : 0;
    return g_gold_ratio_scale ? scale_int32(result, g_gold_multiplier) : result;
}

static int32_t hk_bag_coin_max_drop(void* thiz, void* method) {
    bump(g_hit_max_drop);
    int32_t result = g_orig_bag_coin_max_drop ? g_orig_bag_coin_max_drop(thiz, method) : 0;
    if (!g_max_drop_cap_patch) return result;
    int32_t cap = clamp_max_gold_cap(g_max_gold_cap_value);
    return result < cap ? cap : result;
}

static int32_t cap_max_drop_result(int32_t original_value) {
    if (!g_max_drop_cap_patch) return original_value;
    int32_t cap = clamp_max_drop_cap(g_max_drop_cap_value);
    return original_value < cap ? cap : original_value;
}

static int32_t cap_gold_drop_result(int32_t original_value) {
    if (!g_max_drop_cap_patch) return original_value;
    int32_t cap = clamp_max_gold_cap(g_max_gold_cap_value);
    return original_value < cap ? cap : original_value;
}

#define DEFINE_MAX_DROP_HOOK(symbol) \
static MaxDropFn g_orig_##symbol = nullptr; \
static int32_t hk_##symbol(void* thiz, void* method) { \
    bump(g_hit_max_drop); \
    int32_t result = g_orig_##symbol ? g_orig_##symbol(thiz, method) : 0; \
    return cap_max_drop_result(result); \
}

#define DEFINE_MAX_DROP_ID_HOOK(symbol) \
static MaxDropIdFn g_orig_##symbol = nullptr; \
static int32_t hk_##symbol(void* thiz, int32_t id, void* method) { \
    bump(g_hit_max_drop); \
    int32_t result = g_orig_##symbol ? g_orig_##symbol(thiz, id, method) : 0; \
    return cap_max_drop_result(result); \
}

#define DEFINE_GOLD_CAP_HOOK(symbol) \
static MaxDropFn g_orig_##symbol = nullptr; \
static int32_t hk_##symbol(void* thiz, void* method) { \
    bump(g_hit_max_drop); \
    int32_t result = g_orig_##symbol ? g_orig_##symbol(thiz, method) : 0; \
    return cap_gold_drop_result(result); \
}

DEFINE_GOLD_CAP_HOOK(game_mode_base_adventure_coin_max)
DEFINE_GOLD_CAP_HOOK(game_mode_base_bag_coin_max)
DEFINE_GOLD_CAP_HOOK(game_mode_coop_adventure_coin_max)
DEFINE_GOLD_CAP_HOOK(game_mode_coop_pvp_adventure_coin_max)
DEFINE_GOLD_CAP_HOOK(game_mode_daily_adventure_coin_max)
DEFINE_GOLD_CAP_HOOK(game_mode_gold1_adventure_coin_max)
DEFINE_GOLD_CAP_HOOK(game_mode_level_adventure_coin_max)
DEFINE_GOLD_CAP_HOOK(game_mode_main_challenge_adventure_coin_max)
DEFINE_GOLD_CAP_HOOK(game_mode_sailing_bag_coin_max)
DEFINE_GOLD_CAP_HOOK(game_mode_tower_adventure_coin_max)
DEFINE_GOLD_CAP_HOOK(table_daily_adventure_coin_rate_max)
DEFINE_GOLD_CAP_HOOK(table_daily_bag_coin_max)
DEFINE_GOLD_CAP_HOOK(table_pve_gold_max)
DEFINE_GOLD_CAP_HOOK(table_pve_hard_gold_max)
DEFINE_GOLD_CAP_HOOK(table_ship_bag_coin_max)
DEFINE_GOLD_CAP_HOOK(table_slg_stage_gold_max)
DEFINE_GOLD_CAP_HOOK(table_slg_base_gold_max)
DEFINE_GOLD_CAP_HOOK(table_tower_defense_gold_max)

DEFINE_MAX_DROP_HOOK(sailing_common_item_max)
DEFINE_MAX_DROP_HOOK(sailing_ship_upgrade_max)
DEFINE_MAX_DROP_HOOK(camp_equip_max)
DEFINE_MAX_DROP_HOOK(camp_stone_max)
DEFINE_MAX_DROP_HOOK(camp_skill_stone_max)
DEFINE_MAX_DROP_HOOK(daily_scroll_max)
DEFINE_GOLD_CAP_HOOK(daily_adventure_coin_max)
DEFINE_MAX_DROP_HOOK(daily_loupe_max)
DEFINE_MAX_DROP_HOOK(daily_bone_max)
DEFINE_MAX_DROP_HOOK(daily_horn_max)
DEFINE_MAX_DROP_HOOK(daily_rune_stone_max)
DEFINE_MAX_DROP_HOOK(daily_activity_prop_max)
DEFINE_MAX_DROP_HOOK(daily_stone_max)
DEFINE_MAX_DROP_HOOK(daily_cookie_max)
DEFINE_MAX_DROP_HOOK(daily_soul_stone_max)
DEFINE_MAX_DROP_HOOK(daily_honor_stone_max)
DEFINE_MAX_DROP_HOOK(daily_equip_max)
DEFINE_MAX_DROP_HOOK(daily_blood_stone_max)
DEFINE_MAX_DROP_HOOK(daily_fetter_badge_max)
DEFINE_MAX_DROP_HOOK(daily_act4_items_max)
DEFINE_MAX_DROP_HOOK(daily_act4_exchange_items_max)
DEFINE_MAX_DROP_HOOK(daily_wish_coin_max)
DEFINE_MAX_DROP_HOOK(daily_magic_stone_max)
DEFINE_MAX_DROP_HOOK(daily_dragon_coin_max)
DEFINE_MAX_DROP_HOOK(daily_modstone_max)
DEFINE_MAX_DROP_HOOK(daily_manor_mat_max)
DEFINE_MAX_DROP_HOOK(daily_fountain_use_max)
DEFINE_MAX_DROP_HOOK(daily_fountain_upgrade_max)
DEFINE_MAX_DROP_HOOK(daily_common_item_max)
DEFINE_MAX_DROP_HOOK(daily_equip_quintessence_max)
DEFINE_MAX_DROP_HOOK(daily_chinese_knot_max)
DEFINE_MAX_DROP_HOOK(daily_firecracker_max)
DEFINE_MAX_DROP_HOOK(daily_pet_level_up_items_max)
DEFINE_MAX_DROP_HOOK(daily_pet_exchange_items_max)
DEFINE_MAX_DROP_HOOK(daily_artifact_exchange_items_max)
DEFINE_MAX_DROP_HOOK(daily_imprint_level_up_items_max)
DEFINE_MAX_DROP_HOOK(daily_imprint_exchange_items_max)
DEFINE_MAX_DROP_HOOK(daily_imprint_stone_items_max)
DEFINE_MAX_DROP_HOOK(daily_wing_level_up_items_max)
DEFINE_GOLD_CAP_HOOK(daily_bag_coin_max)
DEFINE_MAX_DROP_ID_HOOK(daily_prop_max_by_id)

using DropMutatorFn = void (*)(void*, void**, void*, void*);
using DropMutatorIdFn = void (*)(void*, int32_t, void**, void*, void*);
using DropMutatorGuidFn = void (*)(void*, void**, int32_t, void*, float, void*);

static void repeat_drop_mutator(DropMutatorFn orig, void* thiz, void** list, void* data, void* method, bool gold) {
    if (!orig) return;
    int repeats = gold ? g_gold_drop_repeats : g_material_drop_repeats;
    bool enabled = gold ? g_gold_drop_repeat : g_material_drop_repeat;
    repeats = enabled ? clamp_repeat(repeats) : 1;
    bump(gold ? g_hit_drop_mutator_gold : g_hit_drop_mutator_material, static_cast<uint64_t>(repeats));
    relax_drop_manager_total_counters(thiz);
    for (int i = 0; i < repeats; ++i) {
        orig(thiz, list, data, method);
        relax_drop_manager_total_counters(thiz);
    }
}

static void repeat_drop_mutator_id(DropMutatorIdFn orig, void* thiz, int32_t id, void** list, void* data, void* method) {
    if (!orig) return;
    int repeats = g_material_drop_repeat ? clamp_repeat(g_material_drop_repeats) : 1;
    bump(g_hit_drop_mutator_material, static_cast<uint64_t>(repeats));
    relax_drop_manager_total_counters(thiz);
    for (int i = 0; i < repeats; ++i) {
        orig(thiz, id, list, data, method);
        relax_drop_manager_total_counters(thiz);
    }
}

static void repeat_drop_mutator_guid(DropMutatorGuidFn orig, void* thiz, void** list, int32_t guid, void* data, float add, void* method) {
    if (!orig) return;
    int repeats = g_material_drop_repeat ? clamp_repeat(g_material_drop_repeats) : 1;
    bump(g_hit_drop_mutator_material, static_cast<uint64_t>(repeats));
    relax_drop_manager_total_counters(thiz);
    for (int i = 0; i < repeats; ++i) {
        orig(thiz, list, guid, data, add, method);
        relax_drop_manager_total_counters(thiz);
    }
}

#define DEFINE_DROP_MUTATOR_HOOK(symbol, is_gold) \
static DropMutatorFn g_orig_##symbol = nullptr; \
static void hk_##symbol(void* thiz, void** list, void* data, void* method) { \
    repeat_drop_mutator(g_orig_##symbol, thiz, list, data, method, is_gold); \
}

DEFINE_DROP_MUTATOR_HOOK(drop_random_gold_hitted, true)
DEFINE_DROP_MUTATOR_HOOK(drop_activity_prop, false)
DEFINE_DROP_MUTATOR_HOOK(drop_stone, false)
DEFINE_DROP_MUTATOR_HOOK(drop_bloodstone, false)
DEFINE_DROP_MUTATOR_HOOK(drop_fetter_badge, false)
DEFINE_DROP_MUTATOR_HOOK(drop_skill_stone, false)
DEFINE_DROP_MUTATOR_HOOK(drop_wish_coin, false)
DEFINE_DROP_MUTATOR_HOOK(drop_modstone, false)
DEFINE_DROP_MUTATOR_HOOK(drop_common_item, false)
DEFINE_DROP_MUTATOR_HOOK(drop_rune_stone, false)
DEFINE_DROP_MUTATOR_HOOK(drop_cookie, false)
DEFINE_DROP_MUTATOR_HOOK(drop_adventure_coin, false)
DEFINE_DROP_MUTATOR_HOOK(drop_loupe, false)
DEFINE_DROP_MUTATOR_HOOK(drop_manor_mat, false)
DEFINE_DROP_MUTATOR_HOOK(drop_soul_stone, false)
DEFINE_DROP_MUTATOR_HOOK(drop_bone, false)
DEFINE_DROP_MUTATOR_HOOK(drop_horn, false)
DEFINE_DROP_MUTATOR_HOOK(drop_equip_exp, false)
DEFINE_DROP_MUTATOR_HOOK(drop_magic_stone, false)
DEFINE_DROP_MUTATOR_HOOK(drop_dragon_coin, false)
DEFINE_DROP_MUTATOR_HOOK(drop_starlight_stone, false)
DEFINE_DROP_MUTATOR_HOOK(drop_drop_ids, false)
DEFINE_DROP_MUTATOR_HOOK(drop_equip_quintessence, false)
DEFINE_DROP_MUTATOR_HOOK(drop_bag_coin, false)

static DropMutatorIdFn g_orig_drop_mat = nullptr;
static void hk_drop_mat(void* thiz, int32_t id, void** list, void* data, void* method) {
    repeat_drop_mutator_id(g_orig_drop_mat, thiz, id, list, data, method);
}

static DropMutatorGuidFn g_orig_random_level = nullptr;
static DropMutatorGuidFn g_orig_random_equip = nullptr;
static void hk_random_level(void* thiz, void** list, int32_t guid, void* data, float add, void* method) {
    repeat_drop_mutator_guid(g_orig_random_level, thiz, list, guid, data, add, method);
}

static void hk_random_equip(void* thiz, void** list, int32_t guid, void* data, float add, void* method) {
    repeat_drop_mutator_guid(g_orig_random_equip, thiz, list, guid, data, add, method);
}

static bool install_hook(uintptr_t base, uintptr_t rva_value, void* replacement, void** original, const char* name) {
    if (!base || !rva_value || !replacement || !original) return false;

    switch (rva_value) {
        case rva::DeadGoodMgr_GetGoldNum:
        case rva::IStageLayerManager_GetAdventureCoinsMaxDrop:
        case rva::GameModeMeadowBattle_GetGoldRatio:
        case rva::GameModeMeadowBattle_GetDropDataGold:
        case rva::GameModeTryPlay_GetGoldRatio:
        case rva::GameModeTryPlay_GetDropDataGold:
            // ASSUMPTION: And64InlineHook needs at least a normal branch-sized
            // prologue window. These v7.9.1 methods are 8-byte bodies followed
            // immediately by the next method, so inline hooking them corrupts
            // adjacent IL2CPP code.
            bump(g_hook_skipped_tiny_count);
            LOGD("Skipped tiny IL2CPP method %s at RVA 0x%lx", name, static_cast<unsigned long>(rva_value));
            return false;
    }

    const char* strategy = "unknown";
    uintptr_t resolved = resolve_hook_target(base, rva_value, name, &strategy);
    if (!resolved || !address_in_libil2cpp_exec(resolved)) {
        bump(g_resolve_fail_count);
        set_last_resolve_error(name, "not_executable");
        LOGD("Unable to resolve %s by metadata", name);
        return false;
    }

    void* target = reinterpret_cast<void*>(resolved);
    A64HookFunction(target, replacement, original);
    bump(g_hook_installed_count);
    LOGD("Hooked %s at %p via %s", name, target, strategy);
    return true;
}

#define HOOK_FN(base, rva_value, hook, orig) \
    install_hook((base), (rva_value), reinterpret_cast<void*>(hook), reinterpret_cast<void**>(&(orig)), #hook)

static void resolve_traversal_helpers(uintptr_t base) {
    const char* strategy = "unknown";
    uintptr_t add_skill = resolve_hook_target(base, rva::EntityBase_AddSkill,
                                              "EntityBase_AddSkill", &strategy);
    if (add_skill && address_in_libil2cpp_exec(add_skill)) {
        g_entitybase_add_skill = reinterpret_cast<EntityAddSkillFn>(add_skill);
        LOGD("Resolved EntityBase_AddSkill helper at %p via %s",
             reinterpret_cast<void*>(add_skill), strategy);
    } else {
        g_entitybase_add_skill = nullptr;
        LOGD("Unable to resolve EntityBase_AddSkill helper");
    }

    strategy = "unknown";
    uintptr_t contains_skill = resolve_hook_target(base, rva::EntityBase_ContainsSkill,
                                                   "EntityBase_ContainsSkill", &strategy);
    if (contains_skill && address_in_libil2cpp_exec(contains_skill)) {
        g_entitybase_contains_skill = reinterpret_cast<EntityContainsSkillFn>(contains_skill);
        LOGD("Resolved EntityBase_ContainsSkill helper at %p via %s",
             reinterpret_cast<void*>(contains_skill), strategy);
    } else {
        g_entitybase_contains_skill = nullptr;
        LOGD("Unable to resolve EntityBase_ContainsSkill helper");
    }
}

static void resolve_movement_helpers(uintptr_t base) {
    const char* strategy = "unknown";
    uintptr_t self_move_by = resolve_hook_target(base, rva::EntityBase_SelfMoveBy,
                                                 "EntityBase_SelfMoveBy", &strategy);
    if (self_move_by && address_in_libil2cpp_exec(self_move_by)) {
        g_entitybase_self_move_by = reinterpret_cast<EntitySelfMoveByFn>(self_move_by);
        LOGD("Resolved EntityBase_SelfMoveBy helper at %p via %s",
             reinterpret_cast<void*>(self_move_by), strategy);
    } else {
        g_entitybase_self_move_by = nullptr;
        LOGD("Unable to resolve EntityBase_SelfMoveBy helper");
    }

    strategy = "unknown";
    uintptr_t delta_time = resolve_hook_target(base, rva::UnityEngine_Time_get_deltaTime,
                                               "UnityEngine_Time_get_deltaTime", &strategy);
    if (delta_time && address_in_libil2cpp_exec(delta_time)) {
        g_time_get_delta_time = reinterpret_cast<TimeDeltaGetterFn>(delta_time);
        LOGD("Resolved UnityEngine.Time.get_deltaTime helper at %p via %s",
             reinterpret_cast<void*>(delta_time), strategy);
    } else {
        g_time_get_delta_time = nullptr;
        LOGD("Unable to resolve UnityEngine.Time.get_deltaTime helper");
    }
}

static void resolve_ad_helpers(uintptr_t base) {
    g_adcallback_control_class = resolve_class_by_metadata_name("", "AdCallbackControl");
    g_wrappeddriver_class = resolve_class_by_metadata_name("", "AdsRequestHelper.WrappedDriver");
    g_combineddriver_class = resolve_class_by_metadata_name("", "AdsRequestHelper.CombinedDriver");
    g_callbackrouter_class = resolve_class_by_metadata_name("", "AdsRequestHelper.CallbackRouter");

    const char* strategy = "unknown";
    uintptr_t on_reward = resolve_hook_target(base, rva::AdCallbackControl_onReward,
                                              "AdCallbackControl_onReward", &strategy);
    if (on_reward && address_in_libil2cpp_exec(on_reward)) {
        g_adcallback_on_reward = reinterpret_cast<AdCallbackEventFn>(on_reward);
        LOGD("Resolved AdCallbackControl.onReward helper at %p via %s",
             reinterpret_cast<void*>(on_reward), strategy);
    } else {
        g_adcallback_on_reward = nullptr;
        LOGD("Unable to resolve AdCallbackControl.onReward helper");
    }

    strategy = "unknown";
    uintptr_t on_close = resolve_hook_target(base, rva::AdCallbackControl_onClose,
                                             "AdCallbackControl_onClose", &strategy);
    if (on_close && address_in_libil2cpp_exec(on_close)) {
        g_adcallback_on_close = reinterpret_cast<AdCallbackEventFn>(on_close);
        LOGD("Resolved AdCallbackControl.onClose helper at %p via %s",
             reinterpret_cast<void*>(on_close), strategy);
    } else {
        g_adcallback_on_close = nullptr;
        LOGD("Unable to resolve AdCallbackControl.onClose helper");
    }

#define RESOLVE_AD_EVENT(storage, rva_value, name_text) \
    do { \
        strategy = "unknown"; \
        uintptr_t event_target = resolve_hook_target(base, (rva_value), (name_text), &strategy); \
        if (event_target && address_in_libil2cpp_exec(event_target)) { \
            (storage) = reinterpret_cast<AdCallbackEventFn>(event_target); \
            LOGD("Resolved %s helper at %p via %s", (name_text), reinterpret_cast<void*>(event_target), strategy); \
        } else { \
            (storage) = nullptr; \
            LOGD("Unable to resolve %s helper", (name_text)); \
        } \
    } while (0)

    RESOLVE_AD_EVENT(g_wrappeddriver_on_reward, rva::AdsRequestHelper_WrappedDriver_onReward, "WrappedDriver_onReward");
    RESOLVE_AD_EVENT(g_wrappeddriver_on_close, rva::AdsRequestHelper_WrappedDriver_onClose, "WrappedDriver_onClose");
    RESOLVE_AD_EVENT(g_combineddriver_on_reward, rva::AdsRequestHelper_CombinedDriver_onReward, "CombinedDriver_onReward");
    RESOLVE_AD_EVENT(g_combineddriver_on_close, rva::AdsRequestHelper_CombinedDriver_onClose, "CombinedDriver_onClose");
    RESOLVE_AD_EVENT(g_callbackrouter_on_reward, rva::AdsRequestHelper_CallbackRouter_onReward, "CallbackRouter_onReward");
    RESOLVE_AD_EVENT(g_callbackrouter_on_close, rva::AdsRequestHelper_CallbackRouter_onClose, "CallbackRouter_onClose");

#undef RESOLVE_AD_EVENT
}

static void install_gold_ratio_hooks(uintptr_t base) {
    HOOK_FN(base, rva::GameModeBase_GetGoldRatio, hk_game_mode_base_gold_ratio, g_orig_game_mode_base_gold_ratio);
    HOOK_FN(base, rva::GameModeBase_GetDropDataGold, hk_game_mode_base_drop_gold, g_orig_game_mode_base_drop_gold);
    HOOK_FN(base, rva::GameModeCooperation_GetGoldRatio, hk_game_mode_coop_gold_ratio, g_orig_game_mode_coop_gold_ratio);
    HOOK_FN(base, rva::GameModeCooperation_GetDropDataGold, hk_game_mode_coop_drop_gold, g_orig_game_mode_coop_drop_gold);
    HOOK_FN(base, rva::GameModeCooperationPVP_GetGoldRatio, hk_game_mode_coop_pvp_gold_ratio, g_orig_game_mode_coop_pvp_gold_ratio);
    HOOK_FN(base, rva::GameModeCooperationPVP_GetDropDataGold, hk_game_mode_coop_pvp_drop_gold, g_orig_game_mode_coop_pvp_drop_gold);
    HOOK_FN(base, rva::GameModeDaily_GetGoldRatio, hk_game_mode_daily_gold_ratio, g_orig_game_mode_daily_gold_ratio);
    HOOK_FN(base, rva::GameModeDaily_GetDropDataGold, hk_game_mode_daily_drop_gold, g_orig_game_mode_daily_drop_gold);
    HOOK_FN(base, rva::GameModeGold1_GetGoldRatio, hk_game_mode_gold1_gold_ratio, g_orig_game_mode_gold1_gold_ratio);
    HOOK_FN(base, rva::GameModeGold1_GetDropDataGold, hk_game_mode_gold1_drop_gold, g_orig_game_mode_gold1_drop_gold);
    HOOK_FN(base, rva::GameModeLevel_GetGoldRatio, hk_game_mode_level_gold_ratio, g_orig_game_mode_level_gold_ratio);
    HOOK_FN(base, rva::GameModeLevel_GetDropDataGold, hk_game_mode_level_drop_gold, g_orig_game_mode_level_drop_gold);
    HOOK_FN(base, rva::GameModeMainChallenge_GetGoldRatio, hk_game_mode_main_challenge_gold_ratio, g_orig_game_mode_main_challenge_gold_ratio);
    HOOK_FN(base, rva::GameModeMainChallenge_GetDropDataGold, hk_game_mode_main_challenge_drop_gold, g_orig_game_mode_main_challenge_drop_gold);
    HOOK_FN(base, rva::GameModeMeadowBattle_GetGoldRatio, hk_game_mode_meadow_gold_ratio, g_orig_game_mode_meadow_gold_ratio);
    HOOK_FN(base, rva::GameModeMeadowBattle_GetDropDataGold, hk_game_mode_meadow_drop_gold, g_orig_game_mode_meadow_drop_gold);
    HOOK_FN(base, rva::GameModeTower_GetGoldRatio, hk_game_mode_tower_gold_ratio, g_orig_game_mode_tower_gold_ratio);
    HOOK_FN(base, rva::GameModeTower_GetDropDataGold, hk_game_mode_tower_drop_gold, g_orig_game_mode_tower_drop_gold);
    HOOK_FN(base, rva::GameModeTryPlay_GetGoldRatio, hk_game_mode_tryplay_gold_ratio, g_orig_game_mode_tryplay_gold_ratio);
    HOOK_FN(base, rva::GameModeTryPlay_GetDropDataGold, hk_game_mode_tryplay_drop_gold, g_orig_game_mode_tryplay_drop_gold);
}

static void install_drop_manager_hooks(uintptr_t base) {
    HOOK_FN(base, rva::DropManager_GetRandomLevel, hk_random_level, g_orig_random_level);
    HOOK_FN(base, rva::DropManager_GetRandomGoldHitted, hk_drop_random_gold_hitted, g_orig_drop_random_gold_hitted);
    HOOK_FN(base, rva::DropManager_GetActivityProp, hk_drop_activity_prop, g_orig_drop_activity_prop);
    HOOK_FN(base, rva::DropManager_GetStone, hk_drop_stone, g_orig_drop_stone);
    HOOK_FN(base, rva::DropManager_GetBloodStone, hk_drop_bloodstone, g_orig_drop_bloodstone);
    HOOK_FN(base, rva::DropManager_GetRandomFetterBadge, hk_drop_fetter_badge, g_orig_drop_fetter_badge);
    HOOK_FN(base, rva::DropManager_GetRandomSkillStone, hk_drop_skill_stone, g_orig_drop_skill_stone);
    HOOK_FN(base, rva::DropManager_GetWishCoin, hk_drop_wish_coin, g_orig_drop_wish_coin);
    HOOK_FN(base, rva::DropManager_GetModstone, hk_drop_modstone, g_orig_drop_modstone);
    HOOK_FN(base, rva::DropManager_GetCommonItem, hk_drop_common_item, g_orig_drop_common_item);
    HOOK_FN(base, rva::DropManager_GetDropMat, hk_drop_mat, g_orig_drop_mat);
    HOOK_FN(base, rva::DropManager_GetRuneStone, hk_drop_rune_stone, g_orig_drop_rune_stone);
    HOOK_FN(base, rva::DropManager_GetCookie, hk_drop_cookie, g_orig_drop_cookie);
    HOOK_FN(base, rva::DropManager_GetAdventureCoin, hk_drop_adventure_coin, g_orig_drop_adventure_coin);
    HOOK_FN(base, rva::DropManager_GetLoupeDrops, hk_drop_loupe, g_orig_drop_loupe);
    HOOK_FN(base, rva::DropManager_GetManorMat, hk_drop_manor_mat, g_orig_drop_manor_mat);
    HOOK_FN(base, rva::DropManager_GetSoulStone, hk_drop_soul_stone, g_orig_drop_soul_stone);
    HOOK_FN(base, rva::DropManager_GetBone, hk_drop_bone, g_orig_drop_bone);
    HOOK_FN(base, rva::DropManager_GetHorn, hk_drop_horn, g_orig_drop_horn);
    HOOK_FN(base, rva::DropManager_GetRandomEquipExp, hk_drop_equip_exp, g_orig_drop_equip_exp);
    HOOK_FN(base, rva::DropManager_GetRandomMagicStone, hk_drop_magic_stone, g_orig_drop_magic_stone);
    HOOK_FN(base, rva::DropManager_GetRandomDragonCoin, hk_drop_dragon_coin, g_orig_drop_dragon_coin);
    HOOK_FN(base, rva::DropManager_GetRandomStarLightStone, hk_drop_starlight_stone, g_orig_drop_starlight_stone);
    HOOK_FN(base, rva::DropManager_GetRandomEquip, hk_random_equip, g_orig_random_equip);
    HOOK_FN(base, rva::DropManager_GetDropIds, hk_drop_drop_ids, g_orig_drop_drop_ids);
    HOOK_FN(base, rva::DropManager_GetEquipQuintessence, hk_drop_equip_quintessence, g_orig_drop_equip_quintessence);
    HOOK_FN(base, rva::DropManager_GetNewPlay125BagCoin, hk_drop_bag_coin, g_orig_drop_bag_coin);
}

static void install_max_cap_hooks(uintptr_t base) {
    HOOK_FN(base, rva::SailingBagBattleStageLayerManager_GetNewPlay125BagCoinMaxDrop, hk_bag_coin_max_drop, g_orig_bag_coin_max_drop);
    HOOK_FN(base, rva::GameModeBase_GetAdventureCoinMaxDrop, hk_game_mode_base_adventure_coin_max, g_orig_game_mode_base_adventure_coin_max);
    HOOK_FN(base, rva::GameModeBase_GetNewPlay125BagCoinMaxDrop, hk_game_mode_base_bag_coin_max, g_orig_game_mode_base_bag_coin_max);
    HOOK_FN(base, rva::GameModeCooperation_GetAdventureCoinMaxDrop, hk_game_mode_coop_adventure_coin_max, g_orig_game_mode_coop_adventure_coin_max);
    HOOK_FN(base, rva::GameModeCooperationPVP_GetAdventureCoinMaxDrop, hk_game_mode_coop_pvp_adventure_coin_max, g_orig_game_mode_coop_pvp_adventure_coin_max);
    HOOK_FN(base, rva::GameModeDaily_GetAdventureCoinMaxDrop, hk_game_mode_daily_adventure_coin_max, g_orig_game_mode_daily_adventure_coin_max);
    HOOK_FN(base, rva::GameModeGold1_GetAdventureCoinMaxDrop, hk_game_mode_gold1_adventure_coin_max, g_orig_game_mode_gold1_adventure_coin_max);
    HOOK_FN(base, rva::GameModeLevel_GetAdventureCoinMaxDrop, hk_game_mode_level_adventure_coin_max, g_orig_game_mode_level_adventure_coin_max);
    HOOK_FN(base, rva::GameModeMainChallenge_GetAdventureCoinMaxDrop, hk_game_mode_main_challenge_adventure_coin_max, g_orig_game_mode_main_challenge_adventure_coin_max);
    HOOK_FN(base, rva::GameModeSailingBagBattle_GetNewPlay125BagCoinMaxDrop, hk_game_mode_sailing_bag_coin_max, g_orig_game_mode_sailing_bag_coin_max);
    HOOK_FN(base, rva::GameModeTower_GetAdventureCoinMaxDrop, hk_game_mode_tower_adventure_coin_max, g_orig_game_mode_tower_adventure_coin_max);
    HOOK_FN(base, rva::TableTool_DailyChapter_get_AdventureCoinRateMax, hk_table_daily_adventure_coin_rate_max, g_orig_table_daily_adventure_coin_rate_max);
    HOOK_FN(base, rva::TableTool_DailyChapter_get_BagCoinMax, hk_table_daily_bag_coin_max, g_orig_table_daily_bag_coin_max);
    HOOK_FN(base, rva::TableTool_PVEStage_get_GoldMax, hk_table_pve_gold_max, g_orig_table_pve_gold_max);
    HOOK_FN(base, rva::TableTool_PVEStage_get_HardGoldMax, hk_table_pve_hard_gold_max, g_orig_table_pve_hard_gold_max);
    HOOK_FN(base, rva::TableTool_ShipStage_get_BagCoinMax, hk_table_ship_bag_coin_max, g_orig_table_ship_bag_coin_max);
    HOOK_FN(base, rva::TableTool_SLGStage_get_GoldMax, hk_table_slg_stage_gold_max, g_orig_table_slg_stage_gold_max);
    HOOK_FN(base, rva::TableTool_SLGBaseLevel_get_GoldMax, hk_table_slg_base_gold_max, g_orig_table_slg_base_gold_max);
    HOOK_FN(base, rva::TableTool_TowerDefense_get_GoldMAX, hk_table_tower_defense_gold_max, g_orig_table_tower_defense_gold_max);
    HOOK_FN(base, rva::SailingBagBattleStageLayerManager_GetCommonItemMaxDrop, hk_sailing_common_item_max, g_orig_sailing_common_item_max);
    HOOK_FN(base, rva::SailingBagBattleStageLayerManager_GetShipUpgradeMaxDrop, hk_sailing_ship_upgrade_max, g_orig_sailing_ship_upgrade_max);
    HOOK_FN(base, rva::CampBattleStageLayerManager_GetEquipMaxDrop, hk_camp_equip_max, g_orig_camp_equip_max);
    HOOK_FN(base, rva::CampBattleStageLayerManager_GetStoneMaxDrop, hk_camp_stone_max, g_orig_camp_stone_max);
    HOOK_FN(base, rva::CampBattleStageLayerManager_GetSkillStoneMaxDrop, hk_camp_skill_stone_max, g_orig_camp_skill_stone_max);
    HOOK_FN(base, rva::DailyStageChapter_GetScrollMaxDrop, hk_daily_scroll_max, g_orig_daily_scroll_max);
    HOOK_FN(base, rva::DailyStageChapter_GetAdventureCoinsMaxDrop, hk_daily_adventure_coin_max, g_orig_daily_adventure_coin_max);
    HOOK_FN(base, rva::DailyStageChapter_GetLoupeMaxDrop, hk_daily_loupe_max, g_orig_daily_loupe_max);
    HOOK_FN(base, rva::DailyStageChapter_GetBoneMaxDrop, hk_daily_bone_max, g_orig_daily_bone_max);
    HOOK_FN(base, rva::DailyStageChapter_GetHornMaxDrop, hk_daily_horn_max, g_orig_daily_horn_max);
    HOOK_FN(base, rva::DailyStageChapter_GetRuneStoneMaxDrop, hk_daily_rune_stone_max, g_orig_daily_rune_stone_max);
    HOOK_FN(base, rva::DailyStageChapter_GetActivityPropMaxDrop, hk_daily_activity_prop_max, g_orig_daily_activity_prop_max);
    HOOK_FN(base, rva::DailyStageChapter_GetStoneMaxDrop, hk_daily_stone_max, g_orig_daily_stone_max);
    HOOK_FN(base, rva::DailyStageChapter_GetCookieMaxDrop, hk_daily_cookie_max, g_orig_daily_cookie_max);
    HOOK_FN(base, rva::DailyStageChapter_GetSoulStoneMaxDrop, hk_daily_soul_stone_max, g_orig_daily_soul_stone_max);
    HOOK_FN(base, rva::DailyStageChapter_GetHonorStoneMaxDrop, hk_daily_honor_stone_max, g_orig_daily_honor_stone_max);
    HOOK_FN(base, rva::DailyStageChapter_GetEquipMaxDrop, hk_daily_equip_max, g_orig_daily_equip_max);
    HOOK_FN(base, rva::DailyStageChapter_GetBloodStoneMaxDrop, hk_daily_blood_stone_max, g_orig_daily_blood_stone_max);
    HOOK_FN(base, rva::DailyStageChapter_GetFetterBadgeMaxDrop, hk_daily_fetter_badge_max, g_orig_daily_fetter_badge_max);
    HOOK_FN(base, rva::DailyStageChapter_GetAct4thItemsMaxDrop, hk_daily_act4_items_max, g_orig_daily_act4_items_max);
    HOOK_FN(base, rva::DailyStageChapter_GetAct4thExchangeItemsMaxDrop, hk_daily_act4_exchange_items_max, g_orig_daily_act4_exchange_items_max);
    HOOK_FN(base, rva::DailyStageChapter_GetWishCoinMaxDrop, hk_daily_wish_coin_max, g_orig_daily_wish_coin_max);
    HOOK_FN(base, rva::DailyStageChapter_GetMagicStoneMaxDrop, hk_daily_magic_stone_max, g_orig_daily_magic_stone_max);
    HOOK_FN(base, rva::DailyStageChapter_GetDragonCoinMaxDrop, hk_daily_dragon_coin_max, g_orig_daily_dragon_coin_max);
    HOOK_FN(base, rva::DailyStageChapter_GetModstoneMaxDrop, hk_daily_modstone_max, g_orig_daily_modstone_max);
    HOOK_FN(base, rva::DailyStageChapter_GetManorMatMaxDrop, hk_daily_manor_mat_max, g_orig_daily_manor_mat_max);
    HOOK_FN(base, rva::DailyStageChapter_GetFountainUseMaxDrop, hk_daily_fountain_use_max, g_orig_daily_fountain_use_max);
    HOOK_FN(base, rva::DailyStageChapter_GetFountainUpgradeMaxDrop, hk_daily_fountain_upgrade_max, g_orig_daily_fountain_upgrade_max);
    HOOK_FN(base, rva::DailyStageChapter_GetCommonItemMaxDrop, hk_daily_common_item_max, g_orig_daily_common_item_max);
    HOOK_FN(base, rva::DailyStageChapter_GetEquipQuintessenceMaxDrop, hk_daily_equip_quintessence_max, g_orig_daily_equip_quintessence_max);
    HOOK_FN(base, rva::DailyStageChapter_GetChineseKnotMaxDrop, hk_daily_chinese_knot_max, g_orig_daily_chinese_knot_max);
    HOOK_FN(base, rva::DailyStageChapter_GetFirecrackerMaxDrop, hk_daily_firecracker_max, g_orig_daily_firecracker_max);
    HOOK_FN(base, rva::DailyStageChapter_GetPetLevelUpItemsMaxDrop, hk_daily_pet_level_up_items_max, g_orig_daily_pet_level_up_items_max);
    HOOK_FN(base, rva::DailyStageChapter_GetPetExchangeItemsMaxDrop, hk_daily_pet_exchange_items_max, g_orig_daily_pet_exchange_items_max);
    HOOK_FN(base, rva::DailyStageChapter_GetArtifactExchangeItemsMaxDrop, hk_daily_artifact_exchange_items_max, g_orig_daily_artifact_exchange_items_max);
    HOOK_FN(base, rva::DailyStageChapter_GetImprintLevelUpItemsMaxDrop, hk_daily_imprint_level_up_items_max, g_orig_daily_imprint_level_up_items_max);
    HOOK_FN(base, rva::DailyStageChapter_GetImprintExchangeItemsMaxDrop, hk_daily_imprint_exchange_items_max, g_orig_daily_imprint_exchange_items_max);
    HOOK_FN(base, rva::DailyStageChapter_GetImprintStoneItemsMaxDrop, hk_daily_imprint_stone_items_max, g_orig_daily_imprint_stone_items_max);
    HOOK_FN(base, rva::DailyStageChapter_GetWingLevelUpItemsMaxDrop, hk_daily_wing_level_up_items_max, g_orig_daily_wing_level_up_items_max);
    HOOK_FN(base, rva::DailyStageChapter_GetNewPlay125BagCoinMaxDrop, hk_daily_bag_coin_max, g_orig_daily_bag_coin_max);
    HOOK_FN(base, rva::DailyStageChapter_GetPropMaxDropById, hk_daily_prop_max_by_id, g_orig_daily_prop_max_by_id);
}

static void install_gold_hooks_once(uintptr_t base) {
    if (!base || g_gold_hooks_installed) return;
    g_gold_hooks_installed = true;

    if (g_gold_add_scale) {
        HOOK_FN(base, rva::BattleModuleData_AddGold_Float, hk_add_gold_float, g_orig_add_gold_float);
        HOOK_FN(base, rva::BattleModuleData_AddGold_Int, hk_add_gold_int, g_orig_add_gold_int);
    }
    if (g_gold_get_fixed || g_gold_get_scale) {
        HOOK_FN(base, rva::BattleModuleData_GetGold, hk_battle_get_gold, g_orig_battle_get_gold);
        HOOK_FN(base, rva::LocalSave_BattleIn_GetGold, hk_battlein_get_gold, g_orig_battlein_get_gold);
    }
    if (g_gold_update_fixed || g_gold_update_scale) {
        HOOK_FN(base, rva::LocalSave_BattleIn_UpdateGold, hk_battlein_update_gold, g_orig_battlein_update_gold);
    }
    if (g_gold_formula_scale) {
        HOOK_FN(base, rva::EntityData_getGold, hk_entity_get_gold, g_orig_entity_get_gold);
    }
    if (g_gold_static_scale) {
        HOOK_FN(base, rva::GameConfig_GetCoin1Wave, hk_get_coin_1_wave, g_orig_get_coin_1_wave);
        HOOK_FN(base, rva::GameConfig_GetBoxDropGold, hk_get_box_drop_gold, g_orig_get_box_drop_gold);
        HOOK_FN(base, rva::GameConfig_GetBoxChooseGold, hk_get_box_choose_gold, g_orig_get_box_choose_gold);
    }
    if (g_gold_drop_scalar) {
        HOOK_FN(base, rva::Drop_DropModel_GetGoldDropPercent, hk_drop_gold_percent, g_orig_drop_gold_percent);
        HOOK_FN(base, rva::Drop_DropModel_GetDropGold, hk_drop_model_get_drop_gold, g_orig_drop_model_get_drop_gold);
    }
    if (g_gold_list_scale || g_material_drop_repeat) {
        HOOK_FN(base, rva::DeadGoodMgr_StartDrop, hk_dead_good_start_drop, g_orig_dead_good_start_drop);
        HOOK_FN(base, rva::DropManager_GetDropList, hk_drop_manager_get_drop_list, g_orig_drop_manager_get_drop_list);
    }
    if (g_gold_list_scale) {
        HOOK_FN(base, rva::DropGold_OnGetHittedList, hk_drop_gold_hitted_list, g_orig_drop_gold_hitted_list);
        HOOK_FN(base, rva::DropGold_OnGetDropDead, hk_drop_gold_dead_list, g_orig_drop_gold_dead_list);
        HOOK_FN(base, rva::GameLogic_GetPureGoldList, hk_get_pure_gold_list, g_orig_get_pure_gold_list);
    }
    if (g_gold_save_realtime) {
        HOOK_FN(base, rva::GameLogic_CanSaveGoldInRealTime, hk_can_save_gold_realtime, g_orig_can_save_gold_realtime);
    }
    if (g_gold_ratio_scale) {
        HOOK_FN(base, rva::StageLevelManager_GetGoldDropPercent, hk_stage_level_gold_percent, g_orig_stage_level_gold_percent);
        HOOK_FN(base, rva::StageLevelManager_GetFreeGold, hk_stage_level_free_gold, g_orig_stage_level_free_gold);
        install_gold_ratio_hooks(base);
    }
    if (g_max_drop_cap_patch) {
        install_max_cap_hooks(base);
    }
    if (g_gold_drop_repeat || g_material_drop_repeat) {
        install_drop_manager_hooks(base);
    }
    LOGD("Optional gold/material hooks installed");
}

static void* hack_thread(void*) {
    uintptr_t il2cpp_base = 0;
    while ((il2cpp_base = get_base_address("libil2cpp.so")) == 0) {
        sleep(1);
    }
    g_il2cpp_base = il2cpp_base;

    load_config_file_once();
    g_il2cpp_metadata_ready = wait_for_il2cpp_metadata_ready();
    LOGD("libil2cpp.so found at %p metadata_ready=%d wait_ms=%d",
         reinterpret_cast<void*>(il2cpp_base),
         g_il2cpp_metadata_ready ? 1 : 0,
         g_il2cpp_metadata_wait_ms);

    resolve_runtime_field_offsets();
    resolve_traversal_helpers(il2cpp_base);
    resolve_movement_helpers(il2cpp_base);
    resolve_ad_helpers(il2cpp_base);
    HOOK_FN(il2cpp_base, rva::EntityData_GetHeadShot, hk_get_headshot, g_orig_get_headshot);
    HOOK_FN(il2cpp_base, rva::EntityData_GetMiss, hk_get_miss, g_orig_get_miss);
    HOOK_FN(il2cpp_base, rva::TableTool_PlayerCharacter_UpgradeModel_GetATKBase, hk_get_atk_base, g_orig_get_atk_base);
    HOOK_FN(il2cpp_base, rva::TableTool_PlayerCharacter_UpgradeModel_GetHPMaxBase, hk_get_hp_base, g_orig_get_hp_base);
    HOOK_FN(il2cpp_base, rva::TableTool_Weapon_weapon_get_Speed, hk_weapon_get_speed, g_orig_weapon_get_speed);
    HOOK_FN(il2cpp_base, rva::TableTool_Weapon_weapon_get_AttackSpeed, hk_weapon_get_attack_speed, g_orig_weapon_get_attack_speed);
    HOOK_FN(il2cpp_base, rva::TableTool_Weapon_weapon_get_bThroughWall, hk_weapon_get_through_wall, g_orig_weapon_get_through_wall);
    HOOK_FN(il2cpp_base, rva::TableTool_Weapon_weapon_get_bThroughInsideWall, hk_weapon_get_through_inside_wall, g_orig_weapon_get_through_inside_wall);
    HOOK_FN(il2cpp_base, rva::BulletTransmit_Init_Simple, hk_bullet_transmit_init_simple, g_orig_bullet_transmit_init_simple);
    HOOK_FN(il2cpp_base, rva::BulletTransmit_Init_Full, hk_bullet_transmit_init_full, g_orig_bullet_transmit_init_full);
    HOOK_FN(il2cpp_base, rva::BulletTransmit_get_ThroughWall, hk_bullet_transmit_get_through_wall, g_orig_bullet_transmit_get_through_wall);
    HOOK_FN(il2cpp_base, rva::BulletBase_HitWall, hk_bulletbase_hit_wall, g_orig_bulletbase_hit_wall);
    HOOK_FN(il2cpp_base, rva::BulletBase_TriggerEnter1_HitWallInternal, hk_bulletbase_hitwall_internal, g_orig_bulletbase_hitwall_internal);
    HOOK_FN(il2cpp_base, rva::EntityBase_SetFlyWater, hk_entitybase_set_fly_water, g_orig_entitybase_set_fly_water);
    HOOK_FN(il2cpp_base, rva::EntityBase_GetFlyWater, hk_entitybase_get_fly_water, g_orig_entitybase_get_fly_water);
    HOOK_FN(il2cpp_base, rva::EntityBase_SetFlyStone, hk_entitybase_set_fly_stone, g_orig_entitybase_set_fly_stone);
    HOOK_FN(il2cpp_base, rva::EntityBase_GetOnCalCanMove, hk_entitybase_get_on_cal_can_move, g_orig_entitybase_get_on_cal_can_move);
    HOOK_FN(il2cpp_base, rva::EntityBase_SetCollider, hk_entitybase_set_collider, g_orig_entitybase_set_collider);
    HOOK_FN(il2cpp_base, rva::EntityBase_SetFlyAll, hk_entitybase_set_fly_all, g_orig_entitybase_set_fly_all);
    HOOK_FN(il2cpp_base, rva::EntityBase_CheckPos, hk_entitybase_check_pos, g_orig_entitybase_check_pos);
    HOOK_FN(il2cpp_base, rva::EntityBase_AddInitSkills, hk_entitybase_add_init_skills, g_orig_entitybase_add_init_skills);
    HOOK_FN(il2cpp_base, rva::EntityHitCtrl_SetFlyOne, hk_entityhitctrl_set_fly_one, g_orig_entityhitctrl_set_fly_one);
    HOOK_FN(il2cpp_base, rva::MoveControl_UpdateProgress, hk_movecontrol_update_progress, g_orig_movecontrol_update_progress);
    HOOK_FN(il2cpp_base, rva::AdCallbackControl_IsLoaded, hk_adcallback_is_loaded, g_orig_adcallback_is_loaded);
    HOOK_FN(il2cpp_base, rva::AdCallbackControl_Show, hk_adcallback_show, g_orig_adcallback_show);
    HOOK_FN(il2cpp_base, rva::AdsRequestHelper_ALMaxRewardedDriver_isLoaded, hk_almax_rewarded_is_loaded, g_orig_almax_rewarded_is_loaded);
    HOOK_FN(il2cpp_base, rva::AdsRequestHelper_ALMaxRewardedDriver_Show, hk_almax_rewarded_show, g_orig_almax_rewarded_show);
    HOOK_FN(il2cpp_base, rva::AdsRequestHelper_WrappedAdapter_isLoaded, hk_wrapped_adapter_is_loaded, g_orig_wrapped_adapter_is_loaded);
    HOOK_FN(il2cpp_base, rva::AdsRequestHelper_WrappedAdapter_Show, hk_wrapped_adapter_show, g_orig_wrapped_adapter_show);
    HOOK_FN(il2cpp_base, rva::AdsRequestHelper_WrappedAdapter_Show_Callback, hk_wrapped_adapter_show_callback, g_orig_wrapped_adapter_show_callback);
    HOOK_FN(il2cpp_base, rva::AdsRequestHelper_WrappedAdapter_Show_Callback_Source, hk_wrapped_adapter_show_callback_source, g_orig_wrapped_adapter_show_callback_source);
    HOOK_FN(il2cpp_base, rva::AdsRequestHelper_rewarded_high_eCPM_isLoaded, hk_rewarded_high_ecpm_is_loaded, g_orig_rewarded_high_ecpm_is_loaded);
    HOOK_FN(il2cpp_base, rva::AdsRequestHelper_rewarded_high_eCPM_show, hk_rewarded_high_ecpm_show, g_orig_rewarded_high_ecpm_show);
    HOOK_FN(il2cpp_base, rva::UnityEngine_Time_get_timeScale, hk_time_get_scale, g_orig_time_get_scale);
    HOOK_FN(il2cpp_base, rva::UnityEngine_Time_set_timeScale, hk_time_set_scale, g_orig_time_set_scale);
    g_startup_hooks_ready = true;

    if (g_install_gold_hooks) {
        install_gold_hooks_once(il2cpp_base);
    }
    update_tiny_direct_patches(il2cpp_base);

    LOGD("Archero startup hooks installed");
    return nullptr;
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    pthread_t config;
    pthread_create(&config, nullptr, config_thread, nullptr);
    pthread_detach(config);

    pthread_t hooks;
    pthread_create(&hooks, nullptr, hack_thread, nullptr);
    pthread_detach(hooks);

    return JNI_VERSION_1_6;
}
