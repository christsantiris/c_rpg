# Boss encounter upgrades — proposal for approval

Status: **Approved. Round 1 begins with the Lich King; pause after each boss for review and a manual commit.**

The Lich King, Forest Necromancer, Mountain Goblin King, Drowned Queen,
Swamp Demon, Minotaur, Grave Marshal, Desert Pharaoh, Thorn Regent, Cinder
Lord, Prism Sovereign and Fallen Sun Guardian implementations are complete.
The other 4 bosses remain pending. Pause here for review and a manual commit
before the Red Dragon.

The boss journal defines 17 current encounters. This proposal covers the 16
other than the recently updated Polar Kraken. The Kraken's fixed warnings,
recovery openings, half-health escalation, lake hazards and persistent
Krakenbone Bow provide the reference standard.

## Shared encounter rules

1. Early and middle bosses have two health phases: above and at/below 50%.
   The Sun Guardian, Red Dragon and Lord Veyr have three: above two-thirds,
   between two-thirds and one-third, and at/below one-third. One continuous
   health bar; no extra health bars or forced invulnerability transitions.
2. Each phase changes an attack pattern or arena decision. Initially retain
   current health and base damage; tune after testing the new patterns.
3. A signature attack locks its affected tiles when announced. Show those
   tiles, attack name and remaining player turns. Small attacks allow at least
   one complete player action; wider attacks allow two or three, enough to
   escape with ordinary movement. Damage uses the same footprint as the warning.
4. Pending attacks finish with their original shape even if damage crosses a
   phase threshold. Announce the new phase before its next attack. Killing a
   boss before a later phase remains valid.
5. Signature attacks have at least one recovery turn for a safe counterattack.
   No extra melee retaliation during recovery. A successfully used arena
   counter may extend recovery to two turns. Individual designs below specify
   the available counter.
6. Arena interactions are available to every class through Interact, ordinary
   movement or normal attacks. No required spell, ammunition, quest acceptance
   or consumable. Optional counters make a fight easier; they never make the
   boss impossible to damage if ignored or exhausted.
7. Hazard combinations must leave a reachable safe route. No permanent floor
   saturation, unavoidable overlapping attacks, instant hazards under the
   player, blocked exits, or newly destroyed quest/return routes. Persistent
   damage patches expire; their durations are shown.
8. Summons are finite, announced and start acting on the following turn.
   Summoned/re-raised enemies cannot supply repeat gold, items or experience.
   Regular enemy drops remain gold-only. Existing special potion rewards stay.
9. Use existing turn-advancing actions. Opening inventory/journal and other
   non-turn UI actions do not advance warnings. Freeze pauses the queued attack
   and its pending hazard preparation without changing locked targets; already
   active floor hazards keep their displayed clocks. Retain the Kraken's
   existing separate freeze rules. No new blanket boss immunity.
10. Bosses stay in their arenas. Preserve existing travel and retreat rules,
    including castle regrouping. Ranged engagement and line-of-sight rules must
    prevent attacks from safe corridors without introducing invisible walls.

Warnings need readable outlines/symbols and countdowns, not color alone. The
recent boss artwork remains the base; use pose, weapon/core highlights and
phase messages to communicate escalation.

## Batch 1 — dungeon and early regional bosses

### 1. Lich King — Dungeon floor 5

- **Phase 1, Crypt Sovereign:** alternate a one-turn, fixed necrotic lance lane
  with a recovery opening. Keep the Lich stationed inside its sealed chamber.
- **Phase 2, Unbound Soul:** add a two-turn soul-ring burst. Warn the exact ring;
  its center and outer floor remain safe if reachable. Announce one finite
  skeleton wave during the transition, with safe approach space.
- **Arena mechanic:** two accessible burial seals sustain partial protection
  against the soul burst. Interact to silence each; silencing both also creates
  a two-turn recovery opening. Existing stone cover blocks the lance. Ignoring
  the seals remains viable and never creates complete damage immunity.
- **Guaranteed reward:** preserve **Cryptblade**, its current stats and class
  restriction, plus existing gold/progression. Add persistent unclaimed recovery.
- **Purpose:** establish fixed targeting, cover, an optional interaction and a
  clear punish window in the first dungeon boss encounter.

### 2. Necromancer — Forest stage 4

- **Phase 1, Grove Binder:** warned spirit-bolt lanes alternate with marked
  grave-raising preparations. Retain the existing resurrection identity.
- **Phase 2, Withered Grove:** add two-turn crossing root lanes and a second,
  finite resurrection wave. Roots and summons never resolve together in a way
  that blocks every escape.
- **Arena mechanic:** three grave patches can be consecrated with Interact,
  preventing that patch from raising an enemy. Consecrating a currently warned
  grave interrupts the summon and exposes the Necromancer for two turns.
- **Guaranteed reward:** preserve **Necromancer's Cloak** and existing shortcut
  discovery. Record summon use and consecrations so revisits do not refill them.

### 3. Goblin King — Goblin Mountains stage 4

- **Phase 1, Fortress Chief:** a one-turn thrown-axe lane, then recovery.
- **Phase 2, Powder King:** alternate axe throws with a two-turn bombardment of
  marked floor pockets. Show every landing site; bombs do not follow the player.
- **Arena mechanic:** a small, finite set of powder kegs can be primed with
  Interact or struck by the King's axe. They display a fuse and blast footprint;
  a blast catching the King staggers him for two turns and removes nearby
  bombardment hazards. The player can also be hit by the warned keg explosion.
- **Guaranteed reward:** preserve **Goblin King's Shield** and mountain shortcut.
- **Purpose:** a trap-setting encounter, distinct from the charging Minotaur
  and Castellan.

### 4. Drowned Queen — Sunken Coast stage 5

- **Phase 1, Rising Tide:** two-turn tidal waves cross one fixed row or column.
- **Phase 2, Storm Throne:** alternate perpendicular waves and marked flooded
  floor pockets. Each follow-up receives its own warning; no surprise retarget.
- **Arena mechanic:** two tide controls drain different lanes and expose dry
  approach paths. Raised stone islands remain usable shelter. Flood changes
  are announced, cannot strand the player and never cover all escape routes.
- **Guaranteed reward:** preserve **Tidecaller Robes** and existing coast exit
  and island access. The encounter's controls do not change earlier-stage quest
  beacon progress.

## Batch 2 — melee movement and existing arena systems

### 5. Swamp Demon — Blackwater Swamp stage 4

- **Phase 1, Bog Tyrant:** a one-turn frontal cleave and a two-turn aimed rot
  pool. The pool persists for two turns, with visible expiry.
- **Phase 2, Unshackled Demon:** add a two-turn miasma fan; attacks can leave a
  second temporary rot pocket, within a strict coverage limit.
- **Arena mechanic:** three root vents can be opened with Interact to drain
  nearby rot and briefly expose a flank. They recover on a visible cooldown;
  a vent is optional, not a supply of free healing or immunity.
- **Guaranteed reward:** preserve **Demonic Sword** and swamp shortcut.
- **Purpose:** pressure the player's positioning and retreat space without
  relying on unannounced poison damage or a larger health bar.

### 6. Minotaur — Labyrinth floor 5

- **Phase 1, Vault Keeper:** a fixed charge lane with one-turn warning, or a
  short frontal axe sweep when close. Recovery follows either attack.
- **Phase 2, Bloodied Bull:** two charges separated by a fresh warning, followed
  by a two-turn sweeping axe attack. The player can react between charges.
- **Arena mechanic:** bait a charge into a reinforced pillar or wall to gain
  two turns of stagger. A limited number of designated pillar caps can break
  after a visible crack warning, opening flanking routes. Ordinary maze walls,
  stairs, gate, relic and five-rune quest remain intact.
- **Guaranteed reward:** **Labyrinth Aegis**, a named, bull-and-rune version of
  the current Magic Shield with the same stats/class rules. Existing Magic
  Shields are not globally renamed; future boss victories award the new variant.

### 7. Grave Marshal — Royal Catacombs floor 5

- **Phase 1, Honored Commander:** retain the fixed three-tile polearm sweep,
  approach movement and recovery.
- **Phase 2, Funeral March:** add a two-turn cross-shaped sweep and one announced
  honor-guard wave. Guards appear at marked tomb niches, not on escape tiles.
- **Arena mechanic:** preserve both existing braziers and their 50%/25%/0%
  protection. Extinguishing a brazier during a new marked funeral-channel
  preparation interrupts it and grants a two-turn opening. Extinguished
  braziers stay extinguished; the boss never relights them.
- **Guaranteed reward:** preserve **Gravekeeper's Mantle** and its existing
  unclaimed restoration. Preserve memorial quest and ledger progress.

## Batch 3 — elemental regional bosses

### 8. Desert Pharaoh — Sunscar Wastes stage 5

- **Phase 1, Sun-Crowned:** a one-turn curse beam and two-turn burial-sand patches.
- **Phase 2, Sandbound:** add a two-turn moving storm band that changes position
  between attacks. Its complete next footprint is warned before it moves.
- **Arena mechanic:** four sandstone obelisks provide visible sheltered shadow
  tiles against the storm and block curse beams. Shelter moves with the warned
  storm direction. At least one shadow or clear route is reachable on foot;
  the ambient visual sandstorm remains separate from damaging tiles.
- **Guaranteed reward:** preserve **Sandstorm Staff**, including its existing
  unclaimed persistence. No staff stat or class changes proposed.

### 9. Thorn Regent — Moonveil Gardens stage 5

- **Phase 1, Garden Regent:** one-turn thorn lances and two-turn creeping roots.
- **Phase 2, Briar Crown:** temporary thorn walls grow along warned strips,
  changing flanking routes. They do not grow on occupants or seal the arena.
- **Arena mechanic:** flowering nodes can be tended with Interact to clear an
  adjacent thorn strip and open a route toward the Regent. A visible cooldown
  prevents continuous clearing; the boss has a recovery opening after regrowth.
- **Guaranteed reward:** new **Briarweave Cloak**, plus the existing special
  **Potion of Strength**. Proposed cloak: all-class light armor, +4 defense and
  10% evasion, emphasizing mobility rather than raw protection.
- **Boundary:** arena nodes are independent of Liora's Moonseed quest objects.

### 10. Cinder Lord — Ashen Hollow stage 5

- **Phase 1, Furnace Heart:** one-turn cinder lances and two-turn lava eruptions.
- **Phase 2, Broken Crucible:** alternate eruptions with a two-turn expanding
  ring of embers. Temporary hot tiles expire and leave an escape path.
- **Arena mechanic:** forge sluices can be opened with Interact to redirect
  warned lava into cooling channels. A correctly redirected eruption exposes
  the Lord for two turns. Channels and sluices have visible cooldowns; no
  permanent lava accumulation or required quest completion.
- **Guaranteed reward:** new **Embercleaver**, plus the existing special
  **Potion of Strength**. Proposed Warrior two-handed axe: +11 attack, 25%
  armor penetration and 25% cleave. Its tradeoffs leave the Blacksmith's capstone
  weapons relevant; no new elemental-damage subsystem.
- **Boundary:** do not alter the Emberforge quest's mechanism or repaired forge.

### 11. Prism Sovereign — Glassdeep Caverns stage 5

- **Phase 1, Focused Light:** retain the existing fixed, one-turn beam and add
  an explicit recovery opening.
- **Phase 2, Fractured Crown:** two-turn split beams reflect through marked
  crystal faces. Render every segment and endpoint when the warning begins.
- **Arena mechanic:** rotate crystal reflectors between attacks to redirect
  the next beam toward the Sovereign, producing a two-turn stagger. Rotation
  requires an action; reflectors lock during a warning so its announced route
  cannot change. Beam behavior is deterministic, with no unseen bounces.
- **Guaranteed reward:** new **Prismweave Robes**, plus the existing special
  **Potion of Strength**. Proposed Mage armor: +3 defense, +35 maximum MP and
  12% spell-cost reduction. This remains below Archmage Robes' overall tier.
- **Boundary:** Orin's three quest resonators keep their separate progress.

## Batch 4 — major spectacle encounters

### 12. Fallen Sun Guardian — Ruined Temple summit

- **Phase 1, Sunbound Armor:** one-turn solar lances; Sun stance retains stronger
  armor, while Moon stance opens a less protected attack opportunity.
- **Phase 2, Cracked Core:** add two-turn sunburst rings; lunar sanctuary tiles
  remain safe from this attack.
- **Phase 3, Eclipse:** alternate solar lanes with lunar bursts. Each preparation
  displays the upcoming alignment and two-turn warning before stance matters.
- **Arena mechanic:** use accessible Sun/Moon altars to choose protection versus
  damage opportunities. Both alignments leave ordinary movement escapes.
  Retain the temple's current door, trap and awakened-sentinel rules; no altar
  toggle closes a door on an occupant or creates an unavoidable strike.
- **Guaranteed reward:** new **Sunwarden Mantle** directly on victory, separate
  from Nahla's treasure turn-in. Proposed all-class light armor: +5 defense,
  +15 maximum HP and +15 maximum MP. The buried treasure and quest reward remain.
- **Change:** replace the current immediate ranged retaliation with a visible,
  scheduled response. Ranged classes receive the same warning opportunity.

### 13. Red Dragon — Dragonspine stage 5

- **Phase 1, Hoard Defender:** a two-turn fixed breath cone and one-turn nearby
  claw/tail lanes; keep openings for melee attacks.
- **Phase 2, Scorched Wings:** alternate breath with two-turn wingbeat landing
  zones and three-turn burning patches. No long airborne invulnerability.
- **Phase 3, Last Inferno:** linked breath attacks in different directions,
  each with a separate warning; a longer recovery follows the sequence.
- **Arena mechanic:** basalt columns block breath and create approach cover.
  Designated columns crack visibly before they break, changing cover gradually.
  At least one usable cover route or ordinary movement escape remains. The
  goblet objective and return route cannot be burned or buried.
- **Guaranteed reward:** preserve **Dragon Scale Mantle** and goblet quest.

## Batch 5 — castle finale

### 14. Castellan — Castle floor 2, Iron Keep

- **Phase 1, Shield Commander:** retain the current frontal shield, charge,
  close swing and recovery. Make the health-phase name explicit.
- **Phase 2, Broken Formation:** alternate a wider, two-turn shield sweep with
  a newly warned charge and one finite, marked guard rally.
- **Arena mechanic:** preserve charge collisions with masonry and guards.
  Add arena-side portcullis controls that all classes can use to split his
  guard formation or interrupt a charge. Occupied gates never close on actors.
- **Guaranteed reward:** new **Castellan's Bulwark**, alongside the existing
  100 gold and stair unlock. Proposed Warrior shield: +8 defense and 30% block,
  trading a little defense for more blocking compared with a Magic Shield.

### 15. Royal Arcanist — Castle floor 4, Crown Chapel

- **Phase 1, Warded Scholar:** retain the two-turn marked blast and ward changes.
- **Phase 2, Shattered Liturgy:** add a two-turn cross blast and sequential
  ward-linked lanes. A barrier change cannot invalidate every advertised escape.
- **Arena mechanic:** preserve both interactable ward pedestals and their
  existing damage/protection effects. Disabling a pedestal during a ward-linked
  preparation removes its lane and produces a recovery opening. Disabled wards
  remain spent for the attempt and follow existing castle retreat reset rules.
- **Guaranteed reward:** new **Crown Chapel Vestments**, alongside the existing
  100 gold and stair unlock. Proposed Mage armor: +4 defense, +45 maximum MP
  and 15% spell-cost reduction; below the full Archmage set's bonuses.

### 16. Lord Veyr — Castle floor 6, Throne of No Return

- **Phase 1, Armored Ruler:** preserve frontal armor and fixed close attacks.
  Include a clearly warned throne advance with a flank/recovery opportunity.
- **Phase 2, Broken Throne:** preserve two-turn magic and ward changes; add
  explicit ward-linked lanes and clearer safe-side previews.
- **Phase 3, Unbound Crown:** preserve three-turn expanding crown attacks.
  Alternate them with the existing warned fire lane; do not stack both into
  an unavoidable hit. Keep the current finite escort call limits.
- **Arena mechanic:** combine the existing ward pedestals, usable cover and
  fire-lane timing. When both pedestals are disabled, completing a crown attack
  creates a two-turn opening. Wards do not reappear during the same attempt.
- **Guaranteed reward:** **Crown of the Unbound**, a permanent victory trophy
  recorded in the journal and displayed in the ending/Hall of Fame, alongside
  the existing 10,000 score and campaign victory. Award it before entering the
  ending; the player must not collect floor loot after combat has ended. This
  does not add a helmet slot, postgame progression or a consumable item.
- **Purpose:** refine the existing three-phase finale rather than replace it.

## Reward delivery and balance

The listed new equipment numbers are proposed starting values for approval,
not final balance conclusions. Preserve existing named rewards' stats, class
restrictions and shop progression. New items need themed inventory/equipped
art and debug weapon support where applicable; no new gear slots or elemental
resistance system are included.

Every boss reward must be recorded as earned, unclaimed or claimed. A full
inventory leaves it available in the arena. A full floor-loot list must not
delete other loot to fit it: keep the reward in a reserved arena cache and
restore it on return. Collecting it marks it claimed; leaving, portalling,
cache refreshes, saving and loading cannot erase or duplicate it. Voluntarily
selling or dropping a claimed item does not re-award it.

For existing saves, preserve previous boss defeats and gear. Already defeated
bosses do not respawn. Grant newly introduced thematic rewards once through
their arena reward cache, or through the recorded ending for Veyr. Existing
reward types that lack trustworthy historical claim records must not be
retroactively regenerated; preserve any current dropped copy and known
unclaimed flags. Future victories use the new tracking. Do not globally rename
shop-bought Magic Shields or invent an item provenance for old saves.

Keep the Kraken encounter and reward behavior unchanged. Its existing recovery
and unclaimed-bow tracking serve as reference tests.

## Implementation and review sequence

After approval, implement **one boss per change set**, starting with the Lich
King and only the shared warning/reward support it needs. The five batches above
organize the roster; they do not authorize implementing a whole batch at once.
Stop after each completed boss for review and the next-step prompt.

For each boss:

1. Implement locked targets, phase transitions and recovery; add its arena
   interaction without changing unrelated region maps or quests.
2. Add the reward and its art, delivery/claim tracking and debug access if needed.
3. Save/load every new persistent field in both the active encounter and its
   level caches. Migrate saved arenas in place, preserving health, victories,
   quest flags, exploration, inventory and existing loot. Existing queued
   attacks keep a safe, deterministic resolution.
4. Test melee, bow and spell victories; ordinary walking escapes; threshold
   crossings during windup; freeze, cover, summons and hazard overlap; maximum
   enemy/loot capacity; full inventory; retreat and portals; save/load during
   warnings and recovery; claimed/unclaimed rewards and old saves.
5. Inspect warnings and arena props in the actual renderer at native scale,
   including underneath loot and across different generated arena layouts.
   Verify the shortest possible safe route fits the advertised countdown.
6. Run appropriate builds and regression tests, provide play-test instructions,
   and leave the change uncommitted with a suggested commit message.

## Roster exclusions

The Polar Kraken is already updated. The old dungeon Goblin King, Demon Lord
and Tarrasque still have enum/art compatibility entries but no current campaign
encounter spawn or boss-journal entry. This proposal does not reintroduce them;
the active mountain Goblin King is included above.
