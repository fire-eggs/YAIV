//
// Created by kevin on 5/11/21.
//

#include "humansize.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <FL/Fl.H>
#include <FL/fl_utf8.h> // fl_stat
// -- platform dependancy --
// #include <sys/stat.h>   // stat in Fl/Fl.H

static char *humanSize(size_t bytes, char *buf, int bufsize)
{
    const char *suffix[] = {"B", "K", "M", "G", "T"};
    char length = sizeof(suffix) / sizeof(suffix[0]);

    int i = 0;
    double dblBytes = bytes;

    if (bytes > 1024) {
        for (i = 0; (bytes / 1024) > 0 && i<length-1; i++, bytes /= 1024)
            dblBytes = bytes / 1024.0;
    }

    snprintf(buf, bufsize, "%.02lf%s", dblBytes, suffix[i]);
    return buf;
}

char *humanSize(char *fname, char *buf, int bufsize)
{
    buf[0] = '\0'; // make sure empty string on fl_stat failure
    struct stat s;
    if (fl_stat(fname, &s))
        return NULL;
    return humanSize(s.st_size, buf, bufsize);
}
