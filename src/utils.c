#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "utils.h"
#include <dirent.h>



void to_hex_str(const unsigned char *data, size_t len, char *out) {
    for (size_t i = 0; i < len; i++) {
        sprintf(out + (i * 2), "%02x", data[i]);
    }
    out[len * 2] = '\0';
}

void die(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);

    exit(EXIT_FAILURE);
}

void *xmalloc(size_t size)
{
    void *ret = malloc(size);
    if (!ret && !size)
        ret = malloc(1);
    if (!ret)
        die("Out of memory, malloc failed");
    return ret;
}

FILE *xfopen(const char *path, const char *mode)
{
    FILE *f = fopen(path, mode);
    if (!f) {
        die("Failed to open file '%s'", path);
    }
    return f;
}

DIR *xopendir(const char * path){
    DIR *dir = opendir(path);
    if(!dir){
        die("Failed to open dir '%s'", path);
    }
    return dir;
}


void xmkdir(const char *path, mode_t mode){
    struct stat st;

    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return;
        } 
    }
    if (mkdir(path, mode) == -1) {
        die("dir %s can not have been created", path);
    }
}