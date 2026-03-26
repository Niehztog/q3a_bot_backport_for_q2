// Quake II item configuration for Q3 botlib backport.
// Parsed by be_ai_goal.c::LoadItemConfig().
//
// type:  1=IT_WEAPON  2=IT_AMMO  3=IT_ARMOR  4=IT_HEALTH  5=IT_POWERUP
// index: weapon => WEAP_*  ammo => AMMO_*  armor => ARMOR_*
// modelindex: 0 = matched by classname in BSP only (dynamic model indexes)

// -----------------------------------------------------------------------
// WEAPONS  (type 1)
// -----------------------------------------------------------------------

iteminfo "weapon_blaster"
{
    name        "Blaster"
    model       "models/weapons/v_blast/tris.md2"
    modelindex  0
    type        1
    index       1
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_shotgun"
{
    name        "Shotgun"
    model       "models/weapons/g_shotg/tris.md2"
    modelindex  0
    type        1
    index       2
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_supershotgun"
{
    name        "Super Shotgun"
    model       "models/weapons/g_shotg2/tris.md2"
    modelindex  0
    type        1
    index       3
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_machinegun"
{
    name        "Machinegun"
    model       "models/weapons/g_machn/tris.md2"
    modelindex  0
    type        1
    index       4
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_chaingun"
{
    name        "Chaingun"
    model       "models/weapons/g_chain/tris.md2"
    modelindex  0
    type        1
    index       5
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "ammo_grenades"
{
    name        "Grenades"
    model       "models/items/ammo/grenades/medium/tris.md2"
    modelindex  0
    type        1
    index       6
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_grenadelauncher"
{
    name        "Grenade Launcher"
    model       "models/weapons/g_launch/tris.md2"
    modelindex  0
    type        1
    index       7
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_rocketlauncher"
{
    name        "Rocket Launcher"
    model       "models/weapons/g_rocket/tris.md2"
    modelindex  0
    type        1
    index       8
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_hyperblaster"
{
    name        "HyperBlaster"
    model       "models/weapons/g_hyperb/tris.md2"
    modelindex  0
    type        1
    index       9
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_railgun"
{
    name        "Railgun"
    model       "models/weapons/g_rail/tris.md2"
    modelindex  0
    type        1
    index       10
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_bfg"
{
    name        "BFG10K"
    model       "models/weapons/g_bfg/tris.md2"
    modelindex  0
    type        1
    index       11
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

// -----------------------------------------------------------------------
// AMMO  (type 2)
// AMMO_BULLETS=0  AMMO_SHELLS=1  AMMO_ROCKETS=2  AMMO_GRENADES=3
// AMMO_CELLS=4    AMMO_SLUGS=5
// -----------------------------------------------------------------------

iteminfo "ammo_bullets"
{
    name        "Bullets"
    model       "models/items/ammo/bullets/medium/tris.md2"
    modelindex  0
    type        2
    index       0
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "ammo_shells"
{
    name        "Shells"
    model       "models/items/ammo/shells/medium/tris.md2"
    modelindex  0
    type        2
    index       1
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "ammo_rockets"
{
    name        "Rockets"
    model       "models/items/ammo/rockets/medium/tris.md2"
    modelindex  0
    type        2
    index       2
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "ammo_cells"
{
    name        "Cells"
    model       "models/items/ammo/cells/medium/tris.md2"
    modelindex  0
    type        2
    index       4
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "ammo_slugs"
{
    name        "Slugs"
    model       "models/items/ammo/slugs/medium/tris.md2"
    modelindex  0
    type        2
    index       5
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

// -----------------------------------------------------------------------
// ARMOR  (type 3)
// ARMOR_JACKET=1  ARMOR_COMBAT=2  ARMOR_BODY=3  ARMOR_SHARD=4
// -----------------------------------------------------------------------

iteminfo "item_armor_jacket"
{
    name        "Jacket Armor"
    model       "models/items/armor/jacket/tris.md2"
    modelindex  0
    type        3
    index       1
    respawntime 20
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_armor_combat"
{
    name        "Combat Armor"
    model       "models/items/armor/combat/tris.md2"
    modelindex  0
    type        3
    index       2
    respawntime 20
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_armor_body"
{
    name        "Body Armor"
    model       "models/items/armor/body/tris.md2"
    modelindex  0
    type        3
    index       3
    respawntime 20
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_armor_shard"
{
    name        "Armor Shard"
    model       "models/items/armor/shard/tris.md2"
    modelindex  0
    type        3
    index       4
    respawntime 20
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_power_screen"
{
    name        "Power Screen"
    model       "models/items/armor/screen/tris.md2"
    modelindex  0
    type        3
    index       5
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_power_shield"
{
    name        "Power Shield"
    model       "models/items/armor/shield/tris.md2"
    modelindex  0
    type        3
    index       6
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

// -----------------------------------------------------------------------
// HEALTH  (type 4)
// index: 1=small(+2)  2=medium(+10)  3=large(+25)  4=mega(+100)
// -----------------------------------------------------------------------

iteminfo "item_health_small"
{
    name        "Small Health"
    model       "models/items/healing/stimpack/tris.md2"
    modelindex  0
    type        4
    index       1
    respawntime 20
    mins        { -8, -8, -8 }
    maxs        { 8, 8, 8 }
}

iteminfo "item_health"
{
    name        "Health"
    model       "models/items/healing/medium/tris.md2"
    modelindex  0
    type        4
    index       2
    respawntime 20
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_health_large"
{
    name        "Large Health"
    model       "models/items/healing/large/tris.md2"
    modelindex  0
    type        4
    index       3
    respawntime 20
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_health_mega"
{
    name        "Mega Health"
    model       "models/items/mega_h/tris.md2"
    modelindex  0
    type        4
    index       4
    respawntime 20
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
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
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_invulnerability"
{
    name        "Invulnerability"
    model       "models/items/invulner/tris.md2"
    modelindex  0
    type        5
    index       2
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_silencer"
{
    name        "Silencer"
    model       "models/items/silencer/tris.md2"
    modelindex  0
    type        5
    index       3
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_breather"
{
    name        "Rebreather"
    model       "models/items/breather/tris.md2"
    modelindex  0
    type        5
    index       4
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_enviro"
{
    name        "Environment Suit"
    model       "models/items/envirosuit/tris.md2"
    modelindex  0
    type        5
    index       5
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_adrenaline"
{
    name        "Adrenaline"
    model       "models/items/adrenal/tris.md2"
    modelindex  0
    type        5
    index       6
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_bandolier"
{
    name        "Bandolier"
    model       "models/items/band/tris.md2"
    modelindex  0
    type        5
    index       7
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_pack"
{
    name        "Ammo Pack"
    model       "models/items/pack/tris.md2"
    modelindex  0
    type        5
    index       8
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

// -----------------------------------------------------------------------
// ROGUE / XATRIX MISSION PACK extras
//
// index field = WEAP_* or AMMO_* enum value (used by game DLL for weapon
// slot / ammo type identification, distinct from itemlist[] position).
// The itemlist[] positions (used by the inventory array in weight configs)
// are documented in comments.
//   itemlist: boomer=50  phalanx=51  etf_rifle=56  proxlauncher=57
//             plasmabeam=58  chainfist=59  disintegrator=60
//   ammo:     ammo_trap=52  ammo_magslug=53  ammo_flechettes=61
//             ammo_prox=62  ammo_tesla=63  ammo_nuke=64  ammo_disruptor=65
//   powerups: item_quadfire=54  item_ir_goggles=66  item_double=67
//             item_sphere_vengeance=70  item_sphere_hunter=71
//             item_sphere_defender=72  item_doppleganger=73
// -----------------------------------------------------------------------

iteminfo "weapon_boomer"
{
    name        "Boomer"
    model       "models/weapons/g_shotg2/tris.md2"
    modelindex  0
    type        1
    index       13
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_phalanx"
{
    name        "Phalanx"
    model       "models/weapons/g_shotg2/tris.md2"
    modelindex  0
    type        1
    index       12
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_etf_rifle"
{
    name        "ETF Rifle"
    model       "models/weapons/g_rail/tris.md2"
    modelindex  0
    type        1
    index       15
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_proxlauncher"
{
    name        "Prox Launcher"
    model       "models/weapons/g_launch/tris.md2"
    modelindex  0
    type        1
    index       17
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_plasmabeam"
{
    name        "Plasma Beam"
    model       "models/weapons/g_hyperb/tris.md2"
    modelindex  0
    type        1
    index       16
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_chainfist"
{
    name        "Chainfist"
    model       "models/weapons/g_chain/tris.md2"
    modelindex  0
    type        1
    index       18
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "weapon_disintegrator"
{
    name        "Disintegrator"
    model       "models/weapons/g_rail/tris.md2"
    modelindex  0
    type        1
    index       19
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

// Rogue / Xatrix ammo types
iteminfo "ammo_flechettes"
{
    name        "Flechettes"
    model       "models/items/ammo/flechettes/medium/tris.md2"
    modelindex  0
    type        2
    index       6
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "ammo_prox"
{
    name        "Prox Mines"
    model       "models/items/ammo/rockets/medium/tris.md2"
    modelindex  0
    type        2
    index       7
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "ammo_tesla"
{
    name        "Tesla Mines"
    model       "models/items/ammo/grenades/medium/tris.md2"
    modelindex  0
    type        2
    index       8
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "ammo_nuke"
{
    name        "Nuke"
    model       "models/items/ammo/rockets/medium/tris.md2"
    modelindex  0
    type        2
    index       9
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "ammo_disruptor"
{
    name        "Rounds"
    model       "models/items/ammo/slugs/medium/tris.md2"
    modelindex  0
    type        2
    index       10
    respawntime 30
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

// Rogue powerups
iteminfo "item_quadfire"
{
    name        "Double Damage"
    model       "models/items/quaddama/tris.md2"
    modelindex  0
    type        5
    index       9
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_ir_goggles"
{
    name        "IR Goggles"
    model       "models/items/goggles/tris.md2"
    modelindex  0
    type        5
    index       10
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_double"
{
    name        "Double Damage"
    model       "models/items/quaddama/tris.md2"
    modelindex  0
    type        5
    index       11
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_sphere_vengeance"
{
    name        "Vengeance Sphere"
    model       "models/items/vengnce/tris.md2"
    modelindex  0
    type        5
    index       12
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_sphere_hunter"
{
    name        "Hunter Sphere"
    model       "models/items/hunter/tris.md2"
    modelindex  0
    type        5
    index       13
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_sphere_defender"
{
    name        "Defender Sphere"
    model       "models/items/defender/tris.md2"
    modelindex  0
    type        5
    index       14
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_doppleganger"
{
    name        "Doppleganger"
    model       "models/items/dopple/tris.md2"
    modelindex  0
    type        5
    index       15
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

// -----------------------------------------------------------------------
// CTF FLAGS  (#23 — Team play support)
// -----------------------------------------------------------------------

iteminfo "item_flag_team1"
{
    name        "Red Flag"
    model       "players/male/flag1.md2"
    modelindex  0
    type        6
    index       1
    respawntime 0
    mins        { -16, -16, -24 }
    maxs        { 16, 16, 32 }
}

iteminfo "item_flag_team2"
{
    name        "Blue Flag"
    model       "players/male/flag2.md2"
    modelindex  0
    type        6
    index       2
    respawntime 0
    mins        { -16, -16, -24 }
    maxs        { 16, 16, 32 }
}

// -----------------------------------------------------------------------
// CTF TECH ITEMS
// -----------------------------------------------------------------------

iteminfo "item_tech1"
{
    name        "Disruptor Shield"
    model       "models/ctf/resistance/tris.md2"
    modelindex  0
    type        5
    index       16
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_tech2"
{
    name        "Power Amplifier"
    model       "models/ctf/strength/tris.md2"
    modelindex  0
    type        5
    index       17
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_tech3"
{
    name        "Time Accel"
    model       "models/ctf/haste/tris.md2"
    modelindex  0
    type        5
    index       18
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

iteminfo "item_tech4"
{
    name        "AutoDoc"
    model       "models/ctf/regeneration/tris.md2"
    modelindex  0
    type        5
    index       19
    respawntime 60
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}

// -----------------------------------------------------------------------
// CTF GRAPPLE WEAPON
// -----------------------------------------------------------------------

iteminfo "weapon_grapple"
{
    name        "Grapple"
    model       "models/weapons/grapple/tris.md2"
    modelindex  0
    type        1
    index       12
    respawntime 0
    mins        { -16, -16, -16 }
    maxs        { 16, 16, 16 }
}
