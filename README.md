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

# PA1 Infrastructure
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
- ### use Kconfig to control functionality.
    - understand Kconfig and modify: `nemu/Kconfig`
    - set up config entry for watchpoint function and debug output;
    - change setting by running `make menuconfig` in `nemu/`.


# PA2 RISC-V bare-metal
## pa2.1 implement all RISC-V instruction set: include `RV32I` and `RV32M`
- nemu/src/isa/riscv32/inst.c
- ### Read through risc-V inst set and implement every instruction:
    - support integer arithmetic through I and R instruction decoding.
    - support memory load and store through I/U and S instrucion decoding.
    - support jumping and branching through B and J/I instrucion decoding.
- ### Run a bunch of simple C program's object file through nemu and pass.
    - tests located in `am-kernels/tests/cpu-tests/tests`, and can be run though command: `make ARCH=riscv32-nemu ALL={test program without .c suffix} run`
- ### Abstract-machine lib function
    - look through man pages and implement string functions: `strlen`, `strcpy`, `strncpy`, `strcat`, `strcmp`, `strncmp`; (`abstract-machine/klib/src/string.c`)
    - memory functions: `memset`, `memmove`, `memcmp`, `memcpy`; (`abstract-machine/klib/src/string.c`)
    - print functions: `sprintf`,`vsprintf`,`snprintf`,`vsnprintf`. (`abstract-machine/klib/src/stdio.c`)

## pa2.2 tracing (Infrastructure part 2)
- ### set up Instruction Ring Buffer and integrate into sdb:
    - nemu/src/monitor/sdb/iringbuf.c
    - nemu/include/monitor/iringbuf.h
- ### add Memory Trace function and enabling control through Kconfig:
    - nemu/src/memory/paddr.c
    - nemu/Kconfig
- ### add Function Tracing:
    - **add `-e` argument to main to parse elf file, modify makefile to load elf file**
        - abstract-machine/scripts/platform/nemu.mk
        - nemu/src/monitor/monitor.c
    - **detect function calling and return with `jal` and `jalr` instruction, and print trace appropiately**
        - nemu/src/isa/riscv32/inst.c
    - **modify Kconfig to add option on opening ftrace**
- ### klib testing:
    - add folder for klib testing: `am-kernels/tests/klib-tests`.
    - write testsuite for klib functions, and test on native. 
        - am-kernels/tests/klib-tests/src/tests/test_memory.c
        - am-kernels/tests/klib-tests/src/tests/test_string.c
        - am-kernels/tests/klib-tests/src/tests/test_printf.c
        - am-kernels/tests/klib-tests/src/tests/test_test_setup.c
- ### DIFF test:
    - nemu/src/isa/riscv32/difftest/dut.c
    - add differential testing for nemu,comparing with REF spike. Comparing only register each step!

## pa2.3 IO device
- ### read source code, fix bug in printf when invoking nemu putch.
- ### timer