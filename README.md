# Implementacao 2 - Mandelbrot (metade)

Esta etapa implementa metade do enunciado: a base serial do calculo do conjunto de Mandelbrot e a estrutura para comparar com a segunda saida

## O que foi feito

- Validacao dos argumentos de entrada.
- Geracao da imagem serial em `mandelbrot_davia_serial.pgm`.
- Geracao de uma segunda saida em `mandelbrot_davia_openmp.pgm` para comparacao nesta etapa.
- Registro dos tempos em `times.txt`.
- Makefile simples para compilar e limpar arquivos gerados.
- Base pronta para gerar `evidencias.log` real durante uma sessao gravada no terminal.

## Como compilar

```sh
make
```

## Como executar

```sh
./mandelbrot 800 600 1000 4
```

No Windows com MinGW, o executavel pode ser chamado como:

```sh
./mandelbrot.exe 800 600 1000 4
```
