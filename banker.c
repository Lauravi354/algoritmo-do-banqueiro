#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int lerQuantidade(FILE *arquivo, int caractere, int *valor, int *separador){
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

int pularEspacos(FILE *arquivo, int caractere)
{
    while (caractere == ' ' || caractere == '\t') {
        caractere = fgetc(arquivo);
    }

    return caractere;
}

int lerComando(FILE *arquivo, char *comando, int *cliente, int *quantidades, int tiposDeRecurso, size_t quantidadeClientes){
    int caractere = fgetc(arquivo);

    if (caractere == EOF) {
        if (feof(arquivo) != 0) {
            return 0;
        }

        return -1;
    }

    caractere = pularEspacos(arquivo, caractere);

    if (caractere == '*') {
        *comando = '*';
        caractere = fgetc(arquivo);
    } else {
        if (caractere != 'R') {
            return -1;
        }

        int letra = fgetc(arquivo);

        if (letra != 'Q' && letra != 'L') {
            return -1;
        }

        *comando = letra;
        caractere = fgetc(arquivo);

        for (int i = 0; i <= tiposDeRecurso; i++) {
            if (caractere != ' ' && caractere != '\t') {
                return -1;
            }

            caractere = pularEspacos(arquivo, caractere);

            if (caractere < '0' || caractere > '9') {
                return -1;
            }

            int valor;
            if (lerQuantidade(arquivo, caractere, &valor, &caractere) == 0) {
                return -1;
            }

            if (i == 0) {
                if ((size_t)valor >= quantidadeClientes) {
                    return -1;
                }
                *cliente = valor;
            } else {
                quantidades[i - 1] = valor;
            }
        }
    }

    caractere = pularEspacos(arquivo, caractere);

    if (caractere == '\r') {
        caractere = fgetc(arquivo);
        if (caractere != '\n') {
            return -1;
        }
    }

    if (caractere == '\n' || (caractere == EOF && feof(arquivo) != 0)) {
        return 1;
    }

    return -1;
}

void escreverEstado(FILE *resultado, int *maximum, int *allocation, int *need, int *available, int tiposDeRecurso, size_t quantidadeClientes) {
    fprintf(resultado, "MAXIMUM   |   ALLOCATION  |   NEED\n");

    for (size_t cliente = 0; cliente < quantidadeClientes; cliente++) {
        size_t inicio = cliente * tiposDeRecurso;

        for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
            fprintf(resultado, "%d ", maximum[inicio + recurso]);
        }

        fprintf(resultado, "    |   ");

        for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
            fprintf(resultado, "%d ", allocation[inicio + recurso]);
        }

        fprintf(resultado, "      |   ");

        for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
            if (recurso > 0) {
                fprintf(resultado, " ");
            }
            fprintf(resultado, "%d", need[inicio + recurso]);
        }

        fprintf(resultado, "\n");
    }

    fprintf(resultado, "AVAILABLE");
    for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
        fprintf(resultado, " %d", available[recurso]);
    }
    fprintf(resultado, "\n");
}

int estadoSeguro(int *available, int *allocation, int *need, int tiposDeRecurso, size_t quantidadeClientes){
    int *recursosLivres = calloc(tiposDeRecurso, sizeof(int));
    int *terminou = calloc(quantidadeClientes, sizeof(int));

    if (recursosLivres == NULL || terminou == NULL) {
        free(recursosLivres);
        free(terminou);
        return -1;
    }

    for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
        recursosLivres[recurso] = available[recurso];
    }

    size_t clientesTerminados = 0;

    while (clientesTerminados < quantidadeClientes) {
        int houveProgresso = 0;

        for (size_t cliente = 0; cliente < quantidadeClientes; cliente++) {
            if (terminou[cliente] == 1) {
                continue;
            }

            size_t inicio = cliente * tiposDeRecurso;
            int podeTerminar = 1;

            for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
                if (need[inicio + recurso] > recursosLivres[recurso]) {
                    podeTerminar = 0;
                    break;
                }
            }

            if (podeTerminar == 1) {
                for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
                    recursosLivres[recurso] += allocation[inicio + recurso];
                }

                terminou[cliente] = 1;
                clientesTerminados++;
                houveProgresso = 1;
            }
        }

        if (houveProgresso == 0) {
            break;
        }
    }

    free(recursosLivres);
    free(terminou);

    if (clientesTerminados == quantidadeClientes) {
        return 1;
    }

    return 0;
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

        for (int j = 0; texto[j] != '\0'; j++) {
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
            int primeiroCaractere = fgetc(arquivo);
            int numeroValido = lerQuantidade(arquivo, primeiroCaractere,
                                             &maximum[posicao], &separador);

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

    int *allocation = calloc(quantidadeClientes, tamanhoLinha);
    int *need = calloc(quantidadeClientes, tamanhoLinha);

    if (allocation == NULL || need == NULL) {
        fprintf(stderr, "Erro ao reservar memória para as matrizes.\n");
        free(need);
        free(allocation);
        free(maximum);
        free(available);
        return 1;
    }

    for (size_t cliente = 0; cliente < quantidadeClientes; cliente++) {
        for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
            size_t posicao = cliente * tiposDeRecurso + recurso;
            need[posicao] = maximum[posicao];
        }
    }

        FILE *arquivoComandos = fopen("commands.txt", "r");
    int *quantidades = calloc(tiposDeRecurso, sizeof(int));

    if (arquivoComandos == NULL || quantidades == NULL) {
        fprintf(stderr, "Não foi possível preparar a leitura de commands.txt.\n");
        if (arquivoComandos != NULL) {
            fclose(arquivoComandos);
        }
        free(quantidades);
        free(need);
        free(allocation);
        free(maximum);
        free(available);
        return 1;
    }

    FILE *resultado = fopen("result.txt", "w");

    if (resultado == NULL) {
        printf("Não foi possível criar result.txt.\n");
        fclose(arquivoComandos);
        free(quantidades);
        free(need);
        free(allocation);
        free(maximum);
        free(available);
        return 1;
    }

    char comando;
    int clienteComando;
    int resultadoLeitura;
    int linha = 1;

    while ((resultadoLeitura = lerComando(arquivoComandos, &comando, &clienteComando, quantidades, tiposDeRecurso, quantidadeClientes)) == 1) {

        if (comando == '*') {
            escreverEstado(resultado, maximum, allocation, need, available, tiposDeRecurso, quantidadeClientes);
        } else if (comando == 'L') {
            size_t inicio = (size_t)clienteComando * tiposDeRecurso;
            int devolucaoValida = 1;

            for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
                size_t posicao = inicio + recurso;

                if (quantidades[recurso] > allocation[posicao]) {
                    devolucaoValida = 0;
                    break;
                }
            }

            if (devolucaoValida == 1) {
                for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
                    size_t posicao = inicio + recurso;
                    int quantidade = quantidades[recurso];

                    available[recurso] += quantidade;
                    allocation[posicao] -= quantidade;
                    need[posicao] += quantidade;
                }

                fprintf(resultado, "Release from customer %d the resources",
                        clienteComando);
                for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
                    fprintf(resultado, " %d", quantidades[recurso]);
                }
                fprintf(resultado, "\n");
            } else {
                fprintf(resultado, "Customer %d cannot release more resources than allocated\n", clienteComando);
            }
        }

            else if (comando == 'Q') {
            size_t inicio = (size_t)clienteComando * tiposDeRecurso;
            int dentroDoLimite = 1;
            int temRecursos = 1;

            for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
                if (quantidades[recurso] > need[inicio + recurso]) {
                    dentroDoLimite = 0;
                }

                if (quantidades[recurso] > available[recurso]) {
                    temRecursos = 0;
                }
            }

            if (dentroDoLimite == 0) {
                fprintf(resultado, "The customer %d request", clienteComando);

                for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
                    fprintf(resultado, " %d", quantidades[recurso]);
                }

                fprintf(resultado, " was denied because exceed its maximum allocation\n");
            } else if (temRecursos == 0) {
                fprintf(resultado, "The resources");

                for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
                    fprintf(resultado, " %d", available[recurso]);
                }

                fprintf(resultado, " are not enough to customer %d request", clienteComando);

                for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
                    fprintf(resultado, " %d", quantidades[recurso]);
                }

                fprintf(resultado, "\n");
            } else {
                for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
                    size_t posicao = inicio + recurso;
                    int quantidade = quantidades[recurso];

                    available[recurso] -= quantidade;
                    allocation[posicao] += quantidade;
                    need[posicao] -= quantidade;
                }

                int seguro = estadoSeguro(available, allocation, need,
                                          tiposDeRecurso, quantidadeClientes);

                if (seguro != 1) {
                    for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
                        size_t posicao = inicio + recurso;
                        int quantidade = quantidades[recurso];

                        available[recurso] += quantidade;
                        allocation[posicao] -= quantidade;
                        need[posicao] += quantidade;
                    }
                }

                if (seguro == -1) {
                    printf("Não foi possível verificar o estado seguro.\n");
                    fclose(resultado);
                    fclose(arquivoComandos);
                    free(quantidades);
                    free(need);
                    free(allocation);
                    free(maximum);
                    free(available);
                    return 1;
                }

                if (seguro == 1) {
                    fprintf(resultado, "Allocate to customer %d the resources",
                            clienteComando);
                } else {
                    fprintf(resultado, "The customer %d request", clienteComando);
                }

                for (int recurso = 0; recurso < tiposDeRecurso; recurso++) {
                    fprintf(resultado, " %d", quantidades[recurso]);
                }

                if (seguro == 1) {
                    fprintf(resultado, "\n");
                } else {
                    fprintf(resultado, " was denied because result in an unsafe state\n");
                }
            }
        }

        linha++;
    }

    fclose(resultado);
    fclose(arquivoComandos);
    free(quantidades);

    if (resultadoLeitura == -1) {
        printf("Erro ao ler a linha %d de commands.txt.\n", linha);
        free(need);
        free(allocation);
        free(maximum);
        free(available);
        return 1;
    }
    
    printf("AVAILABLE");
    for (int i = 0; i < tiposDeRecurso; i++) {
        printf(" %d", available[i]);
    }
    printf("\n");

    free(need);
    free(allocation);
    free(maximum);
    free(available);
    return 0;
}