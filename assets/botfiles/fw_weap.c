// Q2 weapon weight template for bot AI.
// Per-bot files #define W_* values then #include this file.
// Mirrors Q3's fw_weap.c but uses Q2 weapons and inventory indices.
//
// Each weapon checks: do we own it? → do we have ammo? → return weight.
// Range-aware weights use ENEMY_HORIZONTAL_DIST (inventory slot 200).
// NOTE: inv.h is NOT included here — the per-bot _w.c file includes it
// before #include-ing this template (matches Q3's original structure).

// Provide defaults so bots without all defines still work.
#ifndef W_BLASTER
#define W_BLASTER           10
#endif
#ifndef W_SHOTGUN
#define W_SHOTGUN           200
#endif
#ifndef W_SUPERSHOTGUN
#define W_SUPERSHOTGUN      250
#endif
#ifndef W_MACHINEGUN
#define W_MACHINEGUN        30
#endif
#ifndef W_CHAINGUN
#define W_CHAINGUN          100
#endif
#ifndef W_GRENADELAUNCHER
#define W_GRENADELAUNCHER   40
#endif
#ifndef W_ROCKETLAUNCHER
#define W_ROCKETLAUNCHER    100
#endif
#ifndef W_HYPERBLASTER
#define W_HYPERBLASTER      80
#endif
#ifndef W_RAILGUN
#define W_RAILGUN           70
#endif
#ifndef W_BFG10K
#define W_BFG10K            95
#endif

// Mission pack weapons. A bot file that does not weigh them gets the weight
// of the base weapon playing the same part, as Gladiator weighed the
// Ionripper like the HyperBlaster and the Phalanx like the Rocket Launcher:
// the ETF Rifle and the Plasma Beam like the HyperBlaster too, the Prox
// Launcher like the Grenade Launcher, the Disruptor like the Railgun. The
// Trap and the Tesla get half the Grenade Launcher's, the Chainfist twice
// the Blaster's. The $evalint defaults must only ever be used as a plain
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
#define W_CHAINFIST         $evalint(W_BLASTER * 2)
#endif
#ifndef W_DISRUPTOR
#define W_DISRUPTOR         W_RAILGUN
#endif
#ifndef W_TESLA
#define W_TESLA             $evalint(W_GRENADELAUNCHER / 2)
#endif

// Blaster: always owned, last resort.
weight "Blaster"
{
    return W_BLASTER;
}

// Shotgun: strong close range, weak far.
weight "Shotgun"
{
    switch(INVENTORY_SHOTGUN)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_SHELLS)
            {
                case 1: return 0;
                default:
                {
                    switch(ENEMY_HORIZONTAL_DIST)
                    {
                        case 200: return W_SHOTGUN;
                        case 500: return $evalint(W_SHOTGUN * 6 / 10);
                        default:  return $evalint(W_SHOTGUN * 2 / 10);
                    }
                }
            }
        }
    }
}

// Super Shotgun: devastating close range, 2 shells a shot.
weight "Super Shotgun"
{
    switch(INVENTORY_SUPERSHOTGUN)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_SHELLS)
            {
                case 2: return 0;
                default:
                {
                    switch(ENEMY_HORIZONTAL_DIST)
                    {
                        case 200: return W_SUPERSHOTGUN;
                        case 400: return $evalint(W_SUPERSHOTGUN * 6 / 10);
                        default:  return $evalint(W_SUPERSHOTGUN * 2 / 10);
                    }
                }
            }
        }
    }
}

// Machinegun: decent all-round.
weight "Machinegun"
{
    switch(INVENTORY_MACHINEGUN)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_BULLETS)
            {
                case 1: return 0;
                default: return W_MACHINEGUN;
            }
        }
    }
}

// Chaingun: excellent sustained damage, eats ammo.
weight "Chaingun"
{
    switch(INVENTORY_CHAINGUN)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_BULLETS)
            {
                case 10: return 0;
                default:
                {
                    switch(ENEMY_HORIZONTAL_DIST)
                    {
                        case 400: return W_CHAINGUN;
                        default:  return $evalint(W_CHAINGUN * 5 / 10);
                    }
                }
            }
        }
    }
}

// Grenade Launcher: area denial, indirect fire.
weight "Grenade Launcher"
{
    switch(INVENTORY_GRENADELAUNCHER)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_GRENADES)
            {
                case 1: return 0;
                default: return W_GRENADELAUNCHER;
            }
        }
    }
}

// Rocket Launcher: high damage, splash.
weight "Rocket Launcher"
{
    switch(INVENTORY_ROCKETLAUNCHER)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_ROCKETS)
            {
                case 1: return 0;
                default:
                {
                    switch(ENEMY_HORIZONTAL_DIST)
                    {
                        case 128: return $evalint(W_ROCKETLAUNCHER * 5 / 10);
                        default:  return W_ROCKETLAUNCHER;
                    }
                }
            }
        }
    }
}

// Hyperblaster: rapid-fire energy weapon.
weight "HyperBlaster"
{
    switch(INVENTORY_HYPERBLASTER)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_CELLS)
            {
                case 1: return 0;
                default:
                {
                    switch(ENEMY_HORIZONTAL_DIST)
                    {
                        case 500: return W_HYPERBLASTER;
                        default:  return $evalint(W_HYPERBLASTER * 5 / 10);
                    }
                }
            }
        }
    }
}

// Railgun: sniper weapon, best at range.
weight "Railgun"
{
    switch(INVENTORY_RAILGUN)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_SLUGS)
            {
                case 1: return 0;
                default:
                {
                    switch(ENEMY_HORIZONTAL_DIST)
                    {
                        case 300: return $evalint(W_RAILGUN * 5 / 10);
                        default:  return W_RAILGUN;
                    }
                }
            }
        }
    }
}

// BFG10K: devastating but expensive, 50 cells a shot.
weight "BFG10K"
{
    switch(INVENTORY_BFG10K)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_CELLS)
            {
                case 50: return 0;
                default: return W_BFG10K;
            }
        }
    }
}

// ===== THE RECKONING =====

// Ionripper: bouncing ion bolts, 2 cells a shot.
weight "Ionripper"
{
    switch(INVENTORY_IONRIPPER)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_CELLS)
            {
                case 2: return 0;
                default:
                {
                    switch(ENEMY_HORIZONTAL_DIST)
                    {
                        case 500: return W_IONRIPPER;
                        default:  return $evalint(W_IONRIPPER * 5 / 10);
                    }
                }
            }
        }
    }
}

// Phalanx: two plasma balls with splash, like the Rocket Launcher.
weight "Phalanx"
{
    switch(INVENTORY_PHALANX)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_MAGSLUGS)
            {
                case 1: return 0;
                default:
                {
                    switch(ENEMY_HORIZONTAL_DIST)
                    {
                        case 128: return $evalint(W_PHALANX * 5 / 10);
                        default:  return W_PHALANX;
                    }
                }
            }
        }
    }
}

// Trap: thrown, pulls in whoever comes within 256 units -- the thrower too.
weight "Trap"
{
    switch(INVENTORY_TRAP)
    {
        case 1: return 0;
        default:
        {
            switch(ENEMY_HORIZONTAL_DIST)
            {
                case 256: return 0;
                default:  return W_TRAP;
            }
        }
    }
}

// ===== GROUND ZERO =====

// ETF Rifle: rapid flechettes that ignore normal armor.
weight "ETF Rifle"
{
    switch(INVENTORY_ETFRIFLE)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_FLECHETTES)
            {
                case 1: return 0;
                default:
                {
                    switch(ENEMY_HORIZONTAL_DIST)
                    {
                        case 600: return W_ETFRIFLE;
                        default:  return $evalint(W_ETFRIFLE * 6 / 10);
                    }
                }
            }
        }
    }
}

// Prox Launcher: lobbed mines that go off when someone comes near.
weight "Prox Launcher"
{
    switch(INVENTORY_PROXLAUNCHER)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_PROX)
            {
                case 1: return 0;
                default:
                {
                    switch(ENEMY_HORIZONTAL_DIST)
                    {
                        case 200: return $evalint(W_PROXLAUNCHER * 3 / 10);
                        case 600: return W_PROXLAUNCHER;
                        default:  return $evalint(W_PROXLAUNCHER * 5 / 10);
                    }
                }
            }
        }
    }
}

// Plasma Beam: hitscan heat beam, 2 cells a frame.
weight "Plasma Beam"
{
    switch(INVENTORY_PLASMABEAM)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_CELLS)
            {
                case 2: return 0;
                default: return W_PLASMABEAM;
            }
        }
    }
}

// Chainfist: melee, needs no ammo. The AI closes in with it (WP_GAUNTLET in
// botlib/ai_q2_compat.h), so by default it weighs just twice the Blaster.
weight "Chainfist"
{
    switch(INVENTORY_CHAINFIST)
    {
        case 1: return 0;
        default: return W_CHAINFIST;
    }
}

// Disruptor: tracker that homes in on the target it was fired at.
weight "Disruptor"
{
    switch(INVENTORY_DISRUPTOR)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_ROUNDS)
            {
                case 1: return 0;
                default: return W_DISRUPTOR;
            }
        }
    }
}

// Tesla: thrown, zaps everyone within 128 units -- the thrower too.
weight "Tesla"
{
    switch(INVENTORY_TESLA)
    {
        case 1: return 0;
        default:
        {
            switch(ENEMY_HORIZONTAL_DIST)
            {
                case 200: return 0;
                default:  return W_TESLA;
            }
        }
    }
}
