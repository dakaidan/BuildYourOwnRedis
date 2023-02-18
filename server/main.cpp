#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

static void process_connection(int connection_file_descriptor) {
    char read_buffer[64] = {};

    ssize_t number_read = read(connection_file_descriptor, read_buffer, sizeof(read_buffer) - 1);

    if(number_read < 0) {
        std::cout << "Failed to read from connection" << std::endl;
        return;
    }

    printf("Received: %s\n", read_buffer);

    char write_buffer[] = "Hello, Client!";

    write(connection_file_descriptor, write_buffer, sizeof(write_buffer));
}

int main()
{
    std::cout << "Hello, Redis Server!" << std::endl;

    /*
     * Get a file descriptor for a socket.
     * AF_INET: IPv4
     * SOCK_STREAM: TCP
     */
    int file_descriptor = socket(AF_INET, SOCK_STREAM, 0);

    int val = 1;

    /*
     * Set the socket option to SO_REUSEADDR to allow the server to bind
     * to the same address if restarted.
     * In the event of a packet being delayed after the shutdown, we can:
     * - Disallow the server from binding to the same addr for some time (2*MSL or 2*RTT).
     *   This is the default behavior. and is typically 30 - 120 seconds.
     * - Allow the program to re-bind to that address immediately. This is what SO_REUSEADDR
     *   does.
     *
     * The potential issue is its ambiguous if the packet is stale and should be dropped or
     * if it is a new packet that should be processed.
     *
     * https://stackoverflow.com/questions/3229860/what-is-the-meaning-of-so-reuseaddr-setsockopt-option-linux
     */
    setsockopt(file_descriptor, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;                  // IPv4
    addr.sin_port = ntohs(1234);                // Port 1234
    addr.sin_addr.s_addr = ntohl(INADDR_ANY);   // Bind to all interfaces (0.0.0.0)

    /*
     * Bind the socket to the address.
     * The socket is now ready to accept connections.
     *
     * We pass in the file descriptor, the address (interpreted as a sockaddr*), and the size
     */
    int ret_val = bind(file_descriptor, (const sockaddr*)&addr, sizeof(addr));

    if(ret_val) {
        std::cout << "Failed to bind to address" << std::endl;
        return 1;
    }

    /*
     * Listen for connections.
     *
     * We pass in the file descriptor and the maximum number of connections to queue.
     */
    ret_val = listen(file_descriptor, SOMAXCONN);

    if(ret_val) {
        std::cout << "Failed to listen for connections" << std::endl;
        return 1;
    }

    while (true) {
        // accept the connection
        sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);

        /*
         * Accept a connection.
         * We get a new file descriptor for the connection.
         */
        int conn_file_descriptor = accept(file_descriptor, (sockaddr*)&client_addr, &socklen);

        if(conn_file_descriptor < 0) {
            std::cout << "Failed to accept connection" << std::endl;
            continue;
        }

        process_connection(conn_file_descriptor);

        // close the connection
        close(conn_file_descriptor);
    }

    // TODO: Handle SIGINT and SIGTERM to gracefully shutdown the server
    close(file_descriptor);

    return 0;
}