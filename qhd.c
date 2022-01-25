/*  ---------------------------------------------------------------- */
/*                                                                   */
/*  Quick HexDump                                                    */
/*                                                                   */
/*  (C) 2022 PQCraft                                                 */
/*  Licensed under the GNU General Public License version 3          */
/*  https://www.gnu.org/licenses/gpl-3.0.txt                         */
/*                                                                   */
/*  ---------------------------------------------------------------- */

const char* VER = "2.0";

#ifndef QHD_ASCII
    #define QHD_ASCII  0 // Enable/disable 7-bit ASCII mode
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/stat.h>

int argc;
char** argv;

void aperror(char* str) {
    fprintf(stderr, "%s: ", argv[0]);
    perror(str);
}

void cperror(char* str) {
    fprintf(stderr, "%s: %s\n", argv[0], str);
}

void putdiv() {
    #if defined(QHD_ASCII) && QHD_ASCII
        putchar('|');
    #else
        printf("│");
    #endif
}

void puthelp() {
    puts("      --help          Display help text and exit");
    puts("      --version       Display version info and exit");
    puts("  -b, --bar           Display a column bar");
    puts("  -e, --emptyline     Display an empty line if the file size is zero");
    puts("  -p, --hide-pos      Hide the position column");
    puts("  -h, --hide-hex      Hide the hexadecimal column");
    puts("  -c, --hide-chr      Hide the character column");
    exit(0);
}

void putver() {
    printf("Quick HexDump version %s\n", VER);
    puts("(C) 2022 PQCraft");
    puts("<https://github.com/PQCraft/QHD>\n");
    puts("Licensed under the GNU GPL version 3");
    puts("<https://www.gnu.org/licenses/gpl-3.0.txt>");
    exit(0);
}

char* fname = NULL;

bool opt_bar = false;
bool opt_emptyln = false;
bool opt_showpos = true;
bool opt_showhex = true;
bool opt_showchr = true;
bool opt_onlyfile = false;

bool readopt() {
    bool setfile = false;
    for (int i = 1; i < argc; ++i) {
        if (!opt_onlyfile && argv[i][0] == '-' && argv[i][1]) {
            if (argv[i][1] == '-') {
                char* opt = argv[i] + 2;
                if (!*opt) {
                    opt_onlyfile = true;
                } else if (!strcmp(opt, "help")) {
                    puthelp();
                } else if (!strcmp(opt, "version")) {
                    putver();
                } else if (!strcmp(opt, "bar")) {
                    opt_bar = true;
                } else if (!strcmp(opt, "emptyline")) {
                    opt_emptyln = true;
                } else if (!strcmp(opt, "hide-pos")) {
                    opt_showpos = false;
                } else if (!strcmp(opt, "hide-hex")) {
                    opt_showhex = false;
                } else if (!strcmp(opt, "hide-chr")) {
                    opt_showchr = false;
                } else {
                    errno = EINVAL;
                    aperror(argv[i]);
                    return false;
                }
            } else {
                char opt = 0;
                for (int j = 1; (opt = argv[i][j]); ++j) {
                    if (opt == 'b') {
                        opt_bar = true;
                    } else if (opt == 'e') {
                        opt_emptyln = true;
                    } else if (opt == 'p') {
                        opt_showpos = false;
                    } else if (opt == 'h') {
                        opt_showhex = false;
                    } else if (opt == 'c') {
                        opt_showchr = false;
                    } else {
                        char tmp[3] = {'-', 0, 0};
                        tmp[1] = opt;
                        errno = EINVAL;
                        aperror(tmp);
                        return false;
                    }
                }
            }
        } else {
            if (setfile) {cperror("File already specified"); return false;}
            fname = argv[i];
            setfile = true;
        }
    }
    return true;
}

int main(int _argc, char** _argv) {
    argc = _argc;
    argv = _argv;
    if (!readopt()) return 1;
    char posfmt[] = " %016lX ";
    if (sizeof(long) == 4) {
        posfmt[3] = '0';
        posfmt[4] = '8';
    }
    FILE* fp;
    if (fname && (!(fname[0] == '-' && !fname[1]) || opt_onlyfile)) {
        fp = fopen(fname, "rb");
        if (!fp) {
            aperror(fname);
            return 1;
        }
        int fd = fileno(fp);
        struct stat statinfo;
        memset(&statinfo, 0, sizeof(statinfo));
        fstat(fd, &statinfo);
        if (S_ISDIR(statinfo.st_mode)) {
            fclose(fp);
            errno = EISDIR;
            aperror(fname);
            return 1;
        }
    } else {
        fp = stdin;
    }
    long pos = 0;
    unsigned char buf[16];
    size_t bread = 0;
    memset(buf, 0, 16);
    while ((bread = fread(buf, 1, 16, fp)) || (opt_emptyln && !pos)) {
        if (opt_bar && !pos) {
            if (sizeof(long) == 4) {
                fputs("           ", stdout);
            } else {
                fputs("                   ", stdout);
            }
            putdiv();
            fputs("  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F ", stdout);
            putdiv();
            putchar('\n');
        }
        size_t i = 0;
        if (opt_showpos) {
            putdiv();
            printf(posfmt, pos);
        }
        if (opt_showhex) {
            putdiv();
            putchar(' ');
            for (; i < bread; ++i) {
                printf("%02X ", buf[i]);
            }
            for (; i < 16; ++i) {
                buf[i] = 0;
                fputs("   ", stdout);
            }
        }
        if (opt_showchr) {
            putdiv();
            putchar(' ');
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
            putchar(' ');
        }
        putdiv();
        pos += 16;
        putchar('\n');
    }
    if (fname) fclose(fp);
    return 0;
}
