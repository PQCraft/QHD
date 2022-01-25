/*  ---------------------------------------------------------------- */
/*                                                                   */
/*  Quick Hex Dump version 1.1                                       */
/*                                                                   */
/*  (C) 2022 PQCraft                                                 */
/*  Licensed under the GNU General Public License version 3.0        */
/*  https://www.gnu.org/licenses/gpl-3.0.txt                         */
/*                                                                   */
/*  ---------------------------------------------------------------- */

#ifndef QHD_ASCII
    #define QHD_ASCII  0 // Enable/disable 7-bit ASCII mode
#endif
#ifndef QHD_COLBAR
    #define QHD_COLBAR 0 // Enable/disable column bar
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

int gargc;
char** gargv;

void aperror(char* str) {
    fprintf(stderr, "%s: ", gargv[0]);
    perror(str);
}

void cperror(char* str) {
    fprintf(stderr, "%s: %s\n", gargv[0], str);
}

int main(int argc, char** argv) {
    gargc = argc;
    gargv = argv;
    if (argc > 2) {cperror("Too many arguments"); return 1;}
    char* posfmt = NULL;
    if (sizeof(long) == 4) {
        #if defined(QHD_ASCII) && QHD_ASCII
            posfmt = strdup("| %08lX | ");
        #else
            posfmt = strdup("│ %08lX │ ");
        #endif
    } else {
        #if defined(QHD_ASCII) && QHD_ASCII
            posfmt = strdup("| %016lX | ");
        #else
            posfmt = strdup("│ %016lX │ ");
        #endif
    }
    FILE* fp;
    if (argc > 1) {
        fp = fopen(argv[1], "rb");
        if (!fp) {
            aperror(argv[1]);
            return 1;
        }
    } else {
        fp = stdin;
    }
    long pos = 0;
    unsigned char buf[16];
    size_t bread = 0;
    memset(buf, 0, 16);
    #if defined(QHD_COLBAR) && QHD_COLBAR
        if (sizeof(long) == 4) {
            #if defined(QHD_ASCII) && QHD_ASCII
                fputs("           | ", stdout);
            #else
                printf("           │ ");
            #endif
        } else {
            #if defined(QHD_ASCII) && QHD_ASCII
                fputs("                   | ", stdout);
            #else
                printf("                   │ ");
            #endif
        }
        fputs(" 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F ", stdout);
        #if defined(QHD_ASCII) && QHD_ASCII
            puts("|");
        #else
            printf("│\n");
        #endif
    #endif
    while ((bread = fread(buf, 1, 16, fp))) {
        size_t i = 0;
        printf(posfmt, pos);
        for (; i < bread; ++i) {
            printf("%02X ", buf[i]);
        }
        for (; i < 16; ++i) {
            buf[i] = 0;
            fputs("   ", stdout);
        }
        #if defined(QHD_ASCII) && QHD_ASCII
            printf("| ");
        #else
            printf("│ ");
        #endif
        for (size_t i = 0; i < 16; ++i) {
            if (buf[i] >= 32 && buf[i] <= 126) {
                putchar(buf[i]);
            } else {
                if (i < bread) {
                    #if defined(QHD_ASCII) && QHD_ASCII
                        putchar('.');
                    #else
                        printf("·");
                    #endif
                } else {
                    putchar(' ');
                }
            }
        }
        #if defined(QHD_ASCII) && QHD_ASCII
            printf(" |");
        #else
            printf(" │");
        #endif
        pos += 16;
        putchar('\n');
    }
    if (argc > 1) fclose(fp);
    return 0;
}
