#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define PI 3.14159265358979323846
#define DOUBLE_PI PI * 2

/*****************************************************************/

/* Compartidas */
int NUM_THREADS, block_size_by_threads, N, bs;
double *A, *B, *C, *T, *R, *M, *R1, *R2, *RA, *RB, average1, average2;
pthread_mutex_t average_lock1, average_lock2;

/*****************************************************************/

/* Multiply (block)submatrices */
void blkmul(double *ablk, double *bblk, double *cblk, int n, int bs)
{
	int i, j, k;    /* Guess what... again... */

	for (i = 0; i < bs; i++)
	{
		for (j = 0; j < bs; j++)
		{
			for  (k = 0; k < bs; k++)
			{
				cblk[i*n + j] += ablk[i*n + k] * bblk[j*n + k];
			}
		}
	}
}

/* Multiply square matrices, blocked version */
void matmulblks(double *a, double *b, double *c, int n, int bs)
{
	int i, j, k;    /* Guess what... */

	for (i = 0; i < n; i += bs)
	{
		for (j = 0; j < n; j += bs)
		{
			for  (k = 0; k < n; k += bs)
			{
				blkmul(&a[i*n + k], &b[j*n + k], &c[i*n + j], n, bs);
			}
		}
	}
}

void calculate(void* id) {
	int start, i, j, k, f, c, h, offset_i, offset_j, offset_c, offset_f, mini_row_index;
	start = *((int *)id);
	start = block_size_by_threads * start;
	double num, aSin, aCos, *ablk, *bblk, *cblk;
	double localAverage1 = 0;
	double localAverage2 = 0;

	// Calcula R1 y R2 y sus promedios

	double mini_size = block_size_by_threads * block_size_by_threads;
	for (i = start; i < start + mini_size; i++) {
		num = (1 - T[i]);
		aSin = sin(M[i]);
		aCos = cos(M[i]);
		R1[i] = num * (1 - aCos) + (T[i] * aSin);
		R2[i] = num * (1 - aSin) + (T[i] * aCos);

		localAverage1 += R1[i];
		localAverage2 += R2[i];
	}

	pthread_mutex_lock(&average_lock1);
	average1 += localAverage1;
	pthread_mutex_unlock(&average_lock1);

	pthread_mutex_lock(&average_lock2);
	average2 += localAverage2;
	pthread_mutex_unlock(&average_lock2);

	// necesitamos una barrera aca?

	// Calcula RA -- tenemos que reemplazar todos los N por (start + block_size_by_threads) o solo el primero?
	for (i = start; i < start + block_size_by_threads; i += bs)
	{
		offset_i = i*N;
		for (j = 0; j < N; j += bs)
		{
			offset_j = j*N;
			cblk = &RA[offset_i + j];

			for  (k = 0; k < N; k += bs)
			{
				ablk = &R1[offset_i + k];
				bblk = &A[offset_j + k];

				for (f = 0; f < bs; f++)
				{
					offset_f = f * N;
					for (c = 0; c < bs; c++)
					{
						offset_c = c * N;
						mini_row_index = offset_f + c;

						for  (h = 0; h < bs; h++)
						{
							cblk[mini_row_index] += ablk[offset_f + h] * bblk[offset_c + h];
						}
					}
				}
			}
		}
	}

	// Aca copypastear lo de arriba para RB
	//
	//


}

/*****************************************************************/

// Para calcular tiempo - Función de la cátedra
double dwalltime(){
	double sec;
	struct timeval tv;

	gettimeofday(&tv,NULL);
	sec = tv.tv_sec + tv.tv_usec / 1000000.0;
	return sec;
}

// Para generar un número aleatorio - Función de la cátedra
double randFP(double min, double max) {
	double range = (max - min);
	double div = RAND_MAX / range;
	return min + (rand() / div);
}

/*****************************************************************/

// Main del programa
int main(int argc, char* argv[]){
	double average, timetick_start, timetick_end;
	int i, size;

	// Controla los argumentos al programa
	if ((argc != 3)
		|| ((N = atoi(argv[1])) <= 0)
		|| ((NUM_THREADS = atoi(argv[1])) <= 0)
		|| ((bs = atoi(argv[2])) <= 0)
		|| ((N % bs) != 0))
	{
		printf("\nError en los parámetros. Usage: ./%s N T\n", argv[0]);
		exit(1);
	}

	// Aloca memoria para las matrices
	size = N*N;
	A = (double*)malloc(sizeof(double)*size); // ordenada por columnas
	B = (double*)malloc(sizeof(double)*size); // ordenada por columnas
	C = (double*)malloc(sizeof(double)*size); // ordenada por filas
	T = (double*)malloc(sizeof(double)*size); // ordenada por filas
	R = (double*)malloc(sizeof(double)*size); // ordenada por filas
	M = (double*)malloc(sizeof(double)*size); // ordenada por filas
	R1 = (double*)malloc(sizeof(double)*size); // ordenada por filas
	R2 = (double*)malloc(sizeof(double)*size); // ordenada por filas
	RA = (double*)malloc(sizeof(double)*size); // ordenada por filas
	RB = (double*)malloc(sizeof(double)*size); // ordenada por filas

	// Inicializa el randomizador
	time_t t;
	srand((unsigned) time(&t));

	// Inicializa las matrices A, B, T, M, RA, y RB
	for(i = 0; i < size ; i++) {
		A[i] = randFP(0, 10);
		B[i] = randFP(0, 10);
		T[i] = randFP(0, 10);
		M[i] = randFP(0, DOUBLE_PI);
		R1[i] = 0;
		R2[i] = 0;
		RA[i] = 0;
		RB[i] = 0;
	}

	// Inicia el timer (hacerlo antes o despues de setup de hilos)
	timetick_start = dwalltime();

	// Setup hilos
	pthread_mutex_init(&average_lock1, NULL);
	pthread_mutex_init(&average_lock2, NULL);
	pthread_t threads[NUM_THREADS];
	int ids[NUM_THREADS];

	block_size_by_threads = N / NUM_THREADS;

	// Crea hilos y calcula R
	for (i = 0; i < NUM_THREADS; i++) {
		ids[i] = i;
		pthread_create(&threads[i], NULL, calculate, &ids[i]);
	}

	// Une hilos
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	// Detiene el timer
	timetick_end = dwalltime();

	printf("Tiempo en segundos %f\n", timetick_end - timetick_start);

	// Verifica el resultado
	int check = 1;
	double correct_result = 1;
	for (i = 0; i < size; i++) {
		if (C[i] != (correct_result)) {
			check = 0;
			break;
		}
	}
	if (check) {
		printf("Multiplicacion de matrices resultado correcto\n");
	}
	else {
		printf("Multiplicacion de matrices resultado erroneo\n");
	}

	// Libera memoria
	free(A);
	free(B);
	free(C);
	free(T);
	free(R);
	free(M);
	free(RA);
	free(RB);

	return(0);
}
