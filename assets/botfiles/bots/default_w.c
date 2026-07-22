// Default weapon weight configuration for Q3 botlib backport to Quake II.
// Loaded by BotLoadWeaponWeights() via be_ai_weap.c::ReadWeightConfig().
//
// Each "weight" entry names a weapon (must match weaponinfo "name" in weapons.c).
//
// The fuzzy switch system gates each weapon's desirability score on
// inventory availability.  Syntax:
//
//   switch(N) {           // check inventory slot N
//       case X: return Y;  // inventory[N] < X  -> Y (first match wins)
//       default: return W; // otherwise          -> W
//   }
//
// N is a slot number from inv.h; the parser (be_ai_weight.c
// ReadFuzzySeperators_r) only understands switch(N), not
// "switch inventory(N)".
//
// Weapons with ammo use a nested switch: outer checks the weapon slot,
// inner checks the ammo slot against the ammo one shot takes.
//
// Range-aware selection uses ENEMY_HORIZONTAL_DIST (slot 200), which the AI
// sets from the enemy's position before it chooses a weapon to fight with.

#include "inv.h"

weight "Blaster"
    return 10;

// Shotgun: excellent close range (< 200), decent medium (< 500), poor far.
weight "Shotgun"
    switch(INVENTORY_SHOTGUN) {
        case 1: return 0;
        default: switch(INVENTORY_SHELLS) {
            case 1: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 200: return 340;
                case 500: return 230;
                default:  return 80;
            }
        }
    }

// Super Shotgun: great close (< 200), good medium (< 400), poor far.
// Two shells a shot.
weight "Super Shotgun"
    switch(INVENTORY_SUPERSHOTGUN) {
        case 1: return 0;
        default: switch(INVENTORY_SHELLS) {
            case 2: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 200: return 400;
                case 400: return 280;
                default:  return 60;
            }
        }
    }

// Machinegun: usable at all ranges, best at medium.
weight "Machinegun"
    switch(INVENTORY_MACHINEGUN) {
        case 1: return 0;
        default: switch(INVENTORY_BULLETS) {
            case 1: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 600: return 250;
                default:  return 190;
            }
        }
    }

// Chaingun: strong close/medium, weaker far.
weight "Chaingun"
    switch(INVENTORY_CHAINGUN) {
        case 1: return 0;
        default: switch(INVENTORY_BULLETS) {
            case 1: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 400: return 370;
                case 700: return 270;
                default:  return 150;
            }
        }
    }

// Hand grenades: not thrown. weapons.c marks them fire-on-release like the
// Trap and the Tesla, so a weight here is all it would take.
weight "Grenades"
    return 0;

// Grenade Launcher: ideal at 100-450 units; dangerous at < 100 (self-damage);
// poor at > 450 (arc trajectory gives enemy too much dodge time).
weight "Grenade Launcher"
    switch(INVENTORY_GRENADELAUNCHER) {
        case 1: return 0;
        default: switch(INVENTORY_GRENADES) {
            case 1: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 100: return 80;
                case 450: return 360;
                default:  return 100;
            }
        }
    }

// Rocket Launcher: dangerous at < 100 (self-damage); excellent 100-600;
// still usable at long range.
weight "Rocket Launcher"
    switch(INVENTORY_ROCKETLAUNCHER) {
        case 1: return 0;
        default: switch(INVENTORY_ROCKETS) {
            case 1: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 100: return 150;
                case 600: return 530;
                default:  return 260;
            }
        }
    }

// HyperBlaster: great at close/medium (< 500), lower at long range.
weight "HyperBlaster"
    switch(INVENTORY_HYPERBLASTER) {
        case 1: return 0;
        default: switch(INVENTORY_CELLS) {
            case 1: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 500: return 390;
                default:  return 210;
            }
        }
    }

// Railgun: hitscan — excellent at all ranges; even better at long range
// where other weapons are weak.
weight "Railgun"
    switch(INVENTORY_RAILGUN) {
        case 1: return 0;
        default: switch(INVENTORY_SLUGS) {
            case 1: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 250: return 360;
                default:  return 530;
            }
        }
    }

// BFG: devastating at close/medium (< 500); too slow at long range.
// 50 cells a shot.
weight "BFG10K"
    switch(INVENTORY_BFG10K) {
        case 1: return 0;
        default: switch(INVENTORY_CELLS) {
            case 50: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 100: return 80;
                case 500: return 650;
                default:  return 210;
            }
        }
    }

// ----- The Reckoning -----

// Phalanx: mid-range plasma cannon.
weight "Phalanx"
    switch(INVENTORY_PHALANX) {
        case 1: return 0;
        default: switch(INVENTORY_MAGSLUGS) {
            case 1: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 500: return 430;
                default:  return 250;
            }
        }
    }

// Ionripper: close-range energy weapon, 2 cells a shot.
weight "Ionripper"
    switch(INVENTORY_IONRIPPER) {
        case 1: return 0;
        default: switch(INVENTORY_CELLS) {
            case 2: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 350: return 360;
                default:  return 180;
            }
        }
    }

// Trap: thrown; pulls in whoever comes within 256 units, the thrower too.
weight "Trap"
    switch(INVENTORY_TRAP) {
        case 1: return 0;
        default: switch(ENEMY_HORIZONTAL_DIST) {
            case 256: return 0;
            case 700: return 150;
            default:  return 40;
        }
    }

// ----- Ground Zero -----

// Disruptor: long-range precision weapon.
weight "Disruptor"
    switch(INVENTORY_DISRUPTOR) {
        case 1: return 0;
        default: switch(INVENTORY_ROUNDS) {
            case 1: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 300: return 300;
                default:  return 480;
            }
        }
    }

// ETF Rifle: medium-long range flechette weapon.
weight "ETF Rifle"
    switch(INVENTORY_ETFRIFLE) {
        case 1: return 0;
        default: switch(INVENTORY_FLECHETTES) {
            case 1: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 600: return 380;
                default:  return 310;
            }
        }
    }

// Plasma Beam: close/medium range sustained beam, 2 cells a frame.
weight "Plasma Beam"
    switch(INVENTORY_PLASMABEAM) {
        case 1: return 0;
        default: switch(INVENTORY_CELLS) {
            case 2: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 500: return 400;
                default:  return 200;
            }
        }
    }

// Prox Launcher: deploys proximity mines; medium range only.
weight "Prox Launcher"
    switch(INVENTORY_PROXLAUNCHER) {
        case 1: return 0;
        default: switch(INVENTORY_PROX) {
            case 1: return 0;
            default: switch(ENEMY_HORIZONTAL_DIST) {
                case 150: return 80;
                case 500: return 330;
                default:  return 120;
            }
        }
    }

// Tesla: thrown; zaps everyone within 128 units, the thrower too.
weight "Tesla"
    switch(INVENTORY_TESLA) {
        case 1: return 0;
        default: switch(ENEMY_HORIZONTAL_DIST) {
            case 200: return 0;
            case 600: return 150;
            default:  return 40;
        }
    }

// Chainfist: melee — only useful at point-blank range.
weight "Chainfist"
    switch(INVENTORY_CHAINFIST) {
        case 1: return 0;
        default: switch(ENEMY_HORIZONTAL_DIST) {
            case 80: return 300;
            default:  return 30;
        }
    }
