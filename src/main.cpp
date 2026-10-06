#include <ctime>
#include <cstdio>
#include <cstdint>

// Deterministic O(1) direct computation
constexpr int64_t ONE_SEC_NS = 1'000'000'000;

struct timespec add_nanoseconds(struct timespec ts, int64_t ns_to_add) {
    int64_t total_ns = ts.tv_nsec + ns_to_add;
    
    // Constant number of operations regardless of how large ns_to_add is:
    ts.tv_sec  += total_ns / ONE_SEC_NS;
    ts.tv_nsec  = total_ns % ONE_SEC_NS;
    
    return ts;
}
int main()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf("%ld s %ld ns\n", ts.tv_sec, ts.tv_nsec);
    // case 1: Add 50 million nanoseconds (0.5 seconds) to a timespec
    struct timespec new_a = {10, 100000000};
    new_a = add_nanoseconds(new_a, 50000000);
     printf("case1: %ld s  %ld ns   (expect 10 s 150000000 ns)\n",  new_a.tv_sec, new_a.tv_nsec);

    struct timespec b = {10, 900000000};
    b = add_nanoseconds(b, 200000000);
    printf("case2: %ld s  %ld ns   (expect 11 s 100000000 ns)\n", (long)b.tv_sec, b.tv_nsec);

    struct timespec c = {10, 500000000};
    c = add_nanoseconds(c, 2500000000);
    printf("case3: %ld s  %ld ns   (expect 13 s 0 ns)\n", (long)c.tv_sec, c.tv_nsec);
    printf("rt_loop alive\n");

    struct timespec res;
    clock_getres(CLOCK_MONOTONIC, &res);
    printf("resolution: %ld s  %ld ns\n", (long)res.tv_sec, res.tv_nsec);
    return 0;
}