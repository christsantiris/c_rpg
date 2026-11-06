# Stillbury coastal quest

Mara introduces the Ruined Isle from Oakhaven's Inn. She is present from the
start, directs the player to Captain Rowan south of Stillbury, and neither
assigns nor rewards the beacon quest.

Rowan stands on the north bank at Stillbury's river landing, beside the existing
Saltwind skiff dock. He offers **Relight the Drowned Beacons** with Y/N and the
normal three-active-quest limit. The journal names Rowan and identifies this
landing as the reward destination.

The guarded beacons on Sunken Coast 2, 3 and 4 can be discovered and lit before
acceptance. Lower their blue channels and interact with the lights. Accepting
preserves earlier discoveries and Coast caches rather than restarting the
expedition. All three lit beacons move the quest to **Completed**, freeing its
slot before Rowan pays the existing **80 gold and 600 score**, once.

The Drowned Queen is a separate objective. Her defeat opens island passage from
Stillbury without a chart, spare inventory capacity, beacon completion or quest
acceptance. The shore and Rowan remain accessible before victory. Island return
voyages land at these same docks; the existing river skiff, exploration and
dropped items survive sailing. The player can walk north into Stillbury.

All three beacons plus the Queen's defeat restore coastal trade. Stillbury's
butcher and cobbler greet the player more warmly and acknowledge restored
supplies. Known anti-Crown actions and royal checkpoint behavior remain intact.

## Save compatibility

Save version **244** preserves the serialized `mara_quest_state` and
`mara_beacons_lit` fields, including pending and already collected rewards.
No new persistent fields are needed. Older saves gain Mara's introduction,
remove the former coastal dock, and safely accommodate the new river dock.
Existing Coast caches, enemy wounds, exploration, loot, boss victories and
unlocked island voyages remain intact. Local saves are backed up before review;
tests use isolated directories and reserved slots.

## Validation

Debug game and test builds succeed. The gameplay suite passes **8,712 / 8,712**
checks. Native coastal review and the updated Rowan renderer fixtures pass at
both supported review resolutions. Original local saves remain byte-for-byte
unchanged.

The gameplay suite and native renderer review cover assignment and refusal,
the active quest limit, discoveries before acceptance, unchanged caches,
completed and rewarded saves, one-time payment, independent Queen gating,
round-trip sailing, shore loot and existing-save repairs. Coast travel is tested
with real movement across all five stages at both tide states, including
retreats and cached revisits, using seeds 19, 107 and 1301.

Native SDL software rendering is reviewed at 1280×720 and 640×480 for Mara,
Rowan's offer, reward dialogue, the dock label, and the island return. A local
existing save and affected legacy fixtures are loaded and rendered. README's
canonical atlas shows Stillbury Docks, the white dashed island voyage and the
unchanged yellow Saltwind skiff crossing. The built-in imagegen prompt is in
[coastal-quest-map-prompt.md](coastal-quest-map-prompt.md).

The older all-purpose debug harness has an unrelated obsolete
`NORTH_FOREST_BARRIER_Y` reference in its geography scenario; the updated Rowan
renderer cases were compiled and executed separately.

Elderwood now uses Ranger Tamsin's boss-only **Clear Elderwood** quest; the
Heartseed mission is retired. Further Malakar evidence and the new island
amulet remain separate changes.
