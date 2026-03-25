// Default item weight configuration for Q3 botlib backport to Quake II.
// Loaded by BotLoadItemWeights() via be_ai_goal.c.
//
// FuzzyWeight evaluates:  if (inventory[switch_index] < case_value) return weight;
// So "case 1: return W" means "if inventory[N] == 0 (don't have it): return W".
//
// ALL switch indices are Q2 itemlist[] positions from g_items.c itemlist[],
// NOT WEAP_* enum values (those are a separate numbering starting from 1).
//
// Inventory layout (g_items.c itemlist[] order, index 0 = null slot):
//   Armor:   1=BodyArmor  2=CombatArmor  3=JacketArmor  4=ArmorShard
//            5=PowerScreen  6=PowerShield
//   Weapons: 7=Blaster   8=Shotgun   9=SuperShotgun  10=Machinegun
//            11=Chaingun  12=HandGrenades  13=GrenadeLauncher
//            14=RocketLauncher  15=HyperBlaster  16=Railgun  17=BFG10K
//   Ammo:    18=Shells  19=Bullets  20=Cells  21=Rockets  22=Slugs
//   Items:   23=Quad  24=Invuln  25=Silencer  26=Rebreather  27=EnviroSuit
//            28+=health/adrenaline/bandolier/pack (flat weights, no switch)
//
// -----------------------------------------------------------------------
// SCORING CALIBRATION
// -----------------------------------------------------------------------
// Travel time is in milliseconds. The scoring formula is:
//   effective_score = base_weight / (travel_time_ms * 0.01)
//
// For a weapon to reliably beat nearby health/ammo, it must score higher
// than any reachable consumable at close range.  Target balance:
//   - Regular health (weight 25) at t=100ms  → score  25
//   - RL            (weight 5000) at t=5000ms → score 100  (wins 4:1)
//   - RL            (weight 5000) at t=10000ms→ score  50  (still wins 2:1)
// Weapon weights are set 60-300x consumable weights so they dominate the
// goal selection budget even 5-10 seconds away.

// -----------------------------------------------------------------------
// WEAPONS  — high weight when not owned; 0 when already carrying one
// -----------------------------------------------------------------------

// Blaster is always owned; never present in the world as a pickup.
weight "weapon_blaster"
    return 0;

weight "weapon_shotgun"
{
    switch(8)  // itemlist[8] = weapon_shotgun
    {
        case 1: return 2000;
        default: return 0;
    }
}

weight "weapon_supershotgun"
{
    switch(9)  // itemlist[9] = weapon_supershotgun
    {
        case 1: return 2500;
        default: return 0;
    }
}

weight "weapon_machinegun"
{
    switch(10)  // itemlist[10] = weapon_machinegun
    {
        case 1: return 2000;
        default: return 0;
    }
}

weight "weapon_chaingun"
{
    switch(11)  // itemlist[11] = weapon_chaingun
    {
        case 1: return 3000;
        default: return 0;
    }
}

weight "weapon_grenadelauncher"
{
    switch(13)  // itemlist[13] = weapon_grenadelauncher
    {
        case 1: return 4000;
        default: return 0;
    }
}

weight "weapon_rocketlauncher"
{
    switch(14)  // itemlist[14] = weapon_rocketlauncher
    {
        case 1: return 5000;
        default: return 0;
    }
}

weight "weapon_hyperblaster"
{
    switch(15)  // itemlist[15] = weapon_hyperblaster
    {
        case 1: return 3500;
        default: return 0;
    }
}

weight "weapon_railgun"
{
    switch(16)  // itemlist[16] = weapon_railgun
    {
        case 1: return 4500;
        default: return 0;
    }
}

weight "weapon_bfg"
{
    switch(17)  // itemlist[17] = weapon_bfg
    {
        case 1: return 7000;
        default: return 0;
    }
}

// Rogue / Xatrix mission pack weapons.
// Inventory indices confirmed from g_items.c itemlist[] (index = position - 1
// since awk counts from 1): boomer=50 phalanx=51 etf_rifle=56 proxlauncher=57
// plasmabeam=58 chainfist=59 disintegrator=60

weight "weapon_boomer"
{
    switch(50)  // itemlist[50] = weapon_boomer
    {
        case 1: return 2500;
        default: return 0;
    }
}

weight "weapon_phalanx"
{
    switch(51)  // itemlist[51] = weapon_phalanx
    {
        case 1: return 3000;
        default: return 0;
    }
}

weight "weapon_etf_rifle"
{
    switch(56)  // itemlist[56] = weapon_etf_rifle
    {
        case 1: return 3500;
        default: return 0;
    }
}

weight "weapon_proxlauncher"
{
    switch(57)  // itemlist[57] = weapon_proxlauncher
    {
        case 1: return 4000;
        default: return 0;
    }
}

weight "weapon_plasmabeam"
{
    switch(58)  // itemlist[58] = weapon_plasmabeam
    {
        case 1: return 3200;
        default: return 0;
    }
}

weight "weapon_chainfist"
{
    switch(59)  // itemlist[59] = weapon_chainfist
    {
        case 1: return 1500;
        default: return 0;
    }
}

weight "weapon_disintegrator"
{
    switch(60)  // itemlist[60] = weapon_disintegrator
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

// Rogue / Xatrix ammo (itemlist indices 61-65)
weight "ammo_flechettes"
    return 35;

weight "ammo_prox"
    return 40;

weight "ammo_tesla"
    return 25;

weight "ammo_nuke"
    return 60;

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

weight "item_power_screen"
    return 50;

weight "item_power_shield"
    return 80;

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

// -----------------------------------------------------------------------
// POWERUPS  — kept high; these are rare and game-changing
// -----------------------------------------------------------------------

weight "item_quad"
    return 600;

weight "item_invulnerability"
    return 500;

weight "item_silencer"
    return 80;

weight "item_breather"
    return 40;

weight "item_enviro"
    return 40;

weight "item_adrenaline"
    return 150;

weight "item_bandolier"
    return 100;

weight "item_pack"
    return 150;

weight "item_quadfire"
    return 500;

// Rogue / Xatrix powerups (itemlist indices from g_items.c)
//   46-49=tech1-4  54=quadfire  66=ir_goggles  67=double  68=torch
//   69=compass  70=sphere_vengeance  71=sphere_hunter  72=sphere_defender
//   73=doppleganger

weight "item_tech1"
    return 400;

weight "item_tech2"
    return 400;

weight "item_tech3"
    return 400;

weight "item_tech4"
    return 400;

weight "item_ir_goggles"
    return 150;

weight "item_double"
    return 500;

weight "item_torch"
    return 50;

weight "item_compass"
    return 50;

weight "item_sphere_vengeance"
    return 400;

weight "item_sphere_hunter"
    return 400;

weight "item_sphere_defender"
    return 400;

weight "item_doppleganger"
    return 350;
