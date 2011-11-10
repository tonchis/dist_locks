#include <stdio.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
    int n_ranks, mi_rank;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);

    printf("¡Aloha honua! Soy el rank %u de %u.\n", mi_rank, n_ranks);
    
    MPI_Finalize();
    return 0;
}
