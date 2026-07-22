// Quake II item configuration for Q3 botlib backport.
// Parsed by be_ai_goal.c::LoadItemConfig().
//
// One iteminfo per item classname a map can hold (base game, The Reckoning,
// Ground Zero, CTF). The classname is also the name of the item's weight in
// the bots' item weight files (fw_items.c, bots/*_i.c).
//
// name:  the item's pickup name (g_items.c itemlist[]) where it has one; shown
//        in bot chat and looked up by name by the AI ("Red Flag", "Quad
//        Damage", ...).
// model: the item's world model exactly as the game registers it
//        (game_q2/g_items.c itemlist[] world_model; SP_item_health* for
//        health). The botlib resolves it to the runtime model index and uses
//        that to recognise item entities: whether a map item is really there,
//        and dropped weapons/ammo as new goals. A wrong path silently
//        disables both for that item. "" for items without a world model.
// respawntime: seconds, as passed to SetRespawn by the item's pickup
//        function in deathmatch: weapons, ammo and health 30, armor 20, all
//        other items their itemlist[] quantity. The botlib avoids an item it
//        has chosen as a goal for that long.
// mins/maxs: +-15, the box droptofloor() gives every item; the botlib drops
//        the items to the floor with it, as the game does.
// type/index: informational, the botlib reads neither.
//        type:  1=weapon 2=ammo 3=armor 4=health 5=powerup 6=flag
//        index: weapon => WEAP_* (g_local.h)  ammo => AMMO_*  armor => ARMOR_*

// -----------------------------------------------------------------------
// WEAPONS  (type 1)
// -----------------------------------------------------------------------

// never placed in maps and cannot be dropped
iteminfo "weapon_blaster"
{
    name        "Blaster"
    model       ""
    modelindex  0
    type        1
    index       1
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_shotgun"
{
    name        "Shotgun"
    model       "models/weapons/g_shotg/tris.md2"
    modelindex  0
    type        1
    index       2
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_supershotgun"
{
    name        "Super Shotgun"
    model       "models/weapons/g_shotg2/tris.md2"
    modelindex  0
    type        1
    index       3
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_machinegun"
{
    name        "Machinegun"
    model       "models/weapons/g_machn/tris.md2"
    modelindex  0
    type        1
    index       4
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_chaingun"
{
    name        "Chaingun"
    model       "models/weapons/g_chain/tris.md2"
    modelindex  0
    type        1
    index       5
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// hand grenades: ammo and weapon at once
iteminfo "ammo_grenades"
{
    name        "Grenades"
    model       "models/items/ammo/grenades/medium/tris.md2"
    modelindex  0
    type        1
    index       6
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_grenadelauncher"
{
    name        "Grenade Launcher"
    model       "models/weapons/g_launch/tris.md2"
    modelindex  0
    type        1
    index       7
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_rocketlauncher"
{
    name        "Rocket Launcher"
    model       "models/weapons/g_rocket/tris.md2"
    modelindex  0
    type        1
    index       8
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_hyperblaster"
{
    name        "HyperBlaster"
    model       "models/weapons/g_hyperb/tris.md2"
    modelindex  0
    type        1
    index       9
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_railgun"
{
    name        "Railgun"
    model       "models/weapons/g_rail/tris.md2"
    modelindex  0
    type        1
    index       10
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_bfg"
{
    name        "BFG10K"
    model       "models/weapons/g_bfg/tris.md2"
    modelindex  0
    type        1
    index       11
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// The Reckoning

iteminfo "weapon_boomer"
{
    name        "Ionripper"
    model       "models/weapons/g_boom/tris.md2"
    modelindex  0
    type        1
    index       13
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_phalanx"
{
    name        "Phalanx"
    model       "models/weapons/g_shotx/tris.md2"
    modelindex  0
    type        1
    index       12
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// Ground Zero

iteminfo "weapon_etf_rifle"
{
    name        "ETF Rifle"
    model       "models/weapons/g_etf_rifle/tris.md2"
    modelindex  0
    type        1
    index       15
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_proxlauncher"
{
    name        "Prox Launcher"
    model       "models/weapons/g_plaunch/tris.md2"
    modelindex  0
    type        1
    index       17
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_plasmabeam"
{
    name        "Plasma Beam"
    model       "models/weapons/g_beamer/tris.md2"
    modelindex  0
    type        1
    index       16
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_chainfist"
{
    name        "Chainfist"
    model       "models/weapons/g_chainf/tris.md2"
    modelindex  0
    type        1
    index       18
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "weapon_disintegrator"
{
    name        "Disruptor"
    model       "models/weapons/g_dist/tris.md2"
    modelindex  0
    type        1
    index       14
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// -----------------------------------------------------------------------
// AMMO  (type 2)
// AMMO_BULLETS=0  AMMO_SHELLS=1  AMMO_ROCKETS=2  AMMO_GRENADES=3
// AMMO_CELLS=4    AMMO_SLUGS=5   AMMO_MAGSLUG=6  AMMO_TRAP=7
// AMMO_FLECHETTES=8  AMMO_TESLA=9  AMMO_PROX=10  AMMO_DISRUPTOR=11
// -----------------------------------------------------------------------

iteminfo "ammo_bullets"
{
    name        "Bullets"
    model       "models/items/ammo/bullets/medium/tris.md2"
    modelindex  0
    type        2
    index       0
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "ammo_shells"
{
    name        "Shells"
    model       "models/items/ammo/shells/medium/tris.md2"
    modelindex  0
    type        2
    index       1
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "ammo_rockets"
{
    name        "Rockets"
    model       "models/items/ammo/rockets/medium/tris.md2"
    modelindex  0
    type        2
    index       2
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "ammo_cells"
{
    name        "Cells"
    model       "models/items/ammo/cells/medium/tris.md2"
    modelindex  0
    type        2
    index       4
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "ammo_slugs"
{
    name        "Slugs"
    model       "models/items/ammo/slugs/medium/tris.md2"
    modelindex  0
    type        2
    index       5
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// The Reckoning

iteminfo "ammo_magslug"
{
    name        "Mag Slug"
    model       "models/objects/ammo/tris.md2"
    modelindex  0
    type        2
    index       6
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// ammo and weapon at once
iteminfo "ammo_trap"
{
    name        "Trap"
    model       "models/weapons/g_trap/tris.md2"
    modelindex  0
    type        2
    index       7
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// Ground Zero

iteminfo "ammo_flechettes"
{
    name        "Flechettes"
    model       "models/ammo/am_flechette/tris.md2"
    modelindex  0
    type        2
    index       8
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "ammo_prox"
{
    name        "Prox"
    model       "models/ammo/am_prox/tris.md2"
    modelindex  0
    type        2
    index       10
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// ammo and weapon at once
iteminfo "ammo_tesla"
{
    name        "Tesla"
    model       "models/ammo/am_tesl/tris.md2"
    modelindex  0
    type        2
    index       9
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "ammo_disruptor"
{
    name        "Rounds"
    model       "models/ammo/am_disr/tris.md2"
    modelindex  0
    type        2
    index       11
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// -----------------------------------------------------------------------
// ARMOR  (type 3)
// ARMOR_JACKET=1  ARMOR_COMBAT=2  ARMOR_BODY=3  ARMOR_SHARD=4
// power armor: 5=screen 6=shield
// -----------------------------------------------------------------------

iteminfo "item_armor_jacket"
{
    name        "Jacket Armor"
    model       "models/items/armor/jacket/tris.md2"
    modelindex  0
    type        3
    index       1
    respawntime 20
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_armor_combat"
{
    name        "Combat Armor"
    model       "models/items/armor/combat/tris.md2"
    modelindex  0
    type        3
    index       2
    respawntime 20
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_armor_body"
{
    name        "Body Armor"
    model       "models/items/armor/body/tris.md2"
    modelindex  0
    type        3
    index       3
    respawntime 20
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_armor_shard"
{
    name        "Armor Shard"
    model       "models/items/armor/shard/tris.md2"
    modelindex  0
    type        3
    index       4
    respawntime 20
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_power_screen"
{
    name        "Power Screen"
    model       "models/items/armor/screen/tris.md2"
    modelindex  0
    type        3
    index       5
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_power_shield"
{
    name        "Power Shield"
    model       "models/items/armor/shield/tris.md2"
    modelindex  0
    type        3
    index       6
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// -----------------------------------------------------------------------
// HEALTH  (type 4)
// index: 1=small(+2)  2=medium(+10)  3=large(+25)  4=mega(+100)
// The mega health respawns 20 seconds after its bonus has worn off.
// -----------------------------------------------------------------------

iteminfo "item_health_small"
{
    name        "Small Health"
    model       "models/items/healing/stimpack/tris.md2"
    modelindex  0
    type        4
    index       1
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_health"
{
    name        "Health"
    model       "models/items/healing/medium/tris.md2"
    modelindex  0
    type        4
    index       2
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_health_large"
{
    name        "Large Health"
    model       "models/items/healing/large/tris.md2"
    modelindex  0
    type        4
    index       3
    respawntime 30
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_health_mega"
{
    name        "Mega Health"
    model       "models/items/mega_h/tris.md2"
    modelindex  0
    type        4
    index       4
    respawntime 20
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// -----------------------------------------------------------------------
// POWERUPS  (type 5)
// -----------------------------------------------------------------------

iteminfo "item_quad"
{
    name        "Quad Damage"
    model       "models/items/quaddama/tris.md2"
    modelindex  0
    type        5
    index       1
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_invulnerability"
{
    name        "Invulnerability"
    model       "models/items/invulner/tris.md2"
    modelindex  0
    type        5
    index       2
    respawntime 300
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_silencer"
{
    name        "Silencer"
    model       "models/items/silencer/tris.md2"
    modelindex  0
    type        5
    index       3
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_breather"
{
    name        "Rebreather"
    model       "models/items/breather/tris.md2"
    modelindex  0
    type        5
    index       4
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_enviro"
{
    name        "Environment Suit"
    model       "models/items/enviro/tris.md2"
    modelindex  0
    type        5
    index       5
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_ancient_head"
{
    name        "Ancient Head"
    model       "models/items/c_head/tris.md2"
    modelindex  0
    type        5
    index       6
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_adrenaline"
{
    name        "Adrenaline"
    model       "models/items/adrenal/tris.md2"
    modelindex  0
    type        5
    index       7
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_bandolier"
{
    name        "Bandolier"
    model       "models/items/band/tris.md2"
    modelindex  0
    type        5
    index       8
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_pack"
{
    name        "Ammo Pack"
    model       "models/items/pack/tris.md2"
    modelindex  0
    type        5
    index       9
    respawntime 180
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// The Reckoning

iteminfo "item_quadfire"
{
    name        "DualFire Damage"
    model       "models/items/quadfire/tris.md2"
    modelindex  0
    type        5
    index       10
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// Ground Zero

iteminfo "item_ir_goggles"
{
    name        "IR Goggles"
    model       "models/items/goggles/tris.md2"
    modelindex  0
    type        5
    index       11
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_double"
{
    name        "Double Damage"
    model       "models/items/ddamage/tris.md2"
    modelindex  0
    type        5
    index       12
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_compass"
{
    name        "compass"
    model       "models/objects/fire/tris.md2"
    modelindex  0
    type        5
    index       13
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_sphere_vengeance"
{
    name        "vengeance sphere"
    model       "models/items/vengnce/tris.md2"
    modelindex  0
    type        5
    index       14
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_sphere_hunter"
{
    name        "hunter sphere"
    model       "models/items/hunter/tris.md2"
    modelindex  0
    type        5
    index       15
    respawntime 120
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_sphere_defender"
{
    name        "defender sphere"
    model       "models/items/defender/tris.md2"
    modelindex  0
    type        5
    index       16
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_doppleganger"
{
    name        "Doppleganger"
    model       "models/items/dopple/tris.md2"
    modelindex  0
    type        5
    index       17
    respawntime 90
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// a powerup despite the classname: used, not fired
iteminfo "ammo_nuke"
{
    name        "A-M Bomb"
    model       "models/weapons/g_nuke/tris.md2"
    modelindex  0
    type        5
    index       18
    respawntime 300
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// Tag deathmatch only; dropped by its carrier, never respawns
iteminfo "dm_tag_token"
{
    name        "Tag Token"
    model       "models/items/tagtoken/tris.md2"
    modelindex  0
    type        5
    index       19
    respawntime 0
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// -----------------------------------------------------------------------
// CTF FLAGS  (type 6)
// -----------------------------------------------------------------------

iteminfo "item_flag_team1"
{
    name        "Red Flag"
    model       "players/male/flag1.md2"
    modelindex  0
    type        6
    index       1
    respawntime 0
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_flag_team2"
{
    name        "Blue Flag"
    model       "players/male/flag2.md2"
    modelindex  0
    type        6
    index       2
    respawntime 0
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// -----------------------------------------------------------------------
// CTF TECH ITEMS
// Spawned at random spawn points rather than placed; found by model.
// -----------------------------------------------------------------------

iteminfo "item_tech1"
{
    name        "Disruptor Shield"
    model       "models/ctf/resistance/tris.md2"
    modelindex  0
    type        5
    index       20
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_tech2"
{
    name        "Power Amplifier"
    model       "models/ctf/strength/tris.md2"
    modelindex  0
    type        5
    index       21
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_tech3"
{
    name        "Time Accel"
    model       "models/ctf/haste/tris.md2"
    modelindex  0
    type        5
    index       22
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

iteminfo "item_tech4"
{
    name        "AutoDoc"
    model       "models/ctf/regeneration/tris.md2"
    modelindex  0
    type        5
    index       23
    respawntime 60
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}

// -----------------------------------------------------------------------
// CTF GRAPPLE WEAPON
// -----------------------------------------------------------------------

// given to every player in CTF, never placed in maps
iteminfo "weapon_grapple"
{
    name        "Grapple"
    model       ""
    modelindex  0
    type        1
    index       0
    respawntime 0
    mins        { -15, -15, -15 }
    maxs        { 15, 15, 15 }
}
