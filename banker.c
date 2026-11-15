#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int tiposDeRecurso = argc - 1;

    if (tiposDeRecurso == 0) {
        fprintf(stderr, "Informe as quantidades. Exemplo: ./banker 10 5 7\n");
        return 1;
    }

    int *available = calloc(tiposDeRecurso, sizeof(int));

    if (available == NULL) {
        fprintf(stderr, "Erro ao reservar memória para os recursos.\n");
        return 1;
    }

    for (int i = 0; i < tiposDeRecurso; i++) {
        const char *texto = argv[i + 1];
        char *fim;

        errno = 0;
        long quantidade = strtol(texto, &fim, 10);

        int textoInvalido = (fim == texto || *fim != '\0');
        int quantidadeInvalida = (errno == ERANGE || quantidade < 0 || quantidade > INT_MAX);

        if (textoInvalido || quantidadeInvalida) {
            fprintf(stderr, "Quantidade inválida: %s\n", texto);
            free(available);
            return 1;
        }

        available[i] = (int)quantidade;
    }

    printf("AVAILABLE");
    for (int i = 0; i < tiposDeRecurso; i++) {
        printf(" %d", available[i]);
    }
    printf("\n");

    free(available);
    return 0;
}