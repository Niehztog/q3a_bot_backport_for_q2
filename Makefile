# ----------------------------------------------------------------- #
# Makefile for the gladiator game and botlib modules for Quake II   #
#                                                                   #
# Just type "make" to compile the                                   #
#  - Gladiator Game (game.so)                                       #
#  - BSPC tool for the Gladiator Bot                                #
#  - Gladiator Botlib (botlib.so)                                   #
#                                                                   #
# Dependencies:                                                     #
# - None, but you need a Quake II to play.                          #
#   While in theory every client should work                        #
#   Yamagi Quake II ist recommended.                                #
#                                                                   #
# Platforms:                                                        #
# - FreeBSD                                                         #
# - Linux                                                           #
# - Mac OS X                                                        #
# - OpenBSD                                                         #
# - Windows                                                         #
# ----------------------------------------------------------------- #

# Detect the OS
ifdef SystemRoot
YQ2_OSTYPE ?= Windows
else
YQ2_OSTYPE ?= $(shell uname -s)
endif

# Special case for MinGW
ifneq (,$(findstring MINGW,$(YQ2_OSTYPE)))
YQ2_OSTYPE := Windows
endif

# Detect the architecture
ifeq ($(YQ2_OSTYPE), Windows)
ifdef MINGW_CHOST
ifeq ($(MINGW_CHOST), x86_64-w64-mingw32)
YQ2_ARCH ?= x86_64
else # i686-w64-mingw32
YQ2_ARCH ?= i386
endif
else # windows, but MINGW_CHOST not defined
ifdef PROCESSOR_ARCHITEW6432
# 64 bit Windows
YQ2_ARCH ?= $(PROCESSOR_ARCHITEW6432)
else
# 32 bit Windows
YQ2_ARCH ?= $(PROCESSOR_ARCHITECTURE)
endif
endif # windows but MINGW_CHOST not defined
else
ifneq ($(YQ2_OSTYPE), Darwin)
# Normalize some abiguous YQ2_ARCH strings
YQ2_ARCH ?= $(shell uname -m | sed -e 's/i.86/i386/' -e 's/amd64/x86_64/' -e 's/arm64/aarch64/' -e 's/^arm.*/arm/')
else
YQ2_ARCH ?= $(shell uname -m)
endif
endif

# On Windows / MinGW $(CC) is undefined by default.
ifeq ($(YQ2_OSTYPE),Windows)
CC ?= gcc
endif

# Detect the compiler
ifeq ($(shell $(CC) -v 2>&1 | grep -c "clang version"), 1)
COMPILER := clang
COMPILERVER := $(shell $(CC)  -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/')
else ifeq ($(shell $(CC) -v 2>&1 | grep -c -E "(gcc version|gcc-Version)"), 1)
COMPILER := gcc
COMPILERVER := $(shell $(CC)  -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/')
else
COMPILER := unknown
endif

# ----------

# Base CFLAGS. These may be overridden by the environment.
# Highest supported optimizations are -O2, higher levels
# will likely break this crappy code.
ifdef DEBUG
CFLAGS ?= -O0 -g -Wall -pipe
else
CFLAGS ?= -Wall -pipe -fomit-frame-pointer
endif

# Always needed are:
#  -fno-strict-aliasing since the source doesn't comply
#   with strict aliasing rules and it's next to impossible
#   to get it there...
#  -fwrapv for defined integer wrapping. MSVC6 did this
#   and the game code requires it.
override CFLAGS += -std=gnu99 -fno-strict-aliasing -fwrapv

# -MMD to generate header dependencies. Unsupported by
#  the Clang shipped with OS X.
ifneq ($(YQ2_OSTYPE), Darwin)
override CFLAGS += -MMD
endif

# OS X architecture.
ifeq ($(YQ2_OSTYPE), Darwin)
override CFLAGS += -arch $(YQ2_ARCH)
endif

# ----------

# Switch of some annoying warnings.
ifeq ($(COMPILER), clang)
	# -Wno-missing-braces because otherwise clang complains
	#  about totally valid 'vec3_t bla = {0}' constructs.
	CFLAGS += -Wno-missing-braces
else ifeq ($(COMPILER), gcc)
	# GCC 8.0 or higher.
	ifeq ($(shell test $(COMPILERVER) -ge 80000; echo $$?),0)
	    # -Wno-format-truncation and -Wno-format-overflow
		# because GCC spams about 50 false positives.
    	CFLAGS += -Wno-format-truncation -Wno-format-overflow
	endif
endif

# ----------

# Defines the operating system and architecture
override CFLAGS += -DYQ2OSTYPE=\"$(YQ2_OSTYPE)\" -DYQ2ARCH=\"$(YQ2_ARCH)\"

# ----------

# For reproduceable builds, look here for details:
# https://reproducible-builds.org/specs/source-date-epoch/
ifdef SOURCE_DATE_EPOCH
CFLAGS += -DBUILD_DATE=\"$(shell date --utc --date="@${SOURCE_DATE_EPOCH}" +"%b %_d %Y" | sed -e 's/ /\\ /g')\"
endif

# ----------

# Using the default x87 float math on 32bit x86 causes rounding trouble
# -ffloat-store could work around that, but the better solution is to
# just enforce SSE - every x86 CPU since Pentium3 supports that
# and this should even improve the performance on old CPUs
ifeq ($(YQ2_ARCH), i386)
override CFLAGS += -msse -mfpmath=sse
endif

# Force SSE math on x86_64. All sane compilers should do this
# anyway, just to protect us from broken Linux distros.
ifeq ($(YQ2_ARCH), x86_64)
override CFLAGS += -mfpmath=sse
endif

ifeq ($(YQ2_OSTYPE), Linux)
override CFLAGS += -DARCH_STRING=\"$(YQ2_ARCH)\"
endif

BOTCFLAGS=-O0

# ----------

# Base LDFLAGS.
LDFLAGS ?=

# Required libaries
ifeq ($(YQ2_OSTYPE), Darwin)
override LDFLAGS += -arch $(YQ2_ARCH)
else ifeq ($(YQ2_OSTYPE), Windows)
override LDFLAGS += -static-libgcc
else
override LDFLAGS += -lm
endif

# ----------

# Phony targets
.PHONY : all clean game bspc botlib

# ----------

# Builds everything
all: game bspc botlib

# ----------

# Cleanup
clean:
	rm -Rf build release/*

cleanall:
	rm -Rf build release

# ----------

# The gladiator game
ifeq ($(YQ2_OSTYPE), Windows)
game:
	mkdir -p release/game
	$(MAKE) release/game/game.dll

build/game/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

release/game/game.dll : LDFLAGS += -shared

else ifeq ($(YQ2_OSTYPE), Darwin)

game:
	mkdir -p release/game
	$(MAKE) release/game/game.dylib

build/game/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

release/game/game.dylib : CFLAGS += -fPIC
release/game/game.dylib : LDFLAGS += -shared

else # not Windows or Darwin

game:
	mkdir -p release/game
	$(MAKE) release/game/game.so

build/game/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

release/game/game.so : CFLAGS += -fPIC -Wno-unused-result
release/game/game.so : LDFLAGS += -shared
endif

# ----------

# The BSPC tool for the Gladiator Bot

ifeq ($(YQ2_OSTYPE), Windows)
bspc:
	mkdir -p release/bspc
	$(MAKE) release/bspc/bspc.exe

build/bspc/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INCLUDE) -Dstricmp=strcasecmp -DMAC_STATIC= -DQDECL= -DBSPC -D_FORTIFY_SOURCE=2 -o $@ $<

release/bspc/bspc.exe : LDFLAGS += -Wl,--allow-multiple-definition

else ifeq ($(YQ2_OSTYPE), Darwin)

bspc:
	mkdir -p release/bspc
	$(MAKE) release/bspc/bspc

build/bspc/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INCLUDE) -Dstricmp=strcasecmp -DMAC_STATIC= -DQDECL= -DBSPC -D_FORTIFY_SOURCE=2 -o $@ $<

release/bspc/bspc : CFLAGS += -fPIC
release/bspc/bspc : LDFLAGS += -Wl,--allow-multiple-definition

else # not Windows or Darwin

bspc:
	mkdir -p release/bspc
	$(MAKE) release/bspc/bspc

build/bspc/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INCLUDE) -Dstricmp=strcasecmp -DMAC_STATIC= -DQDECL= -DBSPC -D_FORTIFY_SOURCE=2 -o $@ $<

release/bspc/bspc : CFLAGS += -fPIC -Wno-unused-result
release/bspc/bspc : LDFLAGS += -Wl,--allow-multiple-definition
endif

# ----------

# The gladiator botlib
ifeq ($(YQ2_OSTYPE), Windows)
botlib:
	mkdir -p release/botlib
	$(MAKE) release/botlib/botlib.dll

build/botlib/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INCLUDE) $(BOTCFLAGS) -DBOTLIB -o $@ $<

release/botlib/botlib.dll : LDFLAGS += -shared

else ifeq ($(YQ2_OSTYPE), Darwin)

botlib:
	mkdir -p release/botlib
	$(MAKE) release/botlib/botlib.dylib

build/botlib/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INCLUDE) $(BOTCFLAGS) -DBOTLIB -o $@ $<

release/botlib/botlib.dylib : CFLAGS += -fPIC
release/botlib/botlib.dylib : LDFLAGS += -shared

else # not Windows or Darwin

botlib:
	mkdir -p release/botlib
	$(MAKE) release/botlib/botlib.so

build/botlib/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(INCLUDE) $(BOTCFLAGS) -DBOTLIB -o $@ $<

release/botlib/botlib.so : CFLAGS += -fPIC -Wno-unused-result
release/botlib/botlib.so : LDFLAGS += -shared
endif

# ----------

GAME_OBJS_ = \
	game_q2/bl_cmd.o \
	game_q2/bl_botcfg.o \
	game_q2/bl_debug.o \
	game_q2/bl_main.o \
	game_q2/bl_redirgi.o \
	game_q2/bl_spawn.o \
	game_q2/dm_ball_rogue.o \
	game_q2/dm_tag_rogue.o \
	game_q2/g_ai.o \
	game_q2/g_arena.o \
	game_q2/g_ch.o \
	game_q2/g_chase.o \
	game_q2/g_cmds.o \
	game_q2/g_combat.o \
	game_q2/g_ctf.o \
	game_q2/g_func.o \
	game_q2/g_items.o \
	game_q2/g_log.o \
	game_q2/g_main.o \
	game_q2/g_misc.o \
	game_q2/g_monster.o \
	game_q2/g_newai_rogue.o \
	game_q2/g_newdm_rogue.o \
	game_q2/g_newfnc_rogue.o \
	game_q2/g_newtarg_rogue.o \
	game_q2/g_newtrig_rogue.o \
	game_q2/g_newweap_rogue.o \
	game_q2/g_phys.o \
	game_q2/g_save.o \
	game_q2/g_spawn.o \
	game_q2/g_sphere_rogue.o \
	game_q2/g_svcmds.o \
	game_q2/g_target.o \
	game_q2/g_trigger.o \
	game_q2/g_turret.o \
	game_q2/g_utils.o \
	game_q2/g_weapon.o \
	game_q2/m_actor.o \
	game_q2/m_berserk.o \
	game_q2/m_boss2.o \
	game_q2/m_boss3.o \
	game_q2/m_boss31.o \
	game_q2/m_boss32.o \
	game_q2/m_boss5_xatrix.o \
	game_q2/m_brain.o \
	game_q2/m_carrier_rogue.o \
	game_q2/m_chick.o \
	game_q2/m_fixbot_xatrix.o \
	game_q2/m_flash.o \
	game_q2/m_flipper.o \
	game_q2/m_float.o \
	game_q2/m_flyer.o \
	game_q2/m_gekk_xatrix.o \
	game_q2/m_gladb_xatrix.o \
	game_q2/m_gladiator.o \
	game_q2/m_gunner.o \
	game_q2/m_hover.o \
	game_q2/m_infantry.o \
	game_q2/m_insane.o \
	game_q2/m_medic.o \
	game_q2/m_move.o \
	game_q2/m_move2_rogue.o \
	game_q2/m_mutant.o \
	game_q2/m_parasite.o \
	game_q2/m_soldier.o \
	game_q2/m_stalker_rogue.o \
	game_q2/m_supertank.o \
	game_q2/m_tank.o \
	game_q2/m_turret_rogue.o \
	game_q2/m_widow2_rogue.o \
	game_q2/m_widow_rogue.o \
	game_q2/p_botmenu.o \
	game_q2/p_client.o \
	game_q2/p_hud.o \
	game_q2/p_lag.o \
	game_q2/p_menu.o \
	game_q2/p_menulib.o \
	game_q2/p_trail.o \
	game_q2/p_view.o \
	game_q2/p_weapon.o \
	game_q2/q_shared.o
	# qcommon_q2/files.o
	# game_q2/p_observer.o \

# ----------

BSPC_OBJS_ = \
	botlib/be_aas_bspq2.o \
	botlib/be_aas_cluster.o \
	botlib/be_aas_move.o \
	botlib/be_aas_optimize.o \
	botlib/be_aas_reach.o \
	botlib/be_aas_sample.o \
	botlib/l_libvar.o \
	botlib/l_precomp.o \
	botlib/l_script.o \
	botlib/l_struct.o \
	bspc/aas_areamerging.o \
	bspc/aas_cfg.o \
	bspc/aas_create.o \
	bspc/aas_edgemelting.o \
	bspc/aas_facemerging.o \
	bspc/aas_file.o \
	bspc/aas_gsubdiv.o \
	bspc/aas_map.o \
	bspc/aas_prunenodes.o \
	bspc/aas_store.o \
	bspc/be_aas_bspc.o \
	bspc/brushbsp.o \
	bspc/bspc.o \
	bspc/csg.o \
	bspc/glfile.o \
	bspc/leakfile.o \
	bspc/l_bsp_ent.o \
	bspc/l_bsp_hl.o \
	bspc/l_bsp_q1.o \
	bspc/l_bsp_q2.o \
	bspc/l_bsp_q3.o \
	bspc/l_bsp_sin.o \
	bspc/l_cmd.o \
	bspc/l_log.o \
	bspc/l_math.o \
	bspc/l_mem.o \
	bspc/l_poly.o \
	bspc/l_qfiles.o \
	bspc/l_threads.o \
	bspc/l_utils.o \
	bspc/map.o \
	bspc/map_hl.o \
	bspc/map_q1.o \
	bspc/map_q2.o \
	bspc/map_q3.o \
	bspc/map_sin.o \
	bspc/nodraw.o \
	bspc/portals.o \
	bspc/textures.o \
	bspc/tree.o \
	bspc/_files.o \
	qcommon_q3/cm_load.o \
	qcommon_q3/cm_patch.o \
	qcommon_q3/cm_test.o \
	qcommon_q3/cm_trace.o \
	qcommon_q3/md4.o \
	qcommon_q3/unzip.o

    #bspc/faces.o \
	#bspc/prtfile.o \
	#bspc/writebsp.o

# ----------

BOTLIB_OBJS_ = \
	botlib/be_aas_bspq2.o \
	botlib/be_aas_cluster.o \
	botlib/be_aas_debug.o \
	botlib/be_aas_entity.o \
	botlib/be_aas_file.o \
	botlib/be_aas_main.o \
	botlib/be_aas_move.o \
	botlib/be_aas_optimize.o \
	botlib/be_aas_reach.o \
	botlib/be_aas_route.o \
	botlib/be_aas_routealt.o \
	botlib/be_aas_sample.o \
	botlib/be_ai_char.o \
	botlib/be_ai_chat.o \
	botlib/be_ai_gen.o \
	botlib/be_ai_goal.o \
	botlib/be_ai_move.o \
	botlib/be_ai_weap.o \
	botlib/be_ai_weight.o \
	botlib/be_ea.o \
	botlib/be_interface.o \
	botlib/l_crc.o \
	botlib/l_libvar.o \
	botlib/l_log.o \
	botlib/l_memory.o \
	botlib/l_precomp.o \
	botlib/l_script.o \
	botlib/l_struct.o \
	botlib/standalone.o \
	bspc/l_utils.o \
	game_q2/q_shared.o

# ----------

# Rewrite pathes to our object directory
GAME_OBJS = $(patsubst %,build/game/%,$(GAME_OBJS_))
BSPC_OBJS = $(patsubst %,build/bspc/%,$(BSPC_OBJS_))
BOTLIB_OBJS = $(patsubst %,build/botlib/%,$(BOTLIB_OBJS_))

# ----------

# Generate header dependencies
GAME_DEPS= $(GAME_OBJS:.o=.d)
BSPC_DEPS= $(BSPC_OBJS:.o=.d)
BOTLIB_DEPS= $(BOTLIB_OBJS:.o=.d)

# ----------

# Suck header dependencies in
-include $(GAME_DEPS)
-include $(BSPC_DEPS)
-include $(BOTLIB_DEPS)

# ----------

# release/baseq2/game.so
ifeq ($(YQ2_OSTYPE), Windows)
release/game/game.dll : $(GAME_OBJS)
	$(CC) -o $@ $(GAME_OBJS) $(LDFLAGS)
else ifeq ($(YQ2_OSTYPE), Darwin)
release/game/game.dylib : $(GAME_OBJS)
	$(CC) -o $@ $(GAME_OBJS) $(LDFLAGS)
else
release/game/game.so : $(GAME_OBJS)
	$(CC) -o $@ $(GAME_OBJS) $(LDFLAGS)
endif

# ----------

# release/bspc/bspc
ifeq ($(YQ2_OSTYPE), Windows)
release/bspc/bspc.exe : $(BSPC_OBJS)
	$(CC) -o $@ $(BSPC_OBJS) $(LDFLAGS)
else ifeq ($(YQ2_OSTYPE), Darwin)
release/bspc/bspc : $(BSPC_OBJS)
	$(CC) -o $@ $(BSPC_OBJS) $(LDFLAGS)
else
release/bspc/bspc : $(BSPC_OBJS)
	$(CC) -o $@ $(BSPC_OBJS) $(LDFLAGS)
endif

# ----------

ifeq ($(YQ2_OSTYPE), Windows)
release/botlib/botlib.dll : $(BOTLIB_OBJS)
	$(CC) -o $@ $(BOTLIB_OBJS) $(LDFLAGS)
else ifeq ($(YQ2_OSTYPE), Darwin)
release/botlib/botlib.dylib : $(BOTLIB_OBJS)
	$(CC) -o $@ $(BOTLIB_OBJS) $(LDFLAGS)
else
release/botlib/botlib.so : $(BOTLIB_OBJS)
	$(CC) -o $@ $(BOTLIB_OBJS) $(LDFLAGS)
endif

# ----------
