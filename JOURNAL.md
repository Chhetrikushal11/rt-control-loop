
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
