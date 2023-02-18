#include <redis.h>

#include <iostream>
#include <netinet/in.h>
#include <cstring>


static int32_t query_server(int connection_file_descriptor, char* text) {
    auto text_length = (uint32_t) strlen(text);

    if (text_length > k_max_msg) {
        std::cout << "Message too long" << std::endl;
        return -1;
    }

    char write_buffer[4 + k_max_msg];
    memcpy(write_buffer, &text_length, 4);
    memcpy(&write_buffer[4], text, text_length);

    if (int32_t err = write_all(connection_file_descriptor, write_buffer, 4 + text_length)) {
        return err;
    }

    char read_buffer[4 + k_max_msg + 1];
    errno = 0;

    int32_t err = read_full(connection_file_descriptor, read_buffer, 4);

    if (err) {
        if (errno == 0) {
            std::cout << "EOF" << std::endl;
        } else {
            std::cout << "Error reading message length" << std::endl;
        }

        return err;
    }

    memcpy(&text_length, read_buffer, 4);
    if (text_length > k_max_msg) {
        std::cout << "Message too long" << std::endl;
        return -1;
    }

    err = read_full(connection_file_descriptor, &read_buffer[4], text_length);
    if (err) {
        std::cout << "Error reading message" << std::endl;
        return err;
    }

    read_buffer[4 + text_length] = '\0'; // Null terminate the string
    printf("Received: %s\n", &read_buffer[4]);
    return 0;
}


int main()
{
    std::cout << "Hello, Redis Client!" << std::endl;

    int file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    if(file_descriptor < 0) {
        std::cout << "Failed to create socket" << std::endl;
        return 1;
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // Bind to loopback interface (127.0.0.1)

    int return_value = connect(file_descriptor, (sockaddr*)&addr, sizeof(addr));

    if(return_value) {
        std::cout << "Failed to connect to server" << std::endl;
        return 1;
    }

    char text[] = "Hello, Server!";

    int32_t err = query_server(file_descriptor, text);
    if (err) {
        goto L_DONE;
    }

    text[strlen(text) - 1] = '?';

    err = query_server(file_descriptor, text);
    if (err) {
        goto L_DONE;
    }

    // change 'Server?' to 'World!!'
    text[strlen(text) - 1] = '~';

    err = query_server(file_descriptor, text);
    if (err) {
        goto L_DONE;
    }

L_DONE:
    close(file_descriptor);
    return 0;
}