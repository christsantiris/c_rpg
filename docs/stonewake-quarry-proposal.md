# Stonewake Quarry — art proposal

Status: approved and implemented. Stonewake replaces Glassdeep in its existing
location south of Stillbury. See [the current gameplay guide](stonewake-quarry.md)
for runtime behavior, rewards and save compatibility. These images record the
approved concept direction; runtime art uses pixelkit.

## Proposed appearance

- [Entrance landmark](art-sources/stonewake-proposal/entrance-concept.png): low
  limestone benches, timber lifting equipment, a central mine entrance and
  short rail track, with a small turquoise excavation pool.
- [Area concept](art-sources/stonewake-proposal/area-concept.png): permanent
  ramp circuits around excavation cuts, optional crane crossings, workshops
  and supply yards. The layout should offer connected detours and discoveries.
- [Enemies and quest reward](art-sources/stonewake-proposal/enemies-concept.png):
  Rockworm, Quarry Construct, Stone Troll, Iron Sentinel and Quarry Colossus,
  compared with the player at a shared scale.

The palette uses warm cream/gray limestone, weathered timber, dark iron,
turquoise pools and restrained green magical runes. These generated images are
art proposals, not runtime sprites or renderer captures. Approved production
art must use the established pixelkit pipeline, with native comparisons and
actual in-game rendering checks.

The future atlas edit must retain the beach below the region and the cliffs
behind it, plus Sunken Coast's cove and the connected Oakhaven docks. Stonewake
occupies the existing Glassdeep footprint without moving surrounding regions.

## Quest reward decision

The quarry quest rewards a **Magic Stone Warhammer**. Its stone head has carved
green runes, iron binding and a leather-wrapped wooden handle.

The user confirmed that its HP bonus increases **maximum HP while equipped**.
It does not permanently increase base HP when the quest is completed. The implemented
reward gives +8 attack and +30 maximum HP, is rare and two handed, and can be
used by all classes. Weapon
swaps must handle the health bonus consistently and must not create repeat
healing or repeat quest rewards.

## Work sequence after approval

1. Implement and complete Stonewake, including its unique generator, machinery,
   enemies, quest conversion, reward, artwork, protected atlas edit, save
   migration, tests and documentation.
2. Create **Crown Road North**, after Catacombs on the approach to Castle Grounds.
   It must be harder than Crown Roads East and West because it offers a shortcut
   from Oakhaven to the castle. Exact encounters and difficulty tuning require
   their own design step.
3. Revamp **Royal Catacombs** toward the original Oakhaven dungeon's room-and-
   corridor structure and exploration experience, at a higher difficulty.
   Retain its royal-undead identity and northern-route role.

Blackwater Swamp, Sunken Coast and Frostfell remain. Ashen Hollow retains its
volcanic theme. Stormwatch is planned east of Dawnmere. These later changes are not part of the
Stonewake art approval.
