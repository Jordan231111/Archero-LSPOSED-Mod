# Archero 7.9.1 Hook And Gold/Drop Report

## A. Sanity Check

Historical Lua source audited during the initial migration:

- Source script: `/Users/jordan/Library/Application Support/com.netease.mumu.nemux/MuMuPlayerProShared.localized/Download/Archero-Debloated.lua`
- Current repo/module state: no Lua file is used or required.

The repo-local `lib/arm64-v8a/libil2cpp.so` was stale and matched the old v7.7.1 addresses. I extracted the v7.9.1 `global-metadata.dat` and `libil2cpp.so` from `/Users/jordan/Downloads/Archero_7.9.1_APKPure.xapk`, then dumped with both Il2CppDumper and Il2CppInspectorRedux.

Current v7.9.1 dump anchors:

- `CodeRegistration = 0x86AA030`
- `MetadataRegistration = 0x88EFF68`
- v7.9.1 `libil2cpp.so` SHA-256: `d0e00bc5c247d049cfccd3e644b8ccaa932583af34bf95ef1eb6218311ab6e58`
- v7.9.1 metadata SHA-256: `55236b7cb202c336c48f82be290aa8091b619e1f140fcd93333433758046c798`

Validated hook report entries:

| RVA | Function | Signature | Behavior | Existing use |
|---:|---|---|---|---|
| `0x4F7A2E8` | `EntityData.GetHeadShot` | `bool EntityData__GetHeadShot(EntityData* this, EntityBase* source, HitStruct* data, MethodInfo* method)` | Calculates whether a hit is a headshot. Native module forces true only for non-hero targets. | Not active in Lua; handled by LSPosed side-aware hook. |
| `0x4F6D280` | `EntityData.GetMiss` | `bool EntityData__GetMiss(EntityData* this, HitStruct* otherhs, MethodInfo* method)` | Calculates whether the receiver misses/evades a hit. Native module forces true only for hero targets. | Not active in Lua; handled by LSPosed side-aware hook. |
| field `0x18` | `EntityData.m_Entity` | `EntityBase*` | Links combat data back to entity. | Used by native side detection. |
| field `0x11B4` | `EntityBase.m_Type` | `EntityType` | Entity classification. | Used by native side detection. |
| enum `1` | `EntityType.Hero` | `int` | Hero side marker. | Used by native side detection. |

No mismatch was found for the report-validated v7.9.1 `GetHeadShot` / `GetMiss` signatures or side offsets. The old report section for v7.7.1 RVAs (`0x4F68364`, `0x4F5B440`) is historical only and must not be used for v7.9.1.

Former Lua startup targets were all unvalidated by the hook report. The Lua file is no longer part of the current implementation; the carried-forward startup features below now resolve through the native metadata-only resolver:

| Former Lua index | Function | v7.9.1 RVA | Signature | Current native use / risk |
|---:|---|---:|---|---|
| 1 | `PlayerCharacter_UpgradeModel.GetATKBase` | `0x5904E60` | `int32_t(..., int32_t charid, MethodInfo*)` | Unvalidated, but return-int patch is type-compatible. |
| 2 | `PlayerCharacter_UpgradeModel.GetHPMaxBase` | `0x5905134` | `int32_t(..., int32_t charid, MethodInfo*)` | Unvalidated, type-compatible. |
| 3 | `Weapon_weapon.get_bThroughWall` | `0x58E2290` | `bool(..., MethodInfo*)` | Native startup hook still forces this getter, but it is only one layer of the current shoot-through-wall stack. The live wall bypass also hooks weapon fields, `BulletTransmit`, and `BulletBase` wall collision. |
| 4 | `MapCreator.CreateGoodNotTrap` | `0x5C53C84` | `GameObject*(..., int,int,int,object[],MethodInfo*)` | Rejected for current native module. Returning null removes too many stage objects, including angels and item shops. |
| 5 | `MapCreator.DealWater` | `0x5C5260C` | `void(..., MethodInfo*)` | Rejected for current native module. Early return removes water generation globally instead of only changing player traversal. |
| 6 | `MapCreator.DealTrap` | `0x5C52030` | `void(..., MethodInfo*)` | Rejected for current native module. Early return is map-generation side, not player traversal side. |
| 7 | `AIBase.OnUpdate` | `0x3BE8938` | `void(..., float delta, MethodInfo*)` | Disables enemy AI by returning before update. |
| 8 | `GameConfig.GetModeLevelKey` | `0x58BBCD0` | `int32_t(MethodInfo*)` | Unvalidated, type-compatible. |
| 9 | `Weapon_weapon.get_Speed` | `0x58E0F80` | `float(..., MethodInfo*)` | Unvalidated, type-compatible. |
| 10 | `Weapon_weapon.get_AttackSpeed` | `0x58E0FB4` | `float(..., MethodInfo*)` | Unvalidated, type-compatible. |
| 11 | `Weapon_weapon.get_Attack` | `0x58E0F1C` | `int32_t(..., MethodInfo*)` | Unvalidated, type-compatible. |

Historical Lua structure notes kept only for audit. No Lua loader or Lua file is used by the current module:

- Lines 9-51 optionally load `archeroSpeedhack.lua`; this is unrelated to RVA validation.
- Lines 53-76 configure package, bitness, script version, and saved-offset cache. `xSVx` was bumped to `16.10` so stale saved offsets are invalidated.
- Lines 122-141 define fixed ARM/ARM64 return stubs. Existing `x100mi` and `x50f` are hardcoded return constants.
- Lines 143-258 add dynamic ARM64 return builders and LSPosed config writing.
- Lines 260-367 define target methods. Existing name-only targets (`CreateGoodNotTrap`, `DealWater`, `DealTrap`, `GetModeLevelKey`) rely on no unsafe overload collision.
- Lines 479-782 perform live metadata search. This is why the script can follow version changes, but also why method names/classes must be precise.
- Lines 785-803 back up seven instructions per target. Revert works because patches overwrite only the first two to four instructions and the backup contains a superset.
- Lines 982-1209 add gold/drop state, numeric inputs, native config toggles, and GG fallback primitive patches.
- Lines 1236-1285 add the gold/drop submenu.

Compatibility assumptions:

- Do not hook `MapCreator.CreateGoodNotTrap`, `MapCreator.DealWater`, or `MapCreator.DealTrap` for traversal. They are generation-side and remove rewards/shops/angels.
- Current traversal hooks mirror the game's own water-walk/stone-walk state instead. The module injects `SkillAlone2080` through `EntityBase.AddSkill(2080)` after `EntityBase.AddInitSkills`, calls the original `SetFlyWater`, `SetFlyStone`, and `SetFlyAll` paths for the hero, and mirrors the valid `EntityBase.m_EntityData` fly counters. This preserves map objects while changing player passability.
- The same `EntityBase.AddSkill(int)` metadata helper now injects Smart (`1000041`) and Greed (`1000040`) at battle init. `EntityBase.ContainsSkill(int)` is resolved by metadata and called as a non-hooked confirmation helper; status counters show whether the running game accepted each skill ID. Smart is directly named by `Skill_Smart_Rate = "SlotSkill_1000041%"`; Greed is the accepted adjacent slot ID and is backed by the dump's dedicated `Skill_greedyskill` table.
- Smart is a normal visible slot-skill entry and should usually appear in the acquired-skills UI. Greed is backed by `TableTool.Skill_greedyskill`, not just the regular `TableTool.Skill_skill` display path, so it can be accepted by `EntityBase.ContainsSkill(1000040)` without appearing in the same acquired-skills list. The status file/log confirmation is the stronger signal for Greed.
- Do not hook `EntityData.IsFlyWater` or `EntityData.IsFlyStone` directly. Crash triage showed the getter hook could be reached with an invalid receiver around the `EntityData.IsFlyStone` RVA, so the safer implementation only writes counters through a validated hero `EntityBase`.
- Name-based metadata resolution is strong for this target, but future overloads with the same class/method/arg-count shape should be disambiguated by parameter type.
- GG cannot safely synthesize IL2CPP `List<T>` objects or call instance setters, so real setters/list mutation were moved into LSPosed native hooks.

Server validation boundary:

- The current module does not try to forge or force-accept game-over reward packets. That belongs on the server side, especially now that this target shows explicit transaction/drop state in the client.
- `LocalSave.BattleInBase` carries `transid`, `serveruserid`, in-run `gold`, `skillids`, `goodids`, `equips`, `currentRewardIfWinList`, and server-drop dictionaries such as `m_dicDropEquipDatas`.
- Server-issued equipment drops are represented by `LocalSave.BattleInBase.ServerDropEquips` with `transid`, `time`, and a `ServerDropEquip[]`, while `LocalSave.BattleIn_DropEquipByServer`, `BattleIn_DropEquipDataByTransId(uint)`, and `GameOverModeCtrlBase.CheckDropEquipsByServer(...)` show that item drops can be reconciled against server-provided data.
- `HTTPSendClient.CheckGameOverCheat(NetCacheOne)` and `BattleModuleData.BuildCheatData()` show separate client-side reporting/telemetry around game-over validation.
- For an owned server fork, the clean path is to make the server's battle-settlement validator understand the intended rules: accepted injected skill IDs, reward caps, allowed item-drop transaction IDs, and multiplier bounds. The client should report coherent local battle state; it should not be the authority that forces validation.
- For server operators that want to confirm their server-side validators actually receive a settlement payload from every battle, the default-ON `force_server_validation` config switch installs pass-through hooks on `BattleModuleData.BuildCheatData`, `HTTPSendClient.CheckGameOverCheat(NetCacheOne)`, `LocalSave.BattleIn_DropEquipByServer`, `LocalSave.BattleIn_DropEquipDataByTransId(uint)`, and `GameOverModeCtrlBase.CheckDropEquipsByServer(...)`. The hooks call the original implementation and bump `hits.validate_*` counters in `archero_mod_status.txt`; per-target `hooks.validate_*` flags show which metadata-resolved hooks actually installed. They never alter return values or fabricate server responses, so they cannot be used to bypass server-side validation. Resolution is metadata-only via the existing `HookSpec` table with sentinel placeholder RVAs (`0xFE000001`-`0xFE000005`); if a future Archero version moves the class/method names, the hooks are skipped and `resolver.last_error` reports the failure.
- When `force_server_validation=1` the module also captures the live `BattleModuleData*` argument from each `BattleModuleData.AddGold(float|int)` call and, at settlement time inside `LocalSave.BattleIn_UpdateGold`, re-invokes the unmodified `BattleModuleData.BuildCheatData` on that pointer so the cheat-data payload is rebuilt against the final coherent battle state right before each gameover packet is sent. Per-call counters `hits.force_build_cheat_invoked` and `hits.force_build_cheat_missed` are exposed in `archero_mod_status.txt`.
- `dump_netcacheone=1` is default-on when `force_server_validation=1`; the `HTTPSendClient.CheckGameOverCheat` pass-through walks the live `NetCacheOne*` argument using `il2cpp_object_get_class`, `il2cpp_class_get_fields`, `il2cpp_field_get_name`, `il2cpp_field_get_type`, `il2cpp_field_get_offset`, and `il2cpp_type_get_name`. Each field's name, full .NET type, byte offset, and primitive value (or recursive object summary up to `netcacheone_dump_max_depth`) is written to `archero_mod_netcacheone.txt`. Strings are read via `il2cpp_string_chars` / `il2cpp_string_length` and ASCII-clamped for safety. Nothing in the object is mutated; the dump runs before the original call.
- `replay_netcacheone=1` is default-on when `force_server_validation=1`; the `HTTPSendClient.CheckGameOverCheat` pass-through re-invokes the original implementation a second time with the exact same `HTTPSendClient*` and `NetCacheOne*` arguments the game just used. The replayed packet uses the real `transid` produced by the run, so the server's idempotency / replay-detection behavior on a legitimate request becomes observable. Counters: `hits.netcacheone_replay` (succeeded), `hits.netcacheone_replay_skipped` (preconditions missing).

## B. Gold And Drop Paths

All entries below except `GetHeadShot`/`GetMiss` are recovered from the v7.9.1 dump and are not in the hook report, so they are marked unvalidated-by-report.

| Category | RVA | Signature | Implemented as |
|---|---:|---|---|
| Direct add | `0x372D16C` | `void BattleModuleData__AddGold(BattleModuleData*, float, MethodInfo*)` | Native scales positive add argument. |
| Direct add | `0x372D3CC` | `void BattleModuleData__AddGold(BattleModuleData*, int32_t, MethodInfo*)` | Native scales positive add argument. |
| Direct getter/set-read | `0x372D410` | `float BattleModuleData__GetGold(BattleModuleData*, MethodInfo*)` | Native fixed or scaled read; GG fallback fixed return. |
| Direct setter | `0x5A4CB30` | `void LocalSave__BattleIn_UpdateGold(LocalSave*, float, MethodInfo*)` | Native fixed or scaled setter argument. |
| Direct getter | `0x5A4DFD0` | `float LocalSave__BattleIn_GetGold(LocalSave*, MethodInfo*)` | Native fixed/scaled read; GG fallback fixed return. |
| Player/stat formula | `0x4F69AA4` | `float EntityData__getGold(EntityData*, int64_t, MethodInfo*)` | Native scales return; GG fallback fixed return. |
| Player/stat field | field `EntityAttributeBase.Monster_GoldDrop 0x1450`, `NormalGoldDropPercent 0x1458`, `HeroGoldDropPercent 0x1460`, `Level_CoinPercentValue 0x1648` | `ValueFloatBase*` fields | Not field-written in GG; needs live entity pointer or IL2CPP field helper validation. |
| Config gold | `0x58BC0B0` | `float GameConfig__GetCoin1Wave(MethodInfo*)` | Native scales; GG fallback fixed return. |
| Config gold | `0x58BC528` | `int32_t GameConfig__GetBoxDropGold(MethodInfo*)` | Native scales; GG fallback fixed return. |
| Config gold | `0x58BC580` | `int32_t GameConfig__GetBoxChooseGold(int32_t, MethodInfo*)` | Native scales; GG fallback fixed return. |
| Drop scalar | `0x58F4F70` | `float Drop_DropModel__GetGoldDropPercent(Drop_DropModel*, MethodInfo*)` | Native scales; GG fallback fixed return. |
| Drop scalar | `0x58F52E4` | `int32_t Drop_DropModel__GetDropGold(Drop_DropModel*, List<DropData>*, MethodInfo*)` | Native scales; GG fallback fixed return. |
| Pickup/display scalar | `0x4DF4568` | `int32_t DeadGoodMgr__GetGoldNum(DeadGoodMgr*, MethodInfo*)` | Native scales; GG fallback fixed return. |
| Drop lists | `0x34807F4`, `0x3480994`, `0x58D328C`, `0x5B6D514` | `List<BattleDropData>* ...` | Native scales boxed scalar `BattleDropData.data` for gold/pure-gold. |
| Realtime save | `0x58D5CDC` | `bool GameLogic__CanSaveGoldInRealTime(MethodInfo*)` | Native can force true. |
| Mode/stage reward ratios | `0x5BF9F88`, `0x5BFB464`, `0x5BFF508`, `0x5BFFB30`, `0x5C01B2C`, `0x5C02154`, `0x5C04134`, `0x5C05334`, `0x5C0B1F8`, `0x5C0BA04`, `0x5C0C764`, `0x5C0D140`, `0x5C0E13C`, `0x5C0E9F8`, `0x5C0EBE4`, `0x5C0ED20`, `0x5C1112C`, `0x5C11CA8`, `0x5C13FDC`, `0x5C14604` | `float GetGoldRatio()` / `int GetDropDataGold(Soldier_soldier*)` | Native scales results across mode overrides. |
| Stage free gold | `0x5BAA8A4`, `0x5BAD2EC` | `float StageLevelManager__GetGoldDropPercent(..., int layer, ...)`, `int GetFreeGold()` | Native scales; GG fallback covers primary manager. |
| Material/equipment mutators | `0x5B60B00`, `0x5B63320`, `0x5B635D8`, `0x5B63980`, `0x5B63C38`, `0x5B653B8`, `0x5B65724`, `0x5B659DC`, `0x5B69F68`, `0x5B63EF0`, `0x5B641D8`, `0x5B65108`, `0x5B66034`, `0x5B6A22C`, `0x5B64544`, `0x5B66DD0`, `0x5B67140`, `0x5B61470`, `0x5B674B0`, `0x5B67778`, `0x5B67A40`, `0x5B618B4`, `0x5B647FC`, `0x5B6A4F4`, `0x5B6A7CC` | `void DropManager__Get*(DropManager*, ref List<BattleDropData>, Soldier_soldier*, ...)` | Native repeats original mutator call `N` times. Covers scroll/equip exp, equipment, bloodstone, sapphire/stone-like materials, magic/star stones, runes, cookies, coins, etc. |

## C. Current Native Module

- Native module source: `/Users/jordan/Documents/temp/archerodecompiled/archero_mod/app/src/main/cpp/mod.cpp`
- Built APK: `/Users/jordan/Documents/temp/archerodecompiled/archero_mod/app/build/outputs/apk/debug/app-debug.apk`
- Suggested readable config template: `/Users/jordan/Documents/temp/archerodecompiled/archero_mod_config.low-risk.txt`
- Runtime status file: `/storage/emulated/0/Android/data/com.habby.archero/files/archero_mod_status.txt`

There is no Lua loader in the current flow. The module creates and reloads only the app-owned native config:

`/storage/emulated/0/Android/data/com.habby.archero/files/archero_mod_config.txt`

The default always-on gameplay profile enables headshot, godmode, high damage, high HP, attack speed value `100.0`, projectile shoot-through-wall, walk-through-water, walk-through-walls, Greed skill injection, Smart skill injection, rewarded-ad callback completion, game speed multiplier `4.0`, and config-gated hero movement speed scaling. Optional gold/drop hooks remain off unless explicitly enabled in config.

Current shoot-through-wall targets:

| Feature | Native target | RVA | Behavior |
|---|---|---:|---|
| Weapon through-wall getter | `TableTool.Weapon_weapon.get_bThroughWall()` | `0x58E2290` | Returns true while `shoot_through_walls=1` and writes the cached weapon through-wall fields. |
| Weapon inside-wall getter | `TableTool.Weapon_weapon.get_bThroughInsideWall()` | `0x58E22D8` | Returns true while `shoot_through_walls=1` and writes the cached weapon through-wall fields. |
| Bullet setup | `BulletTransmit.Init(EntityBase,int,bool,float)` | `0x33402BC` | After the original init, hero-owned bullets get `ThroughWall=true` and weapon data is forced through-wall. |
| Bullet setup | `BulletTransmit.Init(EntityBase,int,float,float,int,float,bool,float)` | `0x3340890` | Same as the simple init overload for the longer bullet setup path. |
| Bullet getter | `BulletTransmit.get_ThroughWall()` | `0x3341BB4` | Backup path that returns true and writes the backing field while enabled. |
| Bullet wall collision | `BulletBase.HitWall(Collider)` | `0x5FEF35C` | For hero bullets, refreshes through-wall state and skips the original wall-hit/despawn logic. Non-hero bullets call the original method. |
| Bullet wall collision | `BulletBase.<TriggerEnter1>g__HitWallInternal|337_0(...)` | `0x5FF35E8` | The live-tested wall handler. For hero bullets, refreshes through-wall state and skips the original wall-hit/despawn logic. |

Current shoot-through-wall metadata-resolved fields:

| Owner | Observed offset | Metadata field | Behavior |
|---|---:|---|---|
| `TableTool.Weapon_weapon` | `0x346` | `bThroughWallp` | Forced true on weapon data while shoot-through-wall is enabled. |
| `TableTool.Weapon_weapon` | `0x348` | `bThroughInsideWallp` | Forced true on weapon data while shoot-through-wall is enabled. |
| `BulletTransmit` | `0x50` | `m_Entity` | Used to confirm the bullet owner is the hero. |
| `BulletTransmit` | `0x58` | `weapondata` | Used to reach and force the associated weapon data. |
| `BulletTransmit` | `0xC8` | `<ThroughWall>k__BackingField` | Forced true on hero-owned bullet transmit objects. |
| `BulletBase` | `0x88` | `<m_Entity>k__BackingField` | Used to confirm the bullet owner is the hero. |
| `BulletBase` | `0xA0` | `m_Data` | Used to reach and force the associated weapon data. |
| `BulletBase` | `0xA8` | `mBulletTransmit` | Used to reach and force the associated bullet transmit object. |

Current traversal targets:

| Feature | Native target | RVA | Behavior |
|---|---|---:|---|
| Battle-start skill injection | `EntityBase.AddInitSkills()` | `0x4C33C78` | Calls the original initializer, then injects water walk `2080`, Greed `1000040`, and Smart `1000041` for the hero. |
| Battle-start skill helper | `EntityBase.AddSkill(int)` | `0x4C320FC` | Resolved by metadata and called directly; not hooked. |
| Battle-start skill confirmation | `EntityBase.ContainsSkill(int)` | `0x4C23308` | Resolved by metadata and called directly after injection; not hooked. Updates `hits.skill_confirm_*` status counters. |
| Walk through water | `EntityBase.SetFlyWater(bool)` | `0x4C1C294` | Forces true only for hero entity while enabled and lets the original native path update child collider state. |
| Walk through water | `EntityBase.GetFlyWater()` | `0x4C1C4C4` | Returns true only for hero entity while enabled. |
| Walk through walls/obstacles | `EntityBase.SetFlyStone(bool)` | `0x4C1C418` | Forces true only for hero entity while enabled and lets the original native path update child collider state. |
| Walk through walls/obstacles | `EntityBase.SetFlyAll(bool)` | `0x4C23A34` | Syncs all fly/collider state for the hero after traversal state is forced. |
| Walk through walls/obstacles | `EntityBase.get_OnCalCanMove()` | `0x4C1FD90` | Returns true for the hero while traversal is enabled. |
| Walk through walls/obstacles | `EntityBase.SetCollider(bool)` | `0x4C23850` | Applies traversal runtime sync around collider updates. |
| Walk through walls/obstacles | `EntityBase.check_pos(...)` | `0x4C29A60` | For the hero, returns the requested position while traversal is enabled so obstacle rejection does not snap the player back. |
| Walk through water/walls | `EntityHitCtrl.SetFlyOne(string,bool)` | `0x53094B4` | Forces hero collision layer updates for `Entity2Water`, `Entity2Stone`, `Entity2MapOutWall`, and `Entity2DragonStone`. |

Metadata-resolved field/counter mirrors used by the traversal runtime:

| Owner | Observed offset | Behavior |
|---|---:|---|
| `EntityBase.bFlyWater` | `0x234` | Forced true for the hero while `walk_through_water=1`. |
| `EntityBase.bFlyStone` | `0x235` | Forced true for the hero while `walk_through_walls=1`. |
| `EntityBase.m_EntityData` | `0xB0` | Validated bridge from hero entity to entity data before counter writes. |
| `EntityData.mFlyStoneCount` | `0xA8` | Forced to at least `1` for the hero's entity data. |
| `EntityData.mFlyWaterCount` | `0xAC` | Forced to at least `1` for the hero's entity data. |
| `EntityBase.move_layermask` | `0xE98` | Zeroed for the hero when traversal is enabled. |

These are traversal-state hooks, not map-generation hooks. They preserve water, walls, angels, shops, and other room objects.

Current movement-speed target:

| Feature | Native target | RVA | Behavior |
|---|---|---:|---|
| Hero movement speed | `MoveControl.UpdateProgress()` | `0x5538124` | For hero move controls only, calls the original update exactly once. For multipliers above `1.0x`, it then applies the extra distance through `EntityBase.SelfMoveBy(Vector3)` using the current `MoveDirection * Time.deltaTime`. For multipliers below `1.0x`, it temporarily scales `MoveDirection` for the single original call, then restores the original `ObscuredVector3`. Non-hero movement and disabled/`1.0x` configs pass through unchanged. |

Current movement-speed helpers:

| Feature | Native target | RVA | Behavior |
|---|---|---:|---|
| Extra movement application | `EntityBase.SelfMoveBy(Vector3)` | `0x4C2979C` | Called directly for the added movement distance so the game's own movement/collision path handles the position update. |
| Frame delta helper | `UnityEngine.Time.get_deltaTime()` | `0x84259B4` | Called directly to match the original movement frame scale when applying the extra distance. |

Movement-speed metadata-resolved fields:

| Owner | Observed offset | Metadata field | Behavior |
|---|---:|---|---|
| `MoveControl` | `0x10` | `m_Entity` | Used to confirm the move controller belongs to the hero. |
| `MoveControl` | `0x18` | `bMoveing` | Checked before applying extra `SelfMoveBy` distance. |
| `MoveControl` | `0x70` | `MoveDirection` | Temporarily scaled during `UpdateProgress` substeps. |
| `CodeStage.AntiCheat.ObscuredTypes.ObscuredVector3` | `0x0` | `currentCryptoKey` | Used to decrypt/re-encrypt the hidden vector. |
| `CodeStage.AntiCheat.ObscuredTypes.ObscuredVector3` | `0x4` | `hiddenValue` | Encrypted `Vector3` payload. |
| `CodeStage.AntiCheat.ObscuredTypes.ObscuredVector3` | `0x10` | `inited` | Preserved when restoring the original direction. |
| `CodeStage.AntiCheat.ObscuredTypes.ObscuredVector3` | `0x14` | `fakeValue` | Preserved/restored for anti-cheat shadow value consistency. |
| `CodeStage.AntiCheat.ObscuredTypes.ObscuredVector3` | `0x20` | `fakeValueActive` | Controls whether the fake value is updated during temporary scaling. |

Current rewarded-ad bypass targets:

| Feature | Native target | RVA | Behavior |
|---|---|---:|---|
| High-level loaded check | `AdCallbackControl.IsLoaded(int)` | `0x5897EB4` | Returns loaded while `skip_rewarded_ads=1` and reward/close callback helpers resolved. |
| High-level show | `AdCallbackControl.Show(int)` | `0x589851C` | Completes `AdCallbackControl.onReward` and `onClose` directly instead of opening an ad. |
| High-eCPM loaded check | `AdsRequestHelper.rewarded_high_eCPM_isLoaded()` | `0x58961B8` | Returns loaded while ad skipping is enabled. |
| High-eCPM show | `AdsRequestHelper.rewarded_high_eCPM_Show(AdsCallback, ADSource)` | `0x58961C8` | Completes the supplied callback directly. |
| Driver loaded check | `AdsRequestHelper.ALMaxRewardedDriver.isLoaded()` | `0x589F24C` | Returns loaded for the lower-level AppLovin rewarded driver. |
| Driver show | `AdsRequestHelper.ALMaxRewardedDriver.Show()` | `0x589F478` | Completes the driver's stored callback directly and returns success. |
| Adapter loaded check | `AdsRequestHelper.WrappedAdapter.isLoaded()` | `0x58A4ABC` | Returns loaded for wrapped rewarded adapters. |
| Adapter show overloads | `AdsRequestHelper.WrappedAdapter.Show(...)` | `0x58A4D14`, `0x58A4E5C`, `0x58A4FB0` | Completes stored or supplied callbacks directly and returns success. |
| Callback completion | `AdCallbackControl`, `WrappedDriver`, `CombinedDriver`, `CallbackRouter` reward/close methods | `0x5899010`/`0x5898D64`, `0x58A1200`/`0x58A1038`, `0x58A3490`/`0x58A32E0`, `0x58A4758`/`0x58A4090` | Calls the same reward/close callbacks the ad system normally reaches after a successful rewarded ad. |

Rewarded-ad private fields resolved by metadata:

| Owner | Observed offset | Metadata field | Behavior |
|---|---:|---|---|
| `AdCallbackControl` | `0x30` | `bCallback` | Cleared before direct reward/close completion so the callback is not left in an open/in-progress state. |
| `AdCallbackControl` | `0x31` | `bOpened` | Set before reward/close completion to satisfy the class's own reward guard. |
| `AdsRequestHelper.BaseDriver` | `0x18` | `callback` | Read from lower-level rewarded drivers before completing the reward/close callback directly. |
| `AdsRequestHelper.WrappedAdapter` | `0x10` | `callbacks` | Read from adapter show paths when the callback is stored in the wrapper rather than passed as an argument. |

## D. Why The Implementations Work

Native hooks use `A64HookFunction` on metadata-resolved IL2CPP method pointers. The v7.9.1 RVAs in the tables are dump/spec anchors for identifying targets in the source, not runtime address fallbacks. If metadata cannot resolve a method or field, the module fails closed and skips that hook instead of installing by a fixed offset.

Primitive return hooks are safe because ARM64 IL2CPP returns:

- `bool` / `int32_t` in `W0`
- `float` in `S0`
- object pointers in `X0`

The native module avoids fabricating managed lists. For list-returning gold paths, it calls the original, validates the `List<BattleDropData>` layout from Il2CppInspectorRedux, and only scales boxed `int32_t` payloads for known scalar `FoodType` values. For material/equipment paths, it repeats the original `DropManager.Get*` mutator, letting the game allocate and append its own valid `BattleDropData` instances.

The GG fallbacks use generated ARM64 stubs:

- `MOV W0, imm16`
- `MOVK W0, imm16, LSL #16`
- optional `FMOV S0, W0`
- `RET`

That is only used for primitive return methods. It is intentionally not used for list-returning or void setter methods.

## E. Verification Checklist

1. Install `/Users/jordan/Documents/temp/archerodecompiled/archero_mod/app/build/outputs/apk/debug/app-debug.apk`, enable it in LSPosed, scope `com.habby.archero`, force-stop Archero, relaunch.
2. Clear stale runtime files before a clean run if needed:
   `adb shell rm -f /storage/emulated/0/Android/data/com.habby.archero/files/archero_mod_config.txt /storage/emulated/0/Android/data/com.habby.archero/files/archero_mod_status.txt`
3. Confirm status after launch:
   - `il2cpp_metadata_ready=1`
   - `startup_hooks_ready=1`
   - `hook_installed_count=35`
   - `resolver.metadata=47`
   - `resolver.aob=0`, `resolver.xref=0`, `resolver.rva=0`, `resolver.fail=0`
   - `field_resolver.metadata=30`, `field_resolver.fallback=0`, `field_resolver.fail=0`
   - `field_offsets_ready=1`
   - `shoot_through_walls=1`
   - `walk_through_water=1`, `walk_through_walls=1`
   - `inject_greed_skill=1`, `inject_smart_skill=1`
   - `skip_rewarded_ads=1`
4. Confirm the process stays alive after hook installation with `adb shell pidof com.habby.archero`.
5. For optional gold/drop hooks, edit the app-owned config and verify `config_loads` increments only after the file changes. The module stats the file every two seconds but skips reparsing unchanged content.
6. In gameplay, verify shoot-through-wall on an actual wall. The strongest current status signal is `hits.bullet_hitwall_internal_bypass` increasing, with `hits.weapon_wall_field_apply` and `hits.runtime_walls_init` also nonzero. `hits.bullet_transmit_wall_get` may stay zero if the live path never calls that getter.
7. In gameplay, verify water and obstacle behavior manually in a map containing those tiles. Status counters `hits.walk_water` and `hits.walk_wall` increment only when the relevant hero fly-water/fly-stone paths are exercised.
8. For movement speed, set `move_speed=1` and a visible test value such as `move_speed_multiplier=10`, then verify `hits.move_progress_hero`, `hits.move_progress_apply`, and `hits.move_progress_substeps` increase while moving the hero. For multipliers above `1.0x`, `hits.move_progress_substeps` now counts extra `SelfMoveBy` applications, not extra `UpdateProgress` re-entries.
9. For rewarded ads, set `skip_rewarded_ads=1`, tap a rewarded placement, and verify the app stays in `com.habby.archero/.UnityPlayerActivity` while `hits.ad_skip_reward` and `hits.ad_skip_close` increase.

## F. Latest Device Verification

After rebuilding and installing the shoot-through-wall/movement debug APK on the connected device:

- Launch activity: `com.habby.archero/.UnityPlayerActivity`.
- Process stayed alive: `pidof com.habby.archero` returned `7850` after startup, battle entry, and status/log checks.
- Status showed `il2cpp_metadata_ready=1`, `startup_hooks_ready=1`, and `il2cpp_metadata_wait_ms=2000`.
- Current build installs 35 startup hooks through metadata: `hook_installed_count=35`, `resolver.metadata=47`, `resolver.aob=0`, `resolver.xref=0`, `resolver.rva=0`, `resolver.fail=0`. The extra metadata resolutions include direct-call helpers for movement, rewarded-ad callback completion, and battle-skill injection/confirmation.
- Runtime field offsets also resolved through metadata: `field_resolver.metadata=30`, `field_resolver.fallback=0`, `field_resolver.fail=0`, and `field_offsets_ready=1`. This includes the private rewarded-ad callback/storage fields `AdCallbackControl.bCallback`, `AdCallbackControl.bOpened`, `AdsRequestHelper.BaseDriver.callback`, and `AdsRequestHelper.WrappedAdapter.callbacks`.
- Defaults were active with `walk_through_water=1`, `walk_through_walls=1`, `inject_greed_skill=1`, and `inject_smart_skill=1`.
- Speed defaults were active with `attack_speed_value=100.000000` and `game_speed_multiplier=4.000000`.
- The device test config set `move_speed=1` and `move_speed_multiplier=10`; the stable movement hook path is `MoveControl.UpdateProgress` plus direct `EntityBase.SelfMoveBy` extra distance. Earlier backup movement experiments and the re-entrant `UpdateProgress` multiplier were removed from the active module.
- Rewarded-ad skip was validated from the Hero Patrol "Extra Earnings" placement: tapping the placement stayed in the Unity activity, `hits.ad_skip_driver=2`, `hits.ad_skip_reward=2`, and `hits.ad_skip_close=2`, with no filtered crash/ad-fullscreen logcat matches. A later private-field resolver pass confirmed the ad storage fields resolved through metadata with `field_resolver.state=resolved=30/30 ready=1 ... ad_cb=0x30 base_cb=0x18 adapter_cb=0x10`.
- Shoot-through-wall was confirmed on the live bullet-wall path with `hits.always_walls=161`, `hits.runtime_walls_apply=65`, `hits.runtime_walls_init=1582`, `hits.weapon_wall_field_apply=1873`, and `hits.bullet_hitwall_internal_bypass=65`.
- Battle-init logs showed `Battle skill greed id=1000040 ... confirmed=1` and `Battle skill smart id=1000041 ... confirmed=1`.
- Final status showed `hits.skill_inject_greed=1`, `hits.skill_confirm_greed=1`, `hits.skill_inject_smart=1`, `hits.skill_confirm_smart=1`, `hits.skill_inject_fail=0`, `hits.skill_confirm_fail=0`, and `hits.skill_confirm_unavailable=0`.
- Water traversal still runs through the direct traversal path: `hits.walk_skill_inject=1`, `hits.walk_runtime_apply=1`, and `hits.walk_entitydata_apply=2`. `hits.skill_confirm_water=0` because `ContainsSkill(2080)` does not report that lower-ID traversal ability in the same slot-skill list used by Greed/Smart.
- After clearing logcat and watching another 10 seconds, filtered crash logcat showed no `FATAL EXCEPTION`, native fatal signal, or tombstone for `com.habby.archero`. The process remained alive.
- The earlier `MapCreator` traversal implementation was removed because it suppressed room objects. Current traversal does not hook `MapCreator`, so room objects such as water, walls, angels, and item shops remain generated by the game.
- Latest non-root LSPatch split archive: `/Users/jordan/Documents/temp/archerodecompiled/dist/lspatch_archero_7.9.1_xapk/archero_7.9.1_lspatched_full.apks`, SHA-256 `6482d9a238e44d84541e42b74205e90a5a2d3b6c0a0b29252f4b4d9474730961`. This archive is a local artifact and is not committed because it is about `2.5G`.

## Validation Gaps

- `GetHeadShot`, `GetMiss`, side-detection fields, startup hook installation, Greed/Smart skill acceptance, traversal runtime counters, rewarded-ad callback completion, movement hook installation, and the live `BulletBase` wall-collision bypass were device-validated from logs/status.
- Manual user validation is still useful on a map with water tiles because the current room only validated battle-start water-skill injection, fly-counter writes, and obstacle/movement `check_pos` traversal. The native status confirms those paths are active.
- All gold/material RVAs are v7.9.1 dump-derived and should be promoted into the hook report after live validation.
- Direct profile gold setters (`LocalSave.Set_Gold`, `LocalSave.Modify_Gold`, `LocalSave.UserInfo_SetGold`) were not enabled because they affect lobby/account gold, not just in-stage gold.
- Field writes to `EntityAttributeBase.ValueFloatBase` were not implemented in GG because they require a validated live object pointer path.
- Optional gold/material/drop hooks remain off by default because stage settlement is not purely local. The dump shows transaction IDs, server drop payloads, cached game-over packets, and game-over cheat checks, so direct reward edits should be treated as local experiments only unless the owned server-side validator is changed to accept the same rules.
