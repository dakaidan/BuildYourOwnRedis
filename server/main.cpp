#include <redis.h>

#define LOG_TO_FILE true
#define LOG_FILE SOURCE_DIR "/log.txt"  // Log to the source directory in log.txt
#define LOG_LEVEL 0

#include <logger.h>

#include <netinet/in.h>
#include <cstring>


static void process_connection(int connection_file_descriptor) {
    char read_buffer[64] = {};

    ssize_t number_read = read(connection_file_descriptor, read_buffer, sizeof(read_buffer) - 1);

    if(number_read < 0) {
        LOG_ERROR("Error reading from socket");
        return;
    }

    LOG_TRACE("Received: %s", read_buffer);

    char write_buffer[] = "Hello, Client!";

    write(connection_file_descriptor, write_buffer, sizeof(write_buffer));
}

static int32_t handle_single_request(int connection_file_descriptor) {
    char read_buffer[4 + k_max_msg + 1];

    errno = 0;
    uint32_t message_length = 0;
    int32_t err = parse_buffered_read(connection_file_descriptor, read_buffer, 4 + k_max_msg, &message_length);

    if (err) {
        if (errno == 0) {
            LOG_DEBUG("EOF");
        } else {
            LOG_ERROR("Error reading message length");
        }

        return err;
    }

    if (message_length > k_max_msg) {
        LOG_ERROR("Message too long");
        return -1;
    }

    read_buffer[4 + message_length] = '\0'; // Null terminate the string
    LOG_TRACE("Received: %s", &read_buffer[4]);

    // Write the message back to the client
    const char reply[] = "Hello, Client!";
    char write_buffer[4 + sizeof(reply)];
    message_length = (uint32_t)sizeof(reply);

    memcpy(write_buffer, &message_length, 4);
    memcpy(&write_buffer[4], reply, message_length);
    return write_all(connection_file_descriptor, write_buffer, 4 + message_length);
}

int main()
{
    LOG_INFO("Hello, Redis Server!");

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
        LOG_FATAL("Failed to bind socket to address");
        return 1;
    }

    /*
     * Listen for connections.
     *
     * We pass in the file descriptor and the maximum number of connections to queue.
     */
    ret_val = listen(file_descriptor, SOMAXCONN);

    if(ret_val) {
        LOG_FATAL("Failed to listen for connections");
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
            LOG_ERROR("Failed to accept connection");
            continue;
        }

        // Serve one connection at a time
        while (true) {
            int32_t err = handle_single_request(conn_file_descriptor);
            if(err) {
                break;
            }
        }

        // close the connection
        close(conn_file_descriptor);
    }

    // TODO: Handle SIGINT and SIGTERM to gracefully shutdown the server
    close(file_descriptor);

    return 0;
}