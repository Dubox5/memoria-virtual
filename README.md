# Memoria Virtual
Eduardo Boxwell Simões - ebs2@cesar.school

Como compilar e rodar:
Precisa de gcc e make (testei no Ubuntu pelo WSL).

```
make
./vm addresses.txt lru fifo
```

O primeiro argumento é o arquivo de enderecos, o segundo é o algoritmo de substituicao das paginas (`fifo` ou `lru`) e o terceiro o da TLB (`fifo` ou `lru`). O `BACKING_STORE.bin` precisa estar na mesma pasta. A saida vai para o `correct.txt`. Pra apagar o executavel: `make clean`

Como funciona:
- Enderecos de 16 bits: pagina = 8 bits de cima, deslocamento = 8 bits de baixo
- 256 paginas de 256 bytes e memoria fisica com 128 frames, preenchidos do 0 ao 127
- Quando a memoria enche, a pagina que sai é escolhida por fifo ou lru
- TLB com 16 entradas, substituida por fifo ou lru
- A busca na TLB usa uma thread para cada entrada (16 threads)
- No fnal o `correct.txt` mostra o total de enderecos, as faltas de pagina e os acertos na TLB

Erros:
O programa confere tudo antes de comecar. Se tiver algo errado ele mostra uma mensagem de erro e fecha sem criar o correct.txt. Alguns dos casos que ele pega são:
- numero de argumentos errado
- algoritmo que nao é fifo nem lru
- arquivo de enderecos que nao existe ou nao abre
- arquivo vazio, letra, numero negativo ou acima de 65535, linha vazia no meio
- BACKING_STORE.bin que nao existe ou é menor que 65536 bytes

Evidencias:
O `evidencias.log` tem a saida dos testes que eu fiz: as 4 combinacoes de algoritmo e varios casos de erro