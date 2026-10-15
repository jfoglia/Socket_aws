#include <stdio.h>    // printf, fprintf
#include <stdlib.h>   // strtoll
#include <stdint.h>   // int64_t

double suma_parcial(int64_t inicio, int64_t fin) {
    double suma = 0.0;
    for (int64_t k = inicio; k < fin; k++) {
        double signo = (k % 2 == 0) ? 1.0 : -1.0;
        suma += signo / (2.0 * k + 1.0);
    }
    return suma;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <num_terminos>\n", argv[0]);
        return 1;
    }

    int64_t n = strtoll(argv[1], NULL, 10);
    int64_t mitad = n / 2;

    double suma_a = suma_parcial(0, mitad);   // lo que hará el servidor
    double suma_b = suma_parcial(mitad, n);   // lo que hará el cliente

    double pi = 4.0 * (suma_a + suma_b);
    printf("pi ~= %.15f\n", pi);
    return 0;
}