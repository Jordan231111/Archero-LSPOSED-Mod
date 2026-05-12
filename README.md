# Archero LSPOSED Mod

LSPosed/Xposed module for Archero (com.habby.archero) providing native IL2CPP gameplay hooks.

**Target:** Archero v7.9.1 (arm64-v8a)

## Features

- **Headshot:** Enemies are one-shotted (forced headshot on non-Hero entities)
- **Godmode:** Player is invincible (forced miss on Hero entities)
- **Always-on battle skill injection:** Water walk, Greed, and Smart are injected through metadata-resolved `EntityBase.AddSkill(int)` at battle init and confirmed with `EntityBase.ContainsSkill(int)` where the game tracks that skill ID family.
- **Shoot through walls:** Hero projectile setup forces weapon through-wall fields and `BulletTransmit.ThroughWall`; hero bullet wall-collision handlers are bypassed so walls do not despawn the projectile.
- **Speed defaults:** Attack speed is forced to `100.0`; game speed defaults to `4.0x`; hero movement speed is configurable through `MoveControl.UpdateProgress` substeps.
- **Traversal:** Hero-only water/wall traversal state is mirrored without hooking map generation, preserving walls, water, shops, and angels.
- **Runtime resolution:** Methods and field offsets resolve by IL2CPP metadata first, then fall back to AOB/xref/RVA or validated offset constants where available.

Side-aware logic matches the original mod behavior: only the appropriate side receives the forced result.

The module does not forge or fake game-over server validation results. The server remains the authority for settlement. Optional direct gold/drop hooks stay off by default; for an owned server fork, update server-side settlement rules to match the intended reward policy.

The default-ON `force_server_validation` switch installs pass-through hooks on the client's settlement-validation entry points (`BattleModuleData.BuildCheatData`, `HTTPSendClient.CheckGameOverCheat`, `LocalSave.BattleIn_DropEquipByServer` / `BattleIn_DropEquipDataByTransId`, `GameOverModeCtrlBase.CheckDropEquipsByServer`). The hooks only call the original and bump per-method counters so a server operator can confirm in `archero_mod_status.txt` that each validation path actually fired for a given battle. The status file also reports `hooks.validate_*` install flags for each metadata-resolved hook. They never modify return values or fabricate server responses.

When `force_server_validation=1`, the module additionally:

- caches the live `BattleModuleData*` observed by `BattleModuleData.AddGold` during the run and re-invokes the unmodified `BattleModuleData.BuildCheatData` on it at settlement (inside `LocalSave.BattleIn_UpdateGold`) so the cheat-data payload is rebuilt against the final coherent battle state right before each gameover packet is sent;
- `dump_netcacheone=1` by default walks the live `NetCacheOne` argument of every `HTTPSendClient.CheckGameOverCheat` call with the IL2CPP runtime API and writes the class chain + every field (name, type, byte offset, primitive value) to `archero_mod_netcacheone.txt`, so the server operator can see the exact wire layout sent for a legitimate run;
- `replay_netcacheone=1` by default immediately re-invokes the original `CheckGameOverCheat(HTTPSendClient*, NetCacheOne*)` with the same live arguments the game just produced, sending two identical legitimate settlement packets back-to-back so the server's idempotency / replay-rejection behavior on a real `transid` becomes observable.

None of these steps fabricate IL2CPP objects, alter return values, or change client-side numbers — they only re-invoke real client methods with real game state so the server-validation flow is reproducible end-to-end. Numbers reported to the server stay consistent with what the server computed, so a clean default-config run still validates as a *successful* run on a correctly-configured server.

## Requirements

- LSPosed (or compatible Xposed framework)
- Archero `com.habby.archero`
- arm64-v8a device/emulator

## Build

```bash
cd archero_mod
./gradlew assembleDebug
```

APK output: `archero_mod/app/build/outputs/apk/debug/app-debug.apk`

**Important:** Verify native lib is stored uncompressed:

```bash
unzip -l app/build/outputs/apk/debug/app-debug.apk | grep libarchero_mod
# Must show "Stored", not "Defl:N"
```

## Install

1. Install the APK
2. Enable the module in LSPosed
3. Scope to `com.habby.archero`
4. Force-stop Archero and relaunch

## Documentation

See [IL2CPP-runtime-resolution.md](IL2CPP-runtime-resolution.md), [Archero-7.9.1-hook-gold-report.md](Archero-7.9.1-hook-gold-report.md), and [LSPATCH-NONROOT-BUILD.md](LSPATCH-NONROOT-BUILD.md) for:

- Hook addresses and side-aware logic
- How to update for future game versions
- Packaging requirements (avoid UnsatisfiedLinkError)
- Non-root LSPatch split APK packaging and update-safe signing
- Verification commands

## License

Except where otherwise noted, this repository's original content is licensed
under `CC BY-ND 4.0`. See [LICENSE](LICENSE).

Bundled third-party source files keep their own notices; for example,
`And64InlineHook` remains under MIT in its source headers.

`CC BY-ND 4.0` does not permit redistribution of adapted versions of the
licensed material.

This project does not grant rights to Archero itself or to any Habby-owned
assets, code, trademarks, or content.
