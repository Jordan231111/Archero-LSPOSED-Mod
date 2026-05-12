# Free Story Shield (Tier S.1 + S.2 hooks)

Target: `com.habby.archero` v7.9.1 (arm64-v8a), `libil2cpp.so`.

This document describes the LSPosed module hooks added to silence the Free
Story key-consumption flow. The implementation is the C++ translation of the
findings written up in `free_story_findings.md` (attached to the original PR
discussion) and adds no Lua- or RVA-pinned patching: each hook is resolved at
runtime through IL2CPP metadata, with the v7.9.1 RVAs only kept as anchors
for verification.

## Goal

Block Free Story plays from deducting the player's `Key` counter without
producing any visible fingerprint relative to a no-op. The original Lua hack
patched `GameLogic.GetModeLevelKey` to return `8`, which only changed the
client-side prediction of how many Keys a play would cost; the server still
saw the play and deducted the real cost. We instead intercept the actual
flow:

```
EnterStoryBattle -> ... -> GameLogic.send_use_key ()
        |__ LocalSave.Modify_Key(-cost, false)    // predictive client-side decrement
        |__ DispatchServerRequest(useKeyReq)      // tells server "I am playing"
                |__ server validates Vip.IsFreeStoryNumValid / IsCanFreeStoryToday
                |__ server replies with authoritative Key
                |__ DoLoginCallBack / Modify_Key path resyncs LocalSave.UserInfo.SetKey(value)
```

## Hooks

| Tier | Target                                   | v7.9.1 RVA   | Spec resolved at runtime via                                 | Default |
|------|------------------------------------------|--------------|---------------------------------------------------------------|---------|
| S.1  | `GameLogic.send_use_key()`               | `0x58CB67C`  | `class GameLogic` -> `method send_use_key`, 0 args            | **ON**  |
| S.2a | `LocalSave.Modify_Key(long, bool)`       | `0x5A494C4`  | `class LocalSave` -> `method Modify_Key`, 2 args              | OFF     |
| S.2b | `LocalSave.UserInfo.SetKey(int)`         | `0x5B1E9E8`  | `class LocalSave.UserInfo` -> `method SetKey`, 1 arg          | OFF     |

### S.1 — `hk_send_use_key`

Returns immediately when `free_story=1`. This skips:

1. The local predictive `Modify_Key(-cost, false)` call inside `send_use_key`,
   so the displayed Key value does not change.
2. The outbound `DispatchServerRequest` packet, so the server never observes a
   play in the first place and has nothing to deduct.

Because both halves are skipped, the local view stays in lockstep with the
server view: this is what makes the hook fingerprint-clean. There is no
discrepancy for the server's `Vip.IsFreeStoryNumValid` resync to push back
into the client.

When `free_story=0`, the hook hits its passthrough path
(`g_orig_send_use_key`) and the game behaves stock.

### S.2a — `hk_localsave_modify_key`

Defense in depth. Only used when `free_story_skip_predictive=1`. Drops only
negative deltas, so positive Key gains (ad refills, shop grants, login
bootstrap) still apply. Useful only if a future code path other than
`send_use_key` starts predictively decrementing Keys; otherwise S.1 already
handles the predictive decrement because it lives inside `send_use_key`.

### S.2b — `hk_userinfo_set_key`

Defense in depth. Only used when `free_story_freeze_key=1`. Reads the current
backing field (`<Key>k__BackingField` at offset `0xB0` per the v7.9.1 dump,
resolved at runtime through metadata so it survives game updates) and refuses
the write if the new value is *less* than the current value. Increases still
pass through. The hook falls back to passthrough if the field offset failed
to resolve, so a metadata regression cannot accidentally permanently freeze
keys.

This is intentionally OFF by default — with S.1 enabled there is no server
deduction to resync from, so freezing writes would only hide legitimate
adjustments.

## Config keys

Authoritative path: `/storage/emulated/0/Android/data/com.habby.archero/files/archero_mod_config.txt`

```
free_story=1
free_story_freeze_key=0
free_story_skip_predictive=0
```

The status file (`archero_mod_status.txt`) reports the same flags plus four
hit counters and the resolved Key field offset:

```
free_story=1
free_story_freeze_key=0
free_story_skip_predictive=0
field_offsets.localsave_userinfo_key=0xb0
hits.free_story_send_blocked=N
hits.free_story_modify_blocked=N
hits.free_story_set_key_blocked=N
hits.free_story_passthrough=N
```

## Verification recipe

After installing the module and starting a Free Story run from a save with a
known Key count `K`:

1. Confirm `hits.free_story_send_blocked` increments by 1 per play attempt.
2. Confirm the displayed Key counter remains at `K`.
3. Confirm no `useKey` request appears in the device proxy log.
4. Disable the hook (`free_story=0`), restart Archero, and verify the counter
   decrements and the request appears — i.e. only the hook changes behavior.

## Why not patch `GetModeLevelKey`?

`GetModeLevelKey` only feeds the *displayed cost* in some menus and is
consulted by `Modify_Key` only as an early-out hint. Patching it to return
`8` does nothing to suppress the network request — the server still sees the
play and decrements server-side. The original Lua hack therefore degrades to
an ad-hoc free-play "saving" via the client display only, and the next login
resync exposes the discrepancy.

`send_use_key` is the actual choke point.
