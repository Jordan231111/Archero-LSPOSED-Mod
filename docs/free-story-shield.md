# Free Story Shield (Tier S.1 + S.2 hooks)

Target: `com.habby.archero` v7.9.1 (arm64-v8a), `libil2cpp.so`.

This document describes the LSPosed module hooks added to zero-cost the Free
Story key-consumption flow while preserving the battle-start transaction. The
implementation is the C++ translation of the findings written up in
`free_story_findings.md` (attached to the original PR discussion) and adds no
Lua- or RVA-pinned patching: each hook is resolved at runtime through IL2CPP
metadata, with the v7.9.1 RVAs only kept as anchors for verification.

## Goal

Block Free Story plays from deducting the player's `Key` counter while still
letting the server observe a battle start so stage progression can persist.
The original Lua hack patched `GameLogic.GetModeLevelKey` to return `8`,
which only changed the client-side prediction of how many Keys a play would
cost; the server still saw the play and deducted the real cost. We instead
intercept the actual flow:

```
EnterStoryBattle -> ... -> GameLogic.send_use_key ()
        |__ LocalSave.Modify_Key(-cost, false)    // predictive client-side decrement
        |__ NetManager.SendInternal(CLifeTransPacket) // tells server "I am playing"
                |__ server validates Vip.IsFreeStoryNumValid / IsCanFreeStoryToday
                |__ server replies with authoritative Key
                |__ DoLoginCallBack / Modify_Key path resyncs LocalSave.UserInfo.SetKey(value)
```

## Hooks

| Tier | Target                                   | v7.9.1 RVA   | Spec resolved at runtime via                                 | Default |
|------|------------------------------------------|--------------|---------------------------------------------------------------|---------|
| S.1a | `GameLogic.send_use_key()`               | `0x58CB67C`  | `class GameLogic` -> `method send_use_key`, 0 args            | **ON**  |
| S.1b | `NetManager.SendInternal<object>()`      | `0x6842E28`  | generic instantiation anchor                                  | **ON**  |
| S.2a | `LocalSave.Modify_Key(long, bool)`       | `0x5A494C4`  | `class LocalSave` -> `method Modify_Key`, 2 args              | scoped  |
| S.2b | `LocalSave.UserInfo.SetKey(int)`         | `0x5B1E9E8`  | `class LocalSave.UserInfo` -> `method SetKey`, 1 arg          | OFF     |

### S.1 — `hk_send_use_key`

When `free_story=1`, calls the original `send_use_key()` under a scoped flag
instead of returning early. The companion `NetManager.SendInternal<object>`
hook sees the `CLifeTransPacket` created by that scoped call and rewrites:

```
m_nMaterial: cost -> 0
```

The same scoped flag blocks the local predictive `Modify_Key(-cost, false)`
inside `send_use_key`, so the displayed Key value does not drop before the
server replies.

The earlier return-early strategy kept Key accounting clean but also prevented
the server from seeing a battle-start life transaction. That allowed the local
battle UI to advance temporarily, then login restored the old server stage.

When `free_story=0`, the hook hits its passthrough path
(`g_orig_send_use_key`) and the game behaves stock.

### S.2a — `hk_localsave_modify_key`

Scoped core behavior plus defense in depth. With `free_story=1`, drops the
negative delta made inside the active `send_use_key` call. When
`free_story_skip_predictive=1`, also drops negative deltas outside that scoped
path. Positive Key gains (ad refills, shop grants, login bootstrap) still pass.

### S.2b — `hk_userinfo_set_key`

Defense in depth. Only used when `free_story_freeze_key=1`. Reads the current
backing field (`<Key>k__BackingField` at offset `0xB0` per the v7.9.1 dump,
resolved at runtime through metadata so it survives game updates) and refuses
the write if the new value is *less* than the current value. Increases still
pass through. The hook falls back to passthrough if the field offset failed
to resolve, so a metadata regression cannot accidentally permanently freeze
keys.

This is intentionally OFF by default. If the zero-cost transaction is accepted,
there should be no server deduction to resync from, so freezing writes would
only hide legitimate adjustments.

## Config keys

Authoritative path: `/storage/emulated/0/Android/data/com.habby.archero/files/archero_mod_config.txt`

```
free_story=1
free_story_freeze_key=0
free_story_skip_predictive=0
```

The status file (`archero_mod_status.txt`) reports the same flags, focused hit
counters, the life-packet fields, and the resolved Key field offset:

```
free_story=1
free_story_freeze_key=0
free_story_skip_predictive=0
field_offsets.localsave_userinfo_key=0xb0
hits.free_story_send_original=N
hits.free_story_life_packet_zeroed=N
hits.free_story_modify_blocked=N
hits.free_story_set_key_blocked=N
hits.free_story_passthrough=N
last.free_story_life_material_before=N
last.free_story_life_material_after=0
```

The module also appends a compact trace to
`/storage/emulated/0/Android/data/com.habby.archero/files/archero_mod_trace.txt`.
For Free Story persistence, the important sequence is:

```
send_use_key original_scoped
modify_key blocked delta=-5 over=1 scoped=1
life_start ... material=5->0 battleTrans=<id> chap=<chapter>
creq_item source=send3 type=23 trans=<same id> extra=<new max>
creq_item source=gameover_add type=23 trans=<same id>
creq_item source=gameover_remove type=23 trans=<same id>
stage_init_max current=<next chapter> max_before=<new max> value=<new max>
```

`gameover_remove` is the cache-clear path after the game-over request succeeds;
on the next app launch, `stage_init_max` should reload the same advanced
chapter from server state.

## Verification recipe

After installing the module and starting a Free Story run from a save with a
known Key count `K`:

1. Confirm `hits.free_story_send_original` increments by 1 per play attempt.
2. Confirm `hits.free_story_life_packet_zeroed` increments and
   `last.free_story_life_material_after=0`.
3. Confirm the displayed Key counter remains at `K`.
4. Finish the stage, force-close/relaunch, and confirm server-restored stage
   progress remains advanced.
5. Disable the hook (`free_story=0`), restart Archero, and verify the counter
   decrements — i.e. only the hook changes behavior.

Live emulator verification on the PR branch reached five consecutive zero-cost
Free Story clears with `free_story_freeze_key=0` and
`free_story_skip_predictive=0`. Fresh launches loaded Hero 104, 106, 107, and
108 from server state after the corresponding clears, confirming the server
accepted the zero-material battle starts and the game-over packets.

## Why not patch `GetModeLevelKey`?

`GetModeLevelKey` only feeds the *displayed cost* in some menus and is
consulted by `Modify_Key` only as an early-out hint. Patching it to return
`8` does nothing to suppress the network request — the server still sees the
play and decrements server-side. The original Lua hack therefore degrades to
an ad-hoc free-play "saving" via the client display only, and the next login
resync exposes the discrepancy.

`send_use_key` is the actual choke point.
