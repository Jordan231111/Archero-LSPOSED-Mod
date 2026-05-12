# Archero LSPOSED Mod

LSPosed/Xposed module for Archero (com.habby.archero) providing native IL2CPP gameplay hooks.

**Target:** Archero v7.9.1 (arm64-v8a)

## Features

- **Headshot:** Enemies are one-shotted (forced headshot on non-Hero entities)
- **Godmode:** Player is invincible (forced miss on Hero entities)
- **Always-on battle skill injection:** Water walk, Greed, and Smart are injected through metadata-resolved `EntityBase.AddSkill(int)` at battle init and confirmed with `EntityBase.ContainsSkill(int)` where the game tracks that skill ID family.
- **Shoot through walls:** Hero projectile setup forces weapon through-wall fields and `BulletTransmit.ThroughWall`; hero bullet wall-collision handlers are bypassed so walls do not despawn the projectile.
- **Speed defaults:** Attack speed is forced to `100.0`; game speed defaults to `4.0x`; hero movement speed is configurable. The movement hook calls `MoveControl.UpdateProgress` once, then applies extra hero distance through the game's `EntityBase.SelfMoveBy` path for multipliers above `1.0x`.
- **Rewarded ad bypass:** Rewarded ad load/show paths can be completed immediately through the app's own reward/close callbacks instead of opening the ad activity.
- **Traversal:** Hero-only water/wall traversal state is mirrored without hooking map generation, preserving walls, water, shops, and angels.
- **Runtime resolution:** Methods and game field offsets resolve through IL2CPP metadata and fail closed if a target cannot be identified. v7.9.1 RVAs remain in the source as dump/spec anchors only; runtime hook install does not fall back to fixed field offsets or direct RVA addresses.

Side-aware logic matches the original mod behavior: only the appropriate side receives the forced result.

The module does not force game-over server validation. Optional direct gold/drop hooks stay off by default; for an owned server fork, update server-side settlement rules to match the intended reward policy.

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
