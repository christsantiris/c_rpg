# Loot controls

Use **Interact** (A by default) on or beside a chest to open or close it.
Opening releases its contents onto the ground at the player's feet. The chest
remains in the area, including after collecting its contents. Closing it leaves
released loot on the ground; reopening never generates a second reward.

Use **Pick Up** (P by default) while standing on loose gold or equipment.
Pick Up never opens a chest. Full inventories leave equipment available for a
later pickup; gold does not require inventory space. Existing guarded supplies
still require defeating their guards. Interact also operates doors, switches,
and other regional mechanisms; arrow keys always move.

This applies to dungeon and coastal treasure, regional supply chests, boss reward
reliquaries, castle reward chests, and the Royal Dungeon evidence chest. Equipment and
quest items retain their existing stats and rewards.

Save version 209 stores chest states and released loot independently of the
active floor's dropped-item buffer. Travel, portals, and save/load retain them.
Older saves remain usable: previously searched supply chests start empty, and
chests already removed by older versions stay collected.
