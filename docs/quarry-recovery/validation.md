# Quarry recovery validation — November 11, 2026

The feature covers Orin’s western workers’ camp, three machinery repairs plus
the Colossus objective, one equipment choice, one boss potion choice, Stillbury’s
restored welcome, the western resurrection shrine and save version 249.

Debug `conr` and `test_runner` builds succeed. **9,672 / 9,672 gameplay checks
pass**, including 213 quarry recovery checks. Tests run in isolated
temporary directories with assets linked in. Local saves/settings were backed
up before migration checks and all three original file hashes remain unchanged.

The quarry checks exercise real movement through the western entrance and all
five stages across seeds 11, 12 and 13. Crane controls are used to select raised,
crossing and salvage states; every stage retains forward travel, retreat and
cached-stage revisits. Repairs before accepting count. A real Colossus kill
creates the new strongbox, including beneath its gold drop. The final exit
returns to the local quarry approach without creating a castle or Rosemoor road.

Reward checks cover every equipment/potion choice, full packs, held number keys,
deferral, pending-dialogue save/load, one-time payment, claimed item stats and
selling/dropping rewards. Legacy fixtures preserve old quest completion,
reserved warhammers, unpaid currency, old boss loot, active/cached machinery,
exploration, enemy wounds and retired checkpoint squads. The Crown’s historical
record stays intact. All four checkpoints permit the quarry’s restorer and
Stillbury becomes a safe town anchor on returning.

The western shrine works before quest acceptance or boss victory. Normal paid
resurrection and the Mage’s ritual retain their existing prices, MP cost and
health restoration. The existing Oakhaven shrine remains independently usable.

Native SDL software-renderer review at 1280×720 and 640×480 checks the reused
pixelkit camp/shrine assets, Orin and the workers, terrain, minimap markers and
reward text. Camp ground matches its western approach. An isolated copy of the
existing local save loads and renders with its equipment, wounds and fallen
companion intact. Captures:

- [Workers’ camp, 1280×720](camp-1280.png)
- [Workers’ camp, 640×480](camp-640.png)
- [Western shrine, 640×480](western-shrine-640.png)
- [Equipment choice, 640×480](reward-640.png)
- [Existing-save camp view](existing-save-1280.png)
- [Oakhaven shrine comparison](oakhaven-shrine-1280.png)

The built-in imagegen edit adds only the western shrine to the permanent atlas.
The actual README-linked image was inspected against the previous atlas and the
playable roads. Both URLs use SHA-256 prefix `874f0934ebe2`. Its northern castle
route, escarpments, Oakhaven shrine, island voyage and Act V provisional marking
are preserved. See [the image specification](world-map-prompt.md).
