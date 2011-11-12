#include "srv.h"

/*
 *  Ejemplo de servidor que tiene el "sí fácil" para con su
 *  cliente y no se lleva bien con los demás servidores.
 *
 */

void servidor(int mi_cliente)
{
    MPI_Status status; int origen, tag;
    int hay_pedido_local = FALSE;
    int listo_para_salir = FALSE;
    int sequence_number = 1;

    while( ! listo_para_salir ) {

        MPI_Recv(NULL, 0, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
        origen = status.MPI_SOURCE;
        tag = status.MPI_TAG;

        switch(tag){
          case TAG_PEDIDO:
            assert(origen == mi_cliente);
            debug("Mi cliente solicita acceso exclusivo");
            assert(hay_pedido_local == FALSE);
            hay_pedido_local = TRUE;

            int rank;
            for(rank = 0; rank < cant_ranks, rank += 2){
              if(rank != mi_rank)
                MPI_Send(&sequence_number, 1, MPI_INT, rank, TAG_MESSAGE, COMM_WORLD);
            }

            break;

          case TAG_LIBERO:
            assert(origen == mi_cliente);
            debug("Mi cliente libera su acceso exclusivo");
            assert(hay_pedido_local == TRUE);
            hay_pedido_local = FALSE;
            break;

          case TAG_TERMINE:
            assert(origen == mi_cliente);
            debug("Mi cliente avisa que terminó");
            listo_para_salir = TRUE;
            break;

          case TAG_MESSAGE:
            assert(origen % 2 == 0); // Es de otro srv.
            break;

          case TAG_REPLY:
            assert(origen % 2 == 0); // Es de otro srv.
            // Aviso a mi cliente que le mande cumbia.
            debug("Dándole permiso (frutesco por ahora)");
            // MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
            break;

          default:
            debug("TAG desconocido");
            break;
        }
    }
}

