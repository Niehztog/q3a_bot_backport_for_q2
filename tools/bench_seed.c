/*
 * bench_seed.c -- the seed pin tools/bench.py preloads into every run.
 * Nothing in the tree changes.
 *
 * With BENCH_SEED=<n> set, time() returns a per-seed constant
 * (1000000000 + n * 100000, so n stays below 11475 for a 32-bit time_t) and
 * the libc rand() stream starts from that value.  Unset, both are the real
 * ones.  Neither engine draws from libc rand() (yquake2 has randk, r1q2
 * seedMT), so game and botlib own the whole stream, and the game -- bot
 * line-up, chat, aim, goals -- becomes a function of n.
 *
 * Our build (game.so + botlib.so) never calls srand(): without a pin every
 * run plays glibc's default stream (seed 1).  The native shim therefore seeds
 * it from a constructor, before the engine loads the game.  The 1999
 * Gladiator botlib seeds it itself, from time(0), in BotSetupLibrary; for the
 * originals pinning time() is enough.
 *
 * bench.py builds it twice:
 *   our build   gcc -shared -fPIC -O2 -o bench_seed.so bench_seed.c -ldl
 *   originals   i686-linux-gnu-gcc -DBENCH_BOX86 -shared -fPIC -nostartfiles
 *                   -O2 -o bench_seed_i386.so bench_seed.c -ldl
 *
 * The box86 build (taken from gladiator-bot-restored's tools/bench_seed.c)
 * stands in for libgladshim.so: _isnan, max and min are the three symbols the
 * 3.20 engine exported and the 1999 gamei386.so expects.  It also carries
 * rand() and srand() itself.  Forwarded, they resolve to box86's wrapped
 * native glibc, whose state box86 has already advanced by the time
 * gamei386.so first draws, so two same-seed runs would pick different bots
 * from the first line.  The generator is glibc's TYPE_3 random_r, so the
 * stream still equals a native glibc's.
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

static long seed_base(void)
{
	static long base = -1;

	if (base < 0) {
		const char *s = getenv("BENCH_SEED");

		base = s && *s ? 1000000000L + atol(s) * 100000L : 0;
	}
	return base;
}

time_t time(time_t *t)
{
	static time_t (*real_time)(time_t *);
	time_t now;

	if (!real_time)
		real_time = (time_t (*)(time_t *))dlsym(RTLD_NEXT, "time");
	now = seed_base() ? (time_t)seed_base() : real_time(NULL);
	if (t)
		*t = now;
	return now;
}

#ifndef BENCH_BOX86
__attribute__((constructor))
static void seed_rand(void)
{
	if (seed_base())
		srand((unsigned)seed_base());
}
#else
int _isnan(double x)
{
	return x != x;
}

int max(int a, int b)
{
	return a > b ? a : b;
}

int min(int a, int b)
{
	return a < b ? a : b;
}

static int32_t rtab[34];
static int rpos = -1;

int rand(void);

void srand(unsigned seed)
{
	int i;
	int32_t v = seed ? (int32_t)seed : 1;

	rtab[0] = v;
	for (i = 1; i < 31; i++) {
		int32_t hi = v / 127773, lo = v % 127773;

		v = 16807 * lo - 2836 * hi;
		if (v < 0)
			v += 2147483647;
		rtab[i] = v;
	}
	for (i = 31; i < 34; i++)
		rtab[i] = rtab[i - 31];
	rpos = 34;
	for (i = 0; i < 310; i++)
		rand();
}

int rand(void)
{
	int32_t r;

	if (rpos < 0)
		srand(1);
	r = (int32_t)((uint32_t)rtab[(rpos - 31) % 34] + (uint32_t)rtab[(rpos - 3) % 34]);
	rtab[rpos % 34] = r;
	if (++rpos >= 68)
		rpos -= 34;
	return (int)((uint32_t)r >> 1);
}
#endif
