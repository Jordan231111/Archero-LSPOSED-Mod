# LSPatch Non-Root Split APK Build Guide

This guide documents the repeatable non-root packaging flow for Archero split APK/XAPK releases.

The important rule is signing consistency:

- Every APK in one split install set must be signed by the same certificate.
- Every future update installed over that set must also be signed by that same certificate.
- If the certificate changes, Android will reject the update with `INSTALL_FAILED_UPDATE_INCOMPATIBLE`; uninstalling the app is then required.

The current non-root build uses the default keystore bundled in `tools/lspatch/lspatch.jar`.

Current signing certificate:

```text
Signer #1 certificate SHA-256:
c081890cf2a1adf13e56d7b50a4f3d8edb35b7c46d6ccc732dd997b7e433be1d
```

Current rebuilt 7.9.1 output:

```text
/Users/jordan/Documents/temp/archerodecompiled/dist/lspatch_archero_7.9.1_xapk/archero_7.9.1_lspatched_full.apks
SHA-256: 6482d9a238e44d84541e42b74205e90a5a2d3b6c0a0b29252f4b4d9474730961
Embedded module: assets/lspatch/modules/com.archero.mod.apk, 281098 bytes
Split signing digest: c081890cf2a1adf13e56d7b50a4f3d8edb35b7c46d6ccc732dd997b7e433be1d
```

The full `.apks` archive and Unity asset splits are intentionally local build artifacts. They exceed normal GitHub per-file limits, so `dist/` stays ignored and the archive should be rebuilt from this guide when needed.

## Inputs

Expected local layout:

```text
tools/lspatch/lspatch.jar
archero_mod/app/build/outputs/apk/debug/app-debug.apk
```

For each game update, use a complete XAPK/APKS source that contains the base APK and every required split. For Archero 7.9.1 the complete APKPure XAPK contained:

```text
com.habby.archero.apk
UnityDataAssetPack.apk
UnityStreamingAssetsPack.apk
config.arm64_v8a.apk
manifest.json
```

Missing the Unity asset-pack splits can make the game hang on startup or fail to load.

## Build The Module

```bash
cd /Users/jordan/Documents/temp/archerodecompiled/archero_mod
./gradlew assembleDebug
cd /Users/jordan/Documents/temp/archerodecompiled
```

Verify the module native library is stored uncompressed:

```bash
unzip -lv archero_mod/app/build/outputs/apk/debug/app-debug.apk | grep libarchero_mod
```

The `lib/arm64-v8a/libarchero_mod.so` entry must show `Stored`, not `Defl:N`.

## Prepare Variables

Update `VERSION` and `XAPK` for each release:

```bash
cd /Users/jordan/Documents/temp/archerodecompiled

export VERSION="7.9.1"
export XAPK="/Users/jordan/Downloads/Archero_7.9.1_APKPure.xapk"
export OUT="dist/lspatch_archero_${VERSION}_xapk"
export LSPATCH="tools/lspatch/lspatch.jar"
export MODULE_APK="archero_mod/app/build/outputs/apk/debug/app-debug.apk"
export APKSIGNER="$HOME/Library/Android/sdk/build-tools/36.0.0-rc4/apksigner"
export PATCH_KEY="private-signing/lspatch-default.jks"
```

Check the XAPK contents before patching:

```bash
unzip -l "$XAPK" | grep -E '\.apk$|manifest\.json'
```

## Extract The XAPK

```bash
rm -rf "$OUT"
mkdir -p "$OUT/source" "$OUT/patched" "$OUT/apks"

unzip -j "$XAPK" '*.apk' 'manifest.json' -d "$OUT/source"
```

Confirm the source splits:

```bash
find "$OUT/source" -maxdepth 1 -type f -name '*.apk' -print -exec ls -lh {} \;
```

## Prepare The Signing Key

Use the same key for all future non-root updates. Create this key file once, then keep reusing it.

```bash
mkdir -p private-signing

if [ ! -f "$PATCH_KEY" ]; then
  unzip -p "$LSPATCH" assets/keystore > "$PATCH_KEY"
  chmod 600 "$PATCH_KEY"
fi
```

The default credentials are:

```text
keystore password: 123456
alias: key0
key password: 123456
```

Verify it is the expected key:

```bash
keytool -list -v -keystore "$PATCH_KEY" -storepass 123456 | grep -A1 SHA256
```

If you update or replace `tools/lspatch/lspatch.jar`, do not overwrite this saved key. The installed app updates cleanly because this key stays stable, not because the jar stays the same.

If you ever switch to a custom keystore, you must use it for LSPatch and for every split. Existing installs signed with the default LSPatch key will not update over a custom-key build.

## Patch Only The Base APK

For APKPure Archero XAPKs, the base file is usually `com.habby.archero.apk`.

```bash
java -jar "$LSPATCH" \
  "$OUT/source/com.habby.archero.apk" \
  -m "$MODULE_APK" \
  -o "$OUT/patched" \
  -l 2 \
  -k "$PATCH_KEY" 123456 key0 123456 \
  -f \
  -v

cp "$OUT/patched"/*-lspatched.apk "$OUT/apks/base.apk"
```

Use `-l 2` for LSPatch signature bypass. Do not use `-r` for normal update builds, because update compatibility should follow the real game versionCode.

## Sign Every Split With The Same Key

Do not run LSPatch over the Unity asset splits. Keep their payloads intact and only re-sign them to match the patched base.

```bash
for apk in "$OUT/source"/*.apk; do
  name="$(basename "$apk")"
  if [ "$name" = "com.habby.archero.apk" ]; then
    continue
  fi

  "$APKSIGNER" sign \
    --v4-signing-enabled false \
    --ks "$PATCH_KEY" \
    --ks-pass pass:123456 \
    --ks-key-alias key0 \
    --key-pass pass:123456 \
    --out "$OUT/apks/$name" \
    "$apk"
done
```

Expected `apks/` output for Archero 7.9.1:

```text
base.apk
config.arm64_v8a.apk
UnityDataAssetPack.apk
UnityStreamingAssetsPack.apk
```

## Verify The Build

Check LSPatch config:

```bash
unzip -p "$OUT/apks/base.apk" assets/lspatch/config.json
```

Confirm:

```text
"useManager":false
"sigBypassLevel":2
```

Check the embedded module:

```bash
unzip -l "$OUT/apks/base.apk" | grep 'assets/lspatch/modules/com.archero.mod.apk'
```

Verify every split:

```bash
for apk in "$OUT/apks"/*.apk; do
  echo "== $apk =="
  "$APKSIGNER" verify --verbose --print-certs "$apk" | sed -n '1,16p'
done
```

All files must print `Verifies`, and every `Signer #1 certificate SHA-256 digest` must match:

```text
c081890cf2a1adf13e56d7b50a4f3d8edb35b7c46d6ccc732dd997b7e433be1d
```

If the certificate digest differs on any split, do not install that set.

## Package A Single `.apks` Archive

```bash
cd "$OUT/apks"
zip -0 -q -r "../archero_${VERSION}_lspatched_full.apks" .
cd /Users/jordan/Documents/temp/archerodecompiled
```

`zip -0` stores the APKs without recompressing them, which is faster and avoids wasting space on already-compressed APK payloads.

## Install Or Update

For a first install, uninstall any Play-signed/original Archero first:

```bash
adb uninstall com.habby.archero
```

Install the full split set:

```bash
adb install-multiple \
  "$OUT/apks/base.apk" \
  "$OUT/apks/config.arm64_v8a.apk" \
  "$OUT/apks/UnityDataAssetPack.apk" \
  "$OUT/apks/UnityStreamingAssetsPack.apk"
```

For future updates signed with the same key:

```bash
adb install-multiple -r \
  "$OUT/apks/base.apk" \
  "$OUT/apks/config.arm64_v8a.apk" \
  "$OUT/apks/UnityDataAssetPack.apk" \
  "$OUT/apks/UnityStreamingAssetsPack.apk"
```

You can also install the `.apks` archive with a split APK installer such as SAI, as long as it installs all APKs in the archive together.

## Troubleshooting

`INSTALL_FAILED_UPDATE_INCOMPATIBLE` means the installed app is signed with a different certificate. Either rebuild with the original non-root key or uninstall the existing app before installing.

`INSTALL_FAILED_VERSION_DOWNGRADE` means the source versionCode is older than the installed package. Use a newer source XAPK or uninstall first. Avoid LSPatch `-r` for normal update builds.

`INSTALL_FAILED_INVALID_APK` or version-code inconsistency usually means the split APKs came from different releases. Re-extract every split from the same XAPK.

Game starts but hangs before loading usually means required Unity asset splits are missing. Verify the XAPK `manifest.json` `split_apks` list and make sure each listed APK is present in `$OUT/apks`.

Game starts but the module does not work usually means the game internals changed. Re-check IL2CPP metadata class/method/field names and update the module before rebuilding the LSPatch package. The current module fails closed when metadata cannot resolve a target; it does not use fixed field-offset or direct-RVA runtime fallbacks.
