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

Former Lua startup targets were all unvalidated by the hook report. The Lua file is no longer part of the current implementation; the carried-forward startup features below now resolve through the native metadata-first resolver:

| Former Lua index | Function | v7.9.1 RVA | Signature | Current native use / risk |
|---:|---|---:|---|---|
| 1 | `PlayerCharacter_UpgradeModel.GetATKBase` | `0x5904E60` | `int32_t(..., int32_t charid, MethodInfo*)` | Unvalidated, but return-int patch is type-compatible. |
| 2 | `PlayerCharacter_UpgradeModel.GetHPMaxBase` | `0x5905134` | `int32_t(..., int32_t charid, MethodInfo*)` | Unvalidated, type-compatible. |
| 3 | `Weapon_weapon.get_bThroughWall` | `0x58E2290` | `bool(..., MethodInfo*)` | Native startup hook forces true while `shoot_through_walls=1`. |
| 4 | `MapCreator.CreateGoodNotTrap` | `0x5C53C84` | `GameObject*(..., int,int,int,object[],MethodInfo*)` | Native startup hook returns null while `walk_through_walls=1`; this assumes the caller still tolerates no spawned object. |
| 5 | `MapCreator.DealWater` | `0x5C5260C` | `void(..., MethodInfo*)` | Native startup hook early-returns while `walk_through_water=1`. |
| 6 | `MapCreator.DealTrap` | `0x5C52030` | `void(..., MethodInfo*)` | Native startup hook early-returns while `walk_through_walls=1`. |
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

- Null-returning `CreateGoodNotTrap` is pointer-compatible only if the caller handles null.
- Void `MapCreator` hooks now use real native early-return replacements rather than raw `MOV X0, #0; RET` stubs.
- Name-based metadata resolution is strong for this target, but future overloads with the same class/method/arg-count shape should be disambiguated by parameter type.
- GG cannot safely synthesize IL2CPP `List<T>` objects or call instance setters, so real setters/list mutation were moved into LSPosed native hooks.

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

The default always-on gameplay profile enables headshot, godmode, high damage, high HP, attack speed, projectile shoot-through-wall, walk-through-water, walk-through-walls, and game speed. Optional gold/drop hooks remain off unless explicitly enabled in config.

## D. Why The Implementations Work

Native hooks use `A64HookFunction` on `libil2cpp.so + RVA`. The replacement receives the IL2CPP ABI arguments directly. It can call the original trampoline, scale arguments or returns, and return normally.

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
   - `hook_installed_count=12`
   - `resolver.metadata=12`
   - `resolver.aob=0`, `resolver.xref=0`, `resolver.rva=0`, `resolver.fail=0`
   - `walk_through_water=1`, `walk_through_walls=1`
4. Confirm the process stays alive after hook installation with `adb shell pidof com.habby.archero`.
5. For optional gold/drop hooks, edit the app-owned config and verify `config_loads` increments only after the file changes. The module stats the file every two seconds but skips reparsing unchanged content.
6. In gameplay, verify water and obstacle behavior manually in a map containing those tiles. Status counters `hits.walk_water`, `hits.walk_trap`, and `hits.walk_wall` increment only when the relevant `MapCreator` methods are exercised.

## F. Latest Device Verification

After rebuilding and installing the debug APK on the connected device:

- Launch activity: `com.habby.archero/.UnityPlayerActivity`.
- Process stayed alive: `pidof com.habby.archero` returned `7168` after startup and again after a longer stability wait.
- Status showed `il2cpp_metadata_ready=1`, `startup_hooks_ready=1`, and `il2cpp_metadata_wait_ms=2000`.
- All 12 startup hooks installed through metadata: `hook_installed_count=12`, `resolver.metadata=12`, `resolver.aob=0`, `resolver.xref=0`, `resolver.rva=0`, `resolver.fail=0`.
- Defaults were created with `walk_through_water=1` and `walk_through_walls=1`.
- Config polling stayed cheap during the unchanged-config run: `config_loads=1` while `status_writes` advanced from `8` to `23`.
- A live config edit and restore was picked up without restart; final restored status showed `game_speed_multiplier=3.000000`, `config_loads=4`, and the same running pid.
- Traversal hooks were exercised by startup/map creation: `hits.walk_water=16`, `hits.walk_trap=16`, `hits.walk_wall=315` by the second status read.

## Validation Gaps

- Only `GetHeadShot`, `GetMiss`, side-detection fields, and startup hook installation were device-validated from logs/status.
- `MapCreator.DealWater`, `MapCreator.DealTrap`, and `MapCreator.CreateGoodNotTrap` resolve by metadata and install cleanly, but traversal still needs in-map manual validation on water/obstacle tiles because the status counters fire only when those map paths execute.
- All gold/material RVAs are v7.9.1 dump-derived and should be promoted into the hook report after live validation.
- Direct profile gold setters (`LocalSave.Set_Gold`, `LocalSave.Modify_Gold`, `LocalSave.UserInfo_SetGold`) were not enabled because they affect lobby/account gold, not just in-stage gold.
- Field writes to `EntityAttributeBase.ValueFloatBase` were not implemented in GG because they require a validated live object pointer path.
