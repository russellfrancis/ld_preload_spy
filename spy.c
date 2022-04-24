#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dlfcn.h>

FILE *fopen(const char *path, const char *mode) {
    printf("In our own fopen, opening %s\n", path);

    FILE *(*original_fopen)(const char *, const char *);
    original_fopen = dlsym(RTLD_NEXT, "fopen");
    return (*original_fopen)(path, mode);
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    printf("Connecting to (%d,%s).", addr->sa_family, addr->sa_data);
    int (*original_connect)(int, const struct sockaddr*, socklen_t);
    original_connect = dlsym(RTLD_NEXT, "connect");
    return (*original_connect)(sockfd, addr, addrlen);
}
