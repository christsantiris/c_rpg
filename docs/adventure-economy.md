# Adventure Economy Design

Status: proposed economy. Distinct gold floor pickups are implemented, but the
income and recovery changes below are not.

## Goal

A character who has completed hours of story must always be able to earn the
supplies needed for another expedition. Recovery should take play, not a loan,
gamble, free rest, or a reset of character or quest progress. Adventuring should
earn gold and experience faster than safe work in town.

## Current baseline

- Health and Mana Potions cost 20 gold and restore 30 HP and 20 MP respectively.
- Ordinary enemies have a 10% chance to drop a small amount of gold and a
  separate 5% chance to drop a potion or scroll. This makes routine earnings
  unpredictable, particularly for a Mage who needs MP to earn more gold.
- Gold from ordinary enemies and bosses appears as a gold pickup that needs no
  inventory space; item drops keep their blue marker. Boss equipment and gold
  can be collected separately from the same tile.
- The four regional areas already regenerate ordinary enemies when entered as
  a new expedition. Defeated bosses and story quest rewards are one-time.
- Later boss victories currently increase the difficulty of newly generated
  enemies even in regions the player has already cleared. Those regions may
  cease to be useful for training.
- The Ruined Temple preserves its floors when revisited, and Rook's current
  labyrinth has no enemies. They do not yet offer repeatable combat income.

## Recovery loop: paid work in both towns

The Tavern in Town 1 and Inn in Town 2 offer an always-available local errand.
For example, collect a sealed supply order from the host, deliver it to a town
shop, and report back. Every step stays in a safe town; the player needs no HP,
MP, gold, inventory space, or combat ability to complete it. A job pays **10
gold on completion**, not on acceptance. Two completed jobs buy one potion at
the current price. Jobs can be repeated without a daily limit or story gate,
but award no experience, quest progress, or special items.

The job must require the walk and delivery, rather than repeated dialogue with
one NPC. Keep its payout below the expected return from an ordinary expedition.
Its purpose is to recover from a depleted state; it should not be the best way
to buy equipment. Blacksmith stock remains gated by boss victories, so town
work cannot unlock advanced weapons early.

This is the hard guarantee: from either town, a character at 1 HP, 0 MP,
0 gold, and no potions can work until they can buy an HP potion, an MP potion,
or both. The player keeps all levels, equipment, boss victories, and quests.
The zero-MP Return to Town spell and ordinary exits must still offer a route
back from adventure areas without requiring a consumable.

## Ordinary expedition loop

Gold should remain a chance drop, separate from the chance to find a potion or
scroll. Try a **35% gold-drop chance** with early purses of roughly **5-10
gold**, then tune larger purses for tougher enemies. Some kills should yield
only experience. Dropped coins should appear as a gold-colored pickup on the
floor, distinct from the blue item marker. Picking them up adds gold directly
without using an inventory slot. The income target is about one potion's price
over a short early expedition on average, not a guaranteed reward per kill.

After a regional boss is defeated, the Tavern or Inn board can offer a
repeatable bounty for that cleared region: defeat **8 regular enemies** on a
new expedition, then return for **25 gold**. Only one bounty can be active at a
time. Bosses, quest enemies that no longer exist, and town work do not count.
The bounty awards no extra experience; combat itself supplies experience.
The **25 gold bounty is guaranteed** on completion, while coin drops vary. At
the proposed early drop rate, eight kills should yield about 20 additional
gold on average. A lucky run can buy more supplies; an unlucky one can still
buy one potion from the bounty. Town work remains the hard recovery path.
These amounts are starting targets for playtesting, not a promise that every
class should consume exactly two potions per run. More story quests may be
added later, but they are not needed to repopulate an area.

A new expedition through a cleared region should start with fresh ordinary
enemies at its base regional difficulty, plus only the modest existing
player-level adjustment. It must not inherit the increased order tier from
subsequent boss victories. The region remains useful for practice and income;
its boss and one-time quest rewards remain gone. Apply the same pattern to
future combat areas. The Ruined Temple needs its own fresh-expedition reset
before it can be advertised as repeatable. Rook's labyrinth joins this loop
only after it has enemies.

## Potions and spending

Try Health and Mana Potions as **full restores at 20 gold each**. Their value
then grows with maximum HP and MP, instead of shrinking as characters level.
Drinking a potion in combat consumes one turn, and a potion cannot be wasted
when its resource is already full. The Healer sells HP potions, the Witch sells
MP potions, and the Alchemist keeps both. None provides direct HP or MP service.

One-time quest rewards, boss equipment, existing shop prices, and the
Blacksmith's stock gates stay as they are for the first balance pass. Stronger
potions will help every class, including the Warrior, so do not raise all enemy
HP or attack at the same time. First measure whether Warrior equipment upgrades
become useful and whether the Mage can finish ordinary expeditions with a
nonnegative supply balance. If the Warrior remains too strong, tune encounters
or equipment incentives separately rather than taxing every class equally.

## Acceptance checks

1. From zero gold, zero MP, one HP, and an empty pack in either town, a player
   can complete paid work, buy potions, and resume combat without losing story
   progress.
2. After three boss victories, re-entering a cleared early region still offers
   manageable regular enemies and repeatable XP and gold, but no boss or story
   reward.
3. Some ordinary kills drop no gold. Completing an eight-kill bounty guarantees
   its 25-gold turn-in, regardless of random drops; leaving before completion
   does not award it. Coins on the floor look gold and use no inventory slot.
4. Buying and using potions remains the main recovery path. Return to Town,
   Tavern, and Inn never refill HP or MP for free.
5. A first-boss, three-boss, and late-game run for each class records gold
   earned, potions used, upgrades bought, deaths, and whether another expedition
   is affordable. Tune the proposed payouts against those results.

Implement this in separate steps: potion behavior, town work, predictable
combat income and bounties, then cleared-region scaling and replay support.
Any new job or bounty progress stored in game state must be saved and loaded.
