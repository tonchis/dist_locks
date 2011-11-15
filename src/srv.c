#include "srv.h"
#include "lista.h"

#include <stdio.h>

// ACHTUNG
//
// Por su salud, les recomendamos que al ejecutarlo manden stdout a /dev/null.
//
// END ACHTUNG

// funcion auxiliar para llamar a la funcion debug pasandole algun entero
char debug_buffer[1024];

void debugConValorEntero(char* str, int i) {
  sprintf(debug_buffer, str, i);
  debug(debug_buffer);
}

void servidor(int mi_cliente) {
    // variables auxiliares
    MPI_Status status;
    int origen, tag, rank;
    nodo_t *server;

    // numero de pedido recibido (si es que fue enviado)
    int recv_sequence_number;
    // cantidad de permisos recibidos
    int replies;
    // numero de pedido utilizado
    // la ultima vez que se pidio permiso al resto de los servidores
    int used_sequence_number;
    // cantidad de servidores activos
    // a los cuales se les pidio permiso la ultima vez
    int used_servers;

    // indica si pedimos entrar en la seccion critica
    int hay_pedido_local = FALSE;
    // indica si logramos entrar en la seccion critica
    int en_zona_critica = FALSE;
    // indica si hay que terminar el proceso
    int listo_para_salir = FALSE;
    // el numero para el siguiente pedido
    int sequence_number = 1;
    // servidores que estan esperando mi respuesta
    lista_t* deferred_replies = nuevaLista();

    // lista de servidores activos (sin contarme a mi mismo)
    lista_t* servers = nuevaLista();

    for(rank = 0; rank < cant_ranks; rank+= 2) {
      if(rank != mi_rank)
        agregarALaLista(rank, servers);
    }

    // mientras no tenga que terminar el proceso
    while( ! listo_para_salir ) {

        // espero recibir el siguiente mensaje
        MPI_Recv(&recv_sequence_number, 1, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);

        // obtengo datos del mensaje
        origen = status.MPI_SOURCE;
        tag = status.MPI_TAG;

        // dependiendo del tag
        switch(tag) {

          // pedido del cliente para entrar a la seccion critica
          case TAG_PEDIDO:
            debug("Mi cliente solicita acceso exclusivo");

            assert(origen == mi_cliente);
            assert(hay_pedido_local == FALSE);
            assert(en_zona_critica == FALSE);

            // vamos a pedir entrar a la seccion critica
            hay_pedido_local = TRUE;

            // si hay mas servidores
            if(servers->primero != NULL) {
              // me preparo para recibir el permiso de los servidores activos
              replies = 0;
              used_sequence_number = sequence_number;
              used_servers = servers->longitud;

              debugConValorEntero("Solicito acceso exclusivo a los demas servers (sequence number %d)", used_sequence_number);

              // por cada servidor activo
              server = servers->primero;

              while(server != NULL) {
                // pido permiso con el numero adecuado
                MPI_Send(&used_sequence_number, 1, MPI_INT, server->elemento, TAG_PEDIDO_S, COMM_WORLD);
                server = server->siguiente;
              }
            // si soy el unico servidor
            } else {
              debug("Dándole permiso a mi cliente");

              // entro a la zona critica directamente
              en_zona_critica = TRUE;
              MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
            }

            // actualizo el numero del siguiente pedido
            sequence_number++;

            break;

          // el cliente libera la seccion critica
          case TAG_LIBERO:
            debug("Mi cliente libera su acceso exclusivo");

            assert(origen == mi_cliente);
            assert(hay_pedido_local == TRUE);
            assert(en_zona_critica == TRUE);

            // indico que sali de la seccion critica
            hay_pedido_local = FALSE;
            en_zona_critica = FALSE;

            // por cada servidor esperando permisos
            server = deferred_replies->primero;

            while(server != NULL) {
              debugConValorEntero("Dándole permiso a un servidor (rk%d)", server->elemento);

              // le doy permiso
              MPI_Send(NULL, 0, MPI_INT, server->elemento, TAG_OTORGADO_S, COMM_WORLD);

              server = server->siguiente;
            }

            // vacio la lista de espera
            vaciarLaLista(deferred_replies);

            break;

          // el cliente termino
          case TAG_TERMINE:
            assert(origen == mi_cliente);
            debug("Mi cliente avisa que terminó");

            // por cada servidor activo
            server = servers->primero;

            while(server != NULL) {
              // le aviso que termine
              MPI_Send(NULL, 0, MPI_INT, server->elemento, TAG_TERMINE_S, COMM_WORLD);

              server = server->siguiente;
            }

            // indico que puedo terminar el proceso
            listo_para_salir = TRUE;

            break;

          // pedido de otro servidor para entrar a la seccion critica
          case TAG_PEDIDO_S:
            debugConValorEntero("Un servidor me pide permiso (rk%d)", origen);
            assert(origen % 2 == 0); // Es de otro srv.

            // actualizo el numero del siguiente pedido si es necesario
            if(recv_sequence_number >= sequence_number)
              sequence_number = recv_sequence_number + 1;

            // si no estoy esperando para entrar a la seccion critica
            // (ni estoy en ella)
            if(!hay_pedido_local) {
              // doy permiso directamente
              debugConValorEntero("Dándole permiso a un servidor (rk%d)", origen);
              MPI_Send(NULL, 0, MPI_INT, origen, TAG_OTORGADO_S, COMM_WORLD);
            // si estoy esperando para entrar a la seccion critica
            // (o estoy en ella)
            } else {
              // si estoy en seccion critica
              if(en_zona_critica) {
                // pongo en lista de espera
                debugConValorEntero("Dejo a un servidor en espera (rk%d)", origen);
                agregarALaLista(origen, deferred_replies);
              // si no estoy en seccion critica
              } else {
                // si su numero de pedido es menor
                // o bien, si son iguales pero su rank es menor
                if(recv_sequence_number < used_sequence_number || (recv_sequence_number == used_sequence_number && origen < mi_rank)) {
                  // le doy permiso
                  debugConValorEntero("Dándole permiso a un servidor (rk%d)", origen);
                  MPI_Send(NULL, 0, MPI_INT, origen, TAG_OTORGADO_S, COMM_WORLD);
                } else {
                  // pongo en lista de espera
                  debugConValorEntero("Dejo a un servidor en espera (rk%d)", origen);
                  agregarALaLista(origen, deferred_replies);
                }
              }
            }

            break;

          // otro servidor me dio permiso para entrar a la seccion critica
          case TAG_OTORGADO_S:
            debugConValorEntero("Un servidor me da permiso (rk%d)", origen);

            assert(origen % 2 == 0); // Es de otro srv.
            assert(hay_pedido_local == TRUE);
            assert(en_zona_critica == FALSE);

            // me llego un permiso mas
            replies++;

            assert(replies <= used_servers); //espero no tener replies de mas

            // si ya tengo todos los permisos
            if(replies == used_servers) {
              debug("Dándole permiso a mi cliente");

              // entro en zona critica
              en_zona_critica = TRUE;
              MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
            }

            break;

          // otro servidor termino
          case TAG_TERMINE_S:
            debugConValorEntero("Un servidor avisa que terminó (rk%d)", origen);

            // saco al servidor que termino de la lista de servidores activos
            sacarDeLaLista(origen, servers);
            break;

          // algo invalido
          default:
            debug("TAG desconocido");
            break;
        }
    }

    // libero memoria
    free(deferred_replies);
    free(servers);
}

