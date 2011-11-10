#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

/* Subir este valor hasta causar deadlock... */
#define N 1000

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int *i = malloc(sizeof(int) * N);
    i[0] = rank;
    i[N - 1] = size * (rank + 1);

    printf("Rank %d: voy a enviar ...\n", rank);
    MPI_Send(i, N, MPI_INT, (rank + 1) % size, 1, MPI_COMM_WORLD);
    printf("Rank %d: enviado de %d a %d\n", rank, i[0], i[N - 1]);

    MPI_Recv(i, N, MPI_INT, (rank - 1) % size, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Rank %d: recibido de %d a %d (del rank %d)\n", rank, i[0], i[N - 1], (size + rank - 1) % size);

    MPI_Finalize();
    return 0;
}
