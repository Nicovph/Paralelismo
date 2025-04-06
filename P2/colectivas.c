#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>
#include "colectivas_propias.h"

int main (int argc, char* argv[]) {
    int nprocs, rank, done = 0, n, count_local = 0, count_global = 0;
    double pi, x, y, z;
    double PI25DT = 3.141592653589793238462643;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);  
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    srand(time(NULL) + rank); // Semilla diferente para cada proceso
    
    while (!done) {
        if (rank == 0) {
            printf("Enter the number of points (0 quits): \n");
            scanf("%d", &n);
        }

        // Enviar el valor de `n` a todos los procesos
        MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
        if (n == 0) break;

        // Cada proceso calcula su parte de los puntos con un bucle intercalado
        count_local = 0;
        for (int i = rank; i < n; i += nprocs) {
            x = ((double) rand()) / ((double) RAND_MAX);
            y = ((double) rand()) / ((double) RAND_MAX);
            z = sqrt((x * x) + (y * y));
            if (z <= 1.0) count_local++;
        }

        // Enviar los resultados locales al proceso 0
        MPI_Reduce(&count_local, &count_global, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        // Calcular π en el proceso 0
        if (rank == 0) {
            pi = ((double) count_global / (double) n) * 4.0;
            printf("pi is approx. %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
        }
    }

    MPI_Finalize();
    return 0;
}