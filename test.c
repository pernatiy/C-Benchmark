#include <stdio.h>
#include <time.h>
#include <unistd.h>

int
main()
{
        struct timespec start, end;
        int i, result = 0, sign = -1;
        int msec;

        clock_gettime(CLOCK_REALTIME, &start);
        for (i = 1; i <= 10000000; i++) {
                result += test_func(i % 5 + 1000) * sign;
                sign *= -1;
                result++;
        }
        clock_gettime(CLOCK_REALTIME, &end);

        msec = (long) ((end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec)) / 1000000;
        printf("Execution time : %d ms.\n", msec);
        printf("Result : %d\n", result);

        return 0;
}

int
test_func(int c)
{
        return c * c * c;
}
