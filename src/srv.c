#include "srv.h"
#include "lista.h"

#include <stdio.h>

char debug_buffer[1024];

void debugConValorEntero(char* str, int i) {
  sprintf(debug_buffer, str, i);
  debug(debug_buffer);
}

void servidor(int mi_cliente) {
    MPI_Status status;
    int origen, tag, recv_sequence_number, replies, rank, used_sequence_number, used_servers;
    int hay_pedido_local = FALSE;
    int en_zona_critica = FALSE;
    int listo_para_salir = FALSE;
    int sequence_number = 1;
    lista_t* deferred_replies = nuevaLista();
    lista_t* servers = nuevaLista();
    nodo_t *reply, *server;
    
    for(rank = 0; rank < cant_ranks; rank+= 2) {
      if(rank != mi_rank)
        agregarALaLista(rank, servers);
    }

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

            if(servers->primero != NULL) {
              replies = 0;
              used_sequence_number = sequence_number;
              used_servers = servers->longitud;
              
              debugConValorEntero("Solicito acceso exclusivo a los demas servers (sequence number %d)", used_sequence_number);
              
              server = servers->primero;
              
              while(server != NULL) {
                MPI_Send(&used_sequence_number, 1, MPI_INT, server->elemento, TAG_PEDIDO_S, COMM_WORLD);
                server = server->siguiente;
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
            
            reply = deferred_replies->primero;
            
            while(reply != NULL) {
              debugConValorEntero("Dándole permiso a un servidor (rk%d)", reply->elemento);
              MPI_Send(NULL, 0, MPI_INT, reply->elemento, TAG_OTORGADO_S, COMM_WORLD);
              reply = reply->siguiente;
            }
          
            vaciarLaLista(deferred_replies);
            break;

          case TAG_TERMINE:
            assert(origen == mi_cliente);
            debug("Mi cliente avisa que terminó");
            
            server = servers->primero;
            
            while(server != NULL) {
              if(server->elemento != mi_rank)
                MPI_Send(NULL, 0, MPI_INT, server->elemento, TAG_TERMINE_S, COMM_WORLD);
                
              server = server->siguiente;
            }
            
            listo_para_salir = TRUE;
            break;

          case TAG_PEDIDO_S:
            debugConValorEntero("Un servidor me pide permiso (rk%d)", origen);
            assert(origen % 2 == 0); // Es de otro srv.
            
            if(recv_sequence_number >= sequence_number)
              sequence_number = recv_sequence_number + 1;
              
            if(!hay_pedido_local) {
              debugConValorEntero("Dándole permiso a un servidor (rk%d)", origen);
              MPI_Send(NULL, 0, MPI_INT, origen, TAG_OTORGADO_S, COMM_WORLD);
            } else {
              if(en_zona_critica) {
                debugConValorEntero("Dejo a un servidor en espera (rk%d)", origen);
                agregarALaLista(origen, deferred_replies);
              } else {
                if(recv_sequence_number < used_sequence_number || (recv_sequence_number == used_sequence_number && origen < mi_rank)) {
                  debugConValorEntero("Dándole permiso a un servidor (rk%d)", origen);
                  MPI_Send(NULL, 0, MPI_INT, origen, TAG_OTORGADO_S, COMM_WORLD);
                } else {
                  debugConValorEntero("Dejo a un servidor en espera (rk%d)", origen);
                  agregarALaLista(origen, deferred_replies);
                }
              }
            }
            
            break;

          case TAG_OTORGADO_S:
            debugConValorEntero("Un servidor me da permiso (rk%d)", origen);
            assert(origen % 2 == 0); // Es de otro srv.
            assert(hay_pedido_local == TRUE);
            assert(en_zona_critica == FALSE);
            
            replies++;
            assert(replies <= used_servers); //espero no tener replies de mas
            
            if(replies == used_servers) {
              debug("Dándole permiso a mi cliente");
              en_zona_critica = TRUE;
              MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
            }
            
            break;
            
          case TAG_TERMINE_S:
            debugConValorEntero("Un servidor avisa que terminó (rk%d)", origen);
            sacarDeLaLista(origen, servers);
            break;

          default:
            debug("TAG desconocido");
            break;
        }
    }
    
    free(deferred_replies);
    free(servers);
}

