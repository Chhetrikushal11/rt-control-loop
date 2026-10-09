#include <ctime>
#include <time.h>
#include <cstdio>
#include <cstdint>
#include <algorithm>

// Deterministic O(1) direct computation
constexpr int64_t ONE_SEC_NS = 1'000'000'000;
constexpr int CYCLES = 100000; // for millisecond sleep test
constexpr int64_t PERIOD_NS = 1'000'000; // 1 ms = 1 kHz frequency


//DECISION 1: Array lives in BBS (file-scope static memory)
// 10,000 * 8 bytes = 80KB
// static scope guarantees this memory is allocated and mapped when the executable is loaded, and it is not allocated on the stack or heap. This is important for real-time systems where dynamic memory allocation can introduce unpredictable delays.
// loads, entirely off the hot execution path. Stacking 80 KB could risk stack overflow
// or runtime stack expansion overhead, which can be costly in terms of performnace and predictablility. By using static allocation, we avoid these risks and ensure that the memory is readily available when needed, contributing to the overall efficiency and reliability of the system.

static int64_t jitter_samples[CYCLES]; // 80 KB of static memory for jitter samples

// O(1) Constant-Time Deadline Calculation: Add nanoseconds to a timespec structure
struct timespec add_nanoseconds(struct timespec ts, int64_t ns_to_add) {
    int64_t total_ns = ts.tv_nsec + ns_to_add;
    // Constant number of operations regardless of how large ns_to_add is:
    ts.tv_sec  += total_ns / ONE_SEC_NS;
    ts.tv_nsec  = total_ns % ONE_SEC_NS;
    
    return ts;
}

// Overflow-Safe Time Difference Calculation: Compute the difference in nanoseconds between two timespec structures
int64_t diff_ns(struct timespec later, struct timespec earlier){
    int64_t dt_sec = later.tv_sec - earlier.tv_sec;
    int64_t dt_ns = later.tv_nsec - earlier.tv_nsec;
    return (dt_sec * ONE_SEC_NS + dt_ns);
}
int main()
{
    struct timespec next, actual;

    printf("==============================================================\n");
    printf("            REAL-TIME BASELINE MEASUREING INSTRUMENT         \n");
    printf("==============================================================\n"); 
    printf("Running %d cycles at %ld ns period (1ms / 1kHz)\n", CYCLES, PERIOD_NS);

    // Step1:Read the current time using clock_gettime with CLOCK_MONOTONIC
    clock_gettime(CLOCK_MONOTONIC, &next);
     
    // Step2: The Real-Time Hot Loop (Zero I/O, Zero Allocation)
    for (int i = 0; i < CYCLES; i++) {
        // DECISION 2: Advance target time strictly by adding period (No Drift)
        next = add_nanoseconds(next, PERIOD_NS);

        // DECISION 3: Sleep until ABSOLUTE moment 'next'
        // TIMER_ABSTIME guranteest we sleep to target, not relative duration
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);

        // Record actual wakup time immeditaely after sleep returns, to measure jitter
        clock_gettime(CLOCK_MONOTONIC, &actual);

        // Store jitter sample in pre-allocated static array (O(1) operation)
        jitter_samples[i] = diff_ns(actual, next);
    }
    // POST-LOOP PROCESSING:
    // std::sort is performed AFTER the timing loop completes.
    // Sorting inside the loop would introduce O(N log N) execution overhead and 
    // memory movement, completely corrupting real-time latency measurements.
    std::sort(jitter_samples, jitter_samples + CYCLES);

    // Step 3: Post-loop Statics (Safe to process and print here)
int64_t min_j   = jitter_samples[0];
    int64_t p50_j   = jitter_samples[CYCLES * 50 / 100];
    int64_t p99_j   = jitter_samples[CYCLES * 99 / 100];
    int64_t p99_9_j = jitter_samples[CYCLES * 999 / 1000];
    int64_t max_j   = jitter_samples[CYCLES - 1]; // Last item in sorted array
    int64_t sum_j = 0;
    int deadline_miss_count = 0;


    for (int i = 0; i < CYCLES; i++){

        sum_j += jitter_samples[i];
        int64_t j = jitter_samples[i];
        if (j > PERIOD_NS) {
            deadline_miss_count++;
        }
    }

    double mean_j = (double)sum_j / CYCLES;
    printf("\n-------BASELINE STATISTICS-------\n");
    printf("Min Jitter: %8.3f us (%ld ns)\n", min_j / 1000.0, min_j);
    printf("Mean Jitter: %8.3f us (%ld ns)\n", mean_j /1000.0, (int64_t)mean_j);
    printf("50th Percentile: %8.3f us (%ld ns)\n", p50_j / 1000.0, p50_j);
    printf("99th Percentile: %8.3f us (%ld ns)\n", p99_j / 1000.0, p99_j);
    printf("99.9th Percentile: %8.3f us (%ld ns)\n", p99_9_j / 1000.0, p99_9_j);
    printf("Max Jitter: %8.3f us (%ld ns)\n", max_j / 1000.0, max_j);
    printf("Deadline Misses: %d / %d (%.2f%%)\n", deadline_miss_count, CYCLES, (double)deadline_miss_count / CYCLES * 100.0);
    printf("=================================\n");
    return 0;

}