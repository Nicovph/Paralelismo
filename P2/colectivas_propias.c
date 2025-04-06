#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

int MPI_FlattreeColectiva(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
    int nprocs, rank, error;
    int sendbuf_global = 0;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &nprocs);

    if (rank == root) {
        // Sumar los datos locales del proceso raíz
        sendbuf_global += ((int *)sendbuf)[0];

        // Recibir datos de los demás procesos
        for (int i = 0; i < nprocs; i++) {
            if (i != root) { // Evitar recibir de sí mismo
                int tempbuf;
                error = MPI_Recv(&tempbuf, count, datatype, i, 1234, comm, MPI_STATUS_IGNORE);
                if (error != MPI_SUCCESS) {
                    return error;
                }
                sendbuf_global += tempbuf; // Sumar los datos recibidos
            }
        }

        // Guardar el resultado en recvbuf
        ((int *)recvbuf)[0] = sendbuf_global;
    } else {
        // Enviar los datos locales al proceso raíz
        error = MPI_Send(sendbuf, count, datatype, root, 1234, comm);
        if (error != MPI_SUCCESS) {
            return error;
        }
    }

    return MPI_SUCCESS;
}






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
        MPI_FlattreeColectiva(&count_local, &count_global, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        // Calcular π en el proceso 0
        if (rank == 0) {
            pi = ((double) count_global / (double) n) * 4.0;
            printf("pi is approx. %.16f, Error is %.16f\n", pi, fabs(pi - PI25DT));
        }
    }

    MPI_Finalize();
    return 0;
}