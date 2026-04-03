// Q2 inventory slot definitions for bot weapon/item weight scripts.
// Indices match g_items.c itemlist[] in the Q2 game DLL.

// Weapons (inventory slot = itemlist index)
#define INVENTORY_BLASTER           7
#define INVENTORY_SHOTGUN           8
#define INVENTORY_SUPERSHOTGUN      9
#define INVENTORY_MACHINEGUN        10
#define INVENTORY_CHAINGUN          11
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
#define INVENTORY_GRENADES          12

// Powerups
#define INVENTORY_QUAD              23
#define INVENTORY_INVULNERABILITY   24
#define INVENTORY_SILENCER          25
#define INVENTORY_REBREATHER        26
#define INVENTORY_ENVIRONMENTSUIT   27

// Health items (tracked by type, not slot)
#define INVENTORY_HEALTH            28

// Armor
#define INVENTORY_ARMOR_JACKET      32
#define INVENTORY_ARMOR_COMBAT      33
#define INVENTORY_ARMOR_BODY        34
#define INVENTORY_ARMOR_SHARD       35

// Special bot-logic slots (set by adapter before weapon selection)
#define ENEMY_HORIZONTAL_DIST       200
#define ENEMY_HEIGHT                201
