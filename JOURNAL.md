
Date: 10/5/26
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
