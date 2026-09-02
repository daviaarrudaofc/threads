#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

    if (argc != 5) {
        fprintf(stderr, "Uso: ./mandelbrot largura altura max_iteracoes num_threads\n");
        return 1;
    }

    largura = atoi(argv[1]);
    altura = atoi(argv[2]);
    max_iteracoes = atoi(argv[3]);
    num_threads = atoi(argv[4]);

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
    calcular_imagem(imagem, largura, altura, max_iteracoes);
    fim = clock();
    tempo_openmp = (double)(fim - inicio) / CLOCKS_PER_SEC;

    if (!salvar_imagem("mandelbrot_davia_openmp.pgm", imagem, largura, altura)) {
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
    fclose(tempos);

    free(imagem);
    return 0;
}
