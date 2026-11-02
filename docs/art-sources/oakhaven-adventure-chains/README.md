# Oakhaven's post-Lich adventure chains

Approved Chapter 1 step, 2026-11-01. Alder and Dain now arrive in Oakhaven's
Inn after the Lich King's defeat, independently of Elowen's reunion. Alder
leaves Stillbury's Tavern; Dain leaves Dawnmere's Alehouse, which retains an
ordinary host. Their existing character art is reused.

| NPC | First adventure | Inn reward | Optional follow-up |
| --- | --- | --- | --- |
| Alder | Free two rangers from spider captors on Forest 1 and 2. Defeat each captive's three named Spider Captors, then Talk to the ranger. | 70 gold and 500 score once | Defeat the Necromancer, then continue east to Dawnmere to investigate the evil's source. |
| Dain | Recover two treasure-map halves: an Archer Map Bearer on Mountains 1 and a Warchief Map Bearer on Mountains 2. | 60 gold and 400 score once | Defeat the Goblin King, then continue west to Stillbury to investigate the evil's source. |

Finishing the first two objectives moves that quest straight to Completed,
with its Inn reward pending. Turning it in pays once and immediately offers
the boss follow-up with Y/N controls. Follow-ups use the same three-active-quest
limit. The first part stays Completed when the next part is accepted.

Boss victory finishes the follow-up immediately, with onward directions and
no second Inn turn-in or additional NPC payment. Existing boss loot remains.
Earlier boss victories count when the first part is turned in; the player does
not have to replay a defeated boss. Both bosses and onward roads work without
accepting either quest or speaking to the NPCs. The existing forest trail
landmarks and regional boss road gates remain in effect.

Accepting either part preserves explored maps and regional depth. First-part
objectives are added when previously explored stages are revisited; ordinary
expedition respawn rules still apply. Existing saved quest states and progress
masks record both parts; save version 230 writes their new phase meanings.
As approved for V2, no new migration is added and all checks use isolated fresh
saves. The Lich's all-class [Crypt Ring](../crypt-ring/README.md) is now
implemented. Remaining starting-cast changes, Aldric recruitment and regional
balance remain separate steps.

## Native renderer review

These captures use the actual SDL game and journal renderers at 1280×720:

- [Inn before the Lich](inn-before-lich.png) and [after victory](inn-after-lich.png):
  Alder and Dain appear only after the opening boss; their name labels and
  existing character art remain distinct. Other pending cast changes are visible.
- [Forest 1 ranger and spiders](forest-ranger-captors.png): the ranger has
  three special spider captors amid the existing regional encounters.
- [Mountains 1 map bearer](mountains-map-bearer.png): the first half is carried
  by the marked Archer encounter.
- [First part Completed](alder-first-completed.png): both early-stage objectives
  are checked, the active slot is free and the Inn reward remains pending.
- [Alder's follow-up](alder-boss-offer.png) and [Dain's follow-up](dain-boss-offer.png):
  payment, boss goal, onward town and kingdom investigation are readable.
- [Active boss hunt](dain-boss-active.png) and [completed chain](dain-chain-completed.png):
  separate journal rows preserve the first part, and the boss objective completes
  without an extra return trip or a misleading zero-gold reward line.

Route fixtures deliberately use elevated player stats, reduced target HP and
controlled player turns. They establish interaction, travel and state behavior;
they do not establish normal combat balance.

## Validation

- Release game and test runner build successfully; **9,628/9,628 assertions pass**.
- Twelve native journeys cover both regions with three seeds each (7300–7302),
  with accepted chains and without any quest or Lich victory. They perform
  **22,021 actual movement/combat actions** and pass **120 route assertions**,
  in addition to native harness assertions at each objective and transition.
- Every journey walks from Oakhaven through the safe road into stage 1, follows
  the actual continuous paths through all five stages and kills the boss using
  a player combat action. Forest journeys visit the existing trail landmarks.
  They use actual boundary transitions, not merely saved stair coordinates.
- Accepted-chain journeys rescue both rangers or recover both halves on stages
  1 and 2, backtrack through real exits, walk into the Inn, claim the reward,
  accept the follow-up and re-enter the same region. After boss victory, both
  assigned and unassigned journeys walk the unlocked onward road into Dawnmere
  or Stillbury.
- Unit tests cover post-Lich arrival gating and proximity, former NPC locations,
  duplicate encounter prevention, live spider captors blocking rescue, two
  objectives only, late acceptance on explored maps, one-time rewards, declined
  follow-ups, the active limit, early boss credit and fresh save/load round trips
  during arrival, rescue, map retrieval, boss pursuit and completion.
- The regression fixture for castle-to-catacombs travel now clears the old
  regional enemy/item lists when substituting its Castle Grounds map. This
  prevents unrelated random catacomb enemies from obstructing the castle gate.
- `git diff --check` passes. Changes remain uncommitted for manual review.
