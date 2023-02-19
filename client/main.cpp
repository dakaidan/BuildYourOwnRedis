#include <redis.h>

#define LOG_TO_FILE true
#define LOG_FILE SOURCE_DIR "/log.txt"  // Log to the source directory in log.txt
#define LOG_LEVEL 0
#include <logger.h>

#include <netinet/in.h>
#include <cstring>


static int32_t query_server(int connection_file_descriptor, char* text) {
    auto text_length = (uint32_t) strlen(text);

    if (text_length > k_max_msg) {
        LOG_ERROR("Message too long");
        return -1;
    }

    char write_buffer[4 + k_max_msg + 1];
    memcpy(write_buffer, &text_length, 4);
    strcpy(&write_buffer[4], text);

    if (int32_t err = write_all(connection_file_descriptor, write_buffer, 4 + text_length)) {
        return err;
    }

    char read_buffer[4 + k_max_msg + 1];
    errno = 0;

    int32_t err = read_full(connection_file_descriptor, read_buffer, 4);

    if (err) {
        if (errno == 0) {
            LOG_DEBUG("EOF");
        } else {
            LOG_ERROR("Error reading message length");
        }
        return err;
    }

    memcpy(&text_length, read_buffer, 4);
    if (text_length > k_max_msg) {
        LOG_ERROR("Message too long");
        return -1;
    }

    err = read_full(connection_file_descriptor, &read_buffer[4], text_length);
    if (err) {
        LOG_ERROR("Error reading message");
        return err;
    }

    read_buffer[4 + text_length] = '\0'; // Null terminate the string
    LOG_TRACE("Received: %s", &read_buffer[4]);
    return 0;
}


int main()
{
    LOG_INFO("Starting client");

    int file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    if(file_descriptor < 0) {
        LOG_FATAL("Failed to create socket");
        return 1;
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // Bind to loopback interface (127.0.0.1)

    int return_value = connect(file_descriptor, (sockaddr*)&addr, sizeof(addr));

    if(return_value) {
        LOG_FATAL("Failed to connect to server");
        return 1;
    }

    char text[] = "Hello, Server!";

    int32_t err = query_server(file_descriptor, text);
    if (err) {
        LOG_ERROR("Error querying server");
        close(file_descriptor);
        return 1;
    }

    text[strlen(text) - 1] = '?';

    err = query_server(file_descriptor, text);
    if (err) {
        LOG_ERROR("Error querying server");
        close(file_descriptor);
        return 1;
    }

    // change 'Server?' to 'World!!'
    text[strlen(text) - 1] = '~';

    err = query_server(file_descriptor, text);
    if (err) {
        LOG_ERROR("Error querying server");
        close(file_descriptor);
        return 1;
    }

    close(file_descriptor);
    return 0;
}