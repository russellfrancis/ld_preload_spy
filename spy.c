#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define FD_LIST_MAX_SIZE 4096
static int fd_list_size = 0;
static int fd_list[FD_LIST_MAX_SIZE];

int socket(int domain, int type, int protocol) {
    char *type_string;
    int is_inet_4_socket = 0;
    if (domain == AF_INET) {
        switch (type & 0xFF) {
            case SOCK_STREAM:
                type_string = "SOCK_STREAM";
                break;
            case SOCK_DGRAM:
                type_string = "SOCK_DGRAM";
                break;
            case SOCK_SEQPACKET:
                type_string = "SOCK_SEQPACKET";
                break;
            case SOCK_RAW:
                type_string = "SOCK_RAW";
                break;
            case SOCK_RDM:
                type_string = "SOCK_RDM";
                break;
            case SOCK_PACKET:
                type_string = "SOCK_PACKET";
                break;
            default:
                type_string = "UNKNOWN";
        }
        is_inet_4_socket = 1;
    }

    int (*original_socket)(int, int, int);
    original_socket = dlsym(RTLD_NEXT, "socket");
    int fd = (*original_socket)(domain, type, protocol);

    if (is_inet_4_socket == 1 && fd_list_size < FD_LIST_MAX_SIZE) {
        dprintf(STDERR_FILENO, "PID:%d [%d <- socket(AF_INET, %s, %d)]\n", getpid(), fd, type_string, protocol);
        fd_list[fd_list_size++] = fd;
    }

    return fd;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    // IPv4 network connection.
    if (addr->sa_family == AF_INET) {
        const struct sockaddr_in *sin = (const struct sockaddr_in *) addr;
        char ipv4_address[INET_ADDRSTRLEN];
        if (inet_ntop(sin->sin_family, &sin->sin_addr.s_addr, ipv4_address, INET_ADDRSTRLEN) != NULL) {
            dprintf(STDERR_FILENO, "Connection to IPv4 (address = %s, port = %d).\n", ipv4_address, sin->sin_port);
        }
    }
    // IPv6 network connection.
    else if (addr->sa_family == AF_INET6) {

    }

    int (*original_connect)(int, const struct sockaddr*, socklen_t);
    original_connect = dlsym(RTLD_NEXT, "connect");
    return (*original_connect)(sockfd, addr, addrlen);
}

ssize_t write(int fd, const void *buf, size_t count) {
    for (int i = 0; i < fd_list_size; ++i) {
        if (fd_list[i] == fd) {
            dprintf(STDERR_FILENO, "write(%d) <-", fd);
            unsigned char str[21];
            for (int j = 0; j < count; ++j) {
                if (j % 20 == 0) {
                    str[20] = '\0';
                    dprintf(STDERR_FILENO, "  %s\n\t", str);
                }
                dprintf(STDERR_FILENO, "%02x ", ((unsigned char *) buf)[j]);
                str[j % 20] = ((unsigned char *) buf)[j];
            }
            str[20] = '\0';
            dprintf(STDERR_FILENO, "  %s\n\n", str);
            break;
        }
    }

    int (*original_write)(int, const void *, size_t);
    original_write = dlsym(RTLD_NEXT, "write");
    return (*original_write)(fd, buf, count);
}

ssize_t read(int fd, void *buf, size_t count) {
    for (int i = 0; i < fd_list_size; ++i) {
        if (fd_list[i] == fd) {
            dprintf(STDERR_FILENO, "read(%d) <-", fd);
            unsigned char str[21];
            for (int j = 0; j < count; ++j) {
                if (j % 20 == 0) {
                    str[20] = '\0';
                    dprintf(STDERR_FILENO, "  %s\n\t", str);
                }
                dprintf(STDERR_FILENO, "%02x ", ((unsigned char *) buf)[j]);
                str[j % 20] = ((unsigned char *) buf)[j];
            }
            str[20] = '\0';
            dprintf(STDERR_FILENO, "  %s\n\n", str);
            break;
        }
    }

    int (*original_read)(int, void *, size_t);
    original_read = dlsym(RTLD_NEXT, "read");
    return (*original_read)(fd, buf, count);
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (nmemb > 3) {
        char str[size * nmemb];
        memcpy(str, ptr, size * nmemb);
        str[size * nmemb - 1] = '\0';
        dprintf(STDERR_FILENO, "fwrite(%ld, %ld) <- %s\n\n", size, nmemb, str);
    }

    int (*original_fwrite)(const void *, size_t, size_t, FILE *);
    original_fwrite = dlsym(RTLD_NEXT, "fwrite");
    return (*original_fwrite)(ptr, size, nmemb, stream);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    for (int i = 0; i < fd_list_size; ++i) {
        if (fd_list[i] == sockfd) {
            char str[len];
            memcpy(str, buf, len);
            str[len-1] = '\0';
            dprintf(STDERR_FILENO, "send(%d, %ld, %d) <- %s\n", sockfd, len, flags, (const char *)buf);
        }
    }

    int (*original_send)(int, const void *, size_t, int);
    original_send = dlsym(RTLD_NEXT, "send");
    return (*original_send)(sockfd, buf, len, flags);
}

int close(int fd) {
    for (int i = 0; i < fd_list_size; ++i) {
        if (fd_list[i] == fd) {
            // Then remove the item from the list.
            fd_list_size--;
            if (fd_list_size - i > 0) {
                memcpy(&fd_list[i], &fd_list[i + 1], (fd_list_size - i) * sizeof(int));
            }
            dprintf(STDERR_FILENO, "%d removed from file descriptor list.\n", fd);
            break;
        }
    }

    int (*original_close)(int);
    original_close = dlsym(RTLD_NEXT, "close");
    return (*original_close)(fd);
}
