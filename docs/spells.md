# Spells

The game provides Magic Arrow, Fireball, Heal, Frost Bolt, Teleport, and Return
to Town. Combat spells consume MP and scale with character level and equipped
staff spell power. Magic Arrow, Fireball, and Heal support up to three ranks.

## Spell Summary

| Spell | Type | MP | Base effect | Range | Radius |
| --- | --- | ---: | ---: | ---: | ---: |
| Magic Arrow | Ranged damage | 10 | 15 damage | 6 | - |
| Fireball | Area damage | 20 | 25 damage | 4 | 2 |
| Heal | Self healing | 15 | 40 HP | - | - |
| Return to Town | Utility portal | 0 | Town travel | - | - |
| Frost Bolt | Ranged control | 14 | 18 damage, freeze | 5 | - |
| Teleport | Utility movement | 12 | Move up to 4 tiles | 4 | - |

Damage and healing use the following scaling formula before spell-rank and
staff bonuses are applied:

```text
final effect = base effect + (player level * 2)
```

For example, a level 5 character casts Magic Arrow for 25 damage and Heal for
50 HP.

## Magic Arrow

Magic Arrow launches a blue magical projectile in the player's last movement
direction. It travels up to six tiles and damages the first active enemy in its
path.

- MP cost: 10
- Base damage: 15
- Range: 6 tiles
- Damage at level `L`: `15 + (L * 2)`
- Stops searching after hitting the first enemy
- Cannot pass through walls or other non-walkable tiles
- Uses the dedicated `assets/sfx/magicArrow.mp3` casting sound

The visual effect is a moving blue arrow with a cyan core, magical outline,
trailing sparks, and an impact flash.

## Fireball

Fireball travels four tiles in the player's last movement direction, then
damages every active enemy within a Manhattan-distance radius of two around
the destination.

- MP cost: 20
- Base damage: 25 per affected enemy
- Range: 4 tiles
- Blast radius: 2 tiles
- Damage at level `L`: `25 + (L * 2)` per enemy
- Can damage several enemies with one cast
- Does not damage the player

The radius uses Manhattan distance. A target is affected when:

```text
abs(enemy_x - impact_x) + abs(enemy_y - impact_y) <= 2
```

## Heal

Heal restores the caster's HP and cannot raise health above maximum HP.

- MP cost: 15
- Base healing: 40 HP
- Healing at level `L`: `40 + (L * 2)` HP
- Targets the player only
- Displays a green effect on the player's tile

## Learning Spells

Spells are learned permanently by using their corresponding scroll from the
inventory:

- Scroll: Magic Arrow
- Scroll: Fireball
- Scroll: Heal
- Scroll: Return to Town

Using a new scroll adds its spell to the player's spellbook and consumes the
scroll. Using a scroll for an already-known spell displays a message and does
not consume it. A player can know up to 10 spells.

After regional boss victories, the Alchemist expands its Mage-only stock:

- One boss: Magic Arrow II and Frost Bolt
- Two bosses: Fireball II and Teleport
- Three bosses: Heal II

The Mage begins with a Magic Arrow scroll, but must use that scroll from the
inventory before the spell becomes known. Other classes can learn Magic Arrow,
Fireball, Heal, and Return to Town. Frost Bolt, Teleport, and upgrade tomes are
restricted to the Mage.

Cain stands near the town crossroads and gives every character one Scroll of
Return to Town when spoken to. If the inventory is full, make room and speak to
him again. The scroll is not part of the starting inventory and is not sold by
the Alchemist or included in random enemy drops.

Upgrade tomes increase the existing spell rather than learning a separate
spell. Each Magic Arrow rank adds 10 base damage, one tile of range, and 2 MP
to its cost. Each Fireball rank adds 12 base damage and 3 MP to its cost. Each
Heal rank adds 20 base healing and 2 MP to its cost.

## Return to Town

Return to Town costs 0 MP and does not require an aim direction. Casting it in
an adventure area records the player's stage and position, creates a portal at
the matching town entrance, and immediately transports the player to town.
Stepping onto the town portal returns the player to the recorded position and
closes the portal. Casting it while already in town has no effect.

Inside the Ruined Temple, the portal preserves the exact pyramid tier,
position, and encounter state and appears near the town harbor. Casting Return
to Town on the Ruined Isle instead sails directly back to town without leaving
a return portal. A new portal replaces any portal that was already open.

The Alchemist sells Magic Arrow, Fireball, and Heal scrolls. Random enemy item
drops can contain those same scrolls: early stages emphasize Magic Arrow and
Heal, while Fireball enters the drop table from stage 4 onward.

## Equipping and Casting

1. Press `I` to open the inventory and use a spell scroll.
2. Press `B` during play to open the spellbook.
3. Select a known spell with the arrow keys.
4. Press `Enter` to equip the selected spell.
5. Return to play with `Esc`.
6. Move in the direction you want to aim.
7. Press `C` to cast the equipped spell.

The last movement direction determines the aim direction for ranged, area, and
Teleport spells. Attempting to cast without an equipped spell or enough MP
does not cast the spell. Magic Arrow, Fireball, Frost Bolt, and Heal also
require the player to have moved once to establish a direction.

## Current Behavior Notes

- Heal requires a previous movement direction even though it targets the
  caster. Return to Town does not.
- Spell damage ignores enemy defense; the calculated spell damage is applied
  directly to HP.
- Magic Arrow hits only the first enemy in its path, while Fireball can hit
  every enemy inside its blast radius.
- Mana potions restore 20 MP, up to the player's maximum MP.
- Gaining a character level restores MP to maximum as well as restoring HP.
- Morwen's witch hut restores all missing MP for 1 gold per 3 MP, rounded up.
  If MP is below 25% and the player cannot afford that service, a free
  emergency ritual restores MP to 50%. It can be used once per game.
