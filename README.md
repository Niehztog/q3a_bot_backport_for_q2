# Backport of the Quake III Arena Bot for Quake II

## About the Project

The Gladiator Bot was an advanced bot for Quake II developed by Jan Paul "Mr. Elusive" van Waveren. While the Quake II game-side source code was released publicly, the source code of the bot library (`botlib`) was never released.

This project aims to reconstruct the missing bot library by adapting the later Quake III Arena version of the Gladiator Bot back to the Quake II game API.

## History

### The Gladiator Bot

On **December 8, 1998**, Jan Paul "Mr. Elusive" van Waveren released the first version of his new artificial player for Quake II, called "[The Gladiator Bot](https://mrelusive.com/oldprojects/gladiator/gladiator.html)." In the coming months, his bot grew in popularity, as it featured modern artificial intelligence, making it more realistic and therefore superior to most other bots for Quake II at the time.

Mr. Elusive also started porting his bot to other Quake Engine games, such as Half-Life and SiN, but that work was unfortunately never finished. There are still traces of those attempts in the original source code files.

### The Missing Bot Library

On **June 2, 1999**, Mr. Elusive released the game source code for Gladiator Bot v0.95 to the public, allowing mod developers to integrate the Gladiator Bot directly into their mods. However, only the Quake II game-side source code was released; the source code of the bot library, `botlib`, remained private.

The botlib can be thought of as the "brain" of the bots. It contains the implementation of the artificial intelligence, including map navigation, strategic behavior, and other core bot functionality. Without its source code, the complete Gladiator Bot cannot be recompiled, making it difficult to run on modern Quake II engines and preventing further development, such as adding new features, fixing bugs, or porting it to other operating systems and architectures.

### From Gladiator Bot to Quake III Arena

The Gladiator Bot became sufficiently successful that id Software hired Mr. Elusive to work with its team on Quake III Arena and to port and further develop the Gladiator Bot for the new game, as id Software's Graeme Devine announced on **September 29, 1999**. This gave Mr. Elusive the opportunity to further improve the bot's artificial intelligence and routing behavior. The resulting code became the basis for the single-player AI in Quake III Arena.

The complete Quake III Arena source code was officially released by id Software on **August 19, 2005**, under the GNU GPL v2. This release also contained Mr. Elusive's work on the Quake III Arena bots.

### The Start of This Project

Jan Paul "Mr. Elusive" van Waveren sadly passed away on **January 31, 2017**, after suffering from a severe illness. With him died the last hope of obtaining the original source code of the Gladiator Bot's Quake II bot library.

The [gladiator-bot-restored](https://github.com/Niehztog/gladiator-bot-restored) sibling project therefore set out to reconstruct the missing code from the available binary files as an alternative appropach to this backport project.

## Why Backport the Quake III Bot?

Although the Quake III Arena source code contains the evolved version of the Gladiator Bot, it cannot simply be dropped into Quake II. Over time, the bot library was adapted to Quake III Arena's game API and architecture, making it incompatible with the original Quake II Gladiator Bot game library.

The goal of this project is therefore to **backport the Quake III Arena bot library to the Quake II Gladiator Bot API**.

This provides several potential advantages:

* The Quake III bots feature significantly improved artificial intelligence, potentially providing a much better single-player experience in Quake II multiplayer game modes than the original Gladiator Bot.

* With the complete bot source code available, the bot can be ported to other operating systems and architectures and to modern Quake II engines such as [Yamagi Quake II](https://github.com/yquake2/yquake2) and [Q2Pro](https://github.com/q2pro/q2pro).

* The bot can be integrated into newer Quake II mods, allowing its functionality to be further developed and improved.

## How It Works

### The Adapter Layer

The central component of this backport is an adapter layer (`botlib/be_interface_q2.c`) that sits between the Q2 game DLL and the Q3 bot library.

The Q2 game DLL expects botlib to expose a flat set of approximately 20 functions through a `bot_export_t` struct and calls back into the engine through 10 function pointers in `bot_import_t`.

The Q3 botlib, by contrast, organizes its exports into hierarchical sub-structs (AAS, EA, AI) containing more than 100 functions and expects a richer set of engine callbacks.

The adapter translates between these two interfaces. It presents the original Q2 `bot_export_t` interface to the game DLL while driving the full Q3 botlib internally. It also bridges the Q2 engine callbacks, such as tracing, point-contents queries, memory management, and file I/O, to the interfaces expected by the Q3 botlib.

On each frame, the adapter feeds Q2 entity state - including player positions, item pickups, moving platforms, and projectiles - into the Q3 AAS and AI subsystems. It then converts the resulting bot movement and action decisions back into Q2 input commands.

This approach allows the Q3 botlib to run against a Q2 game without requiring modifications to either the Quake II engine or the game DLL beyond the original Gladiator Bot integration points.

## Quake II Game Library Architecture

One specialty of the Gladiator Bot for Quake II is that it incorporates not only the original game modes, but also both mission packs, Capture The Flag, and even the Rocket Arena II game modes into a single game library.

From an architectural point of view, this has several advantages over having everything in separate libraries residing in different directories. One such advantage is the ability to quickly switch between game modes without having to load a different mod, as well as the ability to share game assets between game modes.

With this architecture, Mr. Elusive was ahead of his time. A similar approach later became standard in Quake III Arena, which incorporates the Capture The Flag game mode into the base game, and even in the 2023 enhanced release of Quake II by Nightdive Studios, which incorporates both mission packs and different multiplayer rulesets into the base game.

This architecture is particularly relevant to the backport because the adapter needs to provide the Q3 bot library with a consistent view of the different game modes supported by the Q2 game library.

## Current Status

All relevant source code files have been identified and brought together in this repository. Wherever possible, the original Git history has been preserved to make the evolution of the code easier to understand.

A working Makefile for compilation has been added, based on the Makefile from Yamagi Quake II.

The adapter layer has been implemented, and the bots are now able to load, spawn, and navigate maps. AAS-based navigation, item goal selection, elevator and mover usage, entity type classification (players, items, projectiles, and movers), range-aware weapon weight configuration, and rocket jump support have all been implemented.

The Rogue and Xatrix mission pack items and weapons are accounted for in the bot configuration files, and CTF grappling hook support has been integrated as well.

The bots are currently functioning well, with all major aspects of their functionality successfully ported and adapted to Q2. However, further testing and tuning across different maps and game modes is still required.

Feel free to contribute by reporting any issues you encounter.

### Installing

Download the latest release from the [releases section](https://github.com/Niehztog/q3a_bot_backport_for_q2/releases) and pick the version matching your system's architecture. Extract the release archive into your Quake II directory. The bot files should now reside in the directory `q3bot`right next to `baseq2`. Don't install it over the Gladiator Bot: its bot and map files have the same names as this bot's, but other formats.

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

Capture the Flag and the mission packs also need their game data in the bot's directory, under names Quake II loads from there: copy `ctf/pak0.pak` and `ctf/pak1.pak` to `q3bot/pak0.pak` and `q3bot/pak1.pak`, `xatrix/pak0.pak` (The Reckoning) to `q3bot/pak2.pak` and `rogue/pak0.pak` (Ground Zero) to `q3bot/pak3.pak` and if you like also add the [Rocket Arena 2](https://www.gamers.org/pub/mirrors/ftp.planetquake.com/servers/arena/ra2250cl.exe) pak files.

### Preparing the maps

The bots find their way around a map with its navigation file, `maps/<map>.aas` in the bot's directory, which `bspc` makes from the map. It reads the maps straight out of Quake II's pak files, and takes seconds to a minute per map. This makes `q2dm1.aas` to `q2dm8.aas` for the eight deathmatch maps:

On Windows: `bspc.exe -bsp2aas "C:\Quake2\baseq2\pak1.pak\maps\q2dm*.bsp" -output C:\Quake2\q3bot\maps`

On Linux: `bspc -bsp2aas "$Q2/baseq2/pak1.pak/maps/q2dm*.bsp" -output $Q2/q3bot/maps`

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

### Quake 3 Player models

The bots appear as their Quake III Arena characters: `bots.cfg` gives each bot the player model and skin that Quake III's `scripts/bots.txt` gives it (`sarge/default`, `biker/cadavre`, ...), and Team Arena's for Fritzkrieg and pi. These models are part of Quake III Arena and Team Arena and are not included here. Convert them from your own copies of the games with [q3player2md2](https://github.com/Niehztog/q3player2md2) and copy what it writes to `out/players/` into `baseq2/players/` of every Quake II installation that plays with the bots. Given the mission packs' data, it also puts their weapons in the players' hands:

```
python3 q3player2md2.py --q3 <quake3>/baseq3 --q2 <quake2>/baseq2 --q2 <quake2>/xatrix --q2 <quake2>/rogue --all
python3 q3player2md2.py --q3 <quake3>/baseq3 --q3 <quake3>/missionpack --q2 <quake2>/baseq2 --q2 <quake2>/xatrix --q2 <quake2>/rogue fritzkrieg pi
```

Without them, Quake II shows these bots as the default `male/grunt` model.

## Development

### Reproducing the Source Setup

To make it easier to understand how this repository was assembled, all necessary steps have been documented and incorporated into a [setup script](./setup.sh).

The script downloads the required sources, extracts them, and assembles them into the structure used by this repository. Anyone wishing to continue this work can use the script as a starting point for setting up the required source tree.

### Technical Documentation

The current analysis of the structural differences between the Q2 Gladiator Bot and the Q3 bot library is documented in the [development documentation](./docs/DEVELOPMENT.md).


## How to compile

This repository and Makefile have been optimized to work with the Yamagi Q2 Build Environment. Follow [this guide](https://github.com/yquake2/yquake2/blob/master/doc/020_installation.md#compiling-from-source) on how to set up the build environment, clone this repo inside mingw32 shell, change to its directory and type `make`.

## Using it from a game

`make` builds three files into `release/`:

- `game/game.so` (`game.dll` on Windows) is **the game**: the Gladiator Bot's game library for Quake II, which plays deathmatch, Capture the Flag, Rocket Arena and both mission packs, and adds the bots to them.
- `botlib/botlib.so` (`botlib.dll`) is **the bot library**: the Quake III Arena bot. The game loads it when the first bot joins.
- `bspc/bspc` (`bspc.exe`) prepares maps for the bots.

The steps below are for [Yamagi Quake II](https://github.com/yquake2/yquake2).


### For game and mod developers

`make botlib bspc` builds just the bot library and `bspc`, all that a game that embeds the bot needs. Give `CFLAGS` and `LDFLAGS` in the environment, not on make's command line: the Makefile adds `-fPIC` and `-shared` to them per target, and make drops such additions to a variable set on its command line. A cross build has to name its target, because the Makefile otherwise builds for the machine it runs on: `make YQ2_OSTYPE=Windows YQ2_ARCH=i386 CC=i686-w64-mingw32-gcc botlib bspc` (`YQ2_ARCH=x86_64` and `CC=x86_64-w64-mingw32-gcc` for 64-bit Windows).

The bot library is loaded the way the Gladiator Bot's was. The game opens it, looks up `GetBotAPI` and calls it with a `bot_import_t`, the functions the bot calls in the game, and gets back a `bot_export_t`, the bot's functions for the game. [botlib/be_interface_q2.h](./botlib/be_interface_q2.h) declares both. They are the Gladiator Bot's tables, so a game written for its library loads this one unchanged; the export table's entries after Gladiator's twenty are this library's own. The structs the tables pass hold no pointers, so they are the same in 32-bit and 64-bit builds, and the header checks their sizes when it is compiled.


## Call for help

The core integration is working, but there is still room for improvement - bot personality tuning, further weapon weight calibration, testing across more maps and game modes, and potential edge cases in the adapter layer. Anyone familiar with Q2 or Q3 engine internals is very welcome to contribute. Join me in my effort of bringing back this brilliant peace of Quake II history to modern engines.
Feel free to fork this repository and to make pull requests. You can contact me at `niehztog (at) gmail.com`.

January 29, 2023
