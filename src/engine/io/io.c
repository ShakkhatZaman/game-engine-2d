#include "io.h"
#include "../utils.h"
#include <stdlib.h>
#include <stdio.h>


File read_file(const char *filepath) {
    File file = {.data = NULL, .size = 0, .is_valid = false};

    FILE *file_ptr = fopen(filepath, "rb");
    if (!file_ptr) {
        ERROR_RETURN(file, "Unable to open file: %s\n", filepath);
    }

    char *tmp = file.data;
    uint64 total_bytes_read = 0;
    uint16 n = 0;

    while (true) {
        // if *buffer size* isn't big enough to read the next chunk then increase size and allocate more space
        if (total_bytes_read + 1024 + 1 > file.size) {
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
        // keep reading untill nothing left
        n = fread(file.data + total_bytes_read, 1, 1024, file_ptr);
        total_bytes_read += n;
        if (n < 1024) break;
    }

    fclose(file_ptr);
    file.data[total_bytes_read] = '\0';
    file.size = total_bytes_read;
    file.is_valid = true;

    return file;
}

bool write_file(const char *filepath, uint64 size, const char *data) {
    FILE *file_ptr = fopen(filepath, "wb");
    if (!file_ptr) {
        ERROR_RETURN(false, "Unable to open file: %s\n", filepath);
    }
    
    uint64 bytes_written = fwrite(data, 1, size, file_ptr);

    fclose(file_ptr);
    if (bytes_written != size) {
        ERROR_RETURN(false, "Unable to write to file: %s. Expected to write %lld bytes got %lld bytes\n",
                     filepath, size, bytes_written);
    }
    return true;
}
