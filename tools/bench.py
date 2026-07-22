#!/usr/bin/env python3
"""Seeded fast-forward bench: play seeded bot games per arm and map, several at
a time and ~75x faster than real time, then compare the arms.

    tools/bench.py [-n 10] [--first 1] [-j 4] [--maps q2dm1,q2dm7] [--ctf]
                   [--minutes 5] [--bots 4] ARM [ARM ...]

An ARM is NAME[=SOURCE][,OPTION...].  SOURCE is `.` (the working tree, the
default), `@REV` (a git revision), a directory holding a checkout of this repo,
or `orig` (the 1999 Gladiator gamei386.so and gladi386.so, under box86 and
r1q2ded-old).  Built arms take these OPTIONs; the originals take only set.CVAR:

    data=all|assets|gladiator   bot data: the revision's assets/botfiles, with
                                botfiles/ laid over it in revisions that have
                                one (all, the default), assets/botfiles alone
                                without the bot personalities (every bot is
                                the default character), or Gladiator's own
                                (its pak7.pak, bots.cfg and version 3 AAS, for
                                its botlib)
    game=PATH, botlib=PATH      a game.so or botlib from elsewhere in place of
                                the arm's own, e.g. the reconstructed
                                Gladiator's (the game loads the file the
                                botlib cvar names: botlib.so, gladi386.so for
                                Gladiator's game, so add set.botlib=botlib.so)
    autoinit=zero|pattern|none  -ftrivial-auto-var-init (default zero)
    coverage=1                  a --coverage build, for --functions
    set.CVAR=VALUE              an extra `+set CVAR VALUE` for this arm's runs

    tools/bench.py head=@HEAD wt                     is my edit behaviour-neutral?
    tools/bench.py -n 20 main=@main head=@HEAD       A/B statistics
    tools/bench.py -n 20 head=@HEAD orig=orig        against the 1999 originals
    tools/bench.py z=. p=.,autoinit=pattern          the stack-uninit detector
    tools/bench.py -n 2 --functions BotCTFSeekGoals c=.,coverage=1
    tools/bench.py wt glad=.,game=$G/release/game/game.so,set.botlib=botlib.so
                                                     our botlib, Gladiator's game

Seed n of every arm is the same game until the arms' code differs.  The first
arm is the reference.  Every other arm on the same engine is checked seed by
seed for a byte-identical game (the log from "==== InitGame ====" to
"Timelimit hit.", timing prints dropped), and the first differing line of each
divergent seed is named.  Per arm and map the report gives mean (sd) per game
of kills, suicides plus world deaths, top frags, chat lines and botlib
warnings (captures with --ctf), each arm's kills and self-deaths relative to
the reference in standard errors, which weapons the kills were made with,
what the self-deaths were, and the most frequent botlib warnings.

Every run is a private dir under --out: the engine and data by symlink, the
arm's libraries and bot data, a copy of the map's AAS, its own HOME.  Nothing
under the engine install or this checkout's release/ is touched; built arms
compile in --out from a copy of their source.  Our botlib reads Q3 AAS files
(version 5): bench.py compiles any missing one with this repo's bspc (--bspc)
from the map in the retail paks, into --out/aas/bspc-<build> (the originals
keep Gladiator's own, version 3, from the install).  Logs stay in --out/logs as ARM_MAP_SEED.log, and
--rescore reports on them without running anything, so batches of seeds can
be pooled.  Exit status 1 means a run failed, or, with --require-identical,
that an arm played a different game.

Linux agent only: the seed pin is an LD_PRELOAD (tools/bench_seed.c).  The
method is gladiator-bot-restored's tools/bench.py; its determinism matrix,
noise levels and traps are in that repo's .claude/memory/fixedtime_bench.md.
"""
import argparse
import hashlib
import math
import os
import re
import shlex
import shutil
import signal
import struct
import subprocess
import sys
import tempfile
import threading
import time
from collections import Counter
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
SHIM_SRC = REPO / "tools" / "bench_seed.c"
DATA = Path.home() / "q2-dev" / "yquake2" / "release_"
ORIGINALS = Path.home() / "q2-dev" / "glad_orig" / "glibc"

# What a built arm compiles and runs from.  botfiles/ exists only in the revisions
# from the Q3 AI port up to its merge into assets/botfiles.
TREE = ("Makefile", "botlib", "bspc", "game_q2", "game_q3", "qcommon_q3", "assets", "botfiles")
GAME = "q3bot"                       # the mod dir of built arms
TIMELIMIT = "Timelimit hit."
ENTERED = " entered the game"
# r1q2 stamps every console line with the time -- the pinned one, so the raw
# log shows at a glance whether the seed pin took.
STAMP = re.compile(r"^\[\d{4}-\d\d-\d\d \d\d:\d\d\] ", re.M)
GAME_START = "==== InitGame ===="
# botlib timing prints ("map loaded in 12 msec") differ from run to run, and
# the game names the run dir ("loaded /tmp/q3bench/slot2/q3bot/botlib.so")
TIMING = re.compile(r"\d+ msec")
RUNDIR = re.compile(r"\S*/slot\d+/")
# An instrumented build prints its counters on lines of their own that start
# with this; they stay in the logs but out of the comparison, so a probe arm
# can be checked to play its base build's games.
PROBE = "PROBE "
# Gladiator's game prints this without a newline (bl_redirgi.c Bot_unicast,
# on Rogue maps), gluing the next line -- "Timelimit hit.", an obituary -- to it
UNICAST = "WARNING: tried to use unicast for a bot"
# "<name> <text>." broadcasts that are not deaths (g_ctf.c, g_target.c).
NOT_DEATHS = ("defends the ", "joined the ", "changed to the ", "exited the level")
# the paks the originals' install dir links, and the ones a built arm gets
# (CTF maps live in ctf/pak0.pak; Gladiator's pak7.pak would hand our botlib
# Gladiator's bot files wherever ours lack one)
PAKS = {"pak0.pak": "ctf/pak0.pak", "pak1.pak": "ctf/pak1.pak",
        "pak2.pak": "xatrix/pak0.pak", "pak3.pak": "rogue/pak0.pak"}
METRICS = ("kills", "self", "top", "chat", "warn")
# The shim's base is 1000000000 + seed * 100000, and must fit a 32-bit time_t.
MAX_SEED = 11474

WEAPONS = {
    "BLASTER": "blaster", "SHOTGUN": "sg", "SSHOTGUN": "ssg", "MACHINEGUN": "mg",
    "CHAINGUN": "cg", "GRENADE": "gl", "G_SPLASH": "gl", "ROCKET": "rl",
    "R_SPLASH": "rl", "HYPERBLASTER": "hb", "RAILGUN": "rg", "BFG_LASER": "bfg",
    "BFG_BLAST": "bfg", "BFG_EFFECT": "bfg", "HANDGRENADE": "hgren",
    "HG_SPLASH": "hgren", "HELD_GRENADE": "hgren", "TELEFRAG": "telefrag",
    "GRAPPLE": "grapple",
}


class Arm:
    """NAME[=SOURCE][,OPTION...] -- see the module docstring."""

    def __init__(self, spec, out):
        head, *opts = spec.split(",")
        self.name, _, source = head.partition("=")
        self.source = source or "."
        if not re.fullmatch(r"[A-Za-z0-9-]+", self.name):
            sys.exit(f"bench: arm name {self.name!r}: use letters, digits and '-'")
        self.orig = self.source == "orig"
        self.autoinit, self.data, self.coverage = "zero", "all", False
        self.cvars, self.prebuilt = [], {}
        for opt in opts:
            key, eq, val = opt.partition("=")
            if key.startswith("set.") and eq and len(key) > 4:
                self.cvars += ["+set", key[4:], val]
            elif self.orig:
                sys.exit(f"bench: arm {self.name}: the originals take only set.CVAR=VALUE")
            elif key == "autoinit" and val in ("zero", "pattern", "none"):
                self.autoinit = val
            elif key == "data" and val in ("all", "assets", "gladiator"):
                self.data = val
            elif key in ("game", "botlib") and val:
                self.prebuilt[key] = Path(val).expanduser().resolve()
                if not self.prebuilt[key].is_file():
                    sys.exit(f"bench: arm {self.name}: {key}={val}: no such file")
            elif key == "coverage" and val in ("0", "1"):
                self.coverage = val == "1"
            else:
                sys.exit(f"bench: arm {self.name}: unknown option {opt!r}")
        self.dir = out / f"arm-{self.name}"
        self.src = self.dir / "src"
        self.botlib = self.prebuilt.get("botlib", self.src / "release" / "botlib" / "botlib.so")
        self.game = self.prebuilt.get("game", self.src / "release" / "game" / "game.so")
        self.botfiles = self.dir / "botfiles"

    def describe(self):
        if self.orig:
            what = "the 1999 Gladiator originals under box86"
        else:
            what = {".": "working tree"}.get(self.source, self.source)
            if self.source.startswith("@"):
                sha = subprocess.run(["git", "-C", str(REPO), "rev-parse", "--short",
                                      self.source[1:]], capture_output=True, text=True)
                what += f" ({sha.stdout.strip() or 'unknown revision'})"
            what += f", data={self.data}, autoinit={self.autoinit}"
            what += "".join(f", {k}={v}" for k, v in self.prebuilt.items())
            if self.coverage:
                what += ", coverage"
        if self.cvars:
            what += ", " + " ".join(self.cvars)
        return what


# ---------------------------------------------------------------- building

def makefile_cflags(makefile):
    """The Makefile's non-DEBUG `CFLAGS ?=` default.  CFLAGS on make's command
    line replaces it, so an arm that adds a flag has to restate it."""
    found = re.findall(r"^CFLAGS \?= (.*)$", makefile.read_text(), re.M)
    if len(found) != 2:
        sys.exit(f"bench: {makefile}: the CFLAGS defaults changed shape; "
                 "update makefile_cflags()")
    return found[1].split()


def extract(source, dst, what):
    """The TREE of a revision (@REV) or a checkout directory into dst."""
    dst.mkdir(parents=True)
    if source.startswith("@"):
        rev = source[1:]
        have = subprocess.run(["git", "-C", str(REPO), "ls-tree", "--name-only", rev],
                              capture_output=True, text=True)
        if have.returncode:
            sys.exit(f"bench: {what}: git ls-tree {rev}: {have.stderr.strip()}")
        paths = [p for p in TREE if p in have.stdout.split()]
        tar = subprocess.run(["git", "-C", str(REPO), "archive", rev] + paths,
                             capture_output=True)
        if tar.returncode:
            sys.exit(f"bench: {what}: git archive {rev}: "
                     f"{tar.stderr.decode(errors='replace').strip()}")
        subprocess.run(["tar", "-x", "-C", str(dst)], input=tar.stdout, check=True)
    else:
        tree = REPO if source == "." else Path(source).resolve()
        if not (tree / "Makefile").is_file():
            sys.exit(f"bench: {what}: {tree} holds no Makefile")
        for p in TREE:
            if (tree / p).is_dir():
                shutil.copytree(tree / p, dst / p, ignore=shutil.ignore_patterns("*.o", "*.d"))
            elif (tree / p).is_file():
                shutil.copy2(tree / p, dst / p)


def build(arm, jobs):
    shutil.rmtree(arm.dir, ignore_errors=True)
    extract(arm.source, arm.src, f"arm {arm.name}")
    targets = [t for t in ("botlib", "game") if t not in arm.prebuilt]
    if targets:
        cmd = ["make", "-C", str(arm.src), f"-j{jobs}"] + targets
        extra = []
        if arm.autoinit != "none":
            extra.append(f"-ftrivial-auto-var-init={arm.autoinit}")
        if arm.coverage:
            extra.append("--coverage")
            cmd.append("LDFLAGS=--coverage")
        if extra:
            cmd.append("CFLAGS=" + " ".join(makefile_cflags(arm.src / "Makefile") + extra))
        log = arm.dir / "build.log"
        with open(log, "w") as f:
            rc = subprocess.run(cmd, stdout=f, stderr=subprocess.STDOUT).returncode
        if rc or not (arm.botlib.is_file() and arm.game.is_file()):
            tail = log.read_text(errors="replace").splitlines()[-15:]
            sys.exit(f"bench: arm {arm.name}: build failed, see {log}\n" + "\n".join(tail))
    if arm.data == "gladiator":
        return
    # the bot data the revision ships: assets/botfiles, then botfiles/ over it
    # where the revision has one
    shutil.copytree(arm.src / "assets" / "botfiles", arm.botfiles)
    if arm.data == "all" and (arm.src / "botfiles").is_dir():
        shutil.copytree(arm.src / "botfiles", arm.botfiles, dirs_exist_ok=True)
    if arm.data == "assets":
        # a bot whose character file is missing is the default character
        for f in (arm.botfiles / "bots").glob("*_c.c"):
            if f.name != "default_c.c":
                f.unlink()


def build_shims(out, native, i386):
    d = out / "shim"
    d.mkdir(parents=True, exist_ok=True)
    shims = {}
    todo = []
    if native:
        todo.append(("native", ["gcc", "-shared", "-fPIC", "-O2", "-Wall"], "bench_seed.so"))
    if i386:
        todo.append(("i386", ["i686-linux-gnu-gcc", "-DBENCH_BOX86", "-shared", "-fPIC",
                              "-nostartfiles", "-O2", "-Wall"], "bench_seed_i386.so"))
    for key, cc, name in todo:
        so = d / name
        r = subprocess.run(cc + ["-o", str(so), str(SHIM_SRC), "-ldl"],
                           capture_output=True, text=True)
        if r.returncode:
            sys.exit(f"bench: {' '.join(cc[:1])} {SHIM_SRC.name}:\n{r.stderr}")
        shims[key] = so
    return shims


# -------------------------------------------------------------------- maps

def aas_version(path):
    if not path.is_file():
        return None
    with open(path, "rb") as f:
        head = f.read(8)
    return struct.unpack("<4si", head)[1] if len(head) == 8 and head[:4] == b"EAAS" else None


def pak_entry(paks, name):
    """The bytes of name from the first pak holding it."""
    for pak in paks:
        if not pak.is_file():
            continue
        with open(pak, "rb") as f:
            ident, off, size = struct.unpack("<4sii", f.read(12))
            if ident != b"PACK":
                continue
            f.seek(off)
            for _ in range(size // 64):
                n, pos, length = struct.unpack("<56sii", f.read(64))
                if n.split(b"\0")[0].decode(errors="replace").lower() == name:
                    f.seek(pos)
                    return f.read(length)
    return None


def build_bspc(args):
    """This repo's bspc, from --bspc (a revision is built once and kept, the
    working tree or a directory every run, tagged by the binary)."""
    source = args.bspc
    tag = None
    if source.startswith("@"):
        sha = subprocess.run(["git", "-C", str(REPO), "rev-parse", source[1:]],
                             capture_output=True, text=True).stdout.strip()
        if not sha:
            sys.exit(f"bench: --bspc {source}: unknown revision")
        tag = sha[:12]
        cached = args.out / "tools" / f"bspc-{tag}"
        if cached.is_file():
            return cached
    src = args.out / "tools" / "src"
    shutil.rmtree(src, ignore_errors=True)
    extract(source, src, "bspc")
    log = args.out / "tools" / "build.log"
    # bspc.c prints __DATE__ and __TIME__: pinned, the same source builds
    # the same binary, and the tag below finds its AAS files again
    env = dict(os.environ, SOURCE_DATE_EPOCH="0")
    with open(log, "w") as f:
        rc = subprocess.run(["make", "-C", str(src), f"-j{os.cpu_count() or 4}", "bspc"],
                            stdout=f, stderr=subprocess.STDOUT, env=env).returncode
    if rc:
        sys.exit(f"bench: building bspc failed, see {log}")
    built = src / "release" / "bspc" / "bspc"
    if tag is None:
        tag = hashlib.md5(built.read_bytes()).hexdigest()[:12]
    bspc = args.out / "tools" / f"bspc-{tag}"
    shutil.copy2(built, bspc)
    return bspc


def ensure_aas(maps, args):
    """Q3 AAS files for our botlib, compiled from the retail BSPs with this
    repo's bspc (--bspc, default HEAD), kept per bspc build.  bspc is
    deterministic: a fresh q2dm1 is byte-identical to one compiled months
    earlier."""
    bspc = build_bspc(args)
    d = args.out / "aas" / bspc.name
    d.mkdir(parents=True, exist_ok=True)
    missing = [m for m in maps if aas_version(d / f"{m}.aas") != 5]
    if not missing:
        return d
    paks = [p for game in ("baseq2", "ctf", "xatrix", "rogue")
            for p in sorted((args.data / game).glob("pak*.pak"))]
    for m in missing:
        bsp = pak_entry(paks, f"maps/{m}.bsp")
        if bsp is None:
            sys.exit(f"bench: maps/{m}.bsp is in none of {', '.join(map(str, paks))}")
        work = Path(tempfile.mkdtemp(dir=d))
        (work / f"{m}.bsp").write_bytes(bsp)
        r = subprocess.run([str(bspc), "-bsp2aas", f"{m}.bsp"], cwd=work,
                           capture_output=True, text=True)
        if r.returncode or aas_version(work / f"{m}.aas") != 5:
            sys.exit(f"bench: bspc {m}: rc={r.returncode}\n{r.stdout[-2000:]}")
        shutil.move(str(work / f"{m}.aas"), d / f"{m}.aas")
        shutil.rmtree(work)
        print(f"bench: compiled {d / m}.aas")
    return d


# ------------------------------------------------------------------ running

def stage(rd, arm, mapname, args):
    """A private run dir: engine and data by symlink, the arm's libraries, a
    COPY of the AAS, and a HOME of its own, so qconsole.log and config.cfg are
    per run."""
    shutil.rmtree(rd, ignore_errors=True)
    for d in (".yq2", ".local/share"):
        (rd / "home" / d).mkdir(parents=True)
    os.symlink((args.data / "baseq2").resolve(), rd / "baseq2")
    if arm.orig:
        glad = rd / "gladiator"
        (glad / "maps").mkdir(parents=True)
        os.symlink((args.data / "r1q2ded-old").resolve(), rd / "r1q2ded-old")
        for f in ("gamei386.so", "gladi386.so", "bots.cfg"):
            shutil.copy2(args.originals / f, glad / f)
        for pak in sorted((args.data / "gladiator").glob("*.pak")):
            os.symlink(pak.resolve(), glad / pak.name)
        # r1q2 writes a log only when logfile arrives from an exec'd config.
        (glad / "autoexec.cfg").write_text(f"set logfile 2\nset developer {args.developer}\n")
        shutil.copy2(args.data / "gladiator" / "maps" / f"{mapname}.aas", glad / "maps")
    else:
        mod = rd / GAME
        (mod / "maps").mkdir(parents=True)
        os.symlink((args.data / "q2ded").resolve(), rd / "q2ded")
        for name, pak in PAKS.items():
            if (args.data / pak).is_file():
                os.symlink((args.data / pak).resolve(), mod / name)
        os.symlink(arm.game, mod / "game.so")
        os.symlink(arm.botlib, mod / "botlib.so")
        if arm.data == "gladiator":
            os.symlink((args.data / "gladiator" / "pak7.pak").resolve(), mod / "pak7.pak")
            shutil.copy2(args.originals / "bots.cfg", mod / "bots.cfg")
            shutil.copy2(args.data / "gladiator" / "maps" / f"{mapname}.aas", mod / "maps")
        else:
            os.symlink(arm.botfiles, mod / "botfiles")
            shutil.copy2(args.botcfg, mod / "bots.cfg")
            shutil.copy2(args.aasdir / f"{mapname}.aas", mod / "maps")


def command(arm, mapname, seed, slot, args, shims):
    rd = args.out / f"slot{slot}"
    env = dict(os.environ, HOME=str(rd / "home"), BENCH_SEED=str(seed))
    files = {}
    common = ["+set", "deathmatch", "1", "+set", "dmflags", "16",
              "+set", "timelimit", str(args.minutes), "+set", "fraglimit", "0",
              "+set", "bots_minplayers", str(args.bots),
              "+set", "minimumplayers", str(args.bots),
              "+set", "maxclients", str(max(8, args.bots + 4)),
              "+set", "developer", str(args.developer),
              "+set", "ip", "127.0.0.1", "+set", "port", str(args.port + 20 * slot),
              "+set", "public", "0"]
    if args.ctf:
        common += ["+set", "ctf", "1"]
    tail = arm.cvars + shlex.split(args.args) + ["+map", mapname]
    if arm.orig:
        env.update(BOX86_LD_PRELOAD=str(shims["i386"]), BOX86_LOG="0", BOX86_NOBANNER="1")
        # r1q2's fixedtime is in milliseconds (3.20); +set basedir doubles its paths.
        cmd = ["box86", "./r1q2ded-old", "+set", "game", "gladiator"] + common + [
            "+set", "fixedtime", "100", "+exec", "autoexec.cfg"] + tail
        log = rd / "gladiator" / "qconsole.log"
    else:
        env["LD_PRELOAD"] = str(shims["native"])
        # yquake2's is in MICROseconds.  One 100 ms game frame per loop is the
        # most SV_Frame takes.  basedir: the game reads ./<game>/bots.cfg from
        # the cwd, and the botlib its files from basedir/gamedir.  The rest
        # goes through a config: yquake2 takes at most 50 arguments.
        sets = common + ["+set", "fixedtime", "100000", "+set", "logfile", "2"]
        files[f"{GAME}/bench.cfg"] = "".join(f"set {sets[i + 1]} {sets[i + 2]}\n"
                                             for i in range(0, len(sets), 3))
        cmd = ["./q2ded", "+set", "basedir", str(rd), "+set", "game", GAME,
               "+exec", "bench.cfg"] + tail
        log = rd / "home" / ".yq2" / GAME / "qconsole.log"
    return rd, cmd, env, log, files


def watch(p, log, stall, timeout):
    """None once the game hits its timelimit, else why the run failed."""
    start = last = time.time()
    size = pos = 0
    carry = b""
    want = TIMELIMIT.encode()
    while True:
        time.sleep(0.1)
        now = time.time()
        try:
            cur = log.stat().st_size
        except FileNotFoundError:
            cur = 0
        if cur != size:
            size, last = cur, now
        if size > pos:
            with open(log, "rb") as f:
                f.seek(pos)
                chunk = carry + f.read(size - pos)
            pos = size
            if want in chunk:
                return None
            carry = chunk[-len(want):]
        if p.poll() is not None:
            return f"exited rc={p.returncode}"
        if now - last > stall:
            return f"stalled: no output for {stall:.0f} s"
        if now - start > timeout:
            return f"timeout after {timeout:.0f} s"


def stop(p):
    """SIGTERM, then SIGKILL.  r1q2ded-old under box86 can deadlock in its
    SIGTERM handler (futex_wait, forever); yquake2 exits cleanly, which the
    coverage counters need."""
    for sig, grace in ((signal.SIGTERM, 2), (signal.SIGKILL, 10)):
        if p.poll() is not None:
            return
        try:
            os.killpg(p.pid, sig)
        except ProcessLookupError:
            return
        try:
            p.wait(timeout=grace)
        except subprocess.TimeoutExpired:
            pass


def run(job, slot, args, shims, live, lock, stopping):
    arm, mapname, seed = job
    label = f"{arm.name}_{mapname}_{seed}"
    dst = args.logdir / f"{label}.log"
    dst.unlink(missing_ok=True)
    (args.logdir / f"{label}.stdout.log").unlink(missing_ok=True)
    rd, cmd, env, log, files = command(arm, mapname, seed, slot, args, shims)
    stage(rd, arm, mapname, args)
    for name, text in files.items():
        (rd / name).write_text(text)
    with open(rd / "stdout.log", "wb") as out:
        with lock:
            # an interrupt must not leave an engine behind in its own session
            if stopping.is_set():
                return "interrupted"
            p = subprocess.Popen(cmd, cwd=rd, env=env, stdin=subprocess.DEVNULL,
                                 stdout=out, stderr=subprocess.STDOUT, start_new_session=True)
            live.add(p)
        try:
            why = watch(p, log, args.stall, args.timeout)
        finally:
            stop(p)
            with lock:
                live.discard(p)
    dst.write_bytes(log.read_bytes() if log.is_file() else b"")
    if why:
        shutil.copy2(rd / "stdout.log", args.logdir / f"{label}.stdout.log")
    return why


def run_all(jobs, args, shims):
    """Worker k owns run dir slot<k> and port --port + 20*k."""
    failed, live, lock, stopping = {}, set(), threading.Lock(), threading.Event()
    queue = list(jobs)

    def worker(slot):
        while True:
            with lock:
                if not queue:
                    return
                job = queue.pop(0)
            why = run(job, slot, args, shims, live, lock, stopping)
            if why:
                arm, mapname, seed = job
                with lock:
                    failed[(arm.name, mapname, seed)] = why
                print(f"  {arm.name} {mapname} seed {seed}: {why}", file=sys.stderr)

    threads = [threading.Thread(target=worker, args=(k,), daemon=True)
               for k in range(min(args.j, len(queue)))]
    try:
        for t in threads:
            t.start()
        for t in threads:
            while t.is_alive():
                t.join(0.5)
    finally:
        with lock:
            stopping.set()
            queue.clear()
            running = list(live)
        for p in running:
            stop(p)
    return failed


# ------------------------------------------------------------------ scoring

def obituaries(src):
    """ClientObituary's messages (game_q2/p_client.c; the 1999 game prints the
    same ones): kill[(message, message2)] -> weapon, own[message] -> cause.
    Self-deaths are world deaths (the first switch) and deaths by one's own
    hand (the `attacker == self` switch)."""
    text = src.read_text(errors="replace")
    body = text[text.index("void ClientObituary"):]
    body = body[:body.index("\n}\n")]
    i_self = body.index("if (attacker == self)")
    i_kill = body.index("if (attacker && attacker->client)", i_self)
    assign = re.compile(r'(message2?)\[(\d+)\]\s*=\s*"((?:[^"\\]|\\.)*)"')
    label = re.compile(r"case\s+MOD_(\w+)\s*:|(default)\s*:")

    def groups(section):
        out, mods, msgs, last_was_case = [], [], [], False
        for line in section.splitlines():
            m = label.search(line)
            if m:
                if not last_was_case and mods:
                    out.append((mods, msgs))
                    mods, msgs = [], []
                mods.append(m.group(1) or "OTHER")
                last_was_case = True
                continue
            for a in assign.finditer(line):
                msgs.append((a.group(1), int(a.group(2)), a.group(3)))
                last_was_case = False
        if mods:
            out.append((mods, msgs))
        return out

    own, kill = {}, {}
    for section, cause in ((body[:i_self], "world"), (body[i_self:i_kill], "own")):
        for mods, msgs in groups(section):
            name = "+".join(sorted({m.lower() for m in mods}))
            for kind, _, s in msgs:
                if kind == "message":
                    own[s] = f"{cause}:{name}"
    for mods, msgs in groups(body[i_kill:]):
        weapon = WEAPONS.get(mods[0], mods[0].lower())
        for kind, k, s in msgs:
            if kind != "message":
                continue
            seconds = [s2 for kind2, k2, s2 in msgs if kind2 == "message2" and k2 == k] or [""]
            for s2 in seconds:
                kill[(s, s2)] = weapon
            kill.setdefault((s, ""), weapon)
    return kill, own


OBITUARY = obituaries(REPO / "game_q2" / "p_client.c")


def normalize(raw):
    """Console text as printed: Q2's green (high-bit) characters folded to
    plain ASCII, r1q2's time stamps, the botlib's timing prints and the lines
    of instrumented builds dropped."""
    text = RUNDIR.sub("<run>/", bytes(b & 0x7f for b in raw).decode("latin-1"))
    text = text.replace(UNICAST, UNICAST + "\n")
    return [l for l in STAMP.sub("", text).splitlines()
            if not TIMING.search(l) and not l.startswith(PROBE)]


class Game:
    """The comparable part of one log, and its score."""

    def __init__(self, raw):
        lines = normalize(raw)
        self.lines = self.start = self.score = None
        if TIMELIMIT not in lines:
            return
        end = lines.index(TIMELIMIT)
        self.start = next((i for i, l in enumerate(lines[:end]) if l == GAME_START), 0)
        self.lines = lines[self.start:end + 1]
        self.score = score(self.lines)


def score(lines):
    """ClientObituary (game_q2/p_client.c) prints "%s %s." for a suicide or
    world death (victim -1), "%s %s %s%s" for a kill (attacker +1), and
    "%s died."."""
    kill_msgs, own_msgs = OBITUARY
    names = sorted({l[:-len(ENTERED)] for l in lines if l.endswith(ENTERED)},
                   key=len, reverse=True)
    frags = dict.fromkeys(names, 0)
    s = dict(kills=0, self=0, top=0, chat=0, caps=0, warn=0, err=0,
             weapons=Counter(), causes=Counter(), warnings=Counter())
    for l in lines:
        if l.startswith("Warning: "):
            s["warn"] += 1
            s["warnings"][re.sub(r"-?\d+(\.\d+)?", "#", l[9:])[:90]] += 1
            continue
        if l.startswith(("Error: ", "Fatal: ")):
            s["err"] += 1
            s["warnings"][re.sub(r"-?\d+(\.\d+)?", "#", l)[:90]] += 1
            continue
        victim = next((n for n in names if l.startswith(n + " ")), None)
        if victim is None:
            if any(l.startswith(n + ": ") or l.startswith(f"({n}): ") for n in names):
                s["chat"] += 1
            continue
        rest = l[len(victim) + 1:]
        weapon = attacker = None
        for (m1, m2), w in kill_msgs.items():
            if not rest.startswith(m1 + " "):
                continue
            tail = rest[len(m1) + 1:]
            a = next((n for n in names if tail.startswith(n) and tail[len(n):] == m2), None)
            if a:
                weapon, attacker = w, a
                break
        if attacker is None:
            # a message the table does not know: any other name counts
            found = min(((rest.find(n), -len(n), n) for n in names if n in rest), default=None)
            if found:
                weapon, attacker = "?", found[2]
        if attacker:
            s["kills"] += 1
            s["weapons"][weapon] += 1
            frags[attacker] += 1
        elif rest.startswith("captured the "):
            s["caps"] += 1
        elif rest.endswith(".") and not rest.startswith(NOT_DEATHS):
            s["self"] += 1
            s["causes"][own_msgs.get(rest[:-1], "died" if rest == "died." else "?")] += 1
            frags[victim] -= 1
    s["top"] = max(frags.values(), default=0)
    return s


def first_diff(a, b):
    """Log line (1-based, in a's file) where two games part, or None."""
    if a.lines == b.lines:
        return None
    i = next((k for k, (x, y) in enumerate(zip(a.lines, b.lines)) if x != y),
             min(len(a.lines), len(b.lines)))
    return a.start + i + 1


def mean_sd(v):
    mu = sum(v) / len(v)
    return mu, math.sqrt(sum((x - mu) ** 2 for x in v) / (len(v) - 1)) if len(v) > 1 else 0.0


def report(arms, maps, seeds, args):
    keys = METRICS + (("caps",) if args.ctf else ())
    ref, differs = arms[0], False
    for m in maps:
        print(f"\n{m}, seeds {seeds[0]}-{seeds[-1]}")
        print("  %-12s %3s  %s" % ("arm", "n", " ".join("%-13s" % k for k in keys)))
        games = {}
        for a in arms:
            games[a.name] = {}
            for s in seeds:
                path = args.logdir / f"{a.name}_{m}_{s}.log"
                g = Game(path.read_bytes()) if path.is_file() else None
                games[a.name][s] = g if g and g.lines else None
        vals = {}
        for a in arms:
            done = [g for g in games[a.name].values() if g]
            if not done:
                print("  %-12s   0  no complete games" % a.name)
                continue
            vals[a.name] = {k: [g.score[k] for g in done] for k in keys}
            print("  %-12s %3d  %s" % (a.name, len(done), " ".join(
                "%-13s" % ("%.1f (%.1f)" % mean_sd(vals[a.name][k])) for k in keys)))
            if a is ref or ref.name not in vals:
                continue
            notes = []
            same = False
            if a.orig == ref.orig:
                both = [s for s in seeds if games[ref.name][s] and games[a.name][s]]
                parted = [(s, first_diff(games[ref.name][s], games[a.name][s])) for s in both]
                parted = [(s, line) for s, line in parted if line]
                note = f"same game as {ref.name} on {len(both) - len(parted)}/{len(both)} seeds"
                if parted:
                    differs = True
                    note += " (seed@line: " + " ".join(f"{s}@{line}" for s, line in parted[:6])
                    note += " ...)" if len(parted) > 6 else ")"
                notes.append(note)
                same = not parted and len(both) == len(vals[a.name]["kills"]) \
                    == len(vals[ref.name]["kills"])
            for k in () if same else ("kills", "self"):
                (ma, sa), (mb, sb) = mean_sd(vals[a.name][k]), mean_sd(vals[ref.name][k])
                se = math.sqrt(sa * sa / len(vals[a.name][k]) + sb * sb / len(vals[ref.name][k]))
                notes.append(f"{k} {ma - mb:+.1f}" + (f" = {(ma - mb) / se:+.1f} se" if se else ""))
            print("  %-12s      %s" % ("", "; ".join(notes)))
        report_breakdown(arms, games, "kills by weapon, % of all kills", "weapons", percent=True)
        report_breakdown(arms, games, "self-deaths by cause, per game", "causes", percent=False)
        for a in arms:
            done = [g for g in games[a.name].values() if g]
            total = sum((g.score["warnings"] for g in done), Counter())
            if total:
                print(f"  {a.name} botlib warnings/errors, per game:")
                for text, n in total.most_common(args.warnings):
                    print(f"    {n / len(done):7.1f}  {text}")
    return differs


CAUSES = {"world:lava": "lava", "world:slime": "slime", "world:water": "drown",
          "world:falling": "fall", "world:crush": "crush", "world:suicide": "kill-cmd",
          "world:bomb+splash+trigger_hurt": "hurt", "world:target_laser": "laser",
          "world:barrel+explosive": "barrel", "own:r_splash": "own-rl",
          "own:g_splash+hg_splash": "own-gren", "own:held_grenade": "held-gren",
          "own:bfg_blast": "own-bfg", "own:other": "own-other"}


def report_breakdown(arms, games, title, key, percent):
    rows = {}
    for a in arms:
        done = [g for g in games[a.name].values() if g]
        if done:
            rows[a.name] = (sum((g.score[key] for g in done), Counter()), len(done))
    cols = sorted({c for tot, _ in rows.values() for c in tot},
                  key=lambda c: -sum(tot[c] / n for tot, n in rows.values()))
    if not cols:
        return
    print(f"  {title}:")
    print("    %-12s %s" % ("", " ".join("%9s" % CAUSES.get(c, c)[-9:] for c in cols)))
    for name, (tot, n) in rows.items():
        whole = sum(tot.values()) or 1
        print("    %-12s %s" % (name, " ".join(
            "%9.1f" % (100 * tot[c] / whole if percent else tot[c] / n) for c in cols)))


def report_coverage(arms, functions):
    for a in arms:
        if not a.coverage or not a.src.is_dir():
            continue
        found = {}
        for gcda in sorted((a.src / "build").rglob("*.gcda")):
            rel = gcda.relative_to(a.src / "build").with_suffix(".c")
            tu = Path(*rel.parts[1:])  # build/<target>/<dir>/<file>.o
            out = subprocess.run(["gcov", "-n", "-f", "-o", str(gcda.parent), str(tu)],
                                 cwd=a.src, capture_output=True, text=True).stdout
            for fn, pct, n in re.findall(r"^Function '([^']+)'\nLines executed:([\d.]+)% of (\d+)",
                                         out, re.M):
                if fn in functions:
                    found.setdefault(fn, []).append(f"{pct}% of {n} lines, {tu}")
        print(f"\ncoverage, arm {a.name}, all its runs:")
        for fn in functions:
            print(f"  {fn:<30} " + ("; ".join(found[fn]) if fn in found else "not found"))


# --------------------------------------------------------------- self-check

SAMPLE = """\
[2001-09-10 07:33] ==== InitGame ====
[2001-09-10 07:33] Bill Gates entered the game
[2001-09-10 07:33] player entered the game
[2001-09-10 07:33] Bill Gates: howdy
[2001-09-10 07:33] (player): the enemy flag will be mine!
[2001-09-10 07:33] player sucked on Bill Gates's boomstick
[2001-09-10 07:33] Bill Gates bites the slug from player
[2001-09-10 07:33] player had his brains blown out by Bill Gates
[2001-09-10 07:33] Bill Gates cratered.
[2001-09-10 07:33] player tripped on his own grenade.
[2001-09-10 07:33] Warning: goal heap overflow
[2001-09-10 07:33] map loaded in 12 msec
[2001-09-10 07:33] player joined the RED team.
[2001-09-10 07:33] player defends the RED flag.
[2001-09-10 07:33] player captured the BLUE flag!
[2001-09-10 07:33] Timelimit hit.
[2001-09-10 07:33] Bill Gates: bunch of loosers
"""


def self_check():
    """The scorer, the arm parser and the shim, without an engine."""
    problems = []
    g = Game(SAMPLE.encode())
    want = dict(kills=3, self=2, top=1, chat=2, caps=1, warn=1)
    got = {k: g.score[k] for k in want}
    if got != want:
        problems.append(f"score {got} != {want}")
    if dict(g.score["weapons"]) != {"rl": 1, "rg": 2}:
        problems.append(f"weapons {dict(g.score['weapons'])}")
    if set(g.score["causes"]) != {"world:falling", "own:g_splash+hg_splash"}:
        problems.append(f"causes {dict(g.score['causes'])}")
    if g.start != 0 or g.lines[0] != GAME_START:
        problems.append(f"game section starts at {g.start}")
    h = Game(SAMPLE.replace("Bill Gates cratered.", "Bill Gates melted.").encode())
    if first_diff(g, h) != 9:
        problems.append(f"first_diff {first_diff(g, h)} != 9")
    if Game(SAMPLE.replace(TIMELIMIT, "").encode()).lines is not None:
        problems.append("a log without the timelimit counted as complete")
    arm = Arm("x=@HEAD,data=assets,autoinit=pattern,set.ctf=1", Path("/nonexistent"))
    if (arm.source, arm.data, arm.autoinit, arm.cvars) != ("@HEAD", "assets", "pattern",
                                                         ["+set", "ctf", "1"]):
        problems.append("arm parser")
    native, i386 = bool(shutil.which("gcc")), bool(shutil.which("i686-linux-gnu-gcc"))
    with tempfile.TemporaryDirectory() as d:
        try:
            build_shims(Path(d), native, i386)
        except SystemExit as e:
            problems.append(f"shim: {e}")
    if not (native and i386):
        print("self-check: shim compile skipped for "
              + ", ".join(n for n, have in (("gcc", native), ("i686-linux-gnu-gcc", i386))
                          if not have))
    for p in problems:
        print(f"self-check: {p}")
    print("self-check: " + ("FAILED" if problems else "ok"))
    return 1 if problems else 0


# --------------------------------------------------------------------- main

def check_setup(arms, maps, args):
    if Path("/mnt/c").is_dir() and os.environ.get("WSL_DISTRO_NAME"):
        sys.exit("bench: Linux agent only -- the seed pin is an LD_PRELOAD")
    need = [args.data / "baseq2"]
    progs = ["gcc"]
    if any(not a.orig for a in arms):
        need += [args.data / "q2ded", args.botcfg]
        progs.append("make")
    if any(a.orig for a in arms):
        need += [args.data / "r1q2ded-old", args.data / "gladiator" / "maps"]
        need += [args.originals / f for f in ("gamei386.so", "gladi386.so", "bots.cfg")]
        progs += ["box86", "i686-linux-gnu-gcc"]
        for m in maps:
            aas = args.data / "gladiator" / "maps" / f"{m}.aas"
            if not aas.is_file():
                sys.exit(f"bench: the originals have no {aas}")
    if any(a.coverage for a in arms):
        progs.append("gcov")
    missing = [str(p) for p in need if not p.exists()] + [p for p in progs if not shutil.which(p)]
    if missing:
        sys.exit("bench: missing " + ", ".join(missing))


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("arms", nargs="*", metavar="ARM")
    ap.add_argument("-n", type=int, default=10, help="seeds per arm and map (default 10)")
    ap.add_argument("--first", type=int, default=1, help="first seed (default 1)")
    ap.add_argument("-j", type=int, default=os.cpu_count() or 4,
                    help="games at a time (default: one per CPU)")
    ap.add_argument("--maps", help="comma-separated (default q2dm1,q2dm7; q2ctf1,q2ctf4 with --ctf)")
    ap.add_argument("--ctf", action="store_true", help="+set ctf 1")
    ap.add_argument("--minutes", type=int, default=5, help="timelimit of each game (default 5)")
    ap.add_argument("--bots", type=int, default=4, help="bots per game (default 4)")
    ap.add_argument("--developer", type=int, default=0, help="developer cvar (default 0)")
    ap.add_argument("--args", default="", help="extra engine arguments for every run")
    ap.add_argument("--port", type=int, default=27800, help="first port; +20 per slot")
    ap.add_argument("--out", type=Path, default=Path("/tmp/q3bench"),
                    help="builds, run dirs and logs (default /tmp/q3bench)")
    ap.add_argument("--data", type=Path, default=DATA,
                    help=f"engine install with q2ded, baseq2/, ctf/, gladiator/ (default {DATA})")
    ap.add_argument("--originals", type=Path, default=ORIGINALS,
                    help=f"the 1999 .so files for `orig` (default {ORIGINALS})")
    ap.add_argument("--bspc", default="@HEAD",
                    help="source of the bspc that compiles missing AAS files: @REV, . or a "
                         "checkout (default @HEAD)")
    ap.add_argument("--botcfg", type=Path, default=REPO / "assets" / "botfiles" / "bots.cfg",
                    help="the bot list every built arm draws from (default assets/botfiles/bots.cfg)")
    # 8-bot CTF on a big map (q2ctf4) under four-way load goes quiet for more
    # than 20 s of real time without being stuck
    ap.add_argument("--stall", type=float, default=60, help="seconds without output = stalled")
    ap.add_argument("--timeout", type=float, default=300, help="seconds per game at most")
    ap.add_argument("--warnings", type=int, default=5, help="botlib warnings listed per arm")
    ap.add_argument("--functions", default="", help="comma-separated: lines executed, per coverage=1 arm")
    ap.add_argument("--rescore", action="store_true", help="report on the logs already in --out")
    ap.add_argument("--require-identical", action="store_true",
                    help="exit 1 unless every arm plays the reference's games")
    ap.add_argument("--self-check", action="store_true", help="test the scorer and shim, no engine")
    args = ap.parse_args()
    if args.self_check:
        return self_check()
    if not args.arms:
        ap.error("give at least one ARM")
    if args.n < 1 or args.first < 1 or args.first + args.n - 1 > MAX_SEED:
        ap.error(f"seeds must lie in 1..{MAX_SEED}")
    args.out = args.out.resolve()
    args.logdir = args.out / "logs"
    args.botcfg = args.botcfg.resolve()
    arms = [Arm(spec, args.out) for spec in args.arms]
    if len({a.name for a in arms}) != len(arms):
        ap.error("arm names must be unique")
    maps = (args.maps or ("q2ctf1,q2ctf4" if args.ctf else "q2dm1,q2dm7")).split(",")
    seeds = list(range(args.first, args.first + args.n))
    functions = [f for f in args.functions.split(",") if f]

    failed = {}
    if not args.rescore:
        check_setup(arms, maps, args)
        args.logdir.mkdir(parents=True, exist_ok=True)
        if any(not a.orig for a in arms):
            args.aasdir = ensure_aas(maps, args)
        jobs = [(a, m, s) for s in seeds for m in maps for a in arms]
        print(f"bench: {len(arms)} arm(s) x {len(maps)} map(s) x {len(seeds)} seed(s) = "
              f"{len(jobs)} games of {args.minutes} min with {args.bots} bots, "
              f"{args.j} at a time; logs in {args.logdir}")
        for a in arms:
            print(f"  {a.name:<12} {a.describe()}")
            if not a.orig:
                build(a, os.cpu_count() or 4)
        shims = build_shims(args.out, any(not a.orig for a in arms), any(a.orig for a in arms))
        t0 = time.time()
        try:
            failed = run_all(jobs, args, shims)
        except KeyboardInterrupt:
            print("bench: interrupted; the engines it had started are stopped", file=sys.stderr)
            return 130
        print(f"bench: {len(jobs)} games in {time.time() - t0:.0f} s")

    differs = report(arms, maps, seeds, args)
    if functions:
        report_coverage(arms, functions)
    if failed:
        print(f"\n{len(failed)} run(s) failed; their engine output is in {args.logdir}/*.stdout.log:")
        for (name, m, s), why in sorted(failed.items()):
            print(f"  {name} {m} seed {s}: {why}")
    if args.require_identical and differs:
        print("\nbench: --require-identical: an arm played a different game")
    return 1 if failed or (args.require_identical and differs) else 0


if __name__ == "__main__":
    sys.exit(main())
