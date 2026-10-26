# Crown Road North renderer review

These are actual 1280×720 SDL software-renderer captures. Controlled fixtures
reveal the terrain for review; they are not full quest playthroughs.

- [Patrols and reconnecting forest trails](patrols.png).
- [A bomber's fixed two-action blast warning](bomb-warning.png).
- [The north boundary exit and Castle Grounds sign](castle-exit.png).
- [Dawnmere's Spice Merchant sign and doorway path](spice-merchant.png).

The six recurring enemy types use their existing native sprites. The woodland
uses the adjoining Elderwood's existing floor and tree art, with a single-tile
stone road, timber barricades and supply caches. Dawnmere's building artwork
retains its established textures and doorway scale; its label and Corvin's
title change to Spice Merchant.

`tests/test_crownroad_north.c` covers terrain connectivity, the complete roster,
shield flanking through combat turns, pack positioning, fixed ranged warnings,
freeze/recovery, supply rewards, both travel directions, respawns around loot
and portal anchors, save/load and version-201 migration. The full SDL debug
suite also renders all six types through `game_draw()` on the road terrain.
The targeted gameplay suite passes AddressSanitizer and UndefinedBehaviorSanitizer.
