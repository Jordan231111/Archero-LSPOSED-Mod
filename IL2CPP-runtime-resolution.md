# IL2CPP Runtime Hook Resolution

## Local Target Finding

The v7.9.1 Archero files in `diffwork/archero_7_9_1/input` are normal Android IL2CPP inputs:

- `global-metadata.dat` starts with `FA B1 1B AF`, the standard IL2CPP metadata magic, so this target is not metadata-encrypted in the checked build.
- `readelf -Ws libil2cpp.so` shows exported runtime APIs including `il2cpp_domain_get`, `il2cpp_domain_get_assemblies`, `il2cpp_assembly_get_image`, `il2cpp_class_from_name`, `il2cpp_class_get_method_from_name`, `il2cpp_class_get_methods`, `il2cpp_image_get_class_count`, `il2cpp_image_get_class`, and method/type helpers.
- On-device testing showed `il2cpp_domain_get()` can crash from the injected module thread in this build. The implementation therefore resolves exports from the already loaded `libil2cpp.so` with `dl_iterate_phdr`, then enumerates assemblies with `il2cpp_domain_get_assemblies(nullptr, &count)`. The wrapper ignores the domain pointer on this target, avoids the crashing domain initializer, and returned 123 assemblies in the verified run.
- `il2cpp_method_get_pointer` is not exported here, so the native resolver falls back to the stable `MethodInfo.methodPointer` first-field layout and validates that the result lands in an executable `libil2cpp.so` mapping before installing a hook.

## Recommendation

Use this runtime policy:

1. IL2CPP metadata/runtime API name resolution.
2. Metadata-resolved game field offsets for every object field the module reads or writes.
3. Fail closed when a method, helper, or field cannot be identified.
4. Runtime dumpers as operator tooling for protected updates, not as the normal launch path.

This is now implemented in the native module. `install_hook()` resolves through `resolve_hook_target()`, optional 8-byte direct patches use the same metadata resolver before writing code, and status output exposes `il2cpp_metadata_ready`, `il2cpp_metadata_wait_ms`, `startup_hooks_ready`, `resolver.metadata`, `resolver.aob`, `resolver.xref`, `resolver.rva`, `resolver.fail`, `resolver.last_error`, `field_resolver.metadata`, `field_resolver.fallback`, `field_resolver.fail`, `field_offsets_ready`, and direct-patch counters. There is no Lua loader or Lua file in this flow. The AOB/xref/RVA counters are retained for status compatibility and should stay at zero in this metadata-only build; the native source no longer carries byte-signature or string-xref resolver fields.

## Technique Survey

**Byte / AOB signatures in `libil2cpp.so`**

Pros: works without metadata names and survives ASLR. Cons: sensitive to compiler output, Unity version, optimization changes, and branch/call target movement. The current module does not use this technique and does not keep AOB signature strings in the hook table.

**IL2CPP metadata-driven resolution**

Pros: best normal path for this target. Class and method names are much more stable than RVAs; exported runtime APIs avoid parsing `global-metadata.dat` by hand. It handles ASLR and most app version bumps automatically. Cons: fails when metadata names are encrypted, stripped, or obfuscated; overloads require parameter-type checks; generic/virtual cases may need inflated method handling. For this target, metadata is standard and exports are present, so this is the primary resolver.

**String-xref anchoring**

Pros: stable when a unique string is retained, especially Unity icall names, logging strings, error strings, and format strings. Cons: not every gameplay method has a durable literal; ARM64 xref recovery must decode `ADRP/ADD` or literal loads; strings can move or be stripped. The current module does not use this technique and does not keep string-xref anchors in the hook table.

**Vtable / RGCTX index resolution**

Pros: useful for virtual methods when class shape is stable and overrides move across versions. Resolve the class by name, then locate the method by slot or use `il2cpp_object_get_virtual_method` for an instance. Cons: class hierarchy changes and generic sharing can break assumptions. Use this for virtual/interface hooks after the non-virtual metadata resolver identifies the declaring method and confirms the slot.

**Symbol-export resolution**

Pros: trivial and robust for exported IL2CPP API functions and native plugin exports. Cons: stripped IL2CPP game methods are rarely exported. In this target, runtime APIs are exported, but game methods are not. The implementation uses exports to resolve metadata APIs, not to hook gameplay methods directly.

**Runtime dumpers**

Pros: strongest operator path for protected or encrypted updates. Cons: operational cost, root/frida requirements, first-launch delay, and more moving parts. Current practical choices:

- Il2CppDumper: still useful offline; supports ELF/ELF64 and Android memory-dumped `libil2cpp.so`, with command-line use `Il2CppDumper.exe <executable-file> <global-metadata> <output-directory>`.
- Il2CppInspectorRedux: better static analysis output for newer metadata versions and JSON/address-map workflows; useful for regenerating a hook spec table after big updates.
- Zygisk-Il2CppDumper: best protected-Android option on rooted devices because it dumps at runtime and is designed to bypass protection/encryption/obfuscation.
- frida-il2cpp-bridge: best interactive option; can dump, trace, intercept, and replace IL2CPP calls at runtime without needing `global-metadata.dat`.

For this module, runtime dumpers should stay outside the normal launch path. If a protected update defeats metadata resolution, invoke a runtime dumper from the operator environment, generate fresh `dump.cs`/JSON/script output, then update the native `HookSpec` table only where names changed.

## Hybrid Pseudocode

```text
wait until libil2cpp.so is mapped
sleep only the minimum bootstrap settle window currently configured as 2000 ms
resolve IL2CPP exports from the loaded module with dl_iterate_phdr
poll every 250 ms until il2cpp_domain_get_assemblies(nullptr, &count) + EntityData.GetHeadShot resolve
stop polling at 30000 ms; if metadata is still unavailable, log the timeout and do not install gameplay hooks

for each hook:
  target = resolve_metadata(namespace, class, method, arg_count, param_type)
  if not executable(target):
    log resolver.last_error and skip hook
  else:
    install inline hook
```

## Files Changed

- `archero_mod/app/src/main/cpp/mod.cpp`: metadata resolver stack, hook spec table, readiness-based metadata wait, always-on gameplay hooks, status counters.
- `archero_mod/app/src/main/cpp/CMakeLists.txt`: links `dl` for `dlopen`/`dlsym`.

## Current Always-On Startup Hooks

The default startup set is metadata-only. Every method target and game field offset resolves through IL2CPP metadata; unresolved targets fail closed:

- `EntityData.GetHeadShot`: headshot behavior for non-hero targets.
- `EntityData.GetMiss`: godmode behavior for the hero.
- `TableTool.PlayerCharacter_UpgradeModel.GetATKBase`: high damage value.
- `TableTool.PlayerCharacter_UpgradeModel.GetHPMaxBase`: high HP value.
- `TableTool.Weapon_weapon.get_Speed` and `get_AttackSpeed`: high attack speed value `100.0`.
- `TableTool.Weapon_weapon.get_bThroughWall` and `get_bThroughInsideWall`: projectile shoot-through-wall getters; while enabled, the hook also writes the private `bThroughWallp` and `bThroughInsideWallp` fields on the weapon data object.
- `BulletTransmit.Init(...)` overloads and `BulletTransmit.get_ThroughWall`: hero bullet setup forces `BulletTransmit.<ThroughWall>k__BackingField` and the weapon-data through-wall fields when the bullet owner is the hero.
- `BulletBase.HitWall(Collider)` and `BulletBase.<TriggerEnter1>g__HitWallInternal|337_0`: hero bullet wall-collision/despawn handlers return early after refreshing through-wall state; non-hero bullets still call the original methods.
- `EntityBase.AddInitSkills`: after the hero's initial battle skills are created, injects always-on battle skills by resolving and calling `EntityBase.AddSkill(int)`: water walk `2080`, Greed `1000040`, and Smart `1000041`. The module also resolves `EntityBase.ContainsSkill(int)` so status output can confirm whether each injected ID is present on the hero.
- Smart should normally show in the acquired-skills UI because it is a regular visible slot skill (`Skill_Smart_Rate = "SlotSkill_1000041%"`). Greed is runtime-confirmed as accepted (`ContainsSkill(1000040)=true`) and backed by `TableTool.Skill_greedyskill`, but that separate table path can keep it out of the regular acquired-skills display.
- `EntityBase.SetFlyWater`, `EntityBase.GetFlyWater`, `EntityBase.SetFlyStone`, `EntityBase.SetFlyAll`, `EntityBase.get_OnCalCanMove`, `EntityBase.SetCollider`, `EntityBase.check_pos`, and `EntityHitCtrl.SetFlyOne`: force only the hero's traversal state while preserving map generation.
- Direct hero state mirroring: sets `EntityBase.bFlyWater`, `EntityBase.bFlyStone`, `EntityBase.move_layermask`, and the valid `EntityBase.m_EntityData` fly counters (`mFlyWaterCount`, `mFlyStoneCount`) after resolving the hero instance. The module does not hook `EntityData.IsFlyWater` or `EntityData.IsFlyStone`; direct getter hooks were rejected after crash triage.
- `MoveControl.UpdateProgress`: config-gated hero-only movement scaling. Multipliers above `1.0x` call the original update once and then apply extra distance through `EntityBase.SelfMoveBy(Vector3)` using `UnityEngine.Time.get_deltaTime`; multipliers below `1.0x` temporarily scale `MoveControl.MoveDirection` for one original call, then restore the original `ObscuredVector3` value.
- Rewarded ad callbacks: `AdCallbackControl`, `AdsRequestHelper.rewarded_high_eCPM_*`, `ALMaxRewardedDriver`, and `WrappedAdapter` load/show paths can complete the app's reward/close callbacks directly when `skip_rewarded_ads=1`.
- `UnityEngine.Time.get_timeScale` and `set_timeScale`: forced game speed, default multiplier `4.0`.

The resolver is intentionally not used to force game-over server validation. The static dump shows transaction IDs, server-drop structures, cached game-over packets, and client-side cheat reporting around settlement. For an owned backend, update the server-side settlement rules to accept the desired skill/reward policy instead of adding client-side packet-forcing behavior.

The config thread still checks the authoritative app-owned config every two seconds, but it stats the file first and only reparses when size or timestamps change. In the verified default run this kept `config_loads=1` while status writes continued, so long sessions do not continually parse the config.

## Device Verification

Verified on the connected Android device after installing the rebuilt debug APK and launching `com.habby.archero/.UnityPlayerActivity`:

- `il2cpp_metadata_ready=1`, `startup_hooks_ready=1`, `il2cpp_metadata_wait_ms=2000`.
- First traversal implementation used `MapCreator.DealWater`, `MapCreator.DealTrap`, and `MapCreator.CreateGoodNotTrap`; that was rejected because it removed walls/water and also suppressed stage objects such as angels and item shops.
- Current traversal implementation no longer hooks `MapCreator` at all.
- `hook_installed_count=35`.
- `resolver.metadata=47`, `resolver.aob=0`, `resolver.xref=0`, `resolver.rva=0`, `resolver.fail=0`. The extra metadata resolutions include direct-call helpers for battle-skill injection/confirmation, movement extra distance, and rewarded-ad callback completion.
- `field_resolver.metadata=30`, `field_resolver.fallback=0`, `field_resolver.fail=0`, `field_offsets_ready=1`. The metadata-resolved fields include movement, traversal, bullet/weapon, `ObscuredVector3`, and rewarded-ad private callback/storage fields.
- Status confirmed all 35 startup hooks installed through metadata, including `EntityBase.AddInitSkills`, the hero traversal hooks, the movement hook, the rewarded-ad bypass hooks, the shoot-through-wall stack, and the Unity timeScale hooks.
- Final speed defaults were device-confirmed: `attack_speed_value=100.000000` and `game_speed_multiplier=4.000000`.
- Hero movement speed installation was device-confirmed with the test config set to `move_speed_multiplier=10`; the current implementation avoids re-entering `MoveControl.UpdateProgress` and uses `EntityBase.SelfMoveBy` for extra distance.
- Rewarded-ad skip was device-confirmed from the Hero Patrol "Extra Earnings" placement: tapping the placement stayed in `com.habby.archero/.UnityPlayerActivity`, and status showed `hits.ad_skip_driver=2`, `hits.ad_skip_reward=2`, and `hits.ad_skip_close=2`.
- Shoot-through-wall was device-confirmed on the live bullet collision path: `hits.weapon_wall_field_apply=1873`, `hits.runtime_walls_apply=65`, `hits.runtime_walls_init=1582`, and `hits.bullet_hitwall_internal_bypass=65`. `hits.bullet_transmit_wall_get=0` and `hits.bullet_hitwall_bypass=0` simply mean the live run used the internal `TriggerEnter1` wall handler rather than those backup paths.
- After battle entry on the Greed/Smart rebuilt APK, status showed `hits.skill_inject_greed=1`, `hits.skill_confirm_greed=1`, `hits.skill_inject_smart=1`, and `hits.skill_confirm_smart=1`, confirming both IDs were accepted by `EntityBase.ContainsSkill(int)` in the running game. `hits.skill_confirm_fail=0`.
- The water skill-list confirmation remained false (`hits.skill_confirm_water=0`), but the traversal path remained active through the previously validated direct state path: `hits.walk_skill_inject=1`, `hits.walk_runtime_apply=1`, and `hits.walk_entitydata_apply=2`.
- After battle entry and letting the battle continue, the process remained alive as pid `7850`; after clearing logcat and watching another 10 seconds, filtered logcat showed no `FATAL EXCEPTION`, native fatal signal, or tombstone for `com.habby.archero`.
- Config polling stayed cheap during the unchanged-config run: `config_loads=1` while `status_writes` continued advancing.

## Sources

- Unity IL2CPP overview: https://docs.unity.cn/Manual/IL2CPP.html
- IL2CPP runtime API reference mirror: https://deepwiki.com/dreamanlan/il2cpp_ref/3.4-method-invocation
- Il2CppDumper: https://github.com/Perfare/Il2CppDumper
- Zygisk-Il2CppDumper: https://github.com/Perfare/Zygisk-Il2CppDumper
- frida-il2cpp-bridge: https://github.com/vfsfitvnm/frida-il2cpp-bridge
- Il2CppInspectorRedux: https://github.com/LukeFZ/Il2CppInspectorRedux
