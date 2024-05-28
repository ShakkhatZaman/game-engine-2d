#include "io.h"
#include "../utils.h"
#include <stdlib.h>
#include <stdio.h>


File read_file(const char *filepath) {
    File file = {.data = NULL, .size = 0, .is_valid = false};

    FILE *file_ptr = fopen(filepath, "rb");
    if (!file_ptr) {
        ERROR_RETURN(file, "Unable to open for file\n");
    }

    char *tmp = file.data;
    uint64 total_bytes_read = 0;
    uint16 n = 0;

    while (true) {
        if (total_bytes_read + 1024 + 1 > file.size) { // if buffer size isn't big enough to read the next chunk then increase size and allocate more space
            file.size = total_bytes_read + 1024 + 1;
            if (file.size <= total_bytes_read) {
                free(file.data);
                fclose(file_ptr);
                file.data = NULL;
                file.size = 0;
                ERROR_RETURN(file, "File too big :O.\n");
            }

            tmp = realloc(file.data, file.size);
            if (!tmp) {
                free(file.data);
                fclose(file_ptr);
                file.data = NULL;
                file.size = 0;
                ERROR_RETURN(file, "Unable to alllocate space for file\n");
            }
            file.data = tmp;
        }
        n = fread(file.data + total_bytes_read, 1, 1024, file_ptr);
        total_bytes_read += n;
        if (n < 1024) break;
    }

    file.data[total_bytes_read] = '\0';
    file.size = total_bytes_read;
    file.is_valid = true;
    fclose(file_ptr);

    return file;
}
