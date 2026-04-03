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

// Super Shotgun: devastating close range.
weight "Super Shotgun"
{
    switch(INVENTORY_SUPERSHOTGUN)
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

// BFG10K: devastating but expensive.
weight "BFG10K"
{
    switch(INVENTORY_BFG10K)
    {
        case 1: return 0;
        default:
        {
            switch(INVENTORY_CELLS)
            {
                case 30: return 0;
                default: return W_BFG10K;
            }
        }
    }
}
