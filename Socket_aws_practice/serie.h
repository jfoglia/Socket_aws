#ifndef SERIE_H
#define SERIE_H

#include <stdint.h>   // int64_t
#include <time.h>     // clock_gettime, struct timespec

// Suma de Leibniz para k en [inicio, fin). NO multiplica por 4.
static inline double suma_parcial(int64_t inicio, int64_t fin) {
    double suma = 0.0;
    for (int64_t k = inicio; k < fin; k++) {
        double signo = (k % 2 == 0) ? 1.0 : -1.0;
        suma += signo / (2.0 * k + 1.0);
    }
    return suma;
}

// Segundos transcurridos desde un punto fijo, con resolución de nanosegundos.
static inline double segundos_ahora(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

#endif