/*
 * bl_chat.h -- Public interface for the Q2 bot chat dispatch layer.
 *
 * BotChat_NotifyDeath() is called from p_client.c on player deaths.
 * BotChat_Frame() is called from bl_main.c each frame for active bots.
 * BotChat_OnEnterGame() / BotChat_OnExitGame() are called from
 * bl_main.c when bots join or leave.
 *
 * The actual chat logic lives in game_q3/ai_chat.c (Q3 code compiled
 * with Q2 compatibility via ai_chat_q2.h).
 */

#ifndef BL_CHAT_H
#define BL_CHAT_H

/* Notification hooks (called from game events) */
void BotChat_NotifyDeath(edict_t *self, edict_t *attacker, int mod);

/* Enter/exit wrappers (called from bl_main.c) */
void BotChat_OnEnterGame(edict_t *bot);
void BotChat_OnExitGame(edict_t *bot);

/* Per-frame chat checks (called from bl_main.c) */
void BotChat_Frame(edict_t *bot);

/* Level start/end (called from g_spawn.c / p_hud.c) */
void BotChat_OnStartLevel(void);
void BotChat_OnEndLevel(void);

/* Player chat notification (called from Cmd_Say_f in g_cmds.c) */
void BotChat_OnPlayerSay(edict_t *sender, const char *msg);

#endif /* BL_CHAT_H */
