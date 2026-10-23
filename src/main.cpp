 #include <cstdio>
        #include <ctime>
        #include <cstdint>
        #include <cstdlib>
        #include <cstring>
        #include <cerrno>
        #include <sched.h>     // Required for sched_setscheduler and struct sched_param
        #include <algorithm>  // Required for std::sort

        constexpr int64_t ONE_SEC_NS = 1'000'000'000;
        constexpr int CYCLES = 100000;            // 100,000 cycles = 100 seconds
        constexpr int64_t PERIOD_NS = 1'000'000; // 1 ms period (1KHz)
        constexpr int64_t MISS_THRESHOLD_NS = 100'000;   // 100 µs, same as baseline

        // Static BSS allocation to avoid runtime dynamic allocations
        static int64_t jitter_samples[CYCLES];

        struct timespec add_nanoseconds(struct timespec ts, int64_t ns_to_add)
        {
            int64_t total_ns = ts.tv_nsec + ns_to_add;
            ts.tv_sec += total_ns / ONE_SEC_NS;
            ts.tv_nsec = total_ns % ONE_SEC_NS;
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
            // ------- SINGLE LEVER: SCHED_FIFO REAL-TIME SCHEDULING SETTING ------
            struct sched_param param;
            std::memset(&param, 0, sizeof(param));
            param.sched_priority = 80; // Mid-range RT priority (1-99 range)

            // pid = 0 targets the calling thread/process
            if (sched_setscheduler(0, SCHED_FIFO, &param) == -1)
            {
                std::fprintf(stderr,"[ERROR] sched_setscheduler(SCHED_FIFO, prio = 80) failed: %s (errno=%d)\n",
                std::strerror(errno), errno);
                
                if(errno == EPERM)
                {
                    std::fprintf(stderr, "[HINT] Process lacks privilege. Re-run with root/sudo or set rtprio rlimits.\n");

                }
                std::exit(EXIT_FAILURE);
            }
            std::printf("[SUCCESS] Real-time policy set to SCHED_FIFO with priority %d\n", param.sched_priority);

            struct timespec next, actual;
            std::printf("Running %d cycles (~100s) at 1KHz under SCHED_FIFO ...\n", CYCLES);
            clock_gettime(CLOCK_MONOTONIC, &next);

            for(int i = 0; i < CYCLES; i++)
            {
                next = add_nanoseconds(next, PERIOD_NS);
                clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
                clock_gettime(CLOCK_MONOTONIC, &actual);
                jitter_samples[i] = diff_ns(actual, next);
            }
            
            // ------ POST-LOOP STATISTICS & PERCENTILES -------------

            std::sort(jitter_samples, jitter_samples + CYCLES);

            int64_t  min_j  = jitter_samples[0];
            int64_t p50_j   = jitter_samples[CYCLES * 50 / 100];
            int64_t p99_j   = jitter_samples[CYCLES * 99 / 100];
            int64_t p99_9_j = jitter_samples[CYCLES * 999/1000];
            int64_t max_j   = jitter_samples[CYCLES -1];

            int64_t sum_j = 0;
            int deadline_miss_count = 0;

            for (int i = 0; i < CYCLES; i++){
                sum_j += jitter_samples[i];
                int64_t j = jitter_samples[i];
                if (j > MISS_THRESHOLD_NS) {
                deadline_miss_count++;
                 }
            }

            double mean_j = (double)sum_j / CYCLES;

            std::printf("\n-- DAY 6 LATENCY PROFILE: SCHED_FIFO ALONE (100k SAMPLES) ------\n");
            std::printf("Min   : %8.3f us\n", min_j / 1000.0);
            std::printf("Mean  : %8.3f us\n", mean_j / 1000.0);
            std::printf("p50   : %8.3f us\n", p50_j / 1000.0);
            std::printf("p99   : %8.3f us\n", p99_j / 1000.0);
            std::printf("p99.9 : %8.3f us\n", p99_9_j / 1000.0);
            std::printf("Max   : %8.3f us\n", max_j / 1000.0);
            std::printf("Deadline Misses: %d / %d (%.2f%%)\n", deadline_miss_count, CYCLES, (double)deadline_miss_count / CYCLES * 100.0);
            std::printf("================================================================\n");
            return 0;

        }