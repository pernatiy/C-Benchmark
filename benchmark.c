#include <stdio.h>
#include <unistd.h>
#include <sys/times.h>

#ifndef NOSCHED
#define __USE_GNU
#endif
#include <sched.h>

struct bench_res {
    clock_t     rtime;
    clock_t     utime;
    clock_t     stime;
    long long   count;
};

/* wait for next tick for got lesser error in time counting */
void
wait_tick(struct tms * s, struct tms * e)
{
    times(e);
    do {
        times(s);
    } while(s->tms_utime - e->tms_utime == 0);
}

void
timeit(int l, int (*sub)(int), int (*base)(int), struct bench_res * res)
{
    struct tms tms_st, tms_en, tmb_st, tmb_en;
    int i, j, r;

    wait_tick(&tms_st, &tms_en);
    for (i = 0; i < l; i++) {
        for (j = 0; j < 1e6; j++) {
            r += sub(j & 0x3FF);
        }
    }
    times(&tms_en);

    wait_tick(&tmb_st, &tmb_en);
    for (i = 0; i < l; i++) {
        for (j = 0; j < 1e6; j++) {
            r += base(j & 0x3FF);
        }
    }
    times(&tmb_en);

    #ifdef DEBUG
    fprintf(stderr, ">>> timeit:\n");
    fprintf(stderr, ">>>   test: Usr: %5d ticks;  Sys: %5d ticks\n", tms_en.tms_utime - tms_st.tms_utime, tms_en.tms_stime - tms_st.tms_stime);
    fprintf(stderr, ">>>   base: Usr: %5d ticks;  Sys: %5d ticks\n", tmb_en.tms_utime - tmb_st.tms_utime, tmb_en.tms_stime - tmb_st.tms_stime);
    #endif

    if (r) sched_yield();

    res->utime = (tms_en.tms_utime - tms_st.tms_utime) - (tmb_en.tms_utime - tmb_st.tms_utime);
    res->stime = (tms_en.tms_stime - tms_st.tms_stime) - (tmb_en.tms_stime - tmb_st.tms_stime);
    res->count = 1LL * l * 1e6;
}

void
countit(int t, int (*sub)(int), int (*base)(int), struct bench_res * res)
{
    struct tms tms_st, tms_en, tmb_st, tmb_en;
    int c, i, j, r;

    c = 0;
    t *= sysconf(_SC_CLK_TCK);

    wait_tick(&tms_st, &tms_en);
    do {
        for (j = 0; j < 1e6; j++) {
            r += sub(j & 0x3FF);
        }
        times(&tms_en);
        if (!++c) break;
    } while(tms_en.tms_utime - tms_st.tms_utime < t);

    res->count = 1LL * c * 1e6;

    wait_tick(&tmb_st, &tmb_en);
    do {
        for (j = 0; j < 1e6; j++) {
            r += base(j & 0x3FF);
        }
        times(&tmb_en);
        if (!--c) break;
    } while(tmb_en.tms_utime - tmb_st.tms_utime < t);

    if (c) fprintf(stderr, "Baseline function runs longer then tested. Do something with this. And \"yes\", you can (or maybe even must) ignore all results.\n");

    if (r) sched_yield();
    #ifdef DEBUG
    fprintf(stderr, ">>> timeit:\n");
    fprintf(stderr, ">>>   test: Usr: %5d ticks;  Sys: %5d ticks\n", tms_en.tms_utime - tms_st.tms_utime, tms_en.tms_stime - tms_st.tms_stime);
    fprintf(stderr, ">>>   base: Usr: %5d ticks;  Sys: %5d ticks\n", tmb_en.tms_utime - tmb_st.tms_utime, tmb_en.tms_stime - tmb_st.tms_stime);
    #endif

    res->utime = (tms_en.tms_utime - tms_st.tms_utime) - (tmb_en.tms_utime - tmb_st.tms_utime);
    res->stime = (tms_en.tms_stime - tms_st.tms_stime) - (tmb_en.tms_stime - tmb_st.tms_stime);
}

int
stub(int c)
{
    return c;
}

int
tester(int c)
{
    return c * c * c;
}

int
main(int argc, char ** argv)
{
    int loops, tps;
    struct bench_res res;
    double time;
    #ifndef NOSCHED
    cpu_set_t cpu_set;
    #endif

    tps = sysconf(_SC_CLK_TCK);
    #ifdef DEBUG
    fprintf(stderr, ">>> On this system cpu counts %d ticks per second\n", tps);
    #endif

    loops = -3;
    if (argc > 1) loops = atoi(argv[1]);
    if (loops == 0) loops = -3;

    #ifndef NOSCHED
    // set affinity to first cpu
    CPU_ZERO(&cpu_set);
    CPU_SET(0, &cpu_set);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set);
    #endif

    if (loops > 0)
    {
        #ifdef DEBUG
        fprintf(stderr, ">>> Will test sub in %d loops\n", loops);
        #endif
        timeit(loops, tester, stub, &res);
    } else {
        #ifdef DEBUG
        fprintf(stderr, ">>> Will test sub for %d CPU seconds.\n", -loops);
        #endif
        countit(-loops, tester, stub, &res);
    }
    #ifdef DEBUG
    fprintf(stderr, ">>> Test sub was executed in %d ticks\n", res.utime);
    #endif

    time = (res.utime ? res.utime : 1)/(1.0*tps);
    if (time < 0.5)
        printf("!!! Too few iteration for reliable count. Try specifying bigger loop count (at least in %.*f times).\n", time > 0.1 ? 1 : 0, 1/time);

    printf("Consumed power: %.3f sec (%.3f nsec per iteration)\n", time, 1e9*time/res.count);
    return 0;
}
