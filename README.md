# ICS2023 Programming Assignment

This project is the programming assignment of the class ICS(Introduction to Computer System)
in Department of Computer Science and Technology, Nanjing University.

For the guide of this programming assignment,
refer to https://nju-projectn.github.io/ics-pa-gitbook/ics2023/

To initialize, run
```bash
bash init.sh subproject-name
```
See `init.sh` for more details.

The following subprojects/components are included. Some of them are not fully implemented.
* [NEMU](https://github.com/NJU-ProjectN/nemu)
* [Abstract-Machine](https://github.com/NJU-ProjectN/abstract-machine)
* [Nanos-lite](https://github.com/NJU-ProjectN/nanos-lite)
* [Navy-apps](https://github.com/NJU-ProjectN/navy-apps)




# WORK CHANGE LIST

## pa1.1 implement `sdb` (simple debugger) mimicing instruction of `gdb`:  
- nemu/src/monitor/sdb/sdb.c
- ### support step run (`si [N]`), info register (`info r`), memory scanning (`x N ADDR_EXPR`).
- ### support EXPR parsing with regex. (Arith Calculator!) Therefore support print command (`p EXPR`)
    - added and tested in pa1.2
- ### support watchpoint (`w EXPR`), info watchpoint (`info w`), delete watchpoint (`d N`)
    - added in pa1.3

## pa1.2 implement a random unit test generator and test `sdb` commands on evaluating expression.
- ### generating sequence of expression using recursion, deal with buffer overflow, zero division, and unsigned int problem. 
    - nemu/tools/gen-expr/gen-expr.c
    - nemu/tools/gen-expr/testout (10000 lines of generated unit test)
- ### Add testing function when booting
    - nemu/src/nemu-main.c:42 test_expr()
## pa1.3 Watchpoint
- ### extend EXPR evaluation functionality, support register value ($reg), ptr dereference (*), and other logic operator (!=,&&)
    - nemu/src/monitor/sdb/sdb.c
- ### set up watchpoint pool API using linkedlist implementation:
    - nemu/src/monitor/sdb/watchpoint.c
- ### scan for watchpoint change in main and set NEMU state to STOP.
    - nemu/src/cpu/cpu-exec.c
    - add functions for printing and checking wp change in `nemu/src/monitor/sdb/watchpoint.c`.