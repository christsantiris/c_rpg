# Act 5 navigation validation — November 11, 2026

Debug `conr` and `test_runner` builds succeed. **10,133 / 10,133 gameplay checks
pass**, including **461 Act 5 navigation checks**. Tests run in isolated temporary
working directories with linked assets. Local saves/settings were backed up and
their original SHA-256 checksums remain unchanged.

Movement tests walk from Castle Grounds through both Crown Roads to the correct
hub, through its exterior and regional approach, and into all four adventures.
All five Ashen Hollow, Frostfell, Dragonspine and Ironfang stages are exercised
across seeds 19, 116 and 213, after terrain and encounters are prepared. Checks
use actual transition tiles, including retreat, cached revisits, final exits,
the return to the hub and the cached road back to Castle Grounds. They require
no new boss victory, quest or reputation gate. Frostfell paths retain the normal
routes around optional slick-ice shortcuts.

Older-save fixtures cover Rosemoor's sealed Ashen gate, active and cached
Frostfell/Dragonspine side entrances, a player standing at the old entrance,
wounded enemies, equipment, floor loot and its sharpened metadata, exploration
and wounded Crown Road patrols. New entrance repairs preserve those records in
place. Loot cannot hide or disable the real transition tile. Existing migration
checks preserve every previously explored tile while allowing discovered gate
markers to be added.

Native SDL software-renderer review at **1280×720** and **640×480** covers Castle
Grounds, both town hubs, both Crown Roads, all four adventure entrances and
loaded version-249 Rosemoor/Frostfell migration fixtures. Labels remain readable
at west, east, north and south viewport edges. Existing town/gate art is reused;
no buildings are redrawn. An isolated copy of the existing local save also loads
and renders successfully.

Selected native captures:

- [Castle west gate](castle-west-640.png), [east gate](castle-east-640.png)
- [Rosemoor to Ashen Hollow](rosemoor-ashen-640.png), [Frostfell](rosemoor-frost-640.png)
- [Ridgeshire to Dragonspine](ridgeshire-dragon-640.png), [Ironfang](ridgeshire-mines-640.png)
- [Crown Road West](crown-west-640.png), [Crown Road East](crown-east-640.png)
- [Frostfell south entrance](frostfell-entry-640.png), [Dragonspine south entrance](dragonspine-entry-640.png)
- [Ironfang entrance](ironfang-entry-640.png)
- [Migrated Frostfell entrance](migrated-frost-new-entry-640.png)
- [Existing-save Rosemoor view](existing-save-rosemoor-1280.png)

The actual canonical image embedded and linked in README was inspected against
the approved routes. West is Rosemoor with Ashen Hollow and Frostfell; east is
Ridgeshire with Dragonspine and Ironfang. The northern Stillbury route, escape
return, two shrines, escarpments and both sea routes remain intact. Act V's story
remains provisional. Both README URLs match checksum **4ab1f192db91**.

No changes have been staged or committed. Storylines, NPC interactions, quests,
rewards and the castle finale await the next change set after the manual commit.
