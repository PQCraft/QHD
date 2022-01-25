#!/bin/bash
gcc -Wall -Wextra -O2 -s qhd.c -o qhd
gcc -m32 -Wall -Wextra -O2 -s qhd.c -o qhd32
x86_64-w64-mingw32-gcc -Wall -Wextra -O2 -s qhd.c -DQHD_ASCII=1 -o qhd.exe
i686-w64-mingw32-gcc -m32 -Wall -Wextra -O2 -s qhd.c -DQHD_ASCII=1 -o qhd32.exe
