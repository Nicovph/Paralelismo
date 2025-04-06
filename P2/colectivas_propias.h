#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

int MPI_FlattreeColectiva(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm);

int MPI_BinomialColectiva(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm);