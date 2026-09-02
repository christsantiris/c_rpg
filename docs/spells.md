# Spells

The game currently provides three learnable spells: Magic Arrow, Fireball, and
Heal. Spells consume MP and become stronger as the player's character level
increases.

## Spell Summary

| Spell | Type | MP | Base effect | Range | Radius |
| --- | --- | ---: | ---: | ---: | ---: |
| Magic Arrow | Ranged damage | 10 | 15 damage | 6 | - |
| Fireball | Area damage | 20 | 25 damage | 4 | 2 |
| Heal | Self healing | 15 | 40 HP | - | - |

Damage and healing use the following scaling formula:

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

Using a new scroll adds its spell to the player's spellbook and consumes the
scroll. Using a scroll for an already-known spell displays a message and does
not consume it. A player can know up to 10 spells.

The Mage begins with a Magic Arrow scroll, but must use that scroll from the
inventory before the spell becomes known. Other classes can learn the same
spells when they find or purchase the relevant scrolls.

Scrolls are sold by the Alchemist and can also appear as enemy drops. Early
floors emphasize Magic Arrow and Heal; Fireball begins appearing in the loot
table from floor 4 onward.

## Equipping and Casting

1. Press `I` to open the inventory and use a spell scroll.
2. Press `B` during play to open the spellbook.
3. Select a known spell with the arrow keys.
4. Press `Enter` to equip the selected spell.
5. Return to play with `Esc`.
6. Move in the direction you want to aim.
7. Press `C` to cast the equipped spell.

The last movement direction determines the aim direction for ranged and area
spells. Attempting to cast without an equipped spell, without enough MP, or
without first establishing a movement direction does not cast the spell.

## Current Behavior Notes

- All spell types perform the movement-direction check before resolving. As a
  result, Heal currently requires the player to have moved at least once even
  though it targets the caster.
- Spell damage ignores enemy defense; the calculated spell damage is applied
  directly to HP.
- Magic Arrow hits only the first enemy in its path, while Fireball can hit
  every enemy inside its blast radius.
- Mana potions restore 20 MP, up to the player's maximum MP.
