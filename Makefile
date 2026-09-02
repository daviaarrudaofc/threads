CC = gcc
CFLAGS = -Wall -Wextra -g -fopenmp

all: mandelbrot

mandelbrot: mandelbrot.c
	$(CC) $(CFLAGS) mandelbrot.c -o mandelbrot

test: mandelbrot
	./mandelbrot 120 80 100 4
	diff mandelbrot_davia_serial.pgm mandelbrot_davia_openmp.pgm

clean:
	rm -f mandelbrot mandelbrot_davia_serial.pgm mandelbrot_davia_openmp.pgm times.txt
