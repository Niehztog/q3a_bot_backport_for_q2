// Q2 item weight template for bot AI.
// Per-bot files #define FS_*/W_*/GWW_* values then #include this file.
// Mirrors Q3's fw_items.c but uses Q2 items and inventory indices.
//
// Weight names must match the "name" field in botfiles/items.c iteminfo entries.
// NOTE: inv.h is NOT included here — the per-bot _i.c file includes it
// before #include-ing this template (matches Q3's original structure).

// Provide defaults so bots without all defines still work.
#ifndef FS_HEALTH
#define FS_HEALTH           1
#endif
#ifndef FS_ARMOR
#define FS_ARMOR            2
#endif

// Weapon pickup weights (when bot does NOT own the weapon)
#ifndef W_SHOTGUN
#define W_SHOTGUN           180
#endif
#ifndef W_SUPERSHOTGUN
#define W_SUPERSHOTGUN      190
#endif
#ifndef W_MACHINEGUN
#define W_MACHINEGUN        150
#endif
#ifndef W_CHAINGUN
#define W_CHAINGUN          170
#endif
#ifndef W_GRENADELAUNCHER
#define W_GRENADELAUNCHER   100
#endif
#ifndef W_ROCKETLAUNCHER
#define W_ROCKETLAUNCHER    200
#endif
#ifndef W_HYPERBLASTER
#define W_HYPERBLASTER      160
#endif
#ifndef W_RAILGUN
#define W_RAILGUN           190
#endif
#ifndef W_BFG10K
#define W_BFG10K            180
#endif

// "Got-weapon" weights (when bot already owns it — pick up for ammo)
#ifndef GWW_SHOTGUN
#define GWW_SHOTGUN         40
#endif
#ifndef GWW_SUPERSHOTGUN
#define GWW_SUPERSHOTGUN    40
#endif
#ifndef GWW_MACHINEGUN
#define GWW_MACHINEGUN      40
#endif
#ifndef GWW_CHAINGUN
#define GWW_CHAINGUN        40
#endif
#ifndef GWW_GRENADELAUNCHER
#define GWW_GRENADELAUNCHER 30
#endif
#ifndef GWW_ROCKETLAUNCHER
#define GWW_ROCKETLAUNCHER  50
#endif
#ifndef GWW_HYPERBLASTER
#define GWW_HYPERBLASTER    40
#endif
#ifndef GWW_RAILGUN
#define GWW_RAILGUN         50
#endif
#ifndef GWW_BFG10K
#define GWW_BFG10K          40
#endif

// Powerup weights
#ifndef W_QUAD
#define W_QUAD              400
#endif
#ifndef W_INVULNERABILITY
#define W_INVULNERABILITY   400
#endif
#ifndef W_SILENCER
#define W_SILENCER          50
#endif
#ifndef W_REBREATHER
#define W_REBREATHER        20
#endif
#ifndef W_ENVIROSUIT
#define W_ENVIROSUIT        20
#endif

// ===== WEAPONS =====
// Botlib switch uses THRESHOLD semantics: "case N" matches when inventory < N.
// So "case 1" fires when inventory=0 (don't have weapon) → high W_* weight.
// "default" fires when inventory>=1 (have weapon) → ammo-dependent GWW_* weight.
// Picking up a weapon you own gives ammo in Q2, so it's still worth it
// when low on ammo but not when fully stocked.

weight "weapon_shotgun"
{
    switch(INVENTORY_SHOTGUN)
    {
        case 1: return W_SHOTGUN; // don't have it (inv < 1) — want it
        default: // have it (inv >= 1) — only worth it for ammo refill
        {
            switch(INVENTORY_SHELLS)
            {
                case 20: return GWW_SHOTGUN;
                case 40: return $evalint(GWW_SHOTGUN / 2);
                case 50: return 0;
                default: return 0;
            }
        }
    }
}

weight "weapon_supershotgun"
{
    switch(INVENTORY_SUPERSHOTGUN)
    {
        case 1: return W_SUPERSHOTGUN;
        default:
        {
            switch(INVENTORY_SHELLS)
            {
                case 20: return GWW_SUPERSHOTGUN;
                case 40: return $evalint(GWW_SUPERSHOTGUN / 2);
                case 50: return 0;
                default: return 0;
            }
        }
    }
}

weight "weapon_machinegun"
{
    switch(INVENTORY_MACHINEGUN)
    {
        case 1: return W_MACHINEGUN;
        default:
        {
            switch(INVENTORY_BULLETS)
            {
                case 50: return GWW_MACHINEGUN;
                case 80: return $evalint(GWW_MACHINEGUN / 2);
                case 100: return 0;
                default: return 0;
            }
        }
    }
}

weight "weapon_chaingun"
{
    switch(INVENTORY_CHAINGUN)
    {
        case 1: return W_CHAINGUN;
        default:
        {
            switch(INVENTORY_BULLETS)
            {
                case 50: return GWW_CHAINGUN;
                case 80: return $evalint(GWW_CHAINGUN / 2);
                case 100: return 0;
                default: return 0;
            }
        }
    }
}

weight "weapon_grenadelauncher"
{
    switch(INVENTORY_GRENADELAUNCHER)
    {
        case 1: return W_GRENADELAUNCHER;
        default:
        {
            switch(INVENTORY_GRENADES)
            {
                case 10: return GWW_GRENADELAUNCHER;
                case 20: return $evalint(GWW_GRENADELAUNCHER / 2);
                case 25: return 0;
                default: return 0;
            }
        }
    }
}

weight "weapon_rocketlauncher"
{
    switch(INVENTORY_ROCKETLAUNCHER)
    {
        case 1: return W_ROCKETLAUNCHER;
        default:
        {
            switch(INVENTORY_ROCKETS)
            {
                case 10: return GWW_ROCKETLAUNCHER;
                case 20: return $evalint(GWW_ROCKETLAUNCHER / 2);
                case 25: return 0;
                default: return 0;
            }
        }
    }
}

weight "weapon_hyperblaster"
{
    switch(INVENTORY_HYPERBLASTER)
    {
        case 1: return W_HYPERBLASTER;
        default:
        {
            switch(INVENTORY_CELLS)
            {
                case 50: return GWW_HYPERBLASTER;
                case 80: return $evalint(GWW_HYPERBLASTER / 2);
                case 100: return 0;
                default: return 0;
            }
        }
    }
}

weight "weapon_railgun"
{
    switch(INVENTORY_RAILGUN)
    {
        case 1: return W_RAILGUN;
        default:
        {
            switch(INVENTORY_SLUGS)
            {
                case 10: return GWW_RAILGUN;
                case 20: return $evalint(GWW_RAILGUN / 2);
                case 25: return 0;
                default: return 0;
            }
        }
    }
}

weight "weapon_bfg"
{
    switch(INVENTORY_BFG10K)
    {
        case 1: return W_BFG10K;
        default:
        {
            switch(INVENTORY_CELLS)
            {
                case 50: return GWW_BFG10K;
                case 80: return $evalint(GWW_BFG10K / 2);
                case 100: return 0;
                default: return 0;
            }
        }
    }
}

// ===== AMMO =====
// Weights scale with how much the bot needs the ammo (low ammo = high weight).
// Matches Q3's graduated approach instead of flat weights.

weight "ammo_shells"
{
    switch(INVENTORY_SHELLS)
    {
        case 10: return 50;
        case 20: return 40;
        case 30: return 25;
        case 40: return 10;
        case 50: return 0;
        default: return 0;
    }
}

weight "ammo_bullets"
{
    switch(INVENTORY_BULLETS)
    {
        case 30: return 50;
        case 50: return 35;
        case 80: return 15;
        case 100: return 0;
        default: return 0;
    }
}

weight "ammo_cells"
{
    switch(INVENTORY_CELLS)
    {
        case 30: return 50;
        case 50: return 35;
        case 80: return 15;
        case 100: return 0;
        default: return 0;
    }
}

weight "ammo_rockets"
{
    switch(INVENTORY_ROCKETS)
    {
        case 5: return 60;
        case 10: return 45;
        case 15: return 30;
        case 20: return 15;
        case 25: return 0;
        default: return 0;
    }
}

weight "ammo_slugs"
{
    switch(INVENTORY_SLUGS)
    {
        case 5: return 60;
        case 10: return 45;
        case 15: return 30;
        case 20: return 15;
        case 25: return 0;
        default: return 0;
    }
}

weight "ammo_grenades"
{
    switch(INVENTORY_GRENADES)
    {
        case 5: return 50;
        case 10: return 35;
        case 15: return 20;
        case 20: return 10;
        case 25: return 0;
        default: return 0;
    }
}

// ===== HEALTH =====

weight "item_health"
{
    return $evalint(25 * FS_HEALTH);
}

weight "item_health_small"
{
    return $evalint(15 * FS_HEALTH);
}

weight "item_health_large"
{
    return $evalint(50 * FS_HEALTH);
}

weight "item_health_mega"
{
    return $evalint(100 * FS_HEALTH);
}

// ===== ARMOR =====

weight "item_armor_body"
{
    return $evalint(80 * FS_ARMOR);
}

weight "item_armor_combat"
{
    return $evalint(60 * FS_ARMOR);
}

weight "item_armor_jacket"
{
    return $evalint(30 * FS_ARMOR);
}

weight "item_armor_shard"
{
    return $evalint(10 * FS_ARMOR);
}

// ===== POWERUPS =====

weight "item_quad"
{
    return W_QUAD;
}

weight "item_invulnerability"
{
    return W_INVULNERABILITY;
}

weight "item_silencer"
{
    return W_SILENCER;
}

weight "item_breather"
{
    return W_REBREATHER;
}

weight "item_enviro"
{
    return W_ENVIROSUIT;
}

// ===== KEYS (navigation triggers, low weight) =====

weight "key_data_cd"
{
    return 10;
}

weight "key_power_cube"
{
    return 10;
}

weight "key_pyramid"
{
    return 10;
}

weight "key_data_spinner"
{
    return 10;
}

weight "key_pass"
{
    return 10;
}

weight "key_blue_key"
{
    return 10;
}

weight "key_red_key"
{
    return 10;
}

weight "key_commander_head"
{
    return 10;
}

weight "key_airstrike_target"
{
    return 10;
}
