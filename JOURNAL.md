
Date: 10/5/26
Day 1
Setup Phase

Since we need to use Windows laptop and Linux workstep
First I install WSL in my laptop, install the distro and connect it with my VS code
now in our rt-contor-loop the distro branch look like
Ran `sudo apt` in PowerShell — PowerShell is Windows, the $ prompt is Linux.
        
    


rt-control-loop
- CMAKELists.txt 
    - we make sure we have CXX standard enable, made 3.16 at least the minimum and C++ 17 is the standard choice
    - then we create a executable and as project proceed we will make it dynamic hence we can make add more files as required
    -    add_executable() is the binary name.
- .gitignore
    - we are ignoring the build file
- Journal.md
    - to keep track of what we did that day
-src
    -main.cpp
        - we have simple main file build with some libraries and printf.
after done building 
we
Ran ./build/rt_loop but the binary is rt_control_loop — the first arg to

Next phase: clock_gettime(CLOCK_MONOTONIC) -print tv_sec and tv_nsec, see the shape of it.

Date: 10/6/26
Day 2
//-------------------------------- Teaching session on timing and normalization of code --------------------------------------------//

Reading the clock
* step 1:- read the clock
for this we need to use 
struct timespec ts;
clock_gettime (CLOCK_MONOTONIC, &ts);
printf("%ld s %ld ns\n", ts.tv_sec, ts.tv_nsec);

//---------------- question ---------------------//
where to make struct timespace ts, inside the main.cpp, where inside the main.cpp inside the int main() or outside?
    - inside the main()
why we need to use syntax "%ld s %ld ns\n" what is %ld and s and ns means?
    - % begin a conversion
    - l length modifier, meansinig long
    - signed decimal integer
    - %ld - pring long in decimal
why we need two tv_sec and tv_nsec?
    - tv_nsec is declared long

* step 2:- the normalizer helper
- how many times does loop run if someone passes one hour of nanoseconds? 3600 iterations. One day 86400 iterations?
The number of iterations depends on the input. That's exactly the property a real-time path can't have. Building a loop where every cylcle must finish inside a know budget. A helper whose execution timescales with argument is unbounded. The thing to that makes malloc and adaptive solver disqualifying. 
Question Remains
    - can we do fixed number of operations, no matter what ns is?
        1. fast modulo  
            - if tv_nsec ends up at 2,500,000,0000 you dont need to discover by repeatad subtraction that it contains two whole seconds. 
            - loop to normaize nanoseconds into seconds introduces an input-dependedent loop bound and O(N) execution path that destroy deterministic execution guarantees. In a real-time hot path, variable-iteration loops are just as catastrophic as dynamic memory allocation because they introduce unbounded tail-latency jitter.
            - so to make time normalize O(1) with fixed execution cost, you eliminate loop entierly using direct integer division and modulo

            // deterministic O(1) direct computation
            constexpr int64_t ONE_SEC_NS = 1'000'000'000; // using constexpr to evalue in compile time , so it never exist in run tiem

            struct timespec add_nanoseconds(struct timespec ts, int64_t ns_to_add)
            {
                int64_t total_ns = ts.tv_nsec + ns_to_add;
                // constant number of operation regardless of how large ns_to_add is:
                // Precondition: ns_to_add >= 0. Negative values can leave tv_nsec negative.
                ts.tv_sec += total_ns / ONE_SEC_NS;
                ts.tv_nsec = total_ns % ONE_SEC_NS;

                return ts;
            }

            // why this guraentee real time bounds
                - fixed instruction count 
                - predictable branching 
                    - zero conditional brachnes (JMP/JNE), keeping the CPU branch pedictor pipeline completely clean and aviod misprediction penalites


            // why dynamic memory allocation create tail latency jitter
            1. Heap Fragment and O(N) Search loops
                - free list travesals - allocation glibc must find a contigous free memory block enough to fulfill the request. If memory is fragment, the allocator must traverse linked lists or bin structures of free blocks. In the worst-case scenario, this search is O(N) where N is the number of fragment memory chunks in the heap

                - block spliting and metadata maintenance - once block is found, allocator splits it, updates adjacent block pointers and update header/footer metadata.
                
                - lock contention: in multi-threaded applications, allocators use mutexes or spinlocks to protect arena free lists. If another thread is allocating or free memory at the exact same moment, your critical hot-path thread stalls waiting for the lock.

            2. Kernel-Level OS Traps (Virtual Memory and Page Faults)
                 - Kernel Context Switch: Calling mmap() or brk() forces a user-to-kernel mode context switch, costing hundreds to thousands of CPU cycles.
            
            3. CPU Cache Coldness & TLB Invalidation
                - Even if malloc succeds quickly, dynamically allocated memory hurt CPU execution pipeline efficiency
                    - Cache Line Misses:
                        - Memory allocated on the heap is rarely pre-warmed in L1/L2 CPU caches. Accessing a brand-new heap pointer forces the CPU memory controller to fetch lines from slow main system RAM(~50-100 ns delay) rather than reading from L1 cache (~ 1ns)
                    - TLB Misses: Randomly allocated heap pointers scatter your data across non-contiguous physical pages, causing fragment Translation Lookaside Buffer (TLB) misses that force the CPU to perform expensive hardware page table walks.




Date: 10/7/26
Day 3 Skipped


Date: 10/8/26
Day 4
    Part 1: Need to create a int64_t diff_ns
int64_t diff_ns(struct timespec later, struct timespec earlier){
    int64_t later_ns = later.tv_sec * ONE_SEC_NS + later.tv_nsec;
    int64_t earlier_ns = earlier.tv_sec * ONE_SEC_NS + earlier.tv_nsec;
    return later_ns - earlier_ns;
}
    - here first we convert the each time to absolute nanosecond count and subtract.
    - this wasy we never get trapped into negative intermeditae trap.
    - tv_sec is seconds since boot, so a few thousnad. But if a timespec from CLOCK_REALTIME ever reached this function.
    - tv_sec is ~1.8 billion and intermediate becomes 1.8 x 10^18. But int64_t tops out around 9.2 x 10^18.This means we will have head room upto year 2262 if we start now.

    Part2: How long does one clock_gettime call take?

        The method: timestamp call it a million times to throwaway variable, timestamp again, divide the elapsed time by a million
        struct timespec t0,t1, sink;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        for (int i = 0; i < 1000000; i++) {
            clock_gettime(CLOCK_MONOTONIC, &sink);
        }
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("clock_gettime: %ld ns per call\n", diff_ns(t1, t0) / 1000000);

Now what are doing after this

    Up to this point we learned:
        1. How struct timespec holds time (tv_sec + tv_nsec)?
        2. How add_nanoseconds calculate future deadlines in O(1) constant time without drifting?
        3. How diff_ns measures the nanosecond gap between two timestamps?
        4. How clock_gettime measure overhead?

    Now, we connect these pieces into a single program that runs a 1kHz loop (1ms per cycle) for 10,000 cycles (10 seconds total)

    During the 10 seconds:
        1. Our thread calculates the exact future nanosecond timestamp it should wake up at(next).
        2. It tells the Linux kernel: "Put me to sleep until next arrives" using
            clock_nanosleep(...., TIMER_ABSTIME, &next,....).
        3. The instant it wakes up, it reads the actual time (actual) and records the error.jitter = actual - next.
        4. It saves that error into an 80kB array in memory. No printf or I/O inside the loop, because the I/O causes high variable delays that destroy accurate measurements.
        5. Once the 10,0000 cycles finish, it loops through the record array to compute and display your system's baseline Min, Mean and Max Jitter.

while we just want to check the interval of jitter we used
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

int64_t diff_ns(struct timespec later, struct timespec earlier){
    int64_t dt_sec = later.tv_sec - earlier.tv_sec;
    int64_t dt_ns = later.tv_nsec - earlier.tv_nsec;
    return (dt_sec * ONE_SEC_NS + dt_ns);
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
    

    struct timespec t1 = {10, 100000000};
    struct timespec t2 = {10, 150000000};
    printf("diff1: %ld ns  (expect 50000000)\n", diff_ns(t2, t1));

    struct timespec t3 = {10, 900000000};
    struct timespec t4 = {11, 100000000};
    printf("diff2: %ld ns  (expect 200000000)\n", diff_ns(t4, t3));

    struct timespec t5 = {10, 0};
    struct timespec t6 = {13, 0};
    printf("diff3: %ld ns  (expect 3000000000)\n", diff_ns(t6, t5));

    struct timespec t0, tn, sink;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < 1000000; i++) {
         clock_gettime(CLOCK_MONOTONIC, &sink);
     }
    clock_gettime(CLOCK_MONOTONIC, &tn);
    printf("clock_gettime: %ld ns per call\n", diff_ns(tn, t0) / 1000000);
    return 0;
}


Date: 10/9/26
Day 5

Now we need to run the 100,000 CYCLES
Period it for 1 milliseconds or 1kHz



now we will need add a loop inside a main function
- here we run the loop for 100,000 cycles
- at first we capture the first timestamp next
- then we ask it to clock_nanosleep to sleep for absolute time till it hit the word next 
- so then we capture the timestamp after that and store it in actual
- now we use our jitter_sample = diff_ns to understand between next and actual how long kernel take to wake  up

- then we create a nother loop 
- here we find, min, max, mean, 50 percentile, 99 percentile and 99.9 percentile
- also we find the misses to find the miss we use where we measure if any sample pass the 100,000 (= 100us) threshold
- what we measured is how late the wake-up was against the deadline you asked for. Scheduling delay is part of it, but so is every other process that go the CPU first.

we ran the two seperate loops 

one for acquisition and another for analysis
- the hot loop does the minimum possible per cycle: advance the deadline,sleep, read the clock, store one number. That's it. Anything you add there becomes part of what you're measuring.

one the first loop
 for each iteration
    1. next = add_nanoseconds(next, PERIOD_NS)
        - no clock read here. Pure arithemetic.
        - Cycle 1 targets T+1ms, cycle 2 target T + 2ms, cycle 500 T+500ms.
        - Every deadline is computed from the anchor, never from "now"
    2. clock_nanosleep(..,TIME_ABSTIME, &next, NULL)
        - BLOCKING. Your thread leaves the run queue entirely.
        - the kernel arms a timer for the absolute moment 'next'.
        - your process uses zero CPU. It does not exist as far as the schedulre cares.

        ... timer fires at = next ....
        ... kernel marks your thread RUNNABLE ...
        ... thread sits in the run queue ...
        ... scheduler eventually picks it and puts it on CPU ...
               -- THIS GAP IS THE JITTER ---
    3. clock_gettime(CLOCK_MONOTONIC, &actual)
        - the first thing your code does on resuming - ~18 ns after the previous line

    "Runnable" and "Running" are different states and the distance between them is what you measured. The timer firing just makes you eligible.


BEFORE the call:
  next = 501 ms          ← YOU set this. add_nanoseconds advanced it from 500 to 501.
  (real time is ~500 ms) ← nobody stored this. It's just what time it is.

YOU CALL:  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL)

INSIDE the kernel:
  reads next   → 501 ms   (from the struct you pointed at)
  reads clock  → 500 ms   (now)
  subtracts    → 1 ms     ← the duration. Kernel computed it, you never saw it.
  sleeps 1 ms

AFTER:
  real time is ~501 ms (+ jitter)
  next is STILL 501 ms    ← kernel did not touch it. It's const.
  you read actual = 501.144 ms
  jitter = actual - next = 144 µs
  Then the loop comes around, add_nanoseconds pushes next to 502, and real time is sitting at 501.144. Kernel subtracts: 502 − 501.144 = 856 µs. Shorter than 1 ms, because you woke up late and the deadline didn’t move.

That’s the self-correction. The sleep isn’t always 1 ms. It’s always “however long is left until the deadline.” It was 856 µs there. If you’d woken 2 ms late it would be negative, the kernel would return immediately without sleeping at all, and you’d see a deadline miss — which is exactly what 78,747 of your samples are.

So to answer your sentence directly: no, we do not sleep 1 ms every cycle. We sleep whatever remains. It averages near 1 ms only because the loop averages 1 ms. The variation in sleep duration is the mechanism that keeps the grid fixed.