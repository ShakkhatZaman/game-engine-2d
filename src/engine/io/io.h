#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdbool.h>
#include "../types.h"

typedef struct file {
    char *data;
    uint64 size;
    bool is_valid;
} File;

File read_file(const char *filepath);
bool write_file(const char *filepath, uint64 size, const char *data);

#endif // !IO_H
