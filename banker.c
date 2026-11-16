#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int lerQuantidade(FILE *arquivo, int *valor, int *separador){
    int caractere = fgetc(arquivo);
    int numero = 0;

    if (caractere < '0' || caractere > '9') {
        return 0;
    }

    while (caractere >= '0' && caractere <= '9') {
        int digito = caractere - '0';

        if (numero > (INT_MAX - digito) / 10) {
            return 0;
        }

        numero = numero * 10 + digito;
        caractere = fgetc(arquivo);
    }

    *valor = numero;
    *separador = caractere;
    return 1;
}

int main(int argc, char *argv[]) {

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
        int quantidade = 0;
        int valido = texto[0] != '\0';

        for (size_t j = 0; texto[j] != '\0'; j++) {
            if (texto[j] < '0' || texto[j] > '9') {
                valido = 0;
                break;
            }

            int digito = texto[j] - '0';

            if (quantidade > (INT_MAX - digito) / 10) {
                valido = 0;
                break;
            }

            quantidade = quantidade * 10 + digito;
        }

        if (valido == 0) {
            fprintf(stderr, "Quantidade inválida: %s\n", texto);
            free(available);
            return 1;
        }

        available[i] = quantidade;
    }

    FILE *arquivo = fopen("customer.txt", "r");

    if (arquivo == NULL) {
        fprintf(stderr, "Não foi possível abrir customer.txt.\n");
        free(available);
        return 1;
    }

    size_t quantidadeClientes = 0;
    int caractere;
    int ultimoCaractere = '\n';

    while ((caractere = fgetc(arquivo)) != EOF) {
        if (caractere == '\n') {
            quantidadeClientes++;
        }

        ultimoCaractere = caractere;
    }

    if (feof(arquivo) == 0) {
        fprintf(stderr, "Erro ao ler customer.txt.\n");
        fclose(arquivo);
        free(available);
        return 1;
    }

    if (ultimoCaractere != '\n') {
        quantidadeClientes++;
    }

    if (quantidadeClientes == 0) {
        fprintf(stderr, "O arquivo customer.txt está vazio.\n");
        fclose(arquivo);
        free(available);
        return 1;
    }

    fclose(arquivo);
    arquivo = fopen("customer.txt", "r");

    if (arquivo == NULL) {
        fprintf(stderr, "Não foi possível reabrir customer.txt.\n");
        free(available);
        return 1;
    }

    size_t tamanhoLinha = (size_t)tiposDeRecurso * sizeof(int);
    int *maximum = calloc(quantidadeClientes, tamanhoLinha);

    if (maximum == NULL) {
        fprintf(stderr, "Erro ao reservar memória para maximum.\n");
        fclose(arquivo);
        free(available);
        return 1;
    }

    for (size_t cliente = 0; cliente < quantidadeClientes; cliente++) {
        for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
            size_t posicao = cliente * tiposDeRecurso + recurso;
            int separador = 0;
            int numeroValido = lerQuantidade(arquivo, &maximum[posicao], &separador);

            int separadorValido;
            if (recurso == tiposDeRecurso - 1) {
                separadorValido = separador == '\n' ||
                    (cliente == quantidadeClientes - 1 &&
                     separador == EOF && feof(arquivo) != 0);
            } else {
                separadorValido = separador == ',';
            }

            if (numeroValido == 0 || separadorValido == 0) {
                fprintf(stderr, "Linha %zu inválida em customer.txt.\n",
                        cliente + 1);
                free(maximum);
                fclose(arquivo);
                free(available);
                return 1;
            }
        }
    }

    fclose(arquivo);

    printf("AVAILABLE");
    for (int i = 0; i < tiposDeRecurso; i++) {
        printf(" %d", available[i]);
    }
    printf("\n");

    free(available);
    return 0;
}