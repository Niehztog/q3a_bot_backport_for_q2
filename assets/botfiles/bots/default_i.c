// Default item weight configuration for Q3 botlib backport to Quake II.
// Loaded by BotLoadItemWeights() via be_ai_goal.c.
//
// Weight names are the item classnames in botfiles/items.c.
//
// FuzzyWeight evaluates:  if (inventory[switch_index] < case_value) return weight;
// So "case 1: return W" means "if inventory[N] == 0 (don't have it): return W".
// The switch indices are the inventory slots from inv.h (Q2 itemlist[]
// positions, NOT WEAP_* enum values).
//
// -----------------------------------------------------------------------
// SCORING CALIBRATION
// -----------------------------------------------------------------------
// Travel time is in hundredths of a second (AAS travel time). The scoring
// formula is:
//   effective_score = base_weight / (travel_time * 0.01)
//
// For a weapon to reliably beat nearby health/ammo, it must score higher
// than any reachable consumable at close range.  Target balance:
//   - Regular health (weight 25) at t=100 (1 s)     -> score  25
//   - RL            (weight 5000) at t=5000 (50 s)  -> score 100  (wins 4:1)
//   - RL            (weight 5000) at t=10000 (100 s)-> score  50  (still wins 2:1)
// Weapon weights are set 60-300x consumable weights so they dominate the
// goal selection budget even far away.

#include "inv.h"

// -----------------------------------------------------------------------
// WEAPONS  — high weight when not owned; 0 when already carrying one
// -----------------------------------------------------------------------

// Blaster is always owned; never present in the world as a pickup.
weight "weapon_blaster"
    return 0;

weight "weapon_shotgun"
{
    switch(INVENTORY_SHOTGUN)
    {
        case 1: return 2000;
        default: return 0;
    }
}

weight "weapon_supershotgun"
{
    switch(INVENTORY_SUPERSHOTGUN)
    {
        case 1: return 2500;
        default: return 0;
    }
}

weight "weapon_machinegun"
{
    switch(INVENTORY_MACHINEGUN)
    {
        case 1: return 2000;
        default: return 0;
    }
}

weight "weapon_chaingun"
{
    switch(INVENTORY_CHAINGUN)
    {
        case 1: return 3000;
        default: return 0;
    }
}

weight "weapon_grenadelauncher"
{
    switch(INVENTORY_GRENADELAUNCHER)
    {
        case 1: return 4000;
        default: return 0;
    }
}

weight "weapon_rocketlauncher"
{
    switch(INVENTORY_ROCKETLAUNCHER)
    {
        case 1: return 5000;
        default: return 0;
    }
}

weight "weapon_hyperblaster"
{
    switch(INVENTORY_HYPERBLASTER)
    {
        case 1: return 3500;
        default: return 0;
    }
}

weight "weapon_railgun"
{
    switch(INVENTORY_RAILGUN)
    {
        case 1: return 4500;
        default: return 0;
    }
}

weight "weapon_bfg"
{
    switch(INVENTORY_BFG10K)
    {
        case 1: return 7000;
        default: return 0;
    }
}

// The Reckoning

// Ionripper
weight "weapon_boomer"
{
    switch(INVENTORY_IONRIPPER)
    {
        case 1: return 2500;
        default: return 0;
    }
}

weight "weapon_phalanx"
{
    switch(INVENTORY_PHALANX)
    {
        case 1: return 3000;
        default: return 0;
    }
}

// Ground Zero

weight "weapon_etf_rifle"
{
    switch(INVENTORY_ETFRIFLE)
    {
        case 1: return 3500;
        default: return 0;
    }
}

weight "weapon_proxlauncher"
{
    switch(INVENTORY_PROXLAUNCHER)
    {
        case 1: return 4000;
        default: return 0;
    }
}

weight "weapon_plasmabeam"
{
    switch(INVENTORY_PLASMABEAM)
    {
        case 1: return 3200;
        default: return 0;
    }
}

weight "weapon_chainfist"
{
    switch(INVENTORY_CHAINFIST)
    {
        case 1: return 1500;
        default: return 0;
    }
}

// Disruptor
weight "weapon_disintegrator"
{
    switch(INVENTORY_DISRUPTOR)
    {
        case 1: return 3800;
        default: return 0;
    }
}

// -----------------------------------------------------------------------
// AMMO  — low flat weight; bot picks these up opportunistically via NBG
// (nearby-goal system, radius 400 units) rather than making long detours.
// Keep weights low so they don't outcompete weapons in LTG selection.
// -----------------------------------------------------------------------

weight "ammo_grenades"
    return 30;

weight "ammo_bullets"
    return 20;

weight "ammo_shells"
    return 20;

weight "ammo_rockets"
    return 50;

weight "ammo_cells"
    return 40;

weight "ammo_slugs"
    return 55;

// The Reckoning

weight "ammo_magslug"
    return 50;

// Trap: ammo and weapon at once
weight "ammo_trap"
    return 30;

// Ground Zero

weight "ammo_flechettes"
    return 35;

weight "ammo_prox"
    return 40;

// Tesla: ammo and weapon at once
weight "ammo_tesla"
    return 25;

weight "ammo_disruptor"
    return 35;

// -----------------------------------------------------------------------
// ARMOR  — low flat weight; bot collects en-route via NBG, not LTG detours.
// Kept well below weapon weights so armor nearby never beats a distant weapon.
// -----------------------------------------------------------------------

weight "item_armor_shard"
    return 10;

weight "item_armor_jacket"
    return 40;

weight "item_armor_combat"
    return 60;

weight "item_armor_body"
    return 100;

// Power armor is switched on at pickup in deathmatch; a second one does
// nothing.
weight "item_power_screen"
{
    switch(INVENTORY_POWERSCREEN)
    {
        case 1: return 50;
        default: return 0;
    }
}

weight "item_power_shield"
{
    switch(INVENTORY_POWERSHIELD)
    {
        case 1: return 80;
        default: return 0;
    }
}

// -----------------------------------------------------------------------
// HEALTH  — low flat weight; picked up via NBG on the way to weapon goals.
// -----------------------------------------------------------------------

weight "item_health_small"
    return 15;

weight "item_health"
    return 25;

weight "item_health_large"
    return 35;

weight "item_health_mega"
    return 200;

// Adrenaline: heals to full health
weight "item_adrenaline"
    return 150;

// Ancient Head: +2 maximum health
weight "item_ancient_head"
    return 20;

// -----------------------------------------------------------------------
// AMMO CAPACITY  — a second one is worth its ammo only
// -----------------------------------------------------------------------

weight "item_bandolier"
{
    switch(INVENTORY_BANDOLIER)
    {
        case 1: return 100;
        default: return 30;
    }
}

weight "item_pack"
{
    switch(INVENTORY_AMMOPACK)
    {
        case 1: return 150;
        default: return 40;
    }
}

// -----------------------------------------------------------------------
// POWERUPS  — kept high; these are rare and game-changing
//
// Q2 keeps them in the inventory until they are used (unless dmflags has
// instant items) and refuses a second one of a kind at skill 2 and above,
// so an item the bot already holds weighs nothing.
// -----------------------------------------------------------------------

weight "item_quad"
{
    switch(INVENTORY_QUAD)
    {
        case 1: return 600;
        default: return 0;
    }
}

weight "item_invulnerability"
{
    switch(INVENTORY_INVULNERABILITY)
    {
        case 1: return 500;
        default: return 0;
    }
}

weight "item_silencer"
{
    switch(INVENTORY_SILENCER)
    {
        case 1: return 80;
        default: return 0;
    }
}

weight "item_breather"
{
    switch(INVENTORY_REBREATHER)
    {
        case 1: return 40;
        default: return 0;
    }
}

weight "item_enviro"
{
    switch(INVENTORY_ENVIRONMENTSUIT)
    {
        case 1: return 40;
        default: return 0;
    }
}

// The Reckoning

// DualFire Damage
weight "item_quadfire"
{
    switch(INVENTORY_DUALFIREDAMAGE)
    {
        case 1: return 500;
        default: return 0;
    }
}

// Ground Zero

weight "item_ir_goggles"
{
    switch(INVENTORY_IRGOGGLES)
    {
        case 1: return 150;
        default: return 0;
    }
}

weight "item_double"
{
    switch(INVENTORY_DOUBLEDAMAGE)
    {
        case 1: return 500;
        default: return 0;
    }
}

weight "item_compass"
{
    switch(INVENTORY_COMPASS)
    {
        case 1: return 50;
        default: return 0;
    }
}

weight "item_sphere_vengeance"
{
    switch(INVENTORY_VENGEANCESPHERE)
    {
        case 1: return 400;
        default: return 0;
    }
}

weight "item_sphere_hunter"
{
    switch(INVENTORY_HUNTERSPHERE)
    {
        case 1: return 400;
        default: return 0;
    }
}

weight "item_sphere_defender"
{
    switch(INVENTORY_DEFENDERSPHERE)
    {
        case 1: return 400;
        default: return 0;
    }
}

weight "item_doppleganger"
{
    switch(INVENTORY_DOPPLEGANGER)
    {
        case 1: return 350;
        default: return 0;
    }
}

// A-M Bomb
weight "ammo_nuke"
{
    switch(INVENTORY_AMBOMB)
    {
        case 1: return 60;
        default: return 0;
    }
}

// Tag deathmatch: its carrier scores double
weight "dm_tag_token"
{
    switch(INVENTORY_TAGTOKEN)
    {
        case 1: return 600;
        default: return 0;
    }
}

// -----------------------------------------------------------------------
// CTF TECHS  — a player carries one tech at most
// -----------------------------------------------------------------------

#define TECH_WEIGHT(w) \
    switch(INVENTORY_TECH1) \
    { \
        case 1: \
        { \
            switch(INVENTORY_TECH2) \
            { \
                case 1: \
                { \
                    switch(INVENTORY_TECH3) \
                    { \
                        case 1: \
                        { \
                            switch(INVENTORY_TECH4) \
                            { \
                                case 1: return w; \
                                default: return 0; \
                            } \
                        } \
                        default: return 0; \
                    } \
                } \
                default: return 0; \
            } \
        } \
        default: return 0; \
    }

weight "item_tech1"
{
    TECH_WEIGHT(400)
}

weight "item_tech2"
{
    TECH_WEIGHT(400)
}

weight "item_tech3"
{
    TECH_WEIGHT(400)
}

weight "item_tech4"
{
    TECH_WEIGHT(400)
}
