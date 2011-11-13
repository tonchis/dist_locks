#include "lista.h"

#include <stdlib.h>
#include <assert.h>

lista_t* nuevaLista()
{
    lista_t* lista = malloc(sizeof(lista_t));
    lista->primero = NULL;
    lista->longitud = 0;
    return lista;
}

void agregarALaLista(int elemento, lista_t* lista) {
  nodo_t* nodo = malloc(sizeof(nodo_t));
  nodo->elemento = elemento;
  
  if(lista->primero != NULL)
    nodo->siguiente = lista->primero;
  else
    nodo->siguiente = NULL;
    
  lista->primero = nodo;
  lista->longitud++;
}

void sacarDeLaLista(int elemento, lista_t* lista) {
  nodo_t* nodo = lista->primero;
  nodo_t* atras = NULL;
                
  while(nodo != NULL && nodo->elemento != elemento) {
    atras = nodo;
    nodo = nodo->siguiente;
  }
    
  assert(nodo != NULL);
  
  if(nodo == lista->primero) {
    lista->primero = nodo->siguiente;
  } else {
    atras->siguiente = nodo->siguiente;
  }

  nodo->siguiente = NULL;
  free(nodo);
  
  lista->longitud--;
}

void vaciarLaLista(lista_t* lista) {
  nodo_t* nodo = lista->primero;
  nodo_t* siguiente;
                
  while(nodo != NULL) {
    siguiente = nodo->siguiente;
    free(nodo);
    nodo = siguiente;
  }
  
  lista->primero = NULL;
  lista->longitud = 0;
}
