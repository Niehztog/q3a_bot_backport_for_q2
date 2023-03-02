## Quake 3 Bot documentation
[The Quake III Arena Bot by J.M.P. van Waveren - Master of Science thesis, Delft University of Technology, June 2001][1]

## Findings:
- in Q2 the botlib is loaded as an external library and in Q3 it is compiled in.
- in Q2 the botlib is apparently only loaded when bots are spawned, in Q3 it is loaded once when the exe is started
- in Q2 it seems that each bot can be controlled by its own botlib instance, in Q3 they are all controlled by the same botlib instance


## Comparison of code locations in Q2 vs Q3:
```
Q2                                  Q3                            Description                             Differences
==========================================================================================================================================================
game/g_save.c                       server/sv_init.c
    - InitGame                          - SV_Init                 This will be called when the dll is     Q3: Only called at main exe startup, not for each game
        -> BotSetup                         -> SV_BotInitBotLib   first loaded, which only happens when
                                                                  a new game is started or a save game
                                                                  is loaded.

game/bl_main.c
    - BotSetup                                                    initialize bot globals and allocate bot
        -> BotSetupBotLibImport                                   states
    
game/bl_main.c                      server/sv_bot.c
    - BotSetupBotLibImport              - SV_BotInitBotLib        assign botlib_import functionpointers   Q3 initializes botlib_export aswell
                                            -> GetBotLibAPI

game/bl_spawn.c                     game/g_bot.c
    - BotAddDeathmatch                  - G_AddBot
        -> AddBotToQueue                    -> AddBotToSpawnQueue
    - AddQueuedBots (<- g_main.c)
        -> BotUseLibrary
    - BotBecomeDeathmatch
        -> BotUseLibrary

game/bl_main.c
    - BotUseLibrary                                               Takes botlib dll path as parameter
        -> BotLoadLibrary                                         and loads botlib if not already loaded
                                                                  or returns already loaded botlib
    - BotLoadLibrary                                              Loads botlib via LoadLibrary/dlopen
        -> GetBotLibAPI (botlib)
        -> BotInitLibrary
    - BotInitLibrary                                              Passes cvars from Base (gi) to botlib
        -> BotSetupLibrary

game/bl_botcfg.c                    game/g_bot.c
    - LoadBots                          - G_LoadBots              Loads list of all configured bots       Q2 makes a directory listing on bots/ and
        -> LoadBotsFromFile                 -> G_LoadBotsFromFile                                         Q3 reads the botlist from scripts/bots.txt
    - LoadBotsFromFile                  - G_LoadBotsFromFile      Loads individual bot config             Q2 writes global variable botlist,
        -> AddBotToList                     -> G_ParseInfos                                               Q3 loads config later in botlib?

game/p_client.c                     game/g_client.c
    - ClientConnect                     - ClientConnect           New player connects to the server
        -> BotMoveToFreeClientEdict         -> G_BotConnect

game/bl_spawn.c                     game/g_bot.c
    - BotMoveToFreeClientEdict          - G_BotConnect
        -> G_SpawnClient                    -> BotAISetupClient
        -> G_InitEdict
        -> BotLib_BotMoveClient

game/bl_main.c                      game/ai_main.c
    - BotLib_BotSetupClient             - BotAISetupClient        Configures bot_settings_t and passes    for Q3 only path to the character file
        -> BotSetupClient                   -> BotLoadCharacter   to botlib to load a character           and the skill is passed, for Q2 bot_settings_t

game/p_client.c                     game/g_client.c
    - ClientUserinfoChanged             - ClientUserinfoChanged   called whenever the player updates a    for Q3 it remains unclear how the botlib learns
        -> BotLib_BotClientSettings                               userinfo variable                       about the change

game/g_main.c                       game/g_main.c
    - GetGameAPI                        - vmMain                  Main-Game-Loop                          Q3 also has G_RunFrame, but the BotLib calls
        -> G_RunFrame                       -> BotAIStartFrame                                            are stored in a separate file.

game/g_main.c                       game/ai_main.c
    - G_RunFrame                        - BotAIStartFrame
        -> AddQueuedBots                    -> G_CheckBotSpawn    Add new bots
        -> BotLib_BotStartFrame             -> BotLibStartFrame   ?
        -> BotLib_BotUpdateEntity           -> BotLibUpdateEntity Pass entity data to BotLib
        -> BotLib_BotUpdateClient           -> ?                  Pass player data to BotLib
        -> BotLib_BotAI                     -> BotAI              Lets bot "think"
        -> BotExecuteInput                  -> BotUpdateInput     Executes bot actions
```

## Q3 Botlib:
```
Q3 BotLib                                            Description
==========================================================================================
botlib/be_interface.c
    - GetBotLibAPI                                   Assign botlib_export functionpointers
        -> Init_AAS_Export (Area Awareness System)
        -> Init_EA_Export (Elementary Actions)
        -> Init_AI_Export (Artificial Intelligence)

    - Export_BotLibSetup (Q3) / BotSetupLibrary (Q2) initializes global variables within
        - botlibglobals.botlibsetup = true;          botlib
```
    
## Transfer points between Q2 Base and Game:
```
Q2 Base                             Q2 Game
=================================================================
server/sv_game.c                    game/g_save.c
    - SV_InitGameProgs                  - InitGame
        import = game import
        -> Sys_GetGameAPI
        -> ge.Init
linux/sys_linux.c                   game/g_main.c
    - Sys_GetGameAPI                    - GetGameAPI
        -> GetGameAPI                       gi = game import
                                            globals = game export
```

[1]: http://www.kbs.twi.tudelft.nl/docs/MSc/2001/Waveren_Jean-Paul_van/thesis.pdf