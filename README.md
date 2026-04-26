# Archero LSPOSED Mod

LSPosed/Xposed module for Archero (com.habby.archero) providing headshot and godmode via IL2CPP hooks.

**Target:** Archero v7.9.1 (arm64-v8a)

## Features

- **Headshot:** Enemies are one-shotted (forced headshot on non-Hero entities)
- **Godmode:** Player is invincible (forced miss on Hero entities)

Side-aware logic matches the original mod behavior: only the appropriate side receives the forced result.

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

See [modmenu_hook_report.txt](modmenu_hook_report.txt) for:

- Hook addresses and side-aware logic
- How to update for future game versions
- Packaging requirements (avoid UnsatisfiedLinkError)
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
