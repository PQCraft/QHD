/*  ---------------------------------------------------------------- */
/*                                                                   */
/*  Quick HexDump                                                    */
/*                                                                   */
/*  (C) 2022 PQCraft                                                 */
/*  Licensed under the GNU General Public License version 3          */
/*  https://www.gnu.org/licenses/gpl-3.0.txt                         */
/*                                                                   */
/*  ---------------------------------------------------------------- */

const char* VER = "2.1";

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

char* fname = NULL;

bool opt_bar = false;
bool opt_emptyln = false;
bool opt_showpos = true;
bool opt_showhex = true;
bool opt_showchr = true;
bool opt_onlyfile = false;
bool opt_color = false;

void aperror(char* str) {
    fprintf(stderr, "%s: ", argv[0]);
    perror(str);
}

void cperror(char* str) {
    fprintf(stderr, "%s: %s\n", argv[0], str);
}

void replacestr(char** ptr, char* str) {
    free(*ptr);
    *ptr = strdup(str);
}

int eqstrcmp(char* str1, char* str2) {
    char* ostr1 = str1;
    while (*str1 && *str2 && *str1 != '=' && *str2 != '=') {
        if (*str1 != *str2) {return -1;}
        ++str1;
        ++str2;
    }
    if ((!*str1 && *str2 && *str2 != '=') || (!*str2 && *str1 && *str1 != '=')) return -1;
    if (*str1 == '=') ++str1;
    return str1 - ostr1;
}

enum {
    COLOR_NORM,
    COLOR_DIV,
    COLOR_POS,
    COLOR_HEX,
    COLOR_HEXH,
    COLOR_HEXZ,
    COLOR_CHR,
    COLOR_CHRN,
    COLORCODE_LIST_SIZE,
};

char* colorcodes[COLORCODE_LIST_SIZE];

void setcolor(int code) {
    if (!opt_color) return;
    if (code != COLOR_NORM) printf("\e[%sm", colorcodes[COLOR_NORM]);
    printf("\e[%sm", colorcodes[code]);
}

void putdiv() {
    setcolor(COLOR_DIV);
    #if defined(QHD_ASCII) && QHD_ASCII
        putchar('|');
    #else
        printf("│");
    #endif
}

void parsecolors(char* instr) {
    if (!instr || !*instr) return;
    char* newstr = strdup(instr);
    char* str = newstr;
    char* oldstr = newstr;
    char** list;
    int listlen = 1;
    for (; *str; ++str) {if (*str == ':') ++listlen;}
    list = malloc(listlen * sizeof(char*));
    int listindex = 0;
    str = oldstr;
    while (1) {
        if (*str == ':' || !*str) {
            bool brk = !*str;
            *str = 0;
            list[listindex] = oldstr;
            oldstr = str + 1;
            ++listindex;
            if (brk) break;
        }
        ++str;
    }
    for (int i = 0; i < listlen; ++i) {
        int offset = 0;
        if ((offset = eqstrcmp(list[i], "div")) > -1) {
            replacestr(&colorcodes[COLOR_DIV], &list[i][offset]);
        } else if ((offset = eqstrcmp(list[i], "pos")) > -1) {
            replacestr(&colorcodes[COLOR_POS], &list[i][offset]);
        } else if ((offset = eqstrcmp(list[i], "hex")) > -1) {
            replacestr(&colorcodes[COLOR_HEX], &list[i][offset]);
        } else if ((offset = eqstrcmp(list[i], "hexh")) > -1) {
            replacestr(&colorcodes[COLOR_HEXH], &list[i][offset]);
        } else if ((offset = eqstrcmp(list[i], "hexz")) > -1) {
            replacestr(&colorcodes[COLOR_HEXZ], &list[i][offset]);
        } else if ((offset = eqstrcmp(list[i], "chr")) > -1) {
            replacestr(&colorcodes[COLOR_CHR], &list[i][offset]);
        } else if ((offset = eqstrcmp(list[i], "chrn")) > -1) {
            replacestr(&colorcodes[COLOR_CHRN], &list[i][offset]);
        }
    }
    free(newstr);
    free(list);
}

void puthelp() {
    printf("Quick HexDump version %s\n\n", VER);
    printf("USAGE: %s [OPTION]... FILE\n\n", argv[0]);
    puts("OPTIONS:");
    puts("      --help          Display help text and exit");
    puts("      --version       Display version info and exit");
    puts("  -b, --bar           Display a column bar");
    puts("  -e, --emptyline     Display an empty line if the file size is zero");
    puts("  -p, --hide-pos      Hide the position column");
    puts("  -h, --hide-hex      Hide the hexadecimal column");
    puts("  -c, --hide-chr      Hide the character column");
    puts("  -C, --color         Enable color escape codes\n");
    puts("ENVIRONMENT:");
    puts("    QHD_COLORS: Sets the pallette to use when color escape codes are enabled");
    puts("        Format is key=value separated by colons");
    puts("        Values are the middle parts of a color escape code (put between \"\\e[\" and \"m\")");
    puts("        Valid keys:");
    puts("            div:  Divider color");
    puts("            pos:  Position column color");
    puts("            hex:  Hexadecimal column color");
    puts("            hexh: Hexadecimal value over 7F color");
    puts("            hexz: Hexadecimal value of zero color");
    puts("            chr:  Character column color");
    puts("            chrn: Non-printable character color");
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
                } else if (!strcmp(opt, "color")) {
                    opt_color = true;
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
                    } else if (opt == 'C') {
                        opt_color = true;
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
    uint8_t retval = 0;
    #define _return(x) {retval = x; goto _ret;}
    if (!readopt()) _return(1);
    if (opt_color) {
        colorcodes[COLOR_NORM] = strdup("0");
        colorcodes[COLOR_DIV] = strdup("1");
        colorcodes[COLOR_POS] = strdup("32");
        colorcodes[COLOR_HEX] = strdup("34");
        colorcodes[COLOR_HEXH] = strdup("35");
        colorcodes[COLOR_HEXZ] = strdup("90");
        colorcodes[COLOR_CHR] = strdup("36");
        colorcodes[COLOR_CHRN] = strdup("90");
        parsecolors(getenv("QHD_COLORS"));
    }
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
            _return(1);
        }
        int fd = fileno(fp);
        struct stat statinfo;
        memset(&statinfo, 0, sizeof(statinfo));
        fstat(fd, &statinfo);
        if (S_ISDIR(statinfo.st_mode)) {
            fclose(fp);
            errno = EISDIR;
            aperror(fname);
            _return(1);
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
            setcolor(COLOR_POS);
            fputs("  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F ", stdout);
            putdiv();
            setcolor(COLOR_NORM);
            putchar('\n');
        }
        size_t i = 0;
        if (opt_showpos) {
            putdiv();
            setcolor(COLOR_POS);
            printf(posfmt, pos);
        }
        if (opt_showhex) {
            putdiv();
            setcolor(COLOR_HEX);
            putchar(' ');
            for (; i < bread; ++i) {
                if (!buf[i] || buf[i] > 0x7F) {
                    if (!buf[i]) {
                        setcolor(COLOR_HEXZ);
                    } else {
                        setcolor(COLOR_HEXH);
                    }
                    printf("%02X", buf[i]);
                    setcolor(COLOR_HEX);
                    putchar(' ');
                } else {
                    printf("%02X ", buf[i]);
                }
            }
            for (; i < 16; ++i) {
                buf[i] = 0;
                fputs("   ", stdout);
            }
        }
        if (opt_showchr) {
            putdiv();
            setcolor(COLOR_CHR);
            putchar(' ');
            for (size_t i = 0; i < 16; ++i) {
                if (buf[i] >= 32 && buf[i] <= 126) {
                    putchar(buf[i]);
                } else {
                    setcolor(COLOR_CHRN);
                    if (i < bread) {
                        #if defined(QHD_ASCII) && QHD_ASCII
                            putchar('.');
                        #else
                            printf("·");
                        #endif
                    } else {
                        putchar(' ');
                    }
                    setcolor(COLOR_CHR);
                }
            }
            putchar(' ');
        }
        putdiv();
        pos += 16;
        setcolor(COLOR_NORM);
        putchar('\n');
    }
    if (fname) fclose(fp);
    _ret:;
    if (opt_color) {
        for (int i = 0; i < COLORCODE_LIST_SIZE; ++i) {
            free(colorcodes[i]);
        }
    }
    return retval;
}
