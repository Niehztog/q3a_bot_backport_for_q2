#! /bin/bash
pathname="q3a_bot_backport_for_q2"

cd /mnt/c/Users/nilsg/q2-dev
mkdir $pathname

curl -O https://www.gamers.org/pub/idgames/idstuff/quake2/source/q2src320.shar.Z
curl -O https://www.gamers.org/pub/idgames/idstuff/quake2/source/xatrixsrc320.shar.Z
curl -O https://www.gamers.org/pub/idgames/idstuff/quake2/source/roguesrc320.shar.Z
curl -O https://www.gamers.org/pub/idgames/idstuff/quake2/ctf/source/q2ctf-1.02-source.shar.Z

for i in *.shar.Z ; do
  mkdir ${i%%.*}
  mv -v "$i" ${i%%.*}
  cd ${i%%.*}
  uncompress "$i"
  /bin/sed -i -e 's:^read ans:ans=yes :' "${i%.*}"
  /bin/sed -i -e 's:^more <<EOF:cat <<EOF:' "${i%.*}"
  echo "Extracting $i ..."
  sh "${i%.*}" >/dev/null
  rm "${i%.*}"
  cd ..
done

curl -O https://mrelusive.com/oldprojects/gladiator/ftp/gladq2096gamesrc.zip
[[ -d gladq2096gamesrc ]] || unzip gladq2096gamesrc.zip -d gladq2096gamesrc
rm gladq2096gamesrc.zip

[[ -d Quake-2 ]] || git clone https://github.com/id-Software/Quake-2.git Quake-2

cd Quake-2
git log --pretty=email --patch-with-stat --reverse --full-index --binary -m --first-parent -- \
  qcommon/files.c \
  qcommon/qcommon.h > patch

cd ../$pathname
git init
git am --committer-date-is-author-date < ../Quake-2/patch
git mv qcommon/ qcommon_q2
git commit -m "rename q2 qcommon directory to qcommon_q2"

mv -vf ../q2src320 ./game_q2
mv -vn ../q2ctf-1/*.{c,h} ./game_q2/
for f in {m_boss5.c,m_fixbot.c,m_fixbot.h,m_gekk.c,m_gekk.h,m_gladb.c,m_soldierh.h}; do mv -v ../xatrixsrc320/"${f}" ./game_q2/"${f%.*}_xatrix.${f##*.}"; done
for f in {dm_ball.c,dm_tag.c,g_newai.c,g_newdm.c,g_newfnc.c,g_newtarg.c,g_newtrig.c,g_newweap.c,g_sphere.c,m_carrier.c,m_carrier.h,m_move2.c,m_rider.h,m_stalker.c,m_stalker.h,m_turret.c,m_turret.h,m_widow2.c,m_widow2.h,m_widow.c,m_widow.h}; do mv -v ../roguesrc320/"${f}" ./game_q2/"${f%.*}_rogue.${f##*.}"; done
find ./game_q2/ -type f -print0 | xargs -0 dos2unix -q
git add game_q2
git commit -m "adds original q2 game sources by id software"

mv -fv ../gladq2096gamesrc/* ./game_q2/
find ./game_q2/ -type f -print0 | xargs -0 dos2unix -q
git add game_q2
git commit -m "adds original gladiator game source by Mr Elusive"

cd ..
[[ -d Quake-III-Arena ]] || git clone https://github.com/id-Software/Quake-III-Arena.git Quake-III-Arena
cd Quake-III-Arena
git log --pretty=email --patch-with-stat --reverse --full-index --binary -m --first-parent -- \
  code/botlib \
  code/bspc \
  code/game/ai_chat.c \
  code/game/ai_chat.h \
  code/game/ai_cmd.c \
  code/game/ai_cmd.h \
  code/game/ai_dmnet.c \
  code/game/ai_dmnet.h \
  code/game/ai_dmq3.c \
  code/game/ai_dmq3.h \
  code/game/ai_main.c \
  code/game/ai_main.h \
  code/game/ai_team.c \
  code/game/ai_team.h \
  code/game/ai_vcmd.c \
  code/game/ai_vcmd.h \
  code/game/be_aas.h \
  code/game/be_ai_char.h \
  code/game/be_ai_chat.h \
  code/game/be_ai_gen.h \
  code/game/be_ai_goal.h \
  code/game/be_ai_move.h \
  code/game/be_ai_weap.h \
  code/game/be_ea.h \
  code/game/botlib.h \
  code/game/g_bot.c \
  code/game/q_shared.h \
  code/game/surfaceflags.h \
  code/qcommon/cm_load.c \
  code/qcommon/cm_local.h \
  code/qcommon/cm_patch.* \
  code/qcommon/cm_polylib.h \
  code/qcommon/cm_public.h \
  code/qcommon/cm_test.c \
  code/qcommon/cm_trace.c \
  code/qcommon/md4.* \
  code/qcommon/qcommon.h \
  code/qcommon/qfiles.h \
  code/qcommon/unzip.* > patch

cd ../$pathname
git am --committer-date-is-author-date < ../Quake-III-Arena/patch

git mv code/botlib/be_aas_bspq3.c code/botlib/be_aas_bspq2.c
git mv code/botlib .
git mv code/bspc .
git mv code/qcommon ./qcommon_q3
git mv code/game ./game_q3
rm -rf code
git commit -m "move botlib, bspc and qcommon (q3) to root dir"

find ./botlib -type f \( -name "*.c" -o -name "*.h" \) -print0 | xargs -0 sed 's~#include "../game/~#include "../game_q3/~' -i
find ./bspc -type f \( -name "*.c" -o -name "*.h" \) -print0 | xargs -0 sed 's~#include "../game/~#include "../game_q3/~' -i
sed -e 's~#include "q_shared.h"~#include "../game_q3/q_shared.h"~' \
  -e 's~#include "qfiles.h"~#include "../qcommon_q3/qfiles.h"~' \
  -e 's~#include "botlib.h"~#include "../game_q3/botlib.h"~' \
  -e 's~//#include "l_utils.h"~#include "l_utils.h"~' \
  -e 's~#include "l_libvar.h"~#include "../botlib/l_libvar.h"~' \
  -e 's~#include "l_memory.h"~#include "../botlib/l_memory.h"~' \
  -e 's~#include "be_interface.h"~#include "../botlib/be_interface.h"~' -i bspc/l_utils.c
sed 's~#include "../game/q_shared.h"~#include "../game_q3/q_shared.h"~' -i qcommon_q3/cm_local.h
find ./botlib -type f \( -name "*.c" -o -name "*.h" \) -print0 | xargs -0 sed 's~qcommon/~qcommon_q3/~' -i
find ./bspc -type f \( -name "*.c" -o -name "*.h" \) -print0 | xargs -0 sed 's~qcommon/~qcommon_q3/~' -i
find ./qcommon_q3 -type f \( -name "*.c" -o -name "*.h" \) -print0 | xargs -0 sed 's~qcommon/~qcommon_q3/~' -i

git add .
git commit -m "move q3 files from /game to /game_q3 and from /qcommon to /qcommon_q3"

find ./bspc/ -type f -print0 | xargs -0 dos2unix -q
find ./botlib/ -type f -print0 | xargs -0 dos2unix -q
find ./qcommon_q2/ -type f -print0 | xargs -0 dos2unix -q
find ./qcommon_q3/ -type f -print0 | xargs -0 dos2unix -q
find ./game_q3/ -type f -print0 | xargs -0 dos2unix -q
git add .
git commit -m "convert line breaks in q2 and q3 source files to unix"

find . -type f \( -name "*.c" -o -name "*.h" \) -print0 | xargs -0 sed -i'' -r 's/q(false|true){1}/\1/g'
git add .
git commit -m "replace q3 style boolean expressions with q2 style"

rm -rf ../gladq2096gamesrc ../q2ctf-1 ../roguesrc320 ../xatrixsrc320 ../Quake-III-Arena

sed -e "s/extern\tint\tjacket_armor_index/static\tint\tjacket_armor_index/" -i ./game_q2/g_local.h
sed -e "s/extern\tint\tcombat_armor_index/static\tint\tcombat_armor_index/" -i ./game_q2/g_local.h
sed -e "s/extern\tint\tbody_armor_index/static\tint\tbody_armor_index/" -i ./game_q2/g_local.h
sed -e "s/extern\tqboolean\tis_quad/static\tqboolean\tis_quad/" -i ./game_q2/g_local.h
git add ./game_q2/g_local.h
git commit -m "fixes compiler error in g_local.h"

sed -e 's~gladiator.dll~botlib.dll~' \
    -e 's~gladi386.so~botlib.so~' -i game_q2/bl_spawn.c
sed -e 's~#endif BOT_DEBUG~#endif //BOT_DEBUG~' -i game_q2/bl_cmd.c
sed -i '245,256d;329d;396,629d' ./game_q2/q_shared.c
sed -i '126,131d;133d' ./game_q2/q_shared.h
git add game_q2/bl_spawn.c game_q2/q_shared* game_q2/bl_cmd.c
git commit -m "fix game compilation errors"

sed -i -e "248s~.*~#if 0~" -e "251s~.*~#endif~" ./bspc/l_bsp_hl.c
sed -i '10d' ./qcommon_q3/unzip.c
sed -i -e "22i#include <stdio.h>" ./qcommon_q3/unzip.h
sed -i "946,947s~//~~" bspc/bspc.c
git add bspc/l_bsp_hl.c qcommon_q3/unzip.* bspc/bspc.c
git commit -m "fix bspc compilation errors"

sed -i -e 's~__inline~static __inline~' botlib/be_aas_route.c
sed -i '32d' botlib/l_utils.h
touch ./botlib/standalone.c
echo '#include "../game_q3/q_shared.h"' >>./botlib/standalone.c
echo '#include "../game_q3/botlib.h"' >>./botlib/standalone.c
echo 'extern botlib_import_t botimport;' >>./botlib/standalone.c
echo 'void Com_Memset (void* dest, const int val, const size_t count)' >>./botlib/standalone.c
echo '{' >>./botlib/standalone.c
echo '	memset(dest, val, count);' >>./botlib/standalone.c
echo '}' >>./botlib/standalone.c
echo 'void Com_Memcpy (void* dest, const void* src, const size_t count)' >>./botlib/standalone.c
echo '{' >>./botlib/standalone.c
echo '	memcpy(dest, src, count);' >>./botlib/standalone.c
echo '}' >>./botlib/standalone.c
echo 'void QDECL Com_Error ( int level, const char *error, ... ) {' >>./botlib/standalone.c
echo '	va_list		argptr;' >>./botlib/standalone.c
echo '	char		text[1024];' >>./botlib/standalone.c
echo '' >>./botlib/standalone.c
echo '	va_start (argptr, error);' >>./botlib/standalone.c
echo '	vsprintf (text, error, argptr);' >>./botlib/standalone.c
echo '	va_end (argptr);' >>./botlib/standalone.c
echo '' >>./botlib/standalone.c
echo '    botimport.Print(PRT_ERROR, "%s", text);' >>./botlib/standalone.c
echo '}' >>./botlib/standalone.c
echo 'void Q_strncpyz( char *dest, const char *src, int destsize ) {' >>./botlib/standalone.c
echo '	strncpy( dest, src, destsize-1 );' >>./botlib/standalone.c
echo '    dest[destsize-1] = 0;' >>./botlib/standalone.c
echo '}' >>./botlib/standalone.c
echo 'void QDECL Com_Printf (const char *msg, ...)' >>./botlib/standalone.c
echo '{' >>./botlib/standalone.c
echo '    va_list		argptr;' >>./botlib/standalone.c
echo '    char		text[1024];' >>./botlib/standalone.c
echo '' >>./botlib/standalone.c
echo '    va_start (argptr, msg);' >>./botlib/standalone.c
echo '    vsprintf (text, msg, argptr);' >>./botlib/standalone.c
echo '    va_end (argptr);' >>./botlib/standalone.c
echo '' >>./botlib/standalone.c
echo '    botimport.Print(PRT_MESSAGE, "%s", text);' >>./botlib/standalone.c
echo '}' >>./botlib/standalone.c
echo 'int COM_Compress( char *data_p ) {' >>./botlib/standalone.c
echo '	return strlen(data_p);' >>./botlib/standalone.c
echo '}' >>./botlib/standalone.c
git add botlib/be_aas_route.c botlib/l_utils.h botlib/standalone.c
git commit -m "fix botlib compilation errors"