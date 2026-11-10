# Caravan Rogue validation

Validated on November 10, 2026.

Garran is recruitable at Sunscar's caravan camp before entering the wastes.
His recruitment and Jarl quest have separate conversation choices and explicit
confirmations. Recruitment is free, works with three active quests, and leaves
the player's Crown standing unchanged. His fixed stats are 90 HP, 4 defense,
14 bow attack at range 5, and 10 dagger attack. Personal arrows are unlimited.

`conr` and `test_runner` build successfully. The full suite passes
**9,220 / 9,220 checks**. The focused companion checks in
`tests/test_garran.c` pass **210 / 210** checks. They exercise:

- Recruitment, decline, held keys, independent quest acceptance and quest limits.
- Waiting and rejoining at camp and Stillbury's Tavern without healing wounds,
  including another companion stepping aside from Garran's waiting place.
  A portal occupying that place prevents dismissal, keeping Garran available
  and leaving the portal intact.
- Bow range, blocked firing lanes, allied obstruction, dagger attacks, neutral
  Crown troops, player XP, the Jarl's defeat and death during enemy turns.
- Paid and Mage resurrection selecting Garran among three fallen companions.
- Actual retreat and onward transitions through every Sunscar and Giant's Peaks
  stage across seeds 19, 107 and 1301, with Aldric and Soren following too.
  Cached revisits, the desert road, Giant's Bridge, Stillbury's western gate and
  its Tavern doorway are traversed through movement actions.
- River boarding, movement, saving aboard and docking, island voyages and the
  player's one-way town portal with all three companions.
- Save/load of membership, wounds, death, waiting home and pending dialogue.
  Version-246 fixtures preserve existing active and cached enemy wounds and
  unanswered Jarl quest offers. Invalid version-247 companion objects are rejected.

Native SDL rendering was inspected at **1280×720** and **640×480** for the camp
sprite beside existing NPCs, recruitment, all three party status rows, waiting
at Stillbury's Tavern and resurrection target selection and confirmation.
Garran uses the existing native NPC art pipeline with a hood, bow, quiver and
dagger. The compact sidebar keeps his health visible at the minimum window size.
His conversation choices also remain visible inside the healer and witch
buildings, where service hints previously replaced them.

The existing local version-240 Sunscar save was loaded and rendered successfully.
The renderer loaded a temporary copy; automated saves were also written only in
isolated temporary working directories. Local save/config files remain unchanged.
