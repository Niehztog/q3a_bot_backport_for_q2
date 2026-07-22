// Quake II weapon configuration for Q3 botlib backport.
// Parsed by be_ai_weap.c::LoadWeaponConfig().
//
// Each weapon requires:
//   number       = WEAP_* (g_local.h); Trap and Tesla, which have none, 19/20
//   weaponindex  = same as number
//   name         = the weapon's pickup name: the bot switches weapons with
//                  "use <name>", and its weight in fw_weap.c/bots/*_w.c has
//                  this name too
//   model        = the view model: the adapter recognises the weapon in hand
//                  by it (ps.gunindex), so every weapon needs its own
//   projectile   = must match a projectileinfo "name" below
//   ammoindex    = AMMO_* enum: BULLETS=0 SHELLS=1 ROCKETS=2 GRENADES=3 CELLS=4
//                  SLUGS=5 MAGSLUG=6 TRAP=7 FLECHETTES=8 TESLA=9 PROX=10
//                  DISRUPTOR=11 (informational, as is ammoamount)
//
// flags: 1 = WFL_FIRERELEASED, the shot leaves when fire is let go. The AI
// then presses fire on every other frame only, so a thrown weapon is never
// held until it goes off in the bot's hand.
//
// The bot AI compares these numbers with its WP_* constants
// (botlib/ai_q2_compat.h); renumbering a weapon here means updating those.
//
// DAMAGETYPE flags: 1=impact  2=radial  4=visible
//
// damage/radius of a radial projectile: what its owner risks. The AI fires
// no shot that would burst within radius of itself (BotCheckAttack) and aims
// at an enemy's feet only when the ground there is farther away than radius
// (BotAimAtEnemy). Q2's T_RadiusDamage deals damage - 0.5 * distance, halved
// for the owner, up to the damage radius measured from the body centre: at
// the edge of a rocket's 120 that is still 30. Q3's splash falls off to 0, so
// its AI fired at enemies right in front of it and aimed at feet 100 units
// away. radius is Q2's damage radius plus 30, since the AI measures from its
// eye; damage is Q2's radius damage. A BFG's owner risks only the ball's
// burst (damage 200, radius 100, g_weapon.c bfg_touch), not its lasers.
//
// speed: projectile speed in units/sec; 0 = instant (hitscan)
//
// hspread/vspread: DEGREES. BotAimAtEnemy adds 6 * spread * (1 - accuracy)
// degrees of random error to the bot's aim every frame, so they stay 0 as in
// Q3's weapons.c; Q2's fire_shotgun spread units (500, 1000) here made the
// bots' view jerk around randomly with every hitscan weapon.

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
    radius      195
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
    radius      190
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
    damage      120
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
    radius      130
    visdamage   10
    damagetype  7
    healthinc   0
    push        500
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

// Mission pack projectiles (deathmatch damage; p_weapon.c, g_weapon.c,
// g_newweap_rogue.c)

// The Reckoning: Phalanx, two per shot, 70-79 damage plus radius damage
projectileinfo
{
    name        "phalanx_plasma"
    model       ""
    flags       0
    gravity     0
    damage      120
    radius      150
    visdamage   0
    damagetype  3
    healthinc   0
    push        120
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

// Ionripper, bounces off walls
projectileinfo
{
    name        "ionripper_bolt"
    model       ""
    flags       0
    gravity     0
    damage      30
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        40
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

// Trap: pulls in whoever comes within 256 units, its thrower included
projectileinfo
{
    name        "trap"
    model       ""
    flags       0
    gravity     1
    damage      125
    radius      256
    visdamage   0
    damagetype  2
    healthinc   0
    push        0
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

// Ground Zero: ETF Rifle, ignores normal armor
projectileinfo
{
    name        "etf_flechette"
    model       ""
    flags       0
    gravity     0
    damage      10
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        75
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

// Prox Launcher: sticks where it lands, goes off when someone comes near
projectileinfo
{
    name        "prox_mine"
    model       ""
    flags       0
    gravity     1
    damage      90
    radius      222
    visdamage   0
    damagetype  3
    healthinc   0
    push        0
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

// Plasma Beam: hitscan heat beam
projectileinfo
{
    name        "plasma_beam_bolt"
    model       ""
    flags       0
    gravity     0
    damage      15
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

// Chainfist: melee, 64 units reach
projectileinfo
{
    name        "chainfist_strike"
    model       ""
    flags       0
    gravity     0
    damage      30
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

// Disruptor: tracker, homes in on the target aimed at when fired
projectileinfo
{
    name        "disruptor_round"
    model       ""
    flags       0
    gravity     0
    damage      30
    radius      0
    visdamage   0
    damagetype  1
    healthinc   0
    push        0
    detonation  0
    bounce      0
    bouncefric  0
    bouncestop  0
}

// Tesla: zaps everyone within 128 units, its thrower included, for 30 s
projectileinfo
{
    name        "tesla"
    model       ""
    flags       0
    gravity     1
    damage      60
    radius      128
    visdamage   0
    damagetype  2
    healthinc   0
    push        0
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
    hspread         0
    vspread         0
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
    hspread         0
    vspread         0
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
    hspread         0
    vspread         0
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
    hspread         0
    vspread         0
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
    flags           1
    projectile      "hand_grenade"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           400
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

// The Reckoning

weaponinfo
{
    number          12
    name            "Phalanx"
    model           "models/weapons/v_shotx/tris.md2"
    weaponindex     12
    flags           0
    projectile      "phalanx_plasma"
    numprojectiles  2
    hspread         0
    vspread         0
    speed           725
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      1
    ammoindex       6
    activate        0.3
    reload          0.8
    spinup          0
    spindown        0
}

weaponinfo
{
    number          13
    name            "Ionripper"
    model           "models/weapons/v_boomer/tris.md2"
    weaponindex     13
    flags           0
    projectile      "ionripper_bolt"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           500
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      2
    ammoindex       4
    activate        0.3
    reload          0.2
    spinup          0
    spindown        0
}

// thrown like a hand grenade
weaponinfo
{
    number          19
    name            "Trap"
    model           "models/weapons/v_trap/tris.md2"
    weaponindex     19
    flags           1
    projectile      "trap"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           400
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  200
    ammoamount      1
    ammoindex       7
    activate        0.3
    reload          3.0
    spinup          0
    spindown        0
}

// Ground Zero

weaponinfo
{
    number          14
    name            "Disruptor"
    model           "models/weapons/v_dist/tris.md2"
    weaponindex     14
    flags           0
    projectile      "disruptor_round"
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
    ammoindex       11
    activate        0.3
    reload          0.6
    spinup          0
    spindown        0
}

weaponinfo
{
    number          15
    name            "ETF Rifle"
    model           "models/weapons/v_etf_rifle/tris.md2"
    weaponindex     15
    flags           0
    projectile      "etf_flechette"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           750
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      1
    ammoindex       8
    activate        0.3
    reload          0.1
    spinup          0
    spindown        0
}

weaponinfo
{
    number          16
    name            "Plasma Beam"
    model           "models/weapons/v_beamer/tris.md2"
    weaponindex     16
    flags           0
    projectile      "plasma_beam_bolt"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           0
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  0
    ammoamount      2
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
    model           "models/weapons/v_plaunch/tris.md2"
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
    extrazvelocity  200
    ammoamount      1
    ammoindex       10
    activate        0.3
    reload          1.0
    spinup          0
    spindown        0
}

weaponinfo
{
    number          18
    name            "Chainfist"
    model           "models/weapons/v_chainf/tris.md2"
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

// thrown like a hand grenade
weaponinfo
{
    number          20
    name            "Tesla"
    model           "models/weapons/v_tesla/tris.md2"
    weaponindex     20
    flags           1
    projectile      "tesla"
    numprojectiles  1
    hspread         0
    vspread         0
    speed           400
    acceleration    0
    recoil          { 0, 0, 0 }
    offset          { 0, 0, 0 }
    angleoffset     { 0, 0, 0 }
    extrazvelocity  200
    ammoamount      1
    ammoindex       9
    activate        0.3
    reload          3.0
    spinup          0
    spindown        0
}
