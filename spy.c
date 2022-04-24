#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int is_ascii_text(const void *buf, size_t len) {
    int result = 1;
    for (long i = 0; i < len; ++i) {
        if (((char *)buf)[i] <= 0 || ((char *)buf)[i] > 127) {
            result = 0;
            break;
        }
    }
    return result;
}

void print_hex(const void *buf, size_t len) {
    for (long j = 0; j < len; ++j) {
        if (j % 20 == 0) {
            dprintf(STDERR_FILENO, "\n\t");
        }
        dprintf(STDERR_FILENO, "%02x ", ((unsigned char *) buf)[j]);
    }
    dprintf(STDERR_FILENO, "\n\n");
}

void print_ascii(const void *buf, size_t len) {
    char str[len];
    memcpy(str, buf, len);
    str[len - 1] = '\0';
    dprintf(STDERR_FILENO, "%s\n", (char *) str);
}

void print(const void *buf, size_t len) {
    if (is_ascii_text(buf, len)) {
        print_ascii(buf, len);
    } else {
        print_hex(buf, len);
    }
}

int socket(int domain, int type, int protocol) {

    // Get a reference to and call the original method.
    int (*original_socket)(int, int, int);
    original_socket = dlsym(RTLD_NEXT, "socket");
    int fd = (*original_socket)(domain, type, protocol);

    // If the is an INET socket lets log a message.
    if (domain == AF_INET) {
        dprintf(STDERR_FILENO, "file descriptor %d opened socket(AF_INET, %d, %d)]\n", fd, type, protocol);
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

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    dprintf(STDERR_FILENO, "file descriptor %d sent %ld bytes.\n", sockfd, len);
    print(buf, len);

    // get a reference to and call the origin send method.
    int (*original_send)(int, const void *, size_t, int);
    original_send = dlsym(RTLD_NEXT, "send");
    return (*original_send)(sockfd, buf, len, flags);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    // get a reference to and call the origin send method.
    int (*original_recv)(int, void *, size_t, int);
    original_recv = dlsym(RTLD_NEXT, "recv");
    size_t bytes = (*original_recv)(sockfd, buf, len, flags);

    if (bytes != -1) {
        dprintf(STDERR_FILENO, "file descriptor %d recv %ld bytes.\n", sockfd, bytes);
        print(buf, bytes);
    } else {
        dprintf(STDERR_FILENO, "file descriptor %d recv failed.\n", sockfd);
    }
    return bytes;
}

ssize_t write(int fd, const void *buf, size_t count) {
    dprintf(STDERR_FILENO, "file descriptor %d write %ld bytes.\n", fd, count);
    print(buf, count);

    // call original method.
    int (*original_write)(int, const void *, size_t);
    original_write = dlsym(RTLD_NEXT, "write");
    return (*original_write)(fd, buf, count);
}

ssize_t read(int fd, void *buf, size_t count) {
    // call original method.
    int (*original_read)(int, void *, size_t);
    original_read = dlsym(RTLD_NEXT, "read");
    ssize_t bytes = (*original_read)(fd, buf, count);

    if (bytes != -1) {
        dprintf(STDERR_FILENO, "file descriptor %d read %ld bytes.\n", fd, bytes);
        print(buf, bytes);
    } else {
        dprintf(STDERR_FILENO, "file descriptor %d read failed.\n", fd);
    }

    return bytes;
}
