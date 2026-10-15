#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <stdint.h>      // int64_t, uint64_t
#include <string.h>      // memcpy
#include <endian.h>      // htobe64, be64toh
#include <sys/types.h>   // ssize_t, size_t
#include <sys/socket.h>  // send, recv
#include <errno.h>

#define PUERTO_DEFECTO 5000

// Envía exactamente 'len' bytes.
// Devuelve 0 si se enviaron todos, -1 si hubo error.
static inline int enviar_todo(int fd, const void *datos, size_t len) {
    const char *p = datos;
    size_t enviados = 0;
    while (enviados < len) {
        ssize_t n = send(fd, p + enviados, len - enviados, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR) continue;   // una señal interrumpió: reintentar
            return -1;
        }
        enviados += (size_t)n;
    }
    return 0;
}

static inline int recibir_todo(int fd, void *datos, size_t len) {
    char *p = datos;
    size_t recibidos = 0;
    while (recibidos < len) {
        ssize_t n = recv(fd, p + recibidos, len - recibidos, 0);
        if (n < 0) {
            if (errno == EINTR) continue;   // una señal interrumpió: reintentar
            return -1;
        }
        if (n == 0) return 0;
        recibidos += (size_t)n;
    }
    return 1;
}

static inline int enviar_i64(int fd, int64_t valor) {
    uint64_t red = htobe64((uint64_t)valor);
    return enviar_todo(fd, &red, sizeof(red));
}

static inline int recibir_i64(int fd, int64_t *valor) {
    uint64_t red;
    int r = recibir_todo(fd, &red, sizeof(red));
    if (r == 1) *valor = (int64_t)be64toh(red);
    return r;
}

static inline int enviar_double(int fd, double valor) {
    uint64_t bits;
    memcpy(&bits, &valor, sizeof(bits));
    bits = htobe64(bits);
    return enviar_todo(fd, &bits, sizeof(bits));
}

static inline int recibir_double(int fd, double *valor) {
    uint64_t bits;
    int r = recibir_todo(fd, &bits, sizeof(bits));
    if (r == 1) {
        bits = be64toh(bits);
        memcpy(valor, &bits, sizeof(bits));
    }
    return r;
}

#endif