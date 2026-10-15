#include <stdio.h>       // printf, fprintf, perror
#include <stdlib.h>      // exit, EXIT_FAILURE, atoi
#include <string.h>      // memset
#include <unistd.h>      // close
#include <arpa/inet.h>   // sockaddr_in, htons, inet_pton
#include <sys/socket.h>  // socket, connect
#include "protocolo.h"
#include "serie.h"

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Uso: %s <ip_servidor> [puerto]\n", argv[0]);
        return 1;
    }

    // 1. Construir y validar la dirección ANTES de crear el socket
    struct sockaddr_in dir;
    memset(&dir, 0, sizeof(dir));
    dir.sin_family = AF_INET;
    dir.sin_port = htons((argc == 3) ? atoi(argv[2]) : PUERTO_DEFECTO);
    if (inet_pton(AF_INET, argv[1], &dir.sin_addr) != 1) {
        fprintf(stderr, "IP invalida: %s\n", argv[1]);
        return 1;
    }

    // 2. Crear el socket y conectarse
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); exit(EXIT_FAILURE); }
    if (connect(fd, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
        perror("connect"); exit(EXIT_FAILURE);
    }
    printf("Conectado al coordinador %s\n", argv[1]);

    // 3. Recibir el rango asignado
    int64_t inicio, fin;
    if (recibir_i64(fd, &inicio) != 1 || recibir_i64(fd, &fin) != 1) {
        fprintf(stderr, "No se pudo recibir el rango completo\n");
        exit(EXIT_FAILURE);
    }

    if (inicio < 0 || fin <= inicio) {
        fprintf(stderr, "Rango invalido recibido: [%lld, %lld)\n",
            (long long)inicio, (long long)fin);
        exit(EXIT_FAILURE);
    }
    printf("Rango asignado: [%lld, %lld)\n", (long long)inicio, (long long)fin);

    // 4. Calcular la suma parcial
    double t0 = segundos_ahora();
    double suma = suma_parcial(inicio, fin);
    double t1 = segundos_ahora();
    printf("Suma parcial = %.17g (%.3f s)\n", suma, t1 - t0);

    // 5. Devolver el resultado y cerrar
    if (enviar_double(fd, suma) < 0) {
        perror("send"); exit(EXIT_FAILURE);
    }
    close(fd);
    return 0;
}