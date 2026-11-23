#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TAM_PAGINA 256
#define NUM_PAGINAS 256
#define NUM_FRAMES 128
#define TAM_LINHA 256

int tabela_paginas[NUM_PAGINAS];
signed char memoria[NUM_FRAMES][TAM_PAGINA];
int pagina_do_frame[NUM_FRAMES];
int uso_frame[NUM_FRAMES];
int frames_usados = 0;
int proximo_fifo_frame = 0;

int alg_pagina;
int alg_tlb;
int tempo = 0;
int *enderecos = NULL;
int num_enderecos = 0;

FILE *backing;
void erro(char *msg, int linha) {
    if (linha > 0)
        fprintf(stderr, "Erro: %s (linha %d)\n", msg, linha);
    else
        fprintf(stderr, "Erro: %s\n", msg);
    exit(1);
}

int le_algoritmo(char *s) {
    if (strcmp(s, "fifo") == 0) return 0;
    if (strcmp(s, "lru") == 0) return 1;
    fprintf(stderr, "Erro: algoritmo '%s' invalido, use fifo ou lru\n", s);
    exit(1);
}

void limpa_fim(char *s) {
    int n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r' || s[n-1] == ' ' || s[n-1] == '\t')) {
        s[n-1] = '\0';
        n--;
    }
}

void le_enderecos(char *nome) {
    FILE *f = fopen(nome, "r");
    if (f == NULL) {
        fprintf(stderr, "Erro: nao consegui abrir o arquivo '%s'\n", nome);
        exit(1);
    }

    char linha[TAM_LINHA];
    int nlinha = 0;
    int teve_vazia = 0;
    while (fgets(linha, TAM_LINHA, f) != NULL) {
        nlinha++;
        limpa_fim(linha);

        if (linha[0] == '\0') {
            teve_vazia = 1;
            continue;
        }
        if (teve_vazia) erro("linha vazia no meio do arquivo de enderecos", nlinha - 1);

        int n = strlen(linha);
        if (n > 5) erro("endereco invalido, tem que ser um numero de 0 a 65535", nlinha);
        for (int i = 0; i < n; i++) {
            if (linha[i] < '0' || linha[i] > '9')
                erro("endereco invalido, tem que ser um numero de 0 a 65535", nlinha);
        }
        int valor = atoi(linha);
        if (valor > 65535) erro("endereco invalido, tem que ser um numero de 0 a 65535", nlinha);

        enderecos = realloc(enderecos, (num_enderecos + 1) * sizeof(int));
        enderecos[num_enderecos] = valor;
        num_enderecos++;
    }
    fclose(f);

    if (num_enderecos == 0) erro("arquivo de enderecos vazio", 0);
}

int escolhe_frame() {
    if (frames_usados < NUM_FRAMES) {
        frames_usados++;
        return frames_usados - 1;
    }

    int f;
    if (alg_pagina == 0) {
        f = proximo_fifo_frame;
        proximo_fifo_frame = (proximo_fifo_frame + 1) % NUM_FRAMES;
    } else {
        f = 0;
        for (int i = 1; i < NUM_FRAMES; i++)
            if (uso_frame[i] < uso_frame[f]) f = i;
    }

    int velha = pagina_do_frame[f];
    tabela_paginas[velha] = -1;

    return f;
}

int carrega_pagina(int pagina) {
    int f = escolhe_frame();

    if (fseek(backing, pagina * TAM_PAGINA, SEEK_SET) != 0 ||
        fread(memoria[f], 1, TAM_PAGINA, backing) != TAM_PAGINA)
        erro("nao consegui ler a pagina do BACKING_STORE.bin", 0);

    tabela_paginas[pagina] = f;
    pagina_do_frame[f] = pagina;
    return f;
}


int main(int argc, char *argv[]) {
    if (argc != 4)
        erro("uso: ./vm <arquivo de enderecos> <fifo|lru> <fifo|lru>", 0);

    alg_pagina = le_algoritmo(argv[2]);
    alg_tlb = le_algoritmo(argv[3]);
    le_enderecos(argv[1]);

    backing = fopen("BACKING_STORE.bin", "rb");
    if (backing == NULL) erro("nao consegui abrir o BACKING_STORE.bin", 0);
    fseek(backing, 0, SEEK_END);
    if (ftell(backing) < NUM_PAGINAS * TAM_PAGINA)
        erro("BACKING_STORE.bin menor que 65536 bytes", 0);

    for (int i = 0; i < NUM_PAGINAS; i++) tabela_paginas[i] = -1;

    FILE *out = fopen("correct.txt", "w");
    if (out == NULL) erro("nao consegui criar o correct.txt", 0);

    int faltas = 0;
    int acertos_tlb = 0;

    for (int k = 0; k < num_enderecos; k++) {
        tempo++;
        int endereco = enderecos[k];
        int pagina = (endereco >> 8) & 0xFF;
        int deslocamento = endereco & 0xFF;
        int frame;

        int idx = -1;
        frame = tabela_paginas[pagina];
        if (frame == -1) {
            faltas++;
            frame = carrega_pagina(pagina);
        }

        uso_frame[frame] = tempo;

        int fisico = frame * TAM_PAGINA + deslocamento;
        int valor = memoria[frame][deslocamento];
        fprintf(out, "Virtual address: %d TLB: %d Physical address: %d Value: %d\n",
                endereco, idx, fisico, valor);
    }

    fprintf(out, "Number of Translated Addresses = %d\n", num_enderecos);
    fprintf(out, "Page Faults = %d\n", faltas);
    fprintf(out, "Page Fault Rate = %.3f\n", (double) faltas / num_enderecos);
    fprintf(out, "TLB Hits = %d\n", acertos_tlb);
    fprintf(out, "TLB Hit Rate = %.3f\n", (double) acertos_tlb / num_enderecos);

    fclose(out);
    fclose(backing);
    return 0;
}
