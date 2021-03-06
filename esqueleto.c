#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define PI 3.14159265358979323846
#define DOUBLE_PI PI * 2

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
	double *A, *B, *C, *T, *R, *M, *RA, *RB, timetick_start, timetick_end;
	int N, i, j, bs, offset_i, offset_j, row_index;
	double average = 0;

	// Controla los argumentos al programa
	if ((argc != 2) || ((N = atoi(argv[1])) <= 0) )
	{
		printf("\nUsar: %s n\n  n: Dimension de la matriz (nxn X nxn)\n", argv[0]);
		exit(1);
	}

	// Aloca memoria para las matrices
	int size = N*N;
	A = (double*)malloc(sizeof(double)*size); // ordenada por columnas
	B = (double*)malloc(sizeof(double)*size); // ordenada por columnas
	C = (double*)malloc(sizeof(double)*size); // ordenada por filas
	T = (double*)malloc(sizeof(double)*size); // ordenada por filas
	R = (double*)malloc(sizeof(double)*size); // ordenada por filas
	M = (double*)malloc(sizeof(double)*size); // ordenada por filas
	Runo = (double*)malloc(sizeof(double)*size); // ordenada por filas
	Rdos = (double*)malloc(sizeof(double)*size); // ordenada por filas

	// Inicializa el randomizador
	time_t t;
	srand((unsigned) time(&t));

	// Inicializa las matrices A, B, T, M, RA, y RB
	for(i = 0; i < size ; i++) {
		A[i] = randFP(0, 10);
		B[i] = randFP(0, 10);
		T[i] = randFP(0, 10);
		M[i] = randFP(0, DOUBLE_PI);
		RA[i] = 0;
		RB[i] = 0;
	}

	// Inicia el timer
	timetick_start = dwalltime();

	// Calcula R y su promedio
	for (i = 0; i < size; i++) {
		R[i] = (1 - T[i]) * (1 - cos(M[i])) + (T[i] * sin(M[i]));
		average += R[i];
	}
	average /= size;

	// Calcula C

	// printf("Multiplying %d x %d matrices\n", n, n);
	matmulblks(Runo, A, C, N, bs);
	matmulblks(Rdos, B, C, N, bs);

	for (i = 0; i < size; i++) {
		C[i] = T[i] + average * (RA[i] + RB[i]);
	}

	// Detiene el timer
	timetick_end = dwalltime();

	printf("Tiempo en segundos %f\n", timetick_end - timetick_start);

	// Verifica el resultado
	int check = 1;
	double correct_result = 2 * N + 1;
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
