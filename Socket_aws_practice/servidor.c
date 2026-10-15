#include <stdio.h>       // printf, fprintf, perror
#include <stdlib.h>      // exit, EXIT_FAILURE, strtoll, atoi
#include <string.h>      // memset
#include <unistd.h>      // close
#include <math.h>        // M_PI, fabs
#include <arpa/inet.h>   // sockaddr_in, htons, htonl, ntohs, inet_ntop
#include <sys/socket.h>  // socket, setsockopt, bind, listen, accept
#include "protocolo.h"
#include "serie.h"

int main(int argc, char *argv[]) {
    // 0. Validar argumentos ANTES de pedir recursos al kernel
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Uso: %s <num_terminos> [puerto]\n", argv[0]);
        return 1;
    }
    char *resto;
    int64_t n = strtoll(argv[1], &resto, 10);
    if (*resto != '\0' || n <= 0) {
        fprintf(stderr, "num_terminos invalido: %s\n", argv[1]);
        return 1;
    }
    int puerto = (argc == 3) ? atoi(argv[2]) : PUERTO_DEFECTO;

    // 1-5. Igual que en el paso 2
    int fd_escucha = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_escucha < 0) { perror("socket"); exit(EXIT_FAILURE); }

    int si = 1;
    if (setsockopt(fd_escucha, SOL_SOCKET, SO_REUSEADDR, &si, sizeof(si)) < 0) {
        perror("setsockopt"); exit(EXIT_FAILURE);
    }

    struct sockaddr_in dir;
    memset(&dir, 0, sizeof(dir));
    dir.sin_family = AF_INET;
    dir.sin_addr.s_addr = htonl(INADDR_ANY);
    dir.sin_port = htons(puerto);

    if (bind(fd_escucha, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
        perror("bind"); exit(EXIT_FAILURE);
    }
    if (listen(fd_escucha, 1) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }
    printf("Coordinador escuchando en el puerto %d (N = %lld)\n", puerto, (long long)n);

    // 6. Esperar al trabajador
    struct sockaddr_in dir_cliente;
    socklen_t tam = sizeof(dir_cliente);
    int fd_cliente = accept(fd_escucha, (struct sockaddr *)&dir_cliente, &tam);
    if (fd_cliente < 0) { perror("accept"); exit(EXIT_FAILURE); }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &dir_cliente.sin_addr, ip, sizeof(ip));
    printf("Trabajador conectado desde %s:%d\n", ip, ntohs(dir_cliente.sin_port));

    // 7. Repartir: el trabajador se queda con [mitad, n)
    double t_inicio = segundos_ahora();
    int64_t mitad = n / 2;
    if (enviar_i64(fd_cliente, mitad) < 0 || enviar_i64(fd_cliente, n) < 0) {
        perror("send"); exit(EXIT_FAILURE);
    }

    // 8. Calcular la mitad propia MIENTRAS el trabajador calcula la suya
    double t_calc = segundos_ahora();
    double suma_a = suma_parcial(0, mitad);
    double t_espera = segundos_ahora();

    // 9. Esperar el resultado del trabajador
    double suma_b;
    int r = recibir_double(fd_cliente, &suma_b);
    if (r < 0) { perror("recv"); exit(EXIT_FAILURE); }
    if (r == 0) {
        fprintf(stderr, "El trabajador cerro la conexion sin enviar su resultado\n");
        exit(EXIT_FAILURE);
    }
    double t_fin = segundos_ahora();

    if (!isfinite(suma_b) || fabs(suma_b) > 1.0) {
        fprintf(stderr, "Suma invalida recibida del trabajador: %g\n", suma_b);
        exit(EXIT_FAILURE);
    }

    // 10. Combinar y reportar
    double pi = 4.0 * (suma_a + suma_b);
    printf("pi    ~= %.15f\n", pi);
    printf("error  = %.3e\n", fabs(pi - M_PI));
    printf("calculo propio: %.3f s | espera al trabajador: %.3f s | total: %.3f s\n",
           t_espera - t_calc, t_fin - t_espera, t_fin - t_inicio);

    close(fd_cliente);
    close(fd_escucha);
    return 0;
}