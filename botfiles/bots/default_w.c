// Default weapon weight configuration for Q3 botlib backport to Quake II.
// Loaded by BotLoadWeaponWeights() via be_ai_weap.c::ReadWeightConfig().
//
// Each "weight" entry names a weapon (must match weaponinfo "name" in weapons.c).
//
// The fuzzy switch system gates each weapon's desirability score on
// inventory availability.  Syntax:
//
//   switch(N) {           // check inventory slot N directly
//       case X: return Y;  // inventory[N] < X  → return Y (first match wins)
//       default: return W; // otherwise          → return W
//   }
//
// NOTE: The parser (be_ai_weight.c ReadFuzzySeperators_r) only understands
// switch(N) — NOT "switch inventory(N)".  Always use the plain switch(N) form.
//
// Weapons with ammo use a nested switch: outer checks the weapon slot,
// inner checks the ammo slot.  Both must be >= 1 to return a positive score.
//
// Range-aware selection uses inventory slots 200 and 201:
//   200 = ENEMY_HORIZONTAL_DIST  (set from bc->enemy_hdist in be_interface_q2.c)
//   201 = ENEMY_HEIGHT           (set from bc->enemy_height)
// When no enemy is visible, these slots hold 9999/0 so the innermost
// "default" branch (max power, any range) is taken.
//
// Inventory indices (from g_items.c itemlist[], index 0 = null slot):
//   7=Blaster  8=Shotgun  9=SuperShotgun  10=Machinegun  11=Chaingun
//   12=ammo_grenades  13=GrenadeLauncher  14=RocketLauncher
//   15=HyperBlaster  16=Railgun  17=BFG10K
//   18=Shells  19=Bullets  20=Cells  21=Rockets  22=Slugs

weight "Blaster"
    return 10;

// Shotgun: excellent close range (< 200), decent medium (< 500), poor far.
weight "Shotgun"
    switch(8) {
        case 1: return 0;
        default: switch(18) {
            case 1: return 0;
            default: switch(200) {
                case 200: return 340;
                case 500: return 230;
                default:  return 80;
            }
        }
    }

// Super Shotgun: great close (< 200), good medium (< 400), poor far.
weight "Super Shotgun"
    switch(9) {
        case 1: return 0;
        default: switch(18) {
            case 1: return 0;
            default: switch(200) {
                case 200: return 400;
                case 400: return 280;
                default:  return 60;
            }
        }
    }

// Machinegun: usable at all ranges, best at medium.
weight "Machinegun"
    switch(10) {
        case 1: return 0;
        default: switch(19) {
            case 1: return 0;
            default: switch(200) {
                case 600: return 250;
                default:  return 190;
            }
        }
    }

// Chaingun: strong close/medium, weaker far.
weight "Chaingun"
    switch(11) {
        case 1: return 0;
        default: switch(19) {
            case 1: return 0;
            default: switch(200) {
                case 400: return 370;
                case 700: return 270;
                default:  return 150;
            }
        }
    }

// Hand grenades: hold-and-release not implemented in the adapter.
weight "Grenades"
    return 0;

// Grenade Launcher: ideal at 100-450 units; dangerous at < 100 (self-damage);
// poor at > 450 (arc trajectory gives enemy too much dodge time).
weight "Grenade Launcher"
    switch(13) {
        case 1: return 0;
        default: switch(12) {
            case 1: return 0;
            default: switch(200) {
                case 100: return 80;
                case 450: return 360;
                default:  return 100;
            }
        }
    }

// Rocket Launcher: dangerous at < 100 (self-damage); excellent 100-600;
// still usable at long range.
weight "Rocket Launcher"
    switch(14) {
        case 1: return 0;
        default: switch(21) {
            case 1: return 0;
            default: switch(200) {
                case 100: return 150;
                case 600: return 530;
                default:  return 260;
            }
        }
    }

// HyperBlaster: great at close/medium (< 500), lower at long range.
weight "HyperBlaster"
    switch(15) {
        case 1: return 0;
        default: switch(20) {
            case 1: return 0;
            default: switch(200) {
                case 500: return 390;
                default:  return 210;
            }
        }
    }

// Railgun: hitscan — excellent at all ranges; even better at long range
// where other weapons are weak.
weight "Railgun"
    switch(16) {
        case 1: return 0;
        default: switch(22) {
            case 1: return 0;
            default: switch(200) {
                case 250: return 360;
                default:  return 530;
            }
        }
    }

// BFG: devastating at close/medium (< 500); too slow at long range.
weight "BFG10K"
    switch(17) {
        case 1: return 0;
        default: switch(20) {
            case 1: return 0;
            default: switch(200) {
                case 100: return 80;
                case 500: return 650;
                default:  return 210;
            }
        }
    }

// Rogue / Xatrix mission pack extras.
// Inventory indices from g_items.c itemlist[] order (confirmed via awk):
//   50=Boomer  51=Phalanx  53=ammo_magslug  56=ETF Rifle  57=ProxLauncher
//   58=PlasmaBeam  59=Chainfist  60=Disintegrator
//   20=Cells  61=ammo_flechettes  62=ammo_prox  65=ammo_disruptor

// Phalanx: mid-range plasma cannon.
weight "Phalanx"
    switch(51) {
        case 1: return 0;
        default: switch(53) {
            case 1: return 0;
            default: switch(200) {
                case 500: return 430;
                default:  return 250;
            }
        }
    }

// Boomer: close-range energy weapon.
weight "Boomer"
    switch(50) {
        case 1: return 0;
        default: switch(20) {
            case 1: return 0;
            default: switch(200) {
                case 350: return 360;
                default:  return 180;
            }
        }
    }

// Disruptor: long-range precision weapon.
weight "Disruptor"
    switch(60) {
        case 1: return 0;
        default: switch(65) {
            case 1: return 0;
            default: switch(200) {
                case 300: return 300;
                default:  return 480;
            }
        }
    }

// ETF Rifle: medium-long range flechette weapon.
weight "ETF Rifle"
    switch(56) {
        case 1: return 0;
        default: switch(61) {
            case 1: return 0;
            default: switch(200) {
                case 600: return 380;
                default:  return 310;
            }
        }
    }

// Plasma Beam: close/medium range sustained beam.
weight "Plasma Beam"
    switch(58) {
        case 1: return 0;
        default: switch(20) {
            case 1: return 0;
            default: switch(200) {
                case 500: return 400;
                default:  return 200;
            }
        }
    }

// Prox Launcher: deploys proximity mines; medium range only.
weight "Prox Launcher"
    switch(57) {
        case 1: return 0;
        default: switch(62) {
            case 1: return 0;
            default: switch(200) {
                case 150: return 80;
                case 500: return 330;
                default:  return 120;
            }
        }
    }

// Chainfist: melee — only useful at point-blank range.
weight "Chainfist"
    switch(59) {
        case 1: return 0;
        default: switch(200) {
            case 80: return 300;
            default: return 30;
        }
    }
