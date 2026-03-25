// Quake II weapon configuration for Q3 botlib backport.
// Parsed by be_ai_weap.c::LoadWeaponConfig().
//
// Each weapon requires:
//   number       = WEAP_* inventory index (g_local.h)
//   weaponindex  = same as number
//   projectile   = must match a projectileinfo "name" below
//   ammoindex    = AMMO_* enum: BULLETS=0 SHELLS=1 ROCKETS=2 GRENADES=3 CELLS=4 SLUGS=5
//
// DAMAGETYPE flags: 1=impact  2=radial  4=visible
//
// speed: projectile speed in units/sec; 0 = instant (hitscan)

// -----------------------------------------------------------------------
// PROJECTILES
// -----------------------------------------------------------------------

projectileinfo
{
    name        "blaster_bolt"
    model       "models/objects/laser/tris.md2"
    flags       0
    gravity     0
    damage      15
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        100
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

projectileinfo
{
    name        "shotgun_pellet"
    model       ""
    flags       0
    gravity     0
    damage      4
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        50
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

projectileinfo
{
    name        "sshotgun_pellet"
    model       ""
    flags       0
    gravity     0
    damage      6
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        50
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

projectileinfo
{
    name        "bullet"
    model       ""
    flags       0
    gravity     0
    damage      8
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        50
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

projectileinfo
{
    name        "chaingun_bullet"
    model       ""
    flags       0
    gravity     0
    damage      6
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        50
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

projectileinfo
{
    name        "hand_grenade"
    model       "models/objects/grenade/tris.md2"
    flags       0
    gravity     1
    damage      125
    radius      160
    visdamage   100
    damagetype  3
    healthinc   0
    push        300
    detonation  3
    bounce      0.6
    bouncefric  0.3
    bouncestop  0.05
}

projectileinfo
{
    name        "grenade"
    model       "models/objects/grenade/tris.md2"
    flags       0
    gravity     1
    damage      120
    radius      150
    visdamage   100
    damagetype  3
    healthinc   0
    push        300
    detonation  0
    bounce      0.6
    bouncefric  0.3
    bouncestop  0.05
}

projectileinfo
{
    name        "rocket"
    model       "models/objects/rocket/tris.md2"
    flags       0
    gravity     0
    damage      100
    radius      150
    visdamage   80
    damagetype  3
    healthinc   0
    push        400
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

projectileinfo
{
    name        "hyperblaster_bolt"
    model       "models/objects/laser/tris.md2"
    flags       0
    gravity     0
    damage      15
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        100
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

projectileinfo
{
    name        "rail_slug"
    model       ""
    flags       0
    gravity     0
    damage      150
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        350
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

projectileinfo
{
    name        "bfg_shot"
    model       "models/objects/laser/tris.md2"
    flags       0
    gravity     0
    damage      200
    radius      1000
    visdamage   10
    damagetype  7
    healthinc   0
    push        500
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

// Rogue extra projectiles
projectileinfo
{
    name        "plasma_beam_bolt"
    model       ""
    flags       0
    gravity     0
    damage      25
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        100
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

projectileinfo
{
    name        "disruptor_round"
    model       ""
    flags       0
    gravity     0
    damage      100
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        250
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

projectileinfo
{
    name        "phalanx_plasma"
    model       ""
    flags       0
    gravity     0
    damage      70
    radius      80
    visdamage   50
    damagetype  3
    healthinc   0
    push        200
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

projectileinfo
{
    name        "boomer_bolt"
    model       ""
    flags       0
    gravity     0
    damage      50
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        150
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

projectileinfo
{
    name        "etf_flechette"
    model       ""
    flags       0
    gravity     0
    damage      50
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        100
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

projectileinfo
{
    name        "prox_mine"
    model       ""
    flags       0
    gravity     1
    damage      120
    radius      192
    visdamage   100
    damagetype  3
    healthinc   0
    push        300
    detonation  0
    bounce      0.4
    bouncefric  0.2
    bouncestop  0.05
}

projectileinfo
{
    name        "chainfist_strike"
    model       ""
    flags       0
    gravity     0
    damage      120
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        400
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

// -----------------------------------------------------------------------
// WEAPONS
// -----------------------------------------------------------------------

weaponinfo
{
    number          1
    name            "Blaster"
    model           "models/weapons/v_blast/tris.md2"
    weaponindex     1
    flags           0
    projectile      "blaster_bolt"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           1000
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      0
    ammoindex       -1
    activate        0.2
    reload          0
    spinup          0
    spindown        0
}

weaponinfo
{
    number          2
    name            "Shotgun"
    model           "models/weapons/v_shotg/tris.md2"
    weaponindex     2
    flags           0
    projectile      "shotgun_pellet"
    numprojectiles  12
    hspread         500
    vspread         500
    speed           0
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      1
    ammoindex       1
    activate        0.3
    reload          1.0
    spinup          0
    spindown        0
}

weaponinfo
{
    number          3
    name            "Super Shotgun"
    model           "models/weapons/v_shotg2/tris.md2"
    weaponindex     3
    flags           0
    projectile      "sshotgun_pellet"
    numprojectiles  20
    hspread         1000
    vspread         500
    speed           0
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      2
    ammoindex       1
    activate        0.3
    reload          0.7
    spinup          0
    spindown        0
}

weaponinfo
{
    number          4
    name            "Machinegun"
    model           "models/weapons/v_machn/tris.md2"
    weaponindex     4
    flags           0
    projectile      "bullet"
    numprojectiles  1
    hspread         200
    vspread         200
    speed           0
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      1
    ammoindex       0
    activate        0.2
    reload          0.1
    spinup          0
    spindown        0
}

weaponinfo
{
    number          5
    name            "Chaingun"
    model           "models/weapons/v_chain/tris.md2"
    weaponindex     5
    flags           0
    projectile      "chaingun_bullet"
    numprojectiles  1
    hspread         300
    vspread         300
    speed           0
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      1
    ammoindex       0
    activate        0.5
    reload          0.1
    spinup          0.5
    spindown        0.5
}

weaponinfo
{
    number          6
    name            "Grenades"
    model           "models/weapons/v_handgr/tris.md2"
    weaponindex     6
    flags           0
    projectile      "hand_grenade"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           600
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  200
    ammoamount      1
    ammoindex       3
    activate        0.3
    reload          1.0
    spinup          0
    spindown        0
}

weaponinfo
{
    number          7
    name            "Grenade Launcher"
    model           "models/weapons/v_launch/tris.md2"
    weaponindex     7
    flags           0
    projectile      "grenade"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           600
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  200
    ammoamount      1
    ammoindex       3
    activate        0.3
    reload          0.8
    spinup          0
    spindown        0
}

weaponinfo
{
    number          8
    name            "Rocket Launcher"
    model           "models/weapons/v_rocket/tris.md2"
    weaponindex     8
    flags           0
    projectile      "rocket"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           650
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      1
    ammoindex       2
    activate        0.3
    reload          0.8
    spinup          0
    spindown        0
}

weaponinfo
{
    number          9
    name            "HyperBlaster"
    model           "models/weapons/v_hyperb/tris.md2"
    weaponindex     9
    flags           0
    projectile      "hyperblaster_bolt"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           1000
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      1
    ammoindex       4
    activate        0.3
    reload          0.1
    spinup          0
    spindown        0
}

weaponinfo
{
    number          10
    name            "Railgun"
    model           "models/weapons/v_rail/tris.md2"
    weaponindex     10
    flags           0
    projectile      "rail_slug"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           0
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      1
    ammoindex       5
    activate        0.3
    reload          1.5
    spinup          0
    spindown        0
}

weaponinfo
{
    number          11
    name            "BFG10K"
    model           "models/weapons/v_bfg/tris.md2"
    weaponindex     11
    flags           0
    projectile      "bfg_shot"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           400
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      50
    ammoindex       4
    activate        0.5
    reload          1.0
    spinup          0
    spindown        0
}

// Rogue mission pack extras

weaponinfo
{
    number          12
    name            "Phalanx"
    model           "models/weapons/v_rocket/tris.md2"
    weaponindex     12
    flags           0
    projectile      "phalanx_plasma"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           800
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      2
    ammoindex       2
    activate        0.3
    reload          0.8
    spinup          0
    spindown        0
}

weaponinfo
{
    number          13
    name            "Boomer"
    model           "models/weapons/v_hyperb/tris.md2"
    weaponindex     13
    flags           0
    projectile      "boomer_bolt"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           1200
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      1
    ammoindex       4
    activate        0.3
    reload          0.1
    spinup          0
    spindown        0
}

weaponinfo
{
    number          14
    name            "Disruptor"
    model           "models/weapons/v_blast/tris.md2"
    weaponindex     14
    flags           0
    projectile      "disruptor_round"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           500
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      1
    ammoindex       5
    activate        0.3
    reload          0.8
    spinup          0
    spindown        0
}

weaponinfo
{
    number          15
    name            "ETF Rifle"
    model           "models/weapons/v_rail/tris.md2"
    weaponindex     15
    flags           0
    projectile      "etf_flechette"
    numprojectiles  1
    hspread         100
    vspread         100
    speed           0
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      1
    ammoindex       5
    activate        0.3
    reload          0.5
    spinup          0
    spindown        0
}

weaponinfo
{
    number          16
    name            "Plasma Beam"
    model           "models/weapons/v_hyperb/tris.md2"
    weaponindex     16
    flags           0
    projectile      "plasma_beam_bolt"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           1000
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      1
    ammoindex       4
    activate        0.3
    reload          0.1
    spinup          0
    spindown        0
}

weaponinfo
{
    number          17
    name            "Prox Launcher"
    model           "models/weapons/v_launch/tris.md2"
    weaponindex     17
    flags           0
    projectile      "prox_mine"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           600
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  100
    ammoamount      1
    ammoindex       3
    activate        0.3
    reload          1.0
    spinup          0
    spindown        0
}

weaponinfo
{
    number          18
    name            "Chainfist"
    model           "models/weapons/v_chain/tris.md2"
    weaponindex     18
    flags           0
    projectile      "chainfist_strike"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           0
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      0
    ammoindex       -1
    activate        0.2
    reload          0.3
    spinup          0
    spindown        0
}
