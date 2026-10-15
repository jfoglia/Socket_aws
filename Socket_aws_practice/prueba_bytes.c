#include <stdio.h>
#include "protocolo.h"

static void mostrar(const char *etiqueta, const void *ptr) {
    const unsigned char *b = ptr;
    printf("%-22s", etiqueta);
    for (int i = 0; i < 8; i++) printf("%02x ", b[i]);
    printf("\n");
}

int main(void) {
    int64_t x = 1000000000;                 // 0x3B9ACA00
    uint64_t x_red = htobe64((uint64_t)x);
    mostrar("int64 en memoria:", &x);
    mostrar("int64 para la red:", &x_red);

    double d = 0.7853981633974483;          // aprox. pi/4
    uint64_t bits;
    memcpy(&bits, &d, sizeof(bits));
    uint64_t bits_red = htobe64(bits);
    mostrar("double en memoria:", &d);
    mostrar("double para la red:", &bits_red);
    return 0;
}