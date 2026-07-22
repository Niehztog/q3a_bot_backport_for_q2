// Q2 item weight template for bot AI.
// Per-bot files #define FS_*/W_*/GWW_* values then #include this file.
// Mirrors Q3's fw_items.c but uses Q2 items and inventory indices.
//
// Weight names are the classnames of the iteminfo entries in botfiles/items.c.
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

// Mission pack weapons. A bot file that does not weigh them gets the weights
// of the base weapon playing the same part, as Gladiator weighed the
// Ionripper like the HyperBlaster and the Phalanx like the Rocket Launcher:
// the ETF Rifle and the Plasma Beam like the HyperBlaster too, the Prox
// Launcher like the Grenade Launcher, the Disruptor like the Railgun. The
// Trap and the Tesla get half the Grenade Launcher's, the Chainfist half the
// Shotgun's. The $evalint defaults must only ever be used as a plain
// "return W_X;" -- an $evalint inside another does not parse.
#ifndef W_IONRIPPER
#define W_IONRIPPER         W_HYPERBLASTER
#endif
#ifndef W_PHALANX
#define W_PHALANX           W_ROCKETLAUNCHER
#endif
#ifndef W_TRAP
#define W_TRAP              $evalint(W_GRENADELAUNCHER / 2)
#endif
#ifndef W_ETFRIFLE
#define W_ETFRIFLE          W_HYPERBLASTER
#endif
#ifndef W_PROXLAUNCHER
#define W_PROXLAUNCHER      W_GRENADELAUNCHER
#endif
#ifndef W_PLASMABEAM
#define W_PLASMABEAM        W_HYPERBLASTER
#endif
#ifndef W_CHAINFIST
#define W_CHAINFIST         $evalint(W_SHOTGUN / 2)
#endif
#ifndef W_DISRUPTOR
#define W_DISRUPTOR         W_RAILGUN
#endif
#ifndef W_TESLA
#define W_TESLA             $evalint(W_GRENADELAUNCHER / 2)
#endif

#ifndef GWW_IONRIPPER
#define GWW_IONRIPPER       GWW_HYPERBLASTER
#endif
#ifndef GWW_PHALANX
#define GWW_PHALANX         GWW_ROCKETLAUNCHER
#endif
#ifndef GWW_TRAP
#define GWW_TRAP            $evalint(GWW_GRENADELAUNCHER / 2)
#endif
#ifndef GWW_ETFRIFLE
#define GWW_ETFRIFLE        GWW_HYPERBLASTER
#endif
#ifndef GWW_PROXLAUNCHER
#define GWW_PROXLAUNCHER    GWW_GRENADELAUNCHER
#endif
#ifndef GWW_PLASMABEAM
#define GWW_PLASMABEAM      GWW_HYPERBLASTER
#endif
#ifndef GWW_DISRUPTOR
#define GWW_DISRUPTOR       GWW_RAILGUN
#endif
#ifndef GWW_TESLA
#define GWW_TESLA           $evalint(GWW_GRENADELAUNCHER / 2)
#endif

// Other item weights
#ifndef W_POWERSCREEN
#define W_POWERSCREEN       $evalint(50 * FS_ARMOR)
#endif
#ifndef W_POWERSHIELD
#define W_POWERSHIELD       $evalint(70 * FS_ARMOR)
#endif
#ifndef W_BANDOLIER
#define W_BANDOLIER         60
#endif
#ifndef W_AMMOPACK
#define W_AMMOPACK          80
#endif
#ifndef W_DUALFIRE
#define W_DUALFIRE          $evalint(W_QUAD * 7 / 10)
#endif
#ifndef W_IRGOGGLES
#define W_IRGOGGLES         W_SILENCER
#endif
#ifndef W_DOUBLEDAMAGE
#define W_DOUBLEDAMAGE      $evalint(W_QUAD / 2)
#endif
#ifndef W_COMPASS
#define W_COMPASS           10
#endif
#ifndef W_SPHERE
#define W_SPHERE            $evalint(W_QUAD / 2)
#endif
#ifndef W_DOPPLEGANGER
#define W_DOPPLEGANGER      $evalint(W_QUAD / 2)
#endif
#ifndef W_AMBOMB
#define W_AMBOMB            $evalint(W_QUAD / 4)
#endif
#ifndef W_TAGTOKEN
#define W_TAGTOKEN          W_QUAD
#endif
#ifndef W_TECH
#define W_TECH              $evalint(W_QUAD / 2)
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

// The Reckoning

// Ionripper
weight "weapon_boomer"
{
    switch(INVENTORY_IONRIPPER)
    {
        case 1: return W_IONRIPPER;
        default:
        {
            switch(INVENTORY_CELLS)
            {
                case 50: return GWW_IONRIPPER;
                case 80: return $evalint(GWW_IONRIPPER / 2);
                case 100: return 0;
                default: return 0;
            }
        }
    }
}

// Phalanx
weight "weapon_phalanx"
{
    switch(INVENTORY_PHALANX)
    {
        case 1: return W_PHALANX;
        default:
        {
            switch(INVENTORY_MAGSLUGS)
            {
                case 10: return GWW_PHALANX;
                case 20: return $evalint(GWW_PHALANX / 2);
                case 25: return 0;
                default: return 0;
            }
        }
    }
}

// Ground Zero

// ETF Rifle
weight "weapon_etf_rifle"
{
    switch(INVENTORY_ETFRIFLE)
    {
        case 1: return W_ETFRIFLE;
        default:
        {
            switch(INVENTORY_FLECHETTES)
            {
                case 50: return GWW_ETFRIFLE;
                case 80: return $evalint(GWW_ETFRIFLE / 2);
                case 100: return 0;
                default: return 0;
            }
        }
    }
}

// Prox Launcher
weight "weapon_proxlauncher"
{
    switch(INVENTORY_PROXLAUNCHER)
    {
        case 1: return W_PROXLAUNCHER;
        default:
        {
            switch(INVENTORY_PROX)
            {
                case 10: return GWW_PROXLAUNCHER;
                case 20: return $evalint(GWW_PROXLAUNCHER / 2);
                case 25: return 0;
                default: return 0;
            }
        }
    }
}

// Plasma Beam
weight "weapon_plasmabeam"
{
    switch(INVENTORY_PLASMABEAM)
    {
        case 1: return W_PLASMABEAM;
        default:
        {
            switch(INVENTORY_CELLS)
            {
                case 50: return GWW_PLASMABEAM;
                case 80: return $evalint(GWW_PLASMABEAM / 2);
                case 100: return 0;
                default: return 0;
            }
        }
    }
}

// Chainfist: no ammo, so worthless once owned
weight "weapon_chainfist"
{
    switch(INVENTORY_CHAINFIST)
    {
        case 1: return W_CHAINFIST;
        default: return 0;
    }
}

// Disruptor
weight "weapon_disintegrator"
{
    switch(INVENTORY_DISRUPTOR)
    {
        case 1: return W_DISRUPTOR;
        default:
        {
            switch(INVENTORY_ROUNDS)
            {
                case 15: return GWW_DISRUPTOR;
                case 30: return $evalint(GWW_DISRUPTOR / 2);
                case 40: return 0;
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

// The Reckoning

// Mag Slug: Phalanx ammo, 10 a pickup, 50 at most
weight "ammo_magslug"
{
    switch(INVENTORY_MAGSLUGS)
    {
        case 5: return 60;
        case 10: return 45;
        case 15: return 30;
        case 20: return 15;
        case 25: return 0;
        default: return 0;
    }
}

// Trap: ammo and weapon at once, 5 at most
weight "ammo_trap"
{
    switch(INVENTORY_TRAP)
    {
        case 1: return W_TRAP;
        case 3: return GWW_TRAP;
        default: return 0;
    }
}

// Ground Zero

// Flechettes: ETF Rifle ammo, 50 a pickup, 200 at most
weight "ammo_flechettes"
{
    switch(INVENTORY_FLECHETTES)
    {
        case 30: return 50;
        case 50: return 35;
        case 80: return 15;
        case 100: return 0;
        default: return 0;
    }
}

// Prox: Prox Launcher ammo, 5 a pickup, 50 at most
weight "ammo_prox"
{
    switch(INVENTORY_PROX)
    {
        case 5: return 50;
        case 10: return 35;
        case 15: return 20;
        case 20: return 10;
        case 25: return 0;
        default: return 0;
    }
}

// Tesla: ammo and weapon at once, 5 a pickup, 50 at most
weight "ammo_tesla"
{
    switch(INVENTORY_TESLA)
    {
        case 1: return W_TESLA;
        case 15: return GWW_TESLA;
        default: return 0;
    }
}

// Rounds: Disruptor ammo, 15 a pickup, 100 at most
weight "ammo_disruptor"
{
    switch(INVENTORY_ROUNDS)
    {
        case 10: return 60;
        case 20: return 45;
        case 30: return 30;
        case 40: return 15;
        case 50: return 0;
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

// Adrenaline: heals to full health
weight "item_adrenaline"
{
    return $evalint(40 * FS_HEALTH);
}

// Ancient Head: +2 maximum health
weight "item_ancient_head"
{
    return $evalint(15 * FS_HEALTH);
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

// Power armor: switched on at pickup in deathmatch, runs on cells. A second
// one does nothing.
weight "item_power_screen"
{
    switch(INVENTORY_POWERSCREEN)
    {
        case 1: return W_POWERSCREEN;
        default: return 0;
    }
}

weight "item_power_shield"
{
    switch(INVENTORY_POWERSHIELD)
    {
        case 1: return W_POWERSHIELD;
        default: return 0;
    }
}

// ===== AMMO CAPACITY =====
// Both raise the ammo limits and bring ammo; the game counts them in the
// inventory, so a second one is worth its ammo only.

weight "item_bandolier"
{
    switch(INVENTORY_BANDOLIER)
    {
        case 1: return W_BANDOLIER;
        default: return 20;
    }
}

weight "item_pack"
{
    switch(INVENTORY_AMMOPACK)
    {
        case 1: return W_AMMOPACK;
        default: return 25;
    }
}

// ===== POWERUPS =====
// Q2 keeps these in the inventory until they are used (unless dmflags has
// instant items) and refuses a second one of a kind at skill 2 and above
// (Pickup_Powerup), so an item the bot already holds weighs nothing.

weight "item_quad"
{
    switch(INVENTORY_QUAD)
    {
        case 1: return W_QUAD;
        default: return 0;
    }
}

weight "item_invulnerability"
{
    switch(INVENTORY_INVULNERABILITY)
    {
        case 1: return W_INVULNERABILITY;
        default: return 0;
    }
}

weight "item_silencer"
{
    switch(INVENTORY_SILENCER)
    {
        case 1: return W_SILENCER;
        default: return 0;
    }
}

weight "item_breather"
{
    switch(INVENTORY_REBREATHER)
    {
        case 1: return W_REBREATHER;
        default: return 0;
    }
}

weight "item_enviro"
{
    switch(INVENTORY_ENVIRONMENTSUIT)
    {
        case 1: return W_ENVIROSUIT;
        default: return 0;
    }
}

// The Reckoning

// DualFire Damage: doubles the rate of fire
weight "item_quadfire"
{
    switch(INVENTORY_DUALFIREDAMAGE)
    {
        case 1: return W_DUALFIRE;
        default: return 0;
    }
}

// Ground Zero

weight "item_ir_goggles"
{
    switch(INVENTORY_IRGOGGLES)
    {
        case 1: return W_IRGOGGLES;
        default: return 0;
    }
}

weight "item_double"
{
    switch(INVENTORY_DOUBLEDAMAGE)
    {
        case 1: return W_DOUBLEDAMAGE;
        default: return 0;
    }
}

// points the way in coop, no use in deathmatch
weight "item_compass"
{
    switch(INVENTORY_COMPASS)
    {
        case 1: return W_COMPASS;
        default: return 0;
    }
}

weight "item_sphere_vengeance"
{
    switch(INVENTORY_VENGEANCESPHERE)
    {
        case 1: return W_SPHERE;
        default: return 0;
    }
}

weight "item_sphere_hunter"
{
    switch(INVENTORY_HUNTERSPHERE)
    {
        case 1: return W_SPHERE;
        default: return 0;
    }
}

weight "item_sphere_defender"
{
    switch(INVENTORY_DEFENDERSPHERE)
    {
        case 1: return W_SPHERE;
        default: return 0;
    }
}

weight "item_doppleganger"
{
    switch(INVENTORY_DOPPLEGANGER)
    {
        case 1: return W_DOPPLEGANGER;
        default: return 0;
    }
}

// A-M Bomb: one at most
weight "ammo_nuke"
{
    switch(INVENTORY_AMBOMB)
    {
        case 1: return W_AMBOMB;
        default: return 0;
    }
}

// Tag deathmatch: its carrier scores double
weight "dm_tag_token"
{
    switch(INVENTORY_TAGTOKEN)
    {
        case 1: return W_TAGTOKEN;
        default: return 0;
    }
}

// ===== CTF TECHS =====
// A player carries one tech at most (CTFPickup_Tech).

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
    TECH_WEIGHT(W_TECH)
}

weight "item_tech2"
{
    TECH_WEIGHT(W_TECH)
}

weight "item_tech3"
{
    TECH_WEIGHT(W_TECH)
}

weight "item_tech4"
{
    TECH_WEIGHT(W_TECH)
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
