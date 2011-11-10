#include <stdio.h>
#include <string.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
    int n_ranks, mi_rank;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);

    printf("Soy el rank %u de %u\n", mi_rank, n_ranks);
    
    if(mi_rank == 0) {
        char *buffer = "Aloha honua";
        int destino = 1;
        int tag = 78;

        printf("R0 enviando mensaje a R1\n");
        MPI_Send(buffer, strlen(buffer)+1, MPI_CHAR, destino, tag, MPI_COMM_WORLD);

        printf("Mensaje enviado\n");
    }
    
    if(mi_rank == 1) {
        char buffer[100];
        int origen = 0;
        int tag = 78;
        
        printf("R1 recibiendo mensaje de R0\n");
        MPI_Recv(buffer, sizeof(buffer), MPI_CHAR, origen, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("Mensaje recibido\n");
        printf("Dice: '%s'\n", buffer);
    }
    
    MPI_Finalize();
    return 0;
}

