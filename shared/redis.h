#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <cassert>
#include <cstdint>
#include <cstring>

const size_t k_max_msg = 4096;

int32_t parse_buffered_read(int connection_file_descriptor, char* buffer, size_t buffer_size, uint32_t* message_length) {
    size_t total_read = 0;
    bool read_length = false;

    while (buffer_size > 0) {
        ssize_t number_read = read(connection_file_descriptor, buffer, buffer_size);

        total_read += number_read;

        if(!read_length && total_read >= 4) {
            // We have read the first 4 bytes, which contains the length of the message
            memcpy(message_length, buffer, 4);
            read_length = true;
        }

        if(read_length && total_read >= 4 + *message_length) {
            // We have read the entire message
            return 0;
        }

        if(number_read <= 0) {
            return -1;
        }

        assert((size_t)number_read <= buffer_size);
        buffer_size -= (size_t)number_read;
        buffer += number_read;
    }

    return 0;
}

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