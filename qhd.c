/*-------------------------------------------------------------------*/
/*                                                                   */
/*  Quick HexDump                                                    */
/*                                                                   */
/*  (C) 2022 PQCraft                                                 */
/*  Licensed under the GNU General Public License version 3          */
/*  https://www.gnu.org/licenses/gpl-3.0.txt                         */
/*                                                                   */
/*-------------------------------------------------------------------*/

const char* VER = "2.3";

#ifndef QHD_ASCII
    #define QHD_ASCII 0 // Enable/disable 7-bit ASCII mode by default
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
bool opt_ascii = QHD_ASCII;
bool opt_extascii = false;
bool opt_startpoint = false;
long opt_start = 0;
bool opt_endpoint = false;
long opt_end = 0;

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
    COLOR_CHRE,
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
    if (opt_ascii) {
        putchar('|');
    } else {
        printf("│");
    }
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
        } else if ((offset = eqstrcmp(list[i], "chre")) > -1) {
            replacestr(&colorcodes[COLOR_CHRE], &list[i][offset]);
        }
    }
    free(newstr);
    free(list);
}

void puthelp() {
    printf("Quick HexDump version %s\n", VER);
    printf("\nUSAGE: %s [OPTION]... FILE\n", argv[0]);
    puts("\nOPTIONS:");
    puts("      --help              Display help text and exit");
    puts("      --version           Display version info and exit");
    puts("  -b, --bar               Display a column bar");
    puts("  -l, --emptyline         Display an empty line if the file size is zero");
    puts("  -p, --hide-pos          Hide the position column");
    puts("  -h, --hide-hex          Hide the hexadecimal column");
    puts("  -c, --hide-chr          Hide the character column");
    puts("  -C, --color             Enable color escape codes");
    puts("  -a, --ascii             Use only 7-bit ASCII characters");
    puts("  -A, --ext-char          Use UTF-8 characters");
    puts("  -e, --ext-ascii         Display extended ASCII in the character panel");
    puts("  -s, --start POS         Start dumping from POS");
    puts("  -S, --stop POS          Stop dumping at POS");
    puts("  -L, --length BYTES      Stop dumping at BYTES after the starting point");
    puts("\nENVIRONMENT:");
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
    puts("            chre: Extended ASCII character color");
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
            bool shortopt = !(argv[i][1] == '-');
            if (!shortopt && !argv[i][2]) {opt_onlyfile = true; continue;}
            char* opt = argv[i];
            char sopt = 0;
            int sopos = 0;
            soret:;
            ++sopos;
            if (shortopt) {
                if (!(sopt = opt[sopos])) continue;
            } else {
                opt += 2;
            }
            if (!shortopt && !strcmp(opt, "help")) {
                puthelp();
            } else if (!shortopt && !strcmp(opt, "version")) {
                putver();
            } else if ((shortopt) ? sopt == 'b' : !strcmp(opt, "bar")) {
                opt_bar = true;
            } else if ((shortopt) ? sopt == 'l' : !strcmp(opt, "emptyline")) {
                opt_emptyln = true;
            } else if ((shortopt) ? sopt == 'p' : !strcmp(opt, "hide-pos")) {
                opt_showpos = false;
            } else if ((shortopt) ? sopt == 'h' : !strcmp(opt, "hide-hex")) {
                opt_showhex = false;
            } else if ((shortopt) ? sopt == 'c' : !strcmp(opt, "hide-chr")) {
                opt_showchr = false;
            } else if ((shortopt) ? sopt == 'C' : !strcmp(opt, "color")) {
                opt_color = true;
            } else if ((shortopt) ? sopt == 'a' : !strcmp(opt, "ascii")) {
                opt_ascii = true;
            } else if ((shortopt) ? sopt == 'A' : !strcmp(opt, "ext-char")) {
                opt_ascii = false;
            } else if ((shortopt) ? sopt == 'e' : !strcmp(opt, "ext-ascii")) {
                opt_extascii = true;
            } else if ((shortopt) ? sopt == 's' : !strcmp(opt, "start")) {
                ++i;
                if (i >= argc) {cperror("Expected argument"); return false;}
                opt_startpoint = true;
                opt_start = atoi(argv[i]);
            } else if ((shortopt) ? sopt == 'S' : !strcmp(opt, "stop")) {
                ++i;
                if (i >= argc) {cperror("Expected argument"); return false;}
                opt_endpoint = true;
                opt_end = atoi(argv[i]);
            } else if ((shortopt) ? sopt == 'L' : !strcmp(opt, "length")) {
                ++i;
                if (i >= argc) {cperror("Expected argument"); return false;}
                opt_endpoint = true;
                opt_end = opt_start + atoi(argv[i]);
            } else {
                errno = EINVAL;
                if (shortopt) {
                    char tmp[3] = {'-', sopt, 0};
                    aperror(tmp);
                } else {
                    aperror(argv[i]);
                }
                return false;
            }
            if (shortopt) goto soret;
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
    if (opt_startpoint || opt_endpoint) {
        if (opt_start < 0) opt_start = 0;
        if (opt_end < 0) opt_end = 0;
        if (opt_start > opt_end) opt_start = opt_end;
    }
    if (opt_color) {
        colorcodes[COLOR_NORM] = strdup("0");
        colorcodes[COLOR_DIV] = strdup("1");
        colorcodes[COLOR_POS] = strdup("32");
        colorcodes[COLOR_HEX] = strdup("34");
        colorcodes[COLOR_HEXH] = strdup("35");
        colorcodes[COLOR_HEXZ] = strdup("90");
        colorcodes[COLOR_CHR] = strdup("36");
        colorcodes[COLOR_CHRN] = strdup("90");
        colorcodes[COLOR_CHRE] = strdup("33");
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
    long offset = 0;
    long pos = 0;
    unsigned char buf[16];
    if (opt_startpoint) {
        for (; offset < opt_start; ++offset) {
            fread(buf, 1, 1, fp);
        }
    }
    size_t bread = 0;
    memset(buf, 0, 16);
    while ((bread = fread(buf, 1, 16, fp)) || (opt_emptyln && !pos)) {
        if (opt_endpoint) {
            long tmp = opt_end - pos - offset;
            if (tmp < 16) bread = tmp;
            if (bread < 1 && !opt_emptyln) break;
        }
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
            printf(posfmt, pos + offset);
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
                } else if (opt_extascii && buf[i] > 127) {
                    setcolor(COLOR_CHRE);
                    if (opt_ascii) {
                        putchar(buf[i]);
                    } else {
                        putchar((buf[i] >> 6) | 0xC0 | (buf[i] < 0xA1) * 0x04);
                        putchar((buf[i] & 0x3F) | 0x80);
                    }
                    setcolor(COLOR_CHR);
                } else {
                    setcolor(COLOR_CHRN);
                    if (i < bread) {
                        if (opt_ascii) {
                            putchar('.');
                        } else {
                            printf("·");
                        }
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
        if (bread < 16) break;
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
