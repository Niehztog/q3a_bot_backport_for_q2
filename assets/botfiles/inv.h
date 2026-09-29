// Q2 inventory slot definitions for bot weapon/item weight scripts and the
// ported Q3 AI (game_q3/*.c include this file).
//
// Each slot is the item's itemlist[] index in game_q2/g_items.c (XATRIX,
// ROGUE and ZOID compiled in, see g_local.h) -- that is the index of the
// item's count in Q2's pers.inventory[], which the adapter copies into the
// bot's inventory every frame. Adding or reordering itemlist[] entries
// shifts every slot after them.
//
// Exceptions, written by Q2BotUpdateClient (botlib/be_interface_q2.c):
//   INVENTORY_HEALTH/INVENTORY_ARMOR use the Ancient Head and Adrenaline
//   slots, which Q2 never fills (both take effect on pickup), for STAT_HEALTH
//   and STAT_ARMOR. Slots 200-202 are derived values (below and
//   INVENTORY_BFGAMMO in botlib/ai_q2_compat.h).
//
// Q2 keeps the points of the armour being worn in that armour's slot (only
// one of body/combat/jacket is non-zero); shards add to it and never have a
// count of their own.

// Armor
#define INVENTORY_ARMOR_BODY        1
#define INVENTORY_ARMOR_COMBAT      2
#define INVENTORY_ARMOR_JACKET      3
#define INVENTORY_ARMOR_SHARD       4
#define INVENTORY_POWERSCREEN       5
#define INVENTORY_POWERSHIELD       6

// Weapons
#define INVENTORY_BLASTER           7
#define INVENTORY_SHOTGUN           8
#define INVENTORY_SUPERSHOTGUN      9
#define INVENTORY_MACHINEGUN        10
#define INVENTORY_CHAINGUN          11
#define INVENTORY_GRENADES          12      // ammo and weapon at once
#define INVENTORY_GRENADELAUNCHER   13
#define INVENTORY_ROCKETLAUNCHER    14
#define INVENTORY_HYPERBLASTER      15
#define INVENTORY_RAILGUN           16
#define INVENTORY_BFG10K            17

// Ammo
#define INVENTORY_SHELLS            18
#define INVENTORY_BULLETS           19
#define INVENTORY_CELLS             20
#define INVENTORY_ROCKETS           21
#define INVENTORY_SLUGS             22

// Powerups
#define INVENTORY_QUAD              23
#define INVENTORY_INVULNERABILITY   24
#define INVENTORY_SILENCER          25
#define INVENTORY_REBREATHER        26
#define INVENTORY_ENVIRONMENTSUIT   27

// Health and armor points (see the exceptions above)
#define INVENTORY_HEALTH            28
#define INVENTORY_ARMOR             29

// Ammo capacity
#define INVENTORY_BANDOLIER         30
#define INVENTORY_AMMOPACK          31

// 32-40 single player keys, 41 health, 42 grapple: no counts a bot needs

// CTF
#define INVENTORY_REDFLAG           43
#define INVENTORY_BLUEFLAG          44
#define INVENTORY_TECH1             45      // Disruptor Shield
#define INVENTORY_TECH2             46      // Power Amplifier
#define INVENTORY_TECH3             47      // Time Accel
#define INVENTORY_TECH4             48      // AutoDoc

// The Reckoning (Xatrix)
#define INVENTORY_IONRIPPER         49
#define INVENTORY_PHALANX           50
#define INVENTORY_TRAP              51      // ammo and weapon at once
#define INVENTORY_MAGSLUGS          52
#define INVENTORY_DUALFIREDAMAGE    53
// 54 green key

// Ground Zero (Rogue)
#define INVENTORY_ETFRIFLE          55
#define INVENTORY_PROXLAUNCHER      56
#define INVENTORY_PLASMABEAM        57
#define INVENTORY_CHAINFIST         58
#define INVENTORY_DISRUPTOR         59
#define INVENTORY_FLECHETTES        60
#define INVENTORY_PROX              61
#define INVENTORY_TESLA             62      // ammo and weapon at once
#define INVENTORY_AMBOMB            63
#define INVENTORY_ROUNDS            64
#define INVENTORY_IRGOGGLES         65
#define INVENTORY_DOUBLEDAMAGE      66
#define INVENTORY_COMPASS           67
#define INVENTORY_VENGEANCESPHERE   68
#define INVENTORY_HUNTERSPHERE      69
#define INVENTORY_DEFENDERSPHERE    70
#define INVENTORY_DOPPLEGANGER      71
#define INVENTORY_TAGTOKEN          72
// 73-74 single player keys

// Special bot-logic slots (set by the AI before weapon selection)
#define ENEMY_HORIZONTAL_DIST       200
#define ENEMY_HEIGHT                201
