# Backport of the Quake III Arena Bot for Quake II

## Introduction

On December 8, 1998, Jan Paul "Mr. Elusive" van Waveren released the first version of his new artificial player for Quake II, called "[The Gladiator Bot](https://mrelusive.com/oldprojects/gladiator/gladiator.html)". In the coming months his bot grew in popularity, as it had modern artificial intelligence, making it more realistic and therefore superior to most other bots for Quake II.

The bot became so successful and popular, that even id software itself decided to hire its author Mr. Elusive to work together with their team on Quake III Arena and to port and advance the Gladiator Bot for [Quake III Arena](https://github.com/id-Software/Quake-III-Arena). It later became the base for the single player AI in the game.

Before moving to id software, Mr. Elusive even started porting his bot for other Quake Engine games like Half-Life and Sin, but that work unfortunately was never finished. There are still traces left of that attempt in the original source code files.

Mr. Elusive released the game source code for Gladiator Bot to the public, allowing mod developers to implement the Gladiator bot right into their mods. Unfortunately he released only the Quake II game source code part of his bot, keeping the source code of the bot library "botlib" private. The botlib contains the implementation of the artificial intelligence with map navigation, strategic behaviour and so on. This makes it unable to recompile his work for Quake II as a whole, making it difficult to run the Gladiator bot on modern Quake II engines and making it impossible to implement new features or bug fixes or to adapting it to new operating system architectures.

On January 31, 2017, Jan Paul "Mr. Elusive" van Waveren sadly passed away after suffering a severe sickness. May he rest in peace.

Before passing away, he of course finished his work of bringing the Gladiator Bot into Quake III, further perfecting its artificial intelligence and routing behaviour.

The whole Quake III Arena source code later was officially released by id software, containing all of Mr. Elusive's work.

## Technical Background

One speciality of the Gladiator Bot for Quake II is, that it incorporates not only the original game modes, but also both mission packs, Capture The Flag and even Rocket Arena II game modes into a single game library. From an architectural point of view this has many advantages over having it all in seperate libraries residing in different directories. One of those being the possibility to quickly switch games modes without loading a different mod or to share game assets between game modes. Quake III Arena uses a similar approach, as it also as Capture The Flag game mode incorporated into the base game.

As the Quake III Arena version of the former Gladiator Bot code has evolved over the time, the bot library from Quake III Arena is no longer compatible with the Quake II Gladiator bot, as it has been adapted to fit Quake III's game API and architecture.

## Goal of this project

The goal of this project is to backport the bot library files from Quake III Arena to work again the Gladiator Bot game library.
Advantages:
- As the Quake III Bots have significantly improved artificial intelligence, this would tremendously improve the single player experience of multiplayer game modes in Quake II compared to the original Gladiator Bot.
- Having the full source of the bot available it would be easy to port the bot to other operating systems or architectures or to modernized versions of the Quake II game engine like [Yamagi Quake II](https://github.com/yquake2/yquake2).
- The bot could as well be implemented into newer mods of the game or its features could be further advanced or improved.

## Current status

All relevant source code files have been identified and brought together in this repository. It was attempted to preserve the git history wherever possible, in order to make the evolution of the code more understandable.
An initial version of a Makefile for compilation has been added. The Makefile from Yamagi Q2 was used as a base for this.

The adapter layer has been implemented and bots are loading, spawning, and moving around maps. AAS-based navigation, item goal selection, elevator and mover usage, entity type classification (players, items, projectiles, movers), range-aware weapon weight configuration, and rocket jump support have all been implemented and attempted. The Rogue and Xatrix mission pack items and weapons are accounted for in the bot configuration files, and CTF grappling hook support is partially wired up pending a one-line change in the game DLL. The bots still are facing major navigation issues and have problems roaming around the map. All of this requires further testing and tuning across different maps and game modes.

## Technical approach

The central piece of this backport is an adapter layer (`botlib/be_interface_q2.c`) that sits between the Q2 game DLL and the Q3 bot library. The Q2 game DLL expects botlib to expose a flat set of about 20 functions through a `bot_export_t` struct and calls back into the engine via 10 function pointers in `bot_import_t`. The Q3 botlib, by contrast, organises its exports into hierarchical sub-structs (AAS, EA, AI) totalling over 100 functions and expects a richer set of engine callbacks. The adapter translates in both directions: it presents the simple Q2 `bot_export_t` interface to the game DLL while driving the full Q3 botlib internally, and it bridges the Q2 engine callbacks (trace, point-contents, memory, file I/O) to the form the Q3 botlib requires. Per-frame, it feeds Q2 entity state - player positions, item pickups, moving platforms, projectiles - into the Q3 AAS and AI subsystems, then converts the resulting bot movement and action decisions back into Q2 input commands. This approach lets the Q3 botlib run unmodified against a Q2 game without requiring changes to either the Q2 engine or the Q2 game DLL beyond the original Gladiator bot integration points.

In order to make it more transparent of how this repository was setup, all necessary steps were documented and built into a [shell script](./setup.sh). This script downloads all necessary sources, extracts them and puts them together as they can be found in this repository. In case someone else wants to continue this work, he may use this script as a base to get started with.
The current analysis results of the code structure differences between Q2 Gladiator and Q3 Bots are documented in a [separate markdown file](./docs/DEVELOPMENT.md).

## How to compile

This repository and Makefile have been optimized to work with the Yamagi Q2 Build Environment. Follow [this guide](https://github.com/yquake2/yquake2/blob/master/doc/020_installation.md#compiling-from-source) on how to set up the build environment, clone this repo inside mingw32 shell, change to its directory and type `make`.

## Using it from a game

`make` builds three files into `release/`:

- `game/game.so` (`game.dll` on Windows) is **the game**: the Gladiator Bot's game library for Quake II, which plays deathmatch, Capture the Flag, Rocket Arena and both mission packs, and adds the bots to them.
- `botlib/botlib.so` (`botlib.dll`) is **the bot library**: the Quake III Arena bot. The game loads it when the first bot joins.
- `bspc/bspc` (`bspc.exe`) prepares maps for the bots.

The steps below are for [Yamagi Quake II](https://github.com/yquake2/yquake2).

### Installing

Give the bot a directory of its own next to `baseq2`, for example `q3bot`. Don't install it over the Gladiator Bot: its bot and map files have the same names as this bot's, but other formats.

```
quake2/
  baseq2/
  q3bot/
    game.so       from release/game/ (game.dll on Windows)
    botlib.so     from release/botlib/ (botlib.dll on Windows)
    bots.cfg      from assets/botfiles/: the bots that can join
    botfiles/     assets/botfiles/ with everything in it: the bots' characters, chats and preferences for items and weapons
    maps/         the bots' map files, see below
```

On Linux, from this directory (on Windows, copy the same files by hand):

```sh
Q2=~/quake2                     # the directory with baseq2 in it
mkdir -p $Q2/q3bot/maps
cp release/game/game.so release/botlib/botlib.so assets/botfiles/bots.cfg $Q2/q3bot/
cp -r assets/botfiles $Q2/q3bot/
```

Capture the Flag and the mission packs also need their game data in the bot's directory, under names Quake II loads from there: copy `ctf/pak0.pak` and `ctf/pak1.pak` to `q3bot/pak0.pak` and `q3bot/pak1.pak`, `xatrix/pak0.pak` (The Reckoning) to `q3bot/pak2.pak` and `rogue/pak0.pak` (Ground Zero) to `q3bot/pak3.pak`.

### Preparing the maps

The bots find their way around a map with its navigation file, `maps/<map>.aas` in the bot's directory, which `bspc` makes from the map. It reads the maps straight out of Quake II's pak files, and takes seconds to a minute per map. This makes `q2dm1.aas` to `q2dm8.aas` for the eight deathmatch maps:

```sh
release/bspc/bspc -bsp2aas "$Q2/baseq2/pak1.pak/maps/q2dm*.bsp" -output $Q2/q3bot/maps
```

On Windows: `release\bspc\bspc.exe -bsp2aas "C:\Quake2\baseq2\pak1.pak\maps\q2dm*.bsp" -output C:\Quake2\q3bot\maps`

The Capture the Flag maps are `q2ctf*.bsp` in `ctf/pak0.pak` and `ctf/pak1.pak`, The Reckoning's `xdm*.bsp` in `xatrix/pak0.pak` and Ground Zero's `rdm*.bsp` in `rogue/pak0.pak`. For a map that is a file of its own, give its path. The `.aas` files of the Gladiator Bot don't work: they are in an older format.

### Playing

Start Quake II with the bot's directory and a map, and from Quake II's own directory: the game and the bot library look for their files relative to it.

```sh
cd $Q2
./quake2 +set game q3bot +set deathmatch 1 +set maxclients 8 +map q2dm1
```

`maxclients` is the number of players, bots included; the bots allow at most 64. Add `+set ctf 1` for Capture the Flag on a `q2ctf` map, `+set xatrix 1` or `+set rogue 1` for a mission pack, and `+set rocketarena 1` for Rocket Arena. In the game, add and remove bots in the console:

| Command | What it does |
|---|---|
| `menu` | opens a menu to add and remove bots |
| `addrandom 3` | adds three bots, picked at random from `bots.cfg` |
| `addbot Sarge sarge/default bots/sarge_c.c sarge` | adds one bot: name, player model/skin, character file and character name, as in the lines of `bots.cfg` |
| `removebot Sarge` | removes this bot; `removebot` alone removes any one |
| `set minimumplayers 6` | keeps 6 players in the game: adds bots while there are fewer, and removes them as people join (not in Rocket Arena) |
| `set bot_skill 2` | how well the bots play, from 1 to 5 (default 4); set it before adding them |

On the console of a dedicated server, the bot commands take `sv` in front (`sv addrandom 3`), and `set serveronlybotcmds 1` keeps players from adding and removing bots.

If no bot joins and the console says `botlib.so not available` (`botlib.dll` on Windows), the lines above it tell why:

- `can't open maps/q2dm1.aas`: the map has no navigation file yet.
- `aas file maps/q2dm1.aas is version 3, not 5`: the file is the Gladiator Bot's. Make a new one with `bspc`.
- `this bot library supports at most 64 clients`: lower `maxclients`.

The bot library writes a log, `botlib.log`, into the directory Quake II was started from (`set log 0` before adding bots turns it off).

### For game and mod developers

`make botlib bspc` builds just the bot library and `bspc`, all that a game that embeds the bot needs. Give `CFLAGS` and `LDFLAGS` in the environment, not on make's command line: the Makefile adds `-fPIC` and `-shared` to them per target, and make drops such additions to a variable set on its command line. A cross build has to name its target, because the Makefile otherwise builds for the machine it runs on: `make YQ2_OSTYPE=Windows YQ2_ARCH=i386 CC=i686-w64-mingw32-gcc botlib bspc` (`YQ2_ARCH=x86_64` and `CC=x86_64-w64-mingw32-gcc` for 64-bit Windows).

The bot library is loaded the way the Gladiator Bot's was. The game opens it, looks up `GetBotAPI` and calls it with a `bot_import_t`, the functions the bot calls in the game, and gets back a `bot_export_t`, the bot's functions for the game. [botlib/be_interface_q2.h](./botlib/be_interface_q2.h) declares both. They are the Gladiator Bot's tables, so a game written for its library loads this one unchanged; the export table's entries after Gladiator's twenty are this library's own. The structs the tables pass hold no pointers, so they are the same in 32-bit and 64-bit builds, and the header checks their sizes when it is compiled.

- **Calling convention.** `GetBotAPI` is a plain C (`__cdecl`) function on every platform, 32-bit Windows included, like the one in Gladiator's `gladiator.dll`. The 1999 Gladiator game source calls it through a `WINAPI` (`__stdcall`) pointer, which leaves the stack four bytes off with either library unless the calling function keeps a frame pointer. Declare the pointer plain, as [game_q2/bl_main.c](./game_q2/bl_main.c) does.
- **Running beside Gladiator's bot library**, as [Colosseum](https://github.com/Niehztog/colosseum) does:
  - `GetBotAPI` is the only symbol the library exports. Everything else is hidden, so neither library's functions can bind to the other's of the same name.
  - `BotVersion()` tells the two apart: this one returns `Q3Backport-` and its version (now `Q3Backport-0.2`), Gladiator's `BotLib v0.96`.
  - Both use the same file names (`maps/<map>.aas`, `botfiles/bots/...`) in different formats. When the game sets the library variable `datadir` (with `BotLibVarSet`, before `BotSetupLibrary`), this library reads and writes its bot files, AAS files and route caches only under `<basedir>/<gamedir>/<datadir>/`. Only the map itself, which it reads from the game's directories and pak files, and `botlib.log`, which goes to the working directory, are elsewhere. Without `datadir`, it searches the game directory, `baseq2` and their pak files, and last the layout of Gladiator's `pak7.pak`, where it can come across Gladiator's files.
- **Limits.** `BotSetupLibrary` refuses a `maxclients` above 64, the size of the library's client tables. Entities numbered 1022 and up are left out of what the bots see, because Quake III's entity numbers end there.
- **Skill.** `bot_skill` (1 to 5) is read for each bot in `BotSetupClient`, so a game that sets it before each bot can give every bot its own. This repository's game sets it once, when the library loads.
- **Teams** follow the library variables `ctf`, `teamplay` and `dmflags` (teams by skin or by model), as with Gladiator's library. Rocket Arena (`ra`) alone is no team game: a game whose arenas play in teams sets `teamplay` or the team `dmflags`.
- **AAS files** are Quake III's, version 5, made with this repository's `bspc`. Gladiator's version-3 files are refused.

## Player models

The bots appear as their Quake III Arena characters: `bots.cfg` gives each bot the player model and skin that Quake III's `scripts/bots.txt` gives it (`sarge/default`, `biker/cadavre`, ...), and Team Arena's for Fritzkrieg and pi. These models are part of Quake III Arena and Team Arena and are not included here. Convert them from your own copies of the games with [q3player2md2](https://github.com/Niehztog/q3player2md2) and copy what it writes to `out/players/` into `baseq2/players/` of every Quake II installation that plays with the bots. Given the mission packs' data, it also puts their weapons in the players' hands:

```
python3 q3player2md2.py --q3 <quake3>/baseq3 --q2 <quake2>/baseq2 --q2 <quake2>/xatrix --q2 <quake2>/rogue --all
python3 q3player2md2.py --q3 <quake3>/baseq3 --q3 <quake3>/missionpack --q2 <quake2>/baseq2 --q2 <quake2>/xatrix --q2 <quake2>/rogue fritzkrieg pi
```

Without them, Quake II shows these bots as `male/grunt`.

## Call for help

The core integration is working, but there is still room for improvement - bot personality tuning, further weapon weight calibration, testing across more maps and game modes, and potential edge cases in the adapter layer. Anyone familiar with Q2 or Q3 engine internals is very welcome to contribute. Join me in my effort of bringing back this brilliant peace of Quake II history to modern engines.
Feel free to fork this repository and to make pull requests. You can contact me at `niehztog (at) gmail.com`.

January 29, 2023
