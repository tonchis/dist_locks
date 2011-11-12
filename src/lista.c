#include "lista.h"

#include <stdlib.h>
#include <assert.h>

void agregarALaLista(int elemento, lista_t* lista) {
  nodo_t* nodo = malloc(sizeof(nodo_t));
  nodo->elemento = elemento;
  
  if(lista->primero != NULL)
    nodo->siguiente = lista->primero;
  else
    nodo->siguiente = NULL;
    
  lista->primero = nodo;
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
}
