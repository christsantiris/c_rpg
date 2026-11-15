# Full-game playtest checklist

Build and launch from the repository root so the game can find `assets/`:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target conr test_runner -j4
./build/conr
```

Start a fresh save for campaign pacing. Keep existing saves in separate slots
for migration checks. Record class, equipment, character level, location/stage,
and the last action when reporting a problem. Note damage spikes, unclear exits,
unreadable warnings, empty stretches, and supplies that feel too scarce or abundant.

## Campaign route

1. **Oakhaven:** prepare with Cain and Bram. Explore the southern dungeon,
   defeat the Lich King, free Elowen and escort her back to the Inn.
2. **Forest and mountains:** explore both branches, follow Alder and Dain’s
   optional quests, defeat their bosses, and read the royal evidence.
3. **Cross the river by either route.** Eastern: Haunted Forest to occupied
   Dawnmere, then Saltwind Marsh: defeat Sszareth to open the river launch
   and take the skiff to Stillbury. Western: Goblin Mountains
   to Sunscar, then all five Giant’s Peaks stages and the Last Jarl, across
   Giant’s Bridge to Stillbury. Check the other branch in a separate run.
4. **Stillbury:** resupply and test optional local quests in Stonewake Quarry,
   the Labyrinth, Elderwood and Sunken Coast. Optional routes include the coastal
   voyage and Stormwatch rescue. Confirm retreat returns to the correct town.
5. **Castle approach:** Stillbury north gate → Blackwater Swamp → Royal
   Catacombs → Crown Road North → Castle Grounds. Defeat the Swamp Demon and
   Grave Marshal to open the route. Explore Rosemoor/Ridgeshire branches and
   their regional adventures if desired; their objectives do not gate the finale.
6. **Castle:** follow the six stages below through the King’s liberation and
   victory screen. Verify the journal’s completed boss record and crown trophy.

## Castle acceptance pass

| Stage | Check |
|---|---|
| Gatehouse | Try both guarded approaches and open the permanent service shortcut. |
| Royal alchemy Wing | Encounter Malakar early, interrupt his restorative, and force his escape at 40% HP. Explore the remaining rooms and claim the optional armory reward. |
| Court and Household | Try both flanks around the Great Hall; speak to Edras with and without royal evidence. |
| Crown Chapel | Explore the nave, reliquary, cloister and choir; disable conduits, defeat the Arcanist and claim the optional vestments. |
| Royal Apartments | Inspect the bedside orders, crown diagrams and recovery ledger; collect optional supplies and open the household shortcut. |
| King’s Chamber | Explore the galleries and royal passage, open its retreat shortcut, and confirm Malakar begins at full health. Dodge vials/surges, interrupt conduit lanes, then dodge crown rupture and throne fire. Free the King and read both ending pages. |

On each floor, walk to the actual retreat stair and use it, then return. Save
and reload before a fight, during a warning and after a boss victory. Verify
warnings, wounds, spent conduits, collected supplies and opened shortcuts persist.
Returning to town regroups surviving defenders; completed minibosses, rewards
and permanent shortcuts remain completed. A living final Malakar resets on
full retreat, as do his conduits and finite escort allowance.

Try the finale as Warrior, Rogue and Mage, with and without companions. Test
small windows as well as normal play size. Compare companion survival with the
ending’s acknowledgments. Existing completed victories should load directly as
completed without another reward; existing unfinished final fights should retain
their wounds and announced attacks after migration.

Automated coverage exercises routes, transitions, combat rules and save
compatibility. A normal-stat campaign playthrough is still needed to judge
balance, resource pacing and how clearly the story and routes read in play.
