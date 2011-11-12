#ifndef __lista_h__
#define __lista_h__

struct nodo {
  int elemento;
  struct nodo* siguiente;
};

typedef struct nodo nodo_t;

typedef struct {
  nodo_t* primero;
} lista_t;

void agregarALaLista(int elemento, lista_t* lista);
void sacarDeLaLista(int elemento, lista_t* lista);
void vaciarLaLista(lista_t* lista);

#endif