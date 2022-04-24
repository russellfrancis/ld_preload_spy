#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/socket.h>

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
