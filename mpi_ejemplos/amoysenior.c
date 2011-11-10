#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mpi.h"

/* estos son tags */
#define HAGAME_ESTA_SUMA         1
#define HAGAME_ESTE_PRODUCTO     2
#define COMO_NO_JEFE_ACA_TIENE   3
#define CERRATE_PIBE             4

/* esto es un rank */
#define THE_BOSS   0


void maestro(int cant)
{
    int veces = 20;
    int operandos[2];
    int resultado;
    int tag, destino;
    MPI_Status status;

    while(veces-- > 0) {
        operandos[0] = veces;
        operandos[1] = veces;
        
        if(veces > 10)
            tag = HAGAME_ESTA_SUMA;
        else
            tag = HAGAME_ESTE_PRODUCTO;

        destino = (veces % (cant-1)) + 1;
        
        MPI_Send(operandos, 2, MPI_INT, destino, tag, MPI_COMM_WORLD);
        
        MPI_Recv(&resultado, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        printf("El esclavo %u dice: %d\n", status.MPI_SOURCE, resultado);
    }
    
    int i;
    for(i = 1; i < cant; ++i)
        MPI_Send(NULL, 0, MPI_INT, i, CERRATE_PIBE, MPI_COMM_WORLD);
}


void esclavo(int rank)
{
    int operandos[2];
    int resultado;
    MPI_Status status;
    int basta = 0;

    while(!basta) {
        MPI_Recv(operandos, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        switch(status.MPI_TAG) {
            case HAGAME_ESTA_SUMA:
                resultado = operandos[0] + operandos[1];
                break;
            case HAGAME_ESTE_PRODUCTO:
                resultado = operandos[0] * operandos[1];
                break;
            case CERRATE_PIBE:
                basta = 1;
                printf("Sayonara (%u se cierra)\n", rank);
                break;
            default:
                assert(0);
        }

        if(!basta) MPI_Send(&resultado, 1, MPI_INT, THE_BOSS, COMO_NO_JEFE_ACA_TIENE, MPI_COMM_WORLD);
    }
}


int main(int argc, char *argv[])
{
    int n_ranks, mi_rank;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);
    
    if(mi_rank == THE_BOSS)
        maestro(n_ranks);
    else
        esclavo(mi_rank);
    
    MPI_Finalize();
    return 0;
}
