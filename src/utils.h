#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

void die(const char *fmt, ...);
void *xmalloc(size_t size);
FILE *xfopen(const char *path, const char *mode);
DIR *xopendir(const char * path);
void xmkdir(const char *path, mode_t mode);
void to_hex_str(const unsigned char *data, size_t len, char *out);

#endif
