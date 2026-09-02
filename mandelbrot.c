#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>

typedef struct {
    int *imagem;
    int largura;
    int altura;
    int max_iteracoes;
    int linha_inicio;
    int linha_fim;
} DadosThread;

int converter_numero(char texto[]) {
    int i;
    int numero = 0;

    for (i = 0; texto[i] != '\0'; i++) {
        if (texto[i] < '0' || texto[i] > '9') {
            return -1;
        }

        numero = numero * 10 + (texto[i] - '0');
    }

    return numero;
}

int calcular_ponto(double c_real, double c_imag, int max_iteracoes) {
    double z_real = 0.0;
    double z_imag = 0.0;
    int i;

    for (i = 0; i < max_iteracoes; i++) {
        double novo_real = z_real * z_real - z_imag * z_imag + c_real;
        double novo_imag = 2.0 * z_real * z_imag + c_imag;

        z_real = novo_real;
        z_imag = novo_imag;

        if (z_real * z_real + z_imag * z_imag > 4.0) {
            break;
        }
    }

    return (i * 255) / max_iteracoes;
}

void calcular_imagem(int *imagem, int largura, int altura, int max_iteracoes) {
    int x;
    int y;

    for (y = 0; y < altura; y++) {
        for (x = 0; x < largura; x++) {
            double real = -2.0 + (3.0 * x) / (largura - 1);
            double imag = -1.5 + (3.0 * y) / (altura - 1);

            imagem[y * largura + x] = calcular_ponto(real, imag, max_iteracoes);
        }
    }
}

void calcular_imagem_openmp(int *imagem, int largura, int altura, int max_iteracoes, int num_threads) {
    int x;
    int y;

    omp_set_num_threads(num_threads);

#pragma omp parallel for private(x)
    for (y = 0; y < altura; y++) {
        for (x = 0; x < largura; x++) {
            double real = -2.0 + (3.0 * x) / (largura - 1);
            double imag = -1.5 + (3.0 * y) / (altura - 1);

            imagem[y * largura + x] = calcular_ponto(real, imag, max_iteracoes);
        }
    }
}

void *calcular_parte_pthreads(void *arg) {
    DadosThread *dados = (DadosThread *)arg;
    int x;
    int y;

    for (y = dados->linha_inicio; y < dados->linha_fim; y++) {
        for (x = 0; x < dados->largura; x++) {
            double real = -2.0 + (3.0 * x) / (dados->largura - 1);
            double imag = -1.5 + (3.0 * y) / (dados->altura - 1);

            dados->imagem[y * dados->largura + x] = calcular_ponto(real, imag, dados->max_iteracoes);
        }
    }

    return NULL;
}

int calcular_imagem_pthreads1(int *imagem, int largura, int altura, int max_iteracoes, int num_threads) {
    pthread_t *threads;
    DadosThread *dados;
    int i;
    int linhas_por_thread;
    int linha_atual = 0;

    threads = malloc(num_threads * sizeof(pthread_t));
    dados = malloc(num_threads * sizeof(DadosThread));

    if (threads == NULL || dados == NULL) {
        fprintf(stderr, "Erro ao alocar memoria para threads.\n");
        free(threads);
        free(dados);
        return 0;
    }

    linhas_por_thread = altura / num_threads;

    for (i = 0; i < num_threads; i++) {
        dados[i].imagem = imagem;
        dados[i].largura = largura;
        dados[i].altura = altura;
        dados[i].max_iteracoes = max_iteracoes;
        dados[i].linha_inicio = linha_atual;

        if (i == num_threads - 1) {
            dados[i].linha_fim = altura;
        } else {
            dados[i].linha_fim = linha_atual + linhas_por_thread;
        }

        linha_atual = dados[i].linha_fim;

        if (pthread_create(&threads[i], NULL, calcular_parte_pthreads, &dados[i]) != 0) {
            fprintf(stderr, "Erro ao criar thread.\n");
            free(threads);
            free(dados);
            return 0;
        }
    }

    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(dados);
    return 1;
}

int salvar_imagem(char nome_arquivo[], int *imagem, int largura, int altura) {
    FILE *arquivo;
    int x;
    int y;

    arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao criar arquivo de saida.\n");
        return 0;
    }

    for (y = 0; y < altura; y++) {
        for (x = 0; x < largura; x++) {
            fprintf(arquivo, "%d", imagem[y * largura + x]);

            if (x < largura - 1) {
                fprintf(arquivo, " ");
            }
        }
        fprintf(arquivo, "\n");
    }

    fclose(arquivo);
    return 1;
}

int main(int argc, char *argv[]) {
    int largura;
    int altura;
    int max_iteracoes;
    int num_threads;
    int *imagem;
    FILE *tempos;
    clock_t inicio;
    clock_t fim;
    double tempo_serial;
    double tempo_openmp;
    double tempo_pthreads1;

    if (argc != 5) {
        fprintf(stderr, "Uso: ./mandelbrot largura altura max_iteracoes num_threads\n");
        return 1;
    }

    largura = converter_numero(argv[1]);
    altura = converter_numero(argv[2]);
    max_iteracoes = converter_numero(argv[3]);
    num_threads = converter_numero(argv[4]);

    if (largura < 2 || altura < 2 || max_iteracoes <= 0 || num_threads <= 0) {
        fprintf(stderr, "Erro: todos os parametros devem ser validos.\n");
        return 1;
    }

    imagem = malloc(largura * altura * sizeof(int));
    if (imagem == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        return 1;
    }

    inicio = clock();
    calcular_imagem(imagem, largura, altura, max_iteracoes);
    fim = clock();
    tempo_serial = (double)(fim - inicio) / CLOCKS_PER_SEC;

    if (!salvar_imagem("mandelbrot_davia_serial.pgm", imagem, largura, altura)) {
        free(imagem);
        return 1;
    }

    inicio = clock();
    calcular_imagem_openmp(imagem, largura, altura, max_iteracoes, num_threads);
    fim = clock();
    tempo_openmp = (double)(fim - inicio) / CLOCKS_PER_SEC;

    if (!salvar_imagem("mandelbrot_davia_openmp.pgm", imagem, largura, altura)) {
        free(imagem);
        return 1;
    }

    inicio = clock();
    if (!calcular_imagem_pthreads1(imagem, largura, altura, max_iteracoes, num_threads)) {
        free(imagem);
        return 1;
    }
    fim = clock();
    tempo_pthreads1 = (double)(fim - inicio) / CLOCKS_PER_SEC;

    if (!salvar_imagem("mandelbrot_davia_pthreads1.pgm", imagem, largura, altura)) {
        free(imagem);
        return 1;
    }

    tempos = fopen("times.txt", "w");
    if (tempos == NULL) {
        fprintf(stderr, "Erro ao criar times.txt.\n");
        free(imagem);
        return 1;
    }

    fprintf(tempos, "serial %.6f\n", tempo_serial);
    fprintf(tempos, "openmp %.6f\n", tempo_openmp);
    fprintf(tempos, "pthreads1 %.6f\n", tempo_pthreads1);
    fclose(tempos);

    free(imagem);
    return 0;
}
