#include <stdio.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
    int mi_rank;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mi_rank);

    if(mi_rank == 0)
        printf("¡Aloha honua! ¡Soy el rank cero!\n");
    else
        printf("¡Aloha honua! Soy uno más del montón ...\n");
    
    MPI_Finalize();
    return 0;
}
