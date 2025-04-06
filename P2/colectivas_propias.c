#include "colectivas_propias.h"

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

int MPI_BinomialColectiva(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
    int numprocs, rank;

    MPI_Comm_size(comm, &numprocs);
    MPI_Comm_rank(comm, &rank);

    for (int i = 0; pow(2, i) <= numprocs; i++)
    {
        if (rank < pow(2, i) && rank + pow(2, i) < numprocs)
            MPI_Send(buffer, count, datatype, rank + (int)pow(2, i), 0, comm);
        if (rank >= pow(2, i) && rank < pow(2, i + 1)) // Intervalo de los procesos que pueden recibir
            MPI_Recv(buffer, count, datatype, rank - (int)pow(2, i), 0, comm, MPI_STATUS_IGNORE);
    }
    return MPI_SUCCESS;
}