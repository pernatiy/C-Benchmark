#include <stdio.h>
#include <unistd.h>
#include <sys/times.h>
#include <sched.h>

int
timeit(int l, int (*sub)(int))
{
    struct tms tms_st, tms_en;
    int i, j, res;

    if (times(&tms_st) < 0) _exit(-1);
    for (i = 0; i < l; i++) {
        for (j = 0; j < 1e6; j++) {
            res += sub(j & 0x3FF);
        }
    }
    if (times(&tms_en) < 0) _exit(-2);

    if (res) sched_yield();
    #ifdef DEBUG
    printf(">>> timeit:\n");
    printf(">>>   User  : %5d ticks\n", tms_en.tms_utime - tms_st.tms_utime);
    printf(">>>   System: %5d ticks\n", tms_en.tms_stime - tms_st.tms_stime);
    #endif

    return tms_en.tms_utime - tms_st.tms_utime;
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
    int loops, res, tps;

    tps = sysconf(_SC_CLK_TCK);
    #ifdef DEBUG
    printf(">>> On this system cpu counts %d ticks per second\n", tps);
    #endif

    if (argc  > 1) loops = atoi(argv[1]);
    if (loops < 1) loops = 1;

    #ifdef DEBUG
    printf(">>> Will test sub in %d loops\n", loops);
    #endif
    res = timeit(loops, tester) - timeit(loops, stub);
    #ifdef DEBUG
    printf(">>> Test sub was executed in %d ticks\n", res);
    #endif
    if (res == 0) res = 1;
    if (res/tps < 2)
        printf("!!! Too few iteration for reliable count. Try specifying bigger loop count (at least in %3.1f times).\n", 2*tps/(1.0*res));

    printf("Consumed power: %.3f sec (%.3f nsec per iteration)\n", res/(1.0*tps), 1e3*(res/(1.0*loops*tps)));
    return 0;
}
