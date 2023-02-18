#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


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

    char write_buffer[] = "Hello, Server!";
    write(file_descriptor, write_buffer, sizeof(write_buffer) - 1);

    char read_buffer[64] = {};
    ssize_t number_read = read(file_descriptor, read_buffer, sizeof(read_buffer) - 1);

    if (number_read < 0) {
        std::cout << "Failed to read from connection" << std::endl;
        return 1;
    }

    printf("Received: %s\n", read_buffer);
    close(file_descriptor);
}