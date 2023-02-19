#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <cassert>

const size_t k_max_msg = 4096;

int32_t read_full(int connection_file_descriptor, char* buffer, size_t buffer_size) {
    while (buffer_size > 0) {
        ssize_t number_read = read(connection_file_descriptor, buffer, buffer_size);

        if(number_read <= 0) {
            return -1;
        }

        assert((size_t)number_read <= buffer_size);
        buffer_size -= (size_t)number_read;
        buffer += number_read;
    }

    return 0;
}

int32_t write_all(int connection_file_descriptor, char* buffer, size_t buffer_size) {
    while (buffer_size > 0) {
        ssize_t number_written = write(connection_file_descriptor, buffer, buffer_size);

        if(number_written <= 0) {
            return -1;
        }

        assert((size_t)number_written <= buffer_size);
        buffer_size -= (size_t)number_written;
        buffer += number_written;
    }

    return 0;
}