#include "containers.h"
#include "mangrove.h"
#include "minotaur.h"
#include "jail.h"
#include "combat_feedback.h"
#include "lich.h"
#include "necromancer.h"
#include "goblin_king.h"
#include "drowned_queen.h"
#include "swamp_demon.h"
#include "grave_marshal.h"
#include "desert_pharaoh.h"
#include "cinder_lord.h"
#include "prism_sovereign.h"
#include "sun_guardian.h"
#include "red_dragon.h"
#include <stdlib.h>

typedef struct {
    int kind;
    int empty;
    int location;
    int base_gold;
    int gold_per_level;
    int score_gold;
    int guard_radius;
} ContainerDefinition;

static const ContainerDefinition definitions[] = {
    {TILE_CRYPT_CACHE, -1, LOCATION_DUNGEON, 10, 2, 1, 0},
    {TILE_MOUNTAIN_CACHE, -1, LOCATION_MOUNTAINS, 15, 4, 1, 0},
    {TILE_COAST_CACHE, -1, LOCATION_COAST, 20, 4, 1, 0},
    {TILE_MANGROVE_CACHE, -1, LOCATION_MANGROVES, 10, 5, 0, 2},
    {TILE_STONEWAKE_SUPPLIES, TILE_STONEWAKE_EMPTY, LOCATION_GLASSDEEP, 20, 5, 0, 3},
    {TILE_STORM_SUPPLIES, TILE_STORM_EMPTY, LOCATION_STORMWATCH, 15, 3, 0, 0},
    {TILE_IRON_CACHE, TILE_IRON_EMPTY, LOCATION_IRONFANG, 15, 5, 1, 0},
    {TILE_GIANTS_CACHE, TILE_GIANTS_EMPTY, LOCATION_GIANTS_CROWN, 25, 5, 0, 0},
    {TILE_CROWN_SUPPLIES, TILE_CROWN_EMPTY, LOCATION_CROWNROAD_NORTH, 25, 0, 0, 4},
    {TILE_LICH_RELIQUARY, -1, LOCATION_DUNGEON, 0, 0, 0, 0},
    {TILE_NECRO_RELIQUARY, -1, LOCATION_FOREST, 0, 0, 0, 0},
    {TILE_GOBLIN_KING_RELIQUARY, -1, LOCATION_MOUNTAINS, 0, 0, 0, 0},
    {TILE_QUEEN_RELIQUARY, -1, LOCATION_COAST, 0, 0, 0, 0},
    {TILE_DEMON_RELIQUARY, -1, LOCATION_SWAMP, 0, 0, 0, 0},
    {TILE_MINOTAUR_RELIQUARY, -1, LOCATION_LABYRINTH, 0, 0, 0, 0},
    {TILE_MARSHAL_RELIQUARY, -1, LOCATION_CATACOMBS, 0, 0, 0, 0},
    {TILE_PHARAOH_RELIQUARY, -1, LOCATION_DESERT, 0, 0, 0, 0},
    {TILE_CINDER_RELIQUARY, -1, LOCATION_ASHEN, 0, 0, 0, 0},
    {TILE_PRISM_RELIQUARY, -1, LOCATION_GLASSDEEP, 0, 0, 0, 0},
    {TILE_GUARDIAN_RELIQUARY, -1, LOCATION_TEMPLE, 0, 0, 0, 0},
    {TILE_DRAGON_RELIQUARY_FLOOR, -1, LOCATION_DRAGONSPINE, 0, 0, 0, 0},
    {CONTAINER_ARMORY, -1, LOCATION_CASTLE_INTERIOR, 0, 0, 0, 0},
    {CONTAINER_CHAPEL, -1, LOCATION_CASTLE_INTERIOR, 0, 0, 0, 0},
    {TILE_JAIL_GEAR_CHEST, -1, LOCATION_JAIL_ARMORY, 0, 0, 0, 1000}
};

static const ContainerDefinition *definition(int kind) {
    for (unsigned int i = 0; i < sizeof(definitions) / sizeof(definitions[0]); i++) {
        if (definitions[i].kind == kind) {
            return &definitions[i];
        }
    }
    return NULL;
}

int containers_kind_location(int kind) {
    const ContainerDefinition *d = definition(kind);
    return d ? d->location : -1;
}

static int same_floor(const GameState *g, const LootContainer *c) {
    return c->location == (int)g->location && c->level == g->level;
}

void containers_forget_floor(GameState *g) {
    // A regenerated floor has new chests; records for its old layout would leave phantom chests.
    // Boss reward claims live in their own state fields, so nothing is lost or repeated.
    int kept = 0;
    for (int i = 0; i < g->loot_container_count; i++) {
        if (!same_floor(g, &g->loot_containers[i])) {
            g->loot_containers[kept++] = g->loot_containers[i];
        }
    }
    g->loot_container_count = kept;
}

const LootContainer *containers_record(const GameState *g, int x, int y) {
    for (int i = 0; i < g->loot_container_count; i++) {
        const LootContainer *c = &g->loot_containers[i];
        if (same_floor(g, c) && c->x == x && c->y == y) {
            return c;
        }
    }
    return NULL;
}

TileType containers_ground(const GameState *g, int x, int y) {
    TileType tile = g->map.tiles[y][x];
    if (tile == TILE_ITEM) {
        for (int i = 0; i < g->floor_item_count; i++) {
            const FloorItem *item = &g->floor_items[i];
            if (item->active && item->x == x && item->y == y) {
                return item->underlying_tile;
            }
        }
    }
    if (tile == TILE_PORTAL && g->portal_active && g->portal_x == x && g->portal_y == y) {
        return g->portal_origin_tile;
    }
    return tile;
}

static void set_ground(GameState *g, int x, int y, TileType tile) {
    if (g->map.tiles[y][x] == TILE_PORTAL) {
        g->portal_origin_tile = tile;
    } else if (g->map.tiles[y][x] != TILE_ITEM) {
        g->map.tiles[y][x] = tile;
    }
    for (int i = 0; i < g->floor_item_count; i++) {
        FloorItem *item = &g->floor_items[i];
        if (item->active && item->x == x && item->y == y) {
            item->underlying_tile = tile;
        }
    }
}

int containers_at(const GameState *g, int x, int y, int *kind, int *opened) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) {
        return 0;
    }
    const LootContainer *c = containers_record(g, x, y);
    if (c) {
        *kind = c->kind;
        *opened = c->opened;
        return 1;
    }
    int tile = containers_ground(g, x, y);
    if (tile == TILE_PRISM_PAVED_RELIQUARY) {
        tile = TILE_PRISM_RELIQUARY;
    } else if (tile == TILE_DRAGON_RELIQUARY_ASH || tile == TILE_DRAGON_RELIQUARY_HOARD) {
        tile = TILE_DRAGON_RELIQUARY_FLOOR;
    }
    for (unsigned int i = 0; i < sizeof(definitions) / sizeof(definitions[0]); i++) {
        const ContainerDefinition *d = &definitions[i];
        if (d->location == (int)g->location && (tile == d->kind || tile == d->empty)) {
            *kind = d->kind;
            *opened = tile == d->empty;
            return 1;
        }
    }
    if (g->location == LOCATION_CASTLE_INTERIOR && x == 53 &&
        ((g->level == 2 && y == 55 && g->castellan_reward_state == 1) ||
        (g->level == 4 && y == 56 && g->arcanist_reward_state == 1))) {
        *kind = g->level == 2 ? CONTAINER_ARMORY : CONTAINER_CHAPEL;
        *opened = 0;
        return 1;
    }
    if (g->location == LOCATION_MANGROVES) {
        for (int i = 0; i < 3; i++) {
            int cx;
            int cy;
            mangrove_cache(g->level, i, &cx, &cy);
            if (x == cx && y == cy && (g->mangroves.searched & (1u << i))) {
                *kind = TILE_MANGROVE_CACHE;
                *opened = 1;
                return 1;
            }
        }
    }
    return 0;
}

static int target(const GameState *g, int *x, int *y, int *kind, int *opened) {
    if ((g->location == LOCATION_MANGROVES && g->mangroves.aboard) ||
        (g->location == LOCATION_IRONFANG && g->level >= 1 && g->level <= IRON_DEPTH && g->ironfang[g->level - 1].aboard)) {
        return 0;
    }
    // An object underfoot takes priority over a chest on a neighboring tile.
    TileType tile = containers_ground(g, g->player.x, g->player.y);
    if (tile == TILE_COAST_BEACON_UNLIT || tile == TILE_COAST_BEACON_LIT || tile == TILE_COAST_TIDE_CONTROL ||
        tile == TILE_COAST_SLUICE_CONTROL || tile == TILE_BROKEN_BURIAL_SEAL ||
        tile == TILE_LICH_SEAL || tile == TILE_NECRO_GRAVE || tile == TILE_GOBLIN_KEG ||
        tile == TILE_QUEEN_CONTROL_H || tile == TILE_QUEEN_CONTROL_V ||
        (tile >= TILE_DEMON_VENT && tile <= TILE_DEMON_VENT_5) ||
        (tile >= TILE_CINDER_SLUICE && tile <= TILE_CINDER_SLUICE_5) ||
        (tile >= TILE_PRISM_LEFT && tile <= TILE_PRISM_RETURN)) {
        return 0;
    }
    const int dx[] = {0, 0, 1, 0, -1};
    const int dy[] = {0, -1, 0, 1, 0};
    for (int i = 0; i < 5; i++) {
        *x = g->player.x + dx[i];
        *y = g->player.y + dy[i];
        if (containers_at(g, *x, *y, kind, opened)) {
            return 1;
        }
    }
    return 0;
}

const char *containers_hint(const GameState *g) {
    int x;
    int y;
    int kind;
    int opened;
    return target(g, &x, &y, &kind, &opened) ? (opened ? "A: CLOSE CHEST" : "A: OPEN CHEST") : NULL;
}

static int pending(const GameState *g, int kind) {
    switch (kind) {
        case TILE_LICH_RELIQUARY: return g->lich_reward_state == 1;
        case TILE_NECRO_RELIQUARY: return g->necromancer_reward_state == 1;
        case TILE_GOBLIN_KING_RELIQUARY: return g->goblin_king_reward_state == 1;
        case TILE_QUEEN_RELIQUARY: return g->drowned_queen_reward_state == 1;
        case TILE_DEMON_RELIQUARY: return g->swamp_demon_reward_state == 1;
        case TILE_MINOTAUR_RELIQUARY: return g->minotaur_reward_state == MINOTAUR_REWARD_AEGIS || g->minotaur_reward_state == MINOTAUR_REWARD_LEGACY;
        case TILE_MARSHAL_RELIQUARY: return g->grave_marshal_reward_state == 1;
        case TILE_PHARAOH_RELIQUARY: return g->desert_pharaoh_reward_state == 1;
        case TILE_CINDER_RELIQUARY: return g->cinder_lord_reward_pending;
        case TILE_PRISM_RELIQUARY: return g->prism_sovereign_reward_pending;
        case TILE_GUARDIAN_RELIQUARY: return g->sun_guardian_reward_state == 1;
        case TILE_DRAGON_RELIQUARY_FLOOR: return g->red_dragon_reward_state == 1;
        case CONTAINER_ARMORY: return g->castellan_reward_state == 1;
        case CONTAINER_CHAPEL: return g->arcanist_reward_state == 1;
        default: return 0;
    }
}

Item containers_reward(const LootContainer *c, int bit) {
    switch (c->kind) {
        case TILE_LICH_RELIQUARY: return item_make_cryptblade();
        case TILE_NECRO_RELIQUARY: return item_make_necromancer_cloak();
        case TILE_GOBLIN_KING_RELIQUARY: return item_make_goblin_king_shield();
        case TILE_QUEEN_RELIQUARY: return item_make_tidecaller_robes();
        case TILE_DEMON_RELIQUARY: return item_make_demonic_sword();
        case TILE_MINOTAUR_RELIQUARY: return c->variant ? item_make_magic_shield() : item_make_labyrinth_aegis();
        case TILE_MARSHAL_RELIQUARY: return item_make_gravekeeper_mantle();
        case TILE_PHARAOH_RELIQUARY: return item_make_sandstorm_staff();
        case TILE_CINDER_RELIQUARY: return bit == 1 ? item_make_embercleaver() : item_make_strength_potion();
        case TILE_PRISM_RELIQUARY: return bit == 1 ? item_make_prismweave_robes() : item_make_strength_potion();
        case TILE_GUARDIAN_RELIQUARY: return item_make_sunwarden_mantle();
        case TILE_DRAGON_RELIQUARY_FLOOR: return item_make_dragon_scale_mantle();
        case CONTAINER_ARMORY: return item_make_castellan_bulwark();
        case CONTAINER_CHAPEL: return item_make_crown_chapel_vestments();
        default: return (Item){0};
    }
}

static void claimed(GameState *g, LootContainer *c) {
    switch (c->kind) {
        case TILE_LICH_RELIQUARY: g->lich_reward_state = 2; break;
        case TILE_NECRO_RELIQUARY: g->necromancer_reward_state = 2; break;
        case TILE_GOBLIN_KING_RELIQUARY: g->goblin_king_reward_state = 2; break;
        case TILE_QUEEN_RELIQUARY: g->drowned_queen_reward_state = 2; break;
        case TILE_DEMON_RELIQUARY: g->swamp_demon_reward_state = 2; break;
        case TILE_MINOTAUR_RELIQUARY: g->minotaur_reward_state = MINOTAUR_REWARD_CLAIMED; break;
        case TILE_MARSHAL_RELIQUARY: g->grave_marshal_reward_state = 2; g->catacombs_mantle_unclaimed = 0; break;
        case TILE_PHARAOH_RELIQUARY: g->desert_pharaoh_reward_state = 2; g->sandstorm_staff_unclaimed = 0; break;
        case TILE_CINDER_RELIQUARY: g->cinder_lord_reward_pending = c->rewards; break;
        case TILE_PRISM_RELIQUARY: g->prism_sovereign_reward_pending = c->rewards; break;
        case TILE_GUARDIAN_RELIQUARY: g->sun_guardian_reward_state = 2; break;
        case TILE_DRAGON_RELIQUARY_FLOOR: g->red_dragon_reward_state = 2; break;
        case CONTAINER_ARMORY: g->castellan_reward_state = 2; break;
        case CONTAINER_CHAPEL: g->arcanist_reward_state = 2; break;
        default: break;
    }
    // Restore native ground beneath loose items; the persistent record draws the chest.
    switch (c->kind) {
        case TILE_LICH_RELIQUARY: lich_restore_reward(g); break;
        case TILE_NECRO_RELIQUARY: necromancer_restore_reward(g); break;
        case TILE_GOBLIN_KING_RELIQUARY: goblin_king_restore_reward(g); break;
        case TILE_QUEEN_RELIQUARY: drowned_queen_restore_reward(g); break;
        case TILE_DEMON_RELIQUARY: swamp_demon_restore_reward(g); break;
        case TILE_MINOTAUR_RELIQUARY: minotaur_restore_reward(g); break;
        case TILE_MARSHAL_RELIQUARY: grave_marshal_restore_reward(g); break;
        case TILE_PHARAOH_RELIQUARY: desert_pharaoh_restore_reward(g); break;
        case TILE_CINDER_RELIQUARY: cinder_lord_restore_reward(g); break;
        case TILE_PRISM_RELIQUARY: prism_sovereign_restore_reward(g); break;
        case TILE_GUARDIAN_RELIQUARY: sun_guardian_restore_reward(g); break;
        case TILE_DRAGON_RELIQUARY_FLOOR: red_dragon_restore_reward(g); break;
        default: break;
    }
}

int containers_interact(GameState *g) {
    int x;
    int y;
    int kind;
    int opened;
    if (!target(g, &x, &y, &kind, &opened)) {
        return 0;
    }
    const ContainerDefinition *d = definition(kind);
    const LootContainer *existing = containers_record(g, x, y);
    if ((!existing || !existing->released) && !opened && d->guard_radius) {
        for (int i = 0; i < g->enemy_count; i++) {
            const Enemy *enemy = &g->enemies[i];
            if (enemy->active && (d->guard_radius == 1000 ? enemy->type == ENEMY_ROYAL_GUARD :
                abs(enemy->x - x) + abs(enemy->y - y) <= d->guard_radius)) {
                push_message(g, "Defeat the nearby guards before opening this chest.");
                return 1;
            }
        }
    }
    if (!existing && g->loot_container_count >= MAX_LOOT_CONTAINERS) {
        push_message(g, "The chest stays closed: there is no safe place to track its contents.");
        return 1;
    }
    LootContainer *c = existing ? &g->loot_containers[existing - g->loot_containers] : &g->loot_containers[g->loot_container_count++];
    if (!existing) {
        *c = (LootContainer){.location = g->location, .level = g->level, .x = x, .y = y, .kind = kind,
            .opened = opened, .released = opened, .loot_x = g->player.x, .loot_y = g->player.y};
    }
    c->opened = !c->opened;
    if (d->empty >= 0) {
        set_ground(g, x, y, d->empty);
    } else if (kind == TILE_MANGROVE_CACHE) {
        set_ground(g, x, y, g->level == 4 ? TILE_MANGROVE_DECK : TILE_MANGROVE_MUD);
    } else if (kind == TILE_CRYPT_CACHE || kind == TILE_MOUNTAIN_CACHE || kind == TILE_COAST_CACHE) {
        set_ground(g, x, y, kind == TILE_CRYPT_CACHE ? TILE_FLOOR : kind == TILE_MOUNTAIN_CACHE ? TILE_MOUNTAIN_CAVE_FLOOR : TILE_COAST_FLOOR);
    }
    if (!c->opened) {
        push_message(g, "Chest closed. Released loot stays on the ground.");
        return 1;
    }
    if (!c->released) {
        c->released = 1;
        c->gold = d->base_gold + d->gold_per_level * (kind == TILE_MOUNTAIN_CACHE ? map_mountain_difficulty(g->level) : g->level);
        c->gold_score = d->score_gold ? c->gold : 0;
        c->rewards = pending(g, kind);
        c->variant = kind == TILE_MINOTAUR_RELIQUARY && g->minotaur_reward_state == MINOTAUR_REWARD_LEGACY;
        if (kind == TILE_MANGROVE_CACHE) {
            for (int i = 0; i < 3; i++) {
                int cx;
                int cy;
                mangrove_cache(g->level, i, &cx, &cy);
                if (x == cx && y == cy) {
                    g->mangroves.searched |= 1u << i;
                }
            }
        }
        if (kind == TILE_CROWN_SUPPLIES) {
            int hp = g->player.hp;
            g->player.hp += 20;
            if (g->player.hp > g->player.max_hp) {
                g->player.hp = g->player.max_hp;
            }
            combat_feedback_add(g, FEEDBACK_HEAL, FEEDBACK_NOW, g->player.x, g->player.y, g->player.hp - hp);
        }
    }
    push_message(g, c->gold || c->rewards || (kind == TILE_JAIL_GEAR_CHEST && g->jail_gear_state == 1) ?
        "Chest opened. Its contents spill at your feet. Press Pick Up to collect them." : "You open the empty chest.");
    return 1;
}

int containers_pickup(GameState *g) {
    for (int i = 0; i < g->loot_container_count; i++) {
        LootContainer *c = &g->loot_containers[i];
        if (!same_floor(g, c) || c->loot_x != g->player.x || c->loot_y != g->player.y) {
            continue;
        }
        if (c->gold) {
            char text[MAX_MESSAGE_LEN];
            snprintf(text, sizeof(text), "Picked up %d gold.", c->gold);
            g->gold += c->gold;
            g->score += c->gold_score;
            c->gold = 0;
            c->gold_score = 0;
            push_message(g, text);
            return 1;
        }
        if (c->kind == TILE_JAIL_GEAR_CHEST && g->jail_gear_state == 1) {
            return jail_recover_gear(g);
        }
        c->rewards &= pending(g, c->kind);
        for (int bit = 1; bit <= 2; bit <<= 1) {
            if (!(c->rewards & bit)) {
                continue;
            }
            if (g->inventory_count >= MAX_INVENTORY) {
                push_message(g, "Your pack is full. The released loot stays safely on the ground.");
                return 1;
            }
            Item item = containers_reward(c, bit);
            g->inventory[g->inventory_count++] = item;
            g->score += item.value;
            c->rewards &= ~bit;
            claimed(g, c);
            char text[MAX_MESSAGE_LEN];
            snprintf(text, sizeof(text), "Picked up %s.", item.name);
            push_message(g, text);
            return 1;
        }
    }
    return 0;
}

int containers_valid(const GameState *g) {
    if (g->loot_container_count < 0 || g->loot_container_count > MAX_LOOT_CONTAINERS) {
        return 0;
    }
    for (int i = 0; i < g->loot_container_count; i++) {
        const LootContainer *c = &g->loot_containers[i];
        if (containers_kind_location(c->kind) != c->location || c->level < (c->location == LOCATION_JAIL_ARMORY ? 0 : 1) || c->level > MAX_REGION_DEPTH ||
            c->x < 0 || c->x >= MAP_W || c->y < 0 || c->y >= MAP_H ||
            c->loot_x < 0 || c->loot_x >= MAP_W || c->loot_y < 0 || c->loot_y >= MAP_H ||
            c->opened < 0 || c->opened > 1 || c->released != 1 || c->gold < 0 || c->gold > 10000 ||
            c->gold_score < 0 || c->gold_score > c->gold || c->rewards < 0 || c->rewards > 3 || c->variant < 0 || c->variant > 1 ||
            (c->rewards > 1 && c->kind != TILE_CINDER_RELIQUARY && c->kind != TILE_PRISM_RELIQUARY) ||
            (c->variant && c->kind != TILE_MINOTAUR_RELIQUARY) ||
            (c->rewards && containers_reward(c, c->rewards & 1 ? 1 : 2).type == ITEM_NONE)) {
            return 0;
        }
        for (int j = 0; j < i; j++) {
            const LootContainer *other = &g->loot_containers[j];
            if (c->location == other->location && c->level == other->level && c->x == other->x && c->y == other->y) {
                return 0;
            }
        }
    }
    return 1;
}

static void restore_position(const GameState *g, int *x, int *y, int chest) {
    if (map_is_walkable(&g->map, *x, *y)) {
        return;
    }
    int best = MAP_W + MAP_H;
    int tx = *x;
    int ty = *y;
    for (int py = 1; py < MAP_H - 1; py++) {
        for (int px = 1; px < MAP_W - 1; px++) {
            TileType tile = g->map.tiles[py][px];
            int plain = tile == TILE_FLOOR || tile == TILE_FOREST_FLOOR || tile == TILE_MOUNTAIN_CAVE_FLOOR ||
                tile == TILE_COAST_FLOOR || tile == TILE_SWAMP_FLOOR || tile == TILE_DRAGON_FLOOR;
            int distance = abs(px - *x) + abs(py - *y);
            if (plain && distance < best && (!chest || !containers_record(g, px, py))) {
                tx = px;
                ty = py;
                best = distance;
            }
        }
    }
    *x = tx;
    *y = ty;
}

void containers_restore(GameState *g) {
    // Accepting an older regional quest can regenerate its maps. Preserve its loot.
    for (int i = 0; i < g->loot_container_count; i++) {
        LootContainer *c = &g->loot_containers[i];
        if (same_floor(g, c)) {
            restore_position(g, &c->x, &c->y, 1);
            restore_position(g, &c->loot_x, &c->loot_y, 0);
        }
    }
}
