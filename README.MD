# Algoritmo do Banqueiro

Implementação em C do algoritmo do banqueiro para evitar deadlocks na alocação de recursos. O programa concede um pedido quando há recursos suficientes, o pedido respeita o limite do cliente e existe uma ordem em que todos os clientes conseguem terminar.

## Compilação

No Linux ou no WSL, execute na pasta do projeto:

```sh
make
```

O comando gera o executável `banker`.

## Arquivos de entrada

- `customer.txt` contém uma linha por cliente, com as quantidades máximas de cada tipo de recurso separadas por vírgulas. Exemplo: `7,5,3`.
- `commands.txt` contém um comando por linha. Os clientes são numerados a partir de `0`.

| Comando | Exemplo | Ação |
| --- | --- | --- |
| `RQ` | `RQ 0 0 2 1` | O cliente 0 solicita 0, 2 e 1 unidades dos respectivos recursos. |
| `RL` | `RL 0 0 1 1` | O cliente 0 devolve 0, 1 e 1 unidades dos respectivos recursos. |
| `*` | `*` | Registra o estado atual dos recursos. |

## Execução

Coloque `customer.txt` e `commands.txt` na pasta do projeto e execute:

```sh
./banker 10 5 7
cat result.txt
```

Cada argumento indica a quantidade inicial disponível de um tipo de recurso. Nesse exemplo, são três tipos, com 10, 5 e 7 unidades. O programa grava em `result.txt` as decisões sobre pedidos e devoluções, além do estado dos recursos a cada comando `*`.

## Limpeza

Para remover o executável gerado pela compilação:

```sh
make clean
```