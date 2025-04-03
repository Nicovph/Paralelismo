#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

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
        if (rank == 0) {
            for (int i = 1; i < nprocs; i++) {
                MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        } else {
            MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

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
        if (rank != 0) {
            MPI_Send(&count_local, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        } else {
            count_global = count_local;
            for (int i = 1; i < nprocs; i++) {
                MPI_Recv(&count_local, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                count_global += count_local;
            }

            // Calcular π en el proceso 0
            pi = ((double) count_global / (double) n) * 4.0;
            printf("pi is approx. %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
        }
    }

    MPI_Finalize();
    return 0;
}