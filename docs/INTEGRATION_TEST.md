# Integration Test Guide

This document describes how to test the Q3→Q2 botlib backport end-to-end with
Yamagi Quake II and the Gladiator game DLL.

## Prerequisites

- Yamagi Quake II (yquake2) built or installed
- A retail Quake II data directory (`baseq2/pak0.pak` etc.)
- A Q2 multiplayer map with a compiled AAS file (see section 3)

---

## Windows

### 1. Build (Windows / MinGW)

Open the MinGW32 shell from `buildenv/` (see the workspace `setup.sh` for how
to set it up), then:

```bash
cd q3a_bot_backport_for_q2
make
```

The Makefile detects `MINGW` in `YQ2_OSTYPE` and produces `.dll` files:

```
release/game/game.dll
release/botlib/botlib.dll
release/bspc/bspc.exe
```

### 2. Install into the Gladiator mod directory (Windows)

`%YQUAKE2%` below is your Yamagi Quake II installation folder
(e.g. `C:\Games\yquake2`).

```bat
set YQUAKE2=C:\Games\yquake2
set MODDIR=%YQUAKE2%\gladiator

mkdir %MODDIR%\botfiles\bots

:: Game library
copy release\game\game.dll       %MODDIR%\game.dll

:: Bot library — matches the default value of the "botlib" cvar
copy release\botlib\botlib.dll   %MODDIR%\botlib.dll

:: Bot data files; the game reads the bot list from the mod directory
xcopy /E /Y assets\botfiles\*  %MODDIR%\botfiles\
move /Y %MODDIR%\botfiles\bots.cfg %MODDIR%\bots.cfg
```

### 3. Compile an AAS navigation file (Windows)

```bat
copy %YQUAKE2%\baseq2\maps\q2dm1.bsp  %YQUAKE2%\baseq2\maps\

release\bspc\bspc.exe -bsp2aas %YQUAKE2%\baseq2\maps\q2dm1.bsp
```

Alternatively, pre-built AAS files for the standard Q2 DM maps ship with some
Gladiator Bot distribution packages.

### 4. Launch the game (Windows)

```bat
cd %YQUAKE2%
quake2.exe +set game gladiator +map q2dm1 +set maxclients 8
```

To use a different filename, override the cvar:

```bat
quake2.exe +set game gladiator +set botlib mybotlib.dll +map q2dm1
```

---

## Linux

### 1. Build (Linux)

```bash
cd q3a_bot_backport_for_q2
make
```

Artifacts:

```
release/game/game.so
release/botlib/botlib.so
release/bspc/bspc
```

### 2. Install into the Gladiator mod directory (Linux)

```bash
YQUAKE2=/path/to/yquake2
MODDIR=$YQUAKE2/gladiator

mkdir -p $MODDIR/botfiles/bots

# Game library
cp release/game/game.so      $MODDIR/game.so

# Bot library — matches the default value of the "botlib" cvar
cp release/botlib/botlib.so  $MODDIR/botlib.so

# Bot data files; the game reads the bot list from the mod directory
cp -r assets/botfiles/*  $MODDIR/botfiles/
mv $MODDIR/botfiles/bots.cfg $MODDIR/bots.cfg
```

### 3. Compile an AAS navigation file (Linux)

```bash
release/bspc/bspc -bsp2aas $YQUAKE2/baseq2/maps/q2dm1.bsp
# Writes: baseq2/maps/q2dm1.aas
```

### 4. Launch the game (Linux)

```bash
cd $YQUAKE2
./quake2 +set game gladiator +map q2dm1 +set maxclients 8
```

---

## Mod directory layout (both platforms)

```
gladiator/
  game.dll          (Windows)  or  game.so        (Linux)
  botlib.dll        (Windows)  or  botlib.so      (Linux)
  botfiles/
    items.c
    weapons.c
    match.c
    bots/
      default_c.c   (character file — references the weight/chat files below)
      default_i.c   (item weights)
      default_w.c   (weapon weights)
      default.c     (chat — minimal stub)
```

---

## 5. Add a bot

From the Quake II console (`~` key):

```
addbot BotName male/flak bots/default_c.c defaultbot
```

| Argument | Meaning | Example |
|---|---|---|
| 1 | Bot player name | `BotName` |
| 2 | Skin (`model/skin`) | `male/flak` |
| 3 | Character file path (relative to `botfiles/`) | `bots/default_c.c` |
| 4 | Chat name (must match `chat "name"` in the chat file) | `defaultbot` |

The game queues the bot and calls `GetBotAPI` → `BotSetupLibrary` →
`BotLoadMap` → `BotSetupClient` in sequence.

---

## 6. What to observe

### Success indicators

- No `"couldn't load"` errors for `botfiles/items.c`, `botfiles/weapons.c`,
  `bots/default_c.c`, `bots/default_i.c`, `bots/default_w.c` in the console.
- The bot player entity spawns and moves toward items in the map.
- No crash or `FATAL` botlib errors in the server console.

### Expected warnings (non-fatal at this stage)

- `"item X has modelindex 0"` — Normal; items are matched by BSP classname
  only.  Runtime entity matching by model index is not yet wired.
- `"BotChooseLTGItem: no item goal found"` — Expected on maps without a valid
  AAS file.
- `"couldn't load chat …"` — Expected; `bots/default.c` is a minimal stub.

### Diagnosing botlib errors

Enable verbose output from the Quake II console:

```
set bot_developer 1
set bot_report 1
```

Check that `basedir`/`gamedir` are visible to the botlib (it uses them to
locate `botfiles/`):

```
set sv_basedir C:\Games\yquake2    (Windows example)
set gamedir gladiator
```

---

## 7. AAS file path convention

The Q3 botlib looks for `maps/<mapname>.aas` in the game search path
(basedir + gamedir).  Either of these locations works:

```
baseq2/maps/q2dm1.aas
gladiator/maps/q2dm1.aas
```

---

## 8. Next development steps after a successful basic test

| Step | What to implement |
|---|---|
| Combat AI | Hook `BotChooseWeapon` / `BotCheckAttack` in `Q2BotAI` |
| Item model index | Map Q2 `modelindex` values in `Q2BotUpdateEntity` so runtime item tracking works |
| Chat | Populate `bots/default.c` with real Q2-appropriate chat messages |
| Mission pack maps | Xatrix/Rogue items are already in `botfiles/items.c`; test with those maps |
