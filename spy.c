#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int socket(int domain, int type, int protocol) {

    // Get a reference to and call the original method.
    int (*original_socket)(int, int, int);
    original_socket = dlsym(RTLD_NEXT, "socket");
    int fd = (*original_socket)(domain, type, protocol);

    // If the is an INET socket lets log a message.
    if (domain == AF_INET) {
        dprintf(STDERR_FILENO, "[%d <- socket(AF_INET, %d, %d)]\n", fd, type, protocol);
    }

    // return the original result.
    return fd;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    // IPv4 network connection.
    if (addr->sa_family == AF_INET) {
        // cast sockaddr -> sockaddr_in
        const struct sockaddr_in *sin = (const struct sockaddr_in *) addr;
        char ipv4_address[INET_ADDRSTRLEN];
        if (inet_ntop(sin->sin_family, &sin->sin_addr.s_addr, ipv4_address, INET_ADDRSTRLEN) != NULL) {
            dprintf(STDERR_FILENO, "file descriptor %d connected to (address = %s, port = %d).\n", sockfd, ipv4_address, sin->sin_port);
        }
    }

    // get a reference to and call the original method.
    int (*original_connect)(int, const struct sockaddr*, socklen_t);
    original_connect = dlsym(RTLD_NEXT, "connect");
    return (*original_connect)(sockfd, addr, addrlen);
}
