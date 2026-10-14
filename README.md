# rt-control-loop

measurement done to understand how Linux Kernel behaves in microseconds to optimize its behaviour

## Results
WSL2 on Windows 11, stock kernel, no real-time tuning. 1 kHz target,
100,000 cycles.
 Jitter = actual wake time − deadline.

| Metric|   Value   |
|-------|-----------|
| min   | 14.87  µs |
| p50   | 144.46 µs |
| p99   | 428.17 µs |
| p99.9 | 608.66 µs |
| max   | 7522.51µs |
| misses > 100 µs |78,747 / 100,000 |

## What this measures

The deadline is an absolute timestamp, computed before the sleep
The number is the gap between that deadline and when the thread actually got the CPU

## Findings

Finding 1: max is twelve fold of 99.9th percentile and 52 fold of 50th percentile. Percentile hide the events that matter. Hard real-time quotes the max, not p99.
Finding 2: run-to-run spread is large (run 1 max 5566 µs, run 2 max was 7523 µs). One run is not a measurement on this platform.
## Why this matters?
- Since we have to integrate more computationally taxing calculation and some algorithms like Kalman filters or we may need to run ML models a learned forecaster, state estimation, may be reinforcement learning about  the behaviour of the nuclear reactor. 
While doing so we may need to substitute industrial PAC and adopt Industrial PC running Linux. When we make this switch we need to know how deterministic is the Industrial PC we are using. For Beckhoff or Rockwell the deterministic trail will be done and someone will have put out a table about how their PAC work in the interval of nanosecond. What does jitter may look like.
As PAC will have watchdog timer with predefined scan cycle time if the misses we measure occur in the industrial setting watchdog will have tripped 78747 times in 100 seconds. In similar fashion we need to predefine the interval to prevent the misses 
## Next

Week 2: 
Apply SCHED_FIFO, mlockall, and core pinning, measuring after each change individually rather than all at once, to attribute improvement to a specific cause.



## Build Instruction
```bash
cmake -B build
cmake --build build
./build/rt_control_loop
```
