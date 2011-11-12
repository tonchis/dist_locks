#include "srv.h"
#include <stdio.h>

struct nodo{
  int origen;
  struct nodo* siguiente;
};

typedef struct nodo nodo_t;

void agregarALaLista(int origen, int sequence_number, nodo_t** lista) {
  nodo_t* reply = malloc(sizeof(nodo_t));
  
  if(lista != NULL)
    reply->siguiente = *lista;
  else
    reply->siguiente = NULL;
    
  *lista = reply;
}

void vaciarLista(nodo_t** lista) {
  nodo_t* reply = *lista;
  nodo_t* siguiente;
                
  while(reply != NULL) {
    siguiente = reply->siguiente;
    free(reply);
    reply = siguiente;
  }
  
  *lista = NULL;
}

void servidor(int mi_cliente) {
    MPI_Status status;
    int origen, tag, recv_sequence_number, replies, rank;
    int hay_pedido_local = FALSE;
    int en_zona_critica = FALSE;
    int listo_para_salir = FALSE;
    int sequence_number = 1;
    int cant_otros_srvs = cant_ranks / 2 - 1;
    nodo_t** deferred_replies = NULL;
    nodo_t* reply;

    while( ! listo_para_salir ) {

        MPI_Recv(&recv_sequence_number, 1, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
        origen = status.MPI_SOURCE;
        tag = status.MPI_TAG;

        switch(tag) {
          case TAG_PEDIDO:
            debug("Mi cliente solicita acceso exclusivo");
            
            assert(origen == mi_cliente);
            assert(hay_pedido_local == FALSE);
            assert(en_zona_critica == FALSE);
            
            hay_pedido_local = TRUE;
            replies = 0;

            if(cant_ranks > 2) {
              debug("Solicito acceso exclusivo a los demas servers");
              for(rank = 0; rank < cant_ranks; rank += 2) {
                if(rank != mi_rank)
                  MPI_Send(&sequence_number, 1, MPI_INT, rank, TAG_MESSAGE, COMM_WORLD);
              }
            } else {
              debug("Dándole permiso a mi cliente");
              en_zona_critica = TRUE;
              MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
            }
            
            sequence_number++;

            break;

          case TAG_LIBERO:
            debug("Mi cliente libera su acceso exclusivo");
            
            assert(origen == mi_cliente);
            assert(hay_pedido_local == TRUE);
            assert(en_zona_critica == TRUE);
            
            hay_pedido_local = FALSE;
            en_zona_critica = FALSE;
            
            if(deferred_replies != NULL) {
              reply = *deferred_replies;
                
              while(reply != NULL) {
                debug("Dándole permiso a un servidor");
                MPI_Send(NULL, 0, MPI_INT, reply->origen, TAG_REPLY, COMM_WORLD);
                reply = reply->siguiente;
              }
              
              vaciarLista(deferred_replies);
            }
            break;

          case TAG_TERMINE:
            assert(origen == mi_cliente);
            debug("Mi cliente avisa que terminó");
            listo_para_salir = TRUE;
            break;

          case TAG_MESSAGE:
            debug("Un servidor me pide permiso");
            assert(origen % 2 == 0); // Es de otro srv.
            
            if(recv_sequence_number >= sequence_number)
              sequence_number = recv_sequence_number + 1;
              
            if(!hay_pedido_local) {
              debug("Dándole permiso a un servidor");
              MPI_Send(NULL, 0, MPI_INT, origen, TAG_REPLY, COMM_WORLD);
            } else {
              if(en_zona_critica) {
                debug("Dejo a un servidor en espera");
                agregarALaLista(origen, recv_sequence_number, deferred_replies);
              } else {
                //desempate
              }
            }
            
            break;

          case TAG_REPLY:
            debug("Un servidor me da permiso");
            assert(origen % 2 == 0); // Es de otro srv.
            assert(hay_pedido_local == TRUE);
            assert(en_zona_critica == FALSE);
            
            replies++;
            assert(replies <= cant_otros_srvs); //espero no tener replies de mas
            
            if(replies == cant_otros_srvs) {
              debug("Dándole permiso a mi cliente");
              en_zona_critica = TRUE;
              MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
            }
            
            break;

          default:
            debug("TAG desconocido");
            break;
        }
    }
}

