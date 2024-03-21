/*Copyright (C) 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * 2023 - francisco dot rodriguez at ingenieria dot unam dot mx
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#include "List.h"

// 29/03/23:
// Esta versión no borra elementos
// Esta versión no modifica los datos originales

#ifndef DBG_HELP
#define DBG_HELP 0
#endif  

#if DBG_HELP > 0
#define DBG_PRINT( ... ) do{ fprintf( stderr, "DBG:" __VA_ARGS__ ); } while( 0 )
#else
#define DBG_PRINT( ... ) ;
#endif  


// Aunque en este ejemplo estamos usando tipos básicos, vamos a usar al alias |Item| para resaltar
// aquellos lugares donde estamos hablando de DATOS y no de índices.
typedef int Item;


//----------------------------------------------------------------------
//                           Vertex stuff: 
//----------------------------------------------------------------------


/**
 * @brief Declara lo que es un vértice.
 */
typedef struct
{
   Data data;
   List* neighbors;
} Vertex;


/**
 * @brief Hace que cursor libre apunte al inicio de la lista de vecinos. Se debe
 * de llamar siempre que se vaya a iniciar un recorrido de dicha lista.
 *
 * @param v El vértice de trabajo (es decir, el vértice del cual queremos obtener 
 * la lista de vecinos).
 */
void Vertex_Start( Vertex* v )
{
   assert( v );

   List_Cursor_front( v->neighbors );
}

/**
 * @brief Mueve al cursor libre un nodo adelante.
 *
 * @param v El vértice de trabajo.
 *
 * @pre El cursor apunta a un nodo válido.
 * @post El cursor se movió un elemento a la derecha en la lista de vecinos.
 */
void Vertex_Next( Vertex* v )
{
   List_Cursor_next( v->neighbors );
}

/**
 * @brief Indica si se alcanzó el final de la lista de vecinos.
 *
 * @param v El vértice de trabajo.
 *
 * @return true si se alcanazó el final de la lista; false en cualquier otro
 * caso.
 */
bool Vertex_End( const Vertex* v )
{
   return List_Cursor_end( v->neighbors );
}


/**
 * @brief Devuelve el índice del vecino al que apunta actualmente el cursor en la lista de vecinos
 * del vértice |v|.
 *
 * @param v El vértice de trabajo (del cual queremos conocer el índice de su vecino).
 *
 * @return El índice del vecino en la lista de vértices.
 *
 * @pre El cursor debe apuntar a un nodo válido en la lista de vecinos.
 *
 * Ejemplo
 * @code
   Vertex* v = Graph_GetVertexByKey( grafo, 100 );
   for( Vertex_Start( v ); !Vertex_End( v ); Vertex_Next( v ) )
   {
      int index = Vertex_GetNeighborIndex( v );

      Item val = Graph_GetDataByIndex( g, index );

      // ...
   }
   @endcode
   @note Esta función debe utilizarse únicamente cuando se recorra el grafo con las funciones 
   Vertex_Start(), Vertex_End() y Vertex_Next().
 */
Data Vertex_GetNeighborIndex( const Vertex* v )
{
   return List_Cursor_get( v->neighbors );
}


//----------------------------------------------------------------------
//                           Graph stuff: 
//----------------------------------------------------------------------

/** Tipo del grafo.
 */
typedef enum 
{ 
   eGraphType_UNDIRECTED, ///< grafo no dirigido
   eGraphType_DIRECTED    ///< grafo dirigido (digraph)
} eGraphType; 

/**
 * @brief Declara lo que es un grafo.
 */
typedef struct
{
   Vertex* vertices; ///< Lista de vértices
   int size;      ///< Tamaño de la lista de vértices

   /**
    * Número de vértices actualmente en el grafo. 
    * Como esta versión no borra vértices, lo podemos usar como índice en la
    * función de inserción
    */
   int len;  

   eGraphType type; ///< tipo del grafo, UNDIRECTED o DIRECTED
} Graph;

//----------------------------------------------------------------------
//                     Funciones privadas
//----------------------------------------------------------------------

// vertices: lista de vértices
// size: número de elementos en la lista de vértices
// key: valor a buscar
// ret: el índice donde está la primer coincidencia; -1 si no se encontró
static int find( Vertex vertices[], int size, int key )
{
   for( int i = 0; i < size; ++i )
   {
      if( vertices[ i ].data.id == key ) return i;
   }

   return -1;
}

// busca en la lista de vecinos si el índice del vértice vecino ya se encuentra ahí
static bool find_neighbor( Vertex* v, int index )
{
   if( v->neighbors )
   {
      return List_Find( v->neighbors, index );
   }
   return false;
}

// vertex: vértice de trabajo
// index: índice en la lista de vértices del vértice vecino que está por insertarse
static void insert( Vertex* vertex, int index, float weight )
{
   // crear la lista si no existe!
   
   if( !vertex->neighbors )
   {
      vertex->neighbors = List_New();
   }

   if( vertex->neighbors && !find_neighbor( vertex, index ) )
   {
      List_Push_back( vertex->neighbors, index, weight );

      DBG_PRINT( "insert():Inserting the neighbor with idx:%d\n", index );
   } 
   else DBG_PRINT( "insert: duplicated index\n" );
}



//----------------------------------------------------------------------
//                     Funciones públicas
//----------------------------------------------------------------------


/**
 * @brief Crea un nuevo grafo.
 *
 * @param size Número de vértices que tendrá el grafo. Este valor no se puede
 * cambiar luego de haberlo creado.
 *
 * @return Un nuevo grafo.
 *
 * @pre El número de elementos es mayor que 0.
 */
Graph* Graph_New( int size, eGraphType type )
{
   assert( size > 0 );

   Graph* g = (Graph*) malloc( sizeof( Graph ) );
   if( g )
   {
      g->size = size;
      g->len = 0;
      g->type = type;

      g->vertices = (Vertex*) calloc( size, sizeof( Vertex ) );

      if( !g->vertices )
      {
         free( g );
         g = NULL;
      }
   }

   return g;
   // el cliente es responsable de verificar que el grafo se haya creado correctamente
}

void Graph_Delete( Graph** g )
{
   assert( *g );

   Graph* graph = *g;
   // para simplificar la notación 

   for( int i = 0; i < graph->size; ++i )
   {
      Vertex* vertex = &graph->vertices[ i ];
      // para simplificar la notación. 
      // La variable |vertex| sólo existe dentro de este for.

      if( vertex->neighbors )
      {
         List_Delete( &(vertex->neighbors) );
      }
   }

   free( graph->vertices );
   free( graph );
   *g = NULL;
}

/**
 * @brief Imprime un reporte del grafo
 *
 * @param g     El grafo.
 * @param depth Cuán detallado deberá ser el reporte (0: lo mínimo)
 */
void Graph_Print( Graph* g, int depth )
{
   if(g->type == eGraphType_UNDIRECTED){
      for( int i = 0; i < g->len; ++i )
      {
         Vertex* vertex = &g->vertices[ i ];
         // para simplificar la notación. 

         printf( "[%d]El aeropuerto con id %d con tiempo UTC= %d con código IATA %s del país %s de la ciudad %s con el nombre de %s "
         ,i, vertex->data.id, vertex->data.utc_time,vertex->data.iata_code, vertex->data.country, vertex->data.city,vertex->data.name) ;
         if( vertex->neighbors )
         {
            printf("es vecino de " );

            for( List_Cursor_front( vertex->neighbors );
               ! List_Cursor_end( vertex->neighbors );
               List_Cursor_next( vertex->neighbors ) )
            {

               Data d = List_Cursor_get( vertex->neighbors );
               int neighbor_idx = d.id;

               printf( "el aeropuerto con IATA %s, ", g->vertices[ neighbor_idx ].data.iata_code);
            }
         }
         printf( "y nada más.\n" );

      }
      printf( "\n" );
   }

    if(g->type == eGraphType_DIRECTED){
      for( int i = 0; i < g->len; ++i )
      {
         Vertex* vertex = &g->vertices[ i ];
         // para simplificar la notación. 
         if( vertex->neighbors )
         {
            printf( "[%d]Los aviones en el aeropuerto con id %d con tiempo UTC= %d con código IATA %s del país %s de la ciudad %s con el nombre de %s ",i, vertex->data.id, vertex->data.utc_time,vertex->data.iata_code, vertex->data.country, vertex->data.city,vertex->data.name) ;
            printf("puede ir a " );

            for( List_Cursor_front( vertex->neighbors );
               ! List_Cursor_end( vertex->neighbors );
               List_Cursor_next( vertex->neighbors ) )
            {

               Data d = List_Cursor_get( vertex->neighbors );
               int neighbor_idx = d.id;

               printf( "el aeropuerto con IATA %s con un tiempo de %0.2f, ", g->vertices[ neighbor_idx ].data.iata_code,d.weight);
            }
         }
         printf( "\n" );
      }
      printf( "\n" );
   }



}


void Graph_AddVertex( Graph* g, int id, char iata[], char country[], char city[], char name[], int utc)
{
   assert( g->len < g->size );

   Vertex* vertex = &g->vertices[ g->len ];
   // para simplificar la notación 

   vertex->data.id      = id;
   vertex->data.utc_time     = utc;
   for (int i = 0; i < sizeof(iata)/sizeof(char); i++)
      vertex->data.iata_code[i]  = iata[i];
   
   for (int i = 0; i < sizeof(country)/sizeof(char); ++i)
      vertex->data.country[i]      = country[i];
   
  for (int i = 0; i < sizeof(city)/sizeof(char) ; ++i)
    vertex->data.city[i]     = city[i];
  
   for (int i = 0; i < sizeof(name)/sizeof(char); ++i)
      vertex->data.name[i]      = name[i];
   
   vertex->neighbors = NULL;

   ++g->len;
}

int Graph_GetSize( Graph* g )
{
   return g->size;
}


/**
 * @brief Inserta una relación de adyacencia del vértice |start| hacia el vértice |finish|.
 *
 * @param g      El grafo.
 * @param start  Vértice de salida (el dato)
 * @param finish Vertice de llegada (el dato)
 *
 * @return false si uno o ambos vértices no existen; true si la relación se creó con éxito.
 *
 * @pre El grafo no puede estar vacío.
 */
bool Graph_AddEdge( Graph* g, int start, int finish  )
{
   assert( g->len > 0 );

   // obtenemos los índices correspondientes:
   int start_idx = find( g->vertices, g->size, start );
   int finish_idx = find( g->vertices, g->size, finish );

   DBG_PRINT( "AddEdge(): from:%d (with index:%d), to:%d (with index:%d)\n", start, start_idx, finish, finish_idx );

   if( start_idx == -1 || finish_idx == -1 ) return false;
   // uno o ambos vértices no existen

   insert( &g->vertices[ start_idx ], finish_idx, 0.0 );
   // insertamos la arista start-finish

   if( g->type == eGraphType_UNDIRECTED ) insert( &g->vertices[ finish_idx ], start_idx, 0.0 );
   // si el grafo no es dirigido, entonces insertamos la arista finish-start

   return true;
}
bool Graph_AddWeightedEdge( Graph* g, int start, int finish, float peso)
{
   assert( g->len > 0 );

   // obtenemos los índices correspondientes:
   int start_idx = find( g->vertices, g->size, start );
   int finish_idx = find( g->vertices, g->size, finish );

   DBG_PRINT( "AddEdge(): from:%d (with index:%d), to:%d (with index:%d)\n", start, start_idx, finish, finish_idx );

   if( start_idx == -1 || finish_idx == -1 ) return false;
   // uno o ambos vértices no existen

   insert( &g->vertices[ start_idx ], finish_idx, peso);
   // insertamos la arista start-finish

   if( g->type == eGraphType_UNDIRECTED ) insert( &g->vertices[ finish_idx ], start_idx, peso );
   // si el grafo no es dirigido, entonces insertamos la arista finish-start

   return true;
}


int Graph_GetLen( Graph* g )
{
   return g->len;
}


/**
 * @brief Devuelve la información asociada al vértice indicado.
 *
 * @param g          Un grafo.
 * @param vertex_idx El índice del vértice del cual queremos conocer su información.
 *
 * @return La información asociada al vértice vertex_idx.
 */
Item Graph_GetDataByIndex( const Graph* g, int vertex_idx )
{
   assert( 0 <= vertex_idx && vertex_idx < g->len );

   return g->vertices[ vertex_idx ].data.id;
}

/**
 * @brief Devuelve una referencia al vértice indicado.
 *
 * Esta función puede ser utilizada con las operaciones @see Vertex_Start(), @see Vertex_End(), @see Vertex_Next().
 *
 * @param g          Un grafo
 * @param vertex_idx El índice del vértice del cual queremos devolver la referencia.
 *
 * @return La referencia al vértice vertex_idx.
 */
Vertex* Graph_GetVertexByIndex( const Graph* g, int vertex_idx )
{
   assert( 0 <= vertex_idx && vertex_idx < g->len );

   return &(g->vertices[ vertex_idx ] );
}

int Graph_getIndexByValue(Graph* g, int vertex_val)
{

   for (int i = 0; i < Graph_GetLen(g); ++i)
   {
      Vertex* vertex = Graph_GetVertexByIndex(g, i);
      if(vertex->data.id == vertex_val)
         return i;
   }
   return -1;

}
 
bool is_Neighbor_Of( Graph* g, int dest, int src)
{
   assert( g->len > 0 );

   // obtenemos los índices correspondientes:
   int src_idx = find( g->vertices, g->size, src);
   int dest_idx = find( g->vertices, g->size, dest );

    if( src_idx == -1 || dest_idx == -1 ) return false;
   // uno o ambos vértices no existen

   Vertex* vertex = &g->vertices[src_idx];
   for (Vertex_Start(vertex); !Vertex_End(vertex); Vertex_Next(vertex))
   {
      Data neighbor = Vertex_GetNeighborIndex(vertex);
      if (neighbor.id == dest_idx)
      {
         return true;
      }
   }
   return false;
}

#define MAX_VERTICES 10


int main() {
  Graph *grafo = Graph_New(MAX_VERTICES, // cantidad máxima de vértices
                           eGraphType_DIRECTED); // será un grafo no dirigido

  // crea los vértices. El orden de inserción no es importante
  Graph_AddVertex(grafo,100,"MEX","Mexico","Mexico_city","Aeropuerto internacional Benito Juarez",-6);
  Graph_AddVertex(grafo,120,"LHR","London","Heathrow","Hetrow Airport",-5);
  Graph_AddVertex(grafo,130,"MAD","Madrid","Barajas","Barajas Airport",-4);
  Graph_AddVertex(grafo,140,"FRA","Alemania","Frankfurt","Flughafen",-4);
  Graph_AddVertex(grafo,150,"CDG","Francia","Paris","Charles de gaulle",-4);
  Graph_AddVertex(grafo,160,"BER","Alemania","Berlin","Branderburg",-4);
  Graph_AddVertex(grafo,170,"HKG","Honk Kong","Honk Kong","Honk kong Airport",-4);

  
  Graph_AddWeightedEdge(grafo, 100, 150, 9.0);
  Graph_AddWeightedEdge(grafo, 100, 140, 10.0);
  Graph_AddWeightedEdge(grafo, 100, 130, 9.5);
  Graph_AddWeightedEdge(grafo, 120, 160, 2.0);
  Graph_AddWeightedEdge(grafo, 130, 150, 1.5);
  Graph_AddWeightedEdge(grafo, 130, 120, 2.0);
  Graph_AddWeightedEdge(grafo, 140, 160, 3.0);
  Graph_AddWeightedEdge(grafo, 140, 120, 2.5);
  Graph_AddWeightedEdge(grafo, 150, 100, 9.0);
  Graph_AddWeightedEdge(grafo, 150, 120, 10.0);
  Graph_AddWeightedEdge(grafo, 150, 170, 14.0);
  Graph_AddWeightedEdge(grafo, 160, 100, 12.0);
  Graph_AddWeightedEdge(grafo, 170, 150, 14.0);


  Graph_Print(grafo, 0);
  
  int vertexbuscado;
  printf("Que Aeropuerto quiere consultar (100,120,etc)\n");
  scanf("%d", &vertexbuscado);

  Vertex* vertex = &grafo->vertices[ Graph_getIndexByValue(grafo,vertexbuscado) ];
  printf( "Los aviones en el aeropuerto con ID %d pueden ir a ",vertexbuscado);
  for( List_Cursor_front( vertex->neighbors );
               ! List_Cursor_end( vertex->neighbors );
               List_Cursor_next( vertex->neighbors ) )
            {

               Data d = List_Cursor_get( vertex->neighbors );
               int neighbor_idx = d.id;

               printf( "el aeropuerto con IATA %s con un tiempo de %0.2f, ", grafo->vertices[ neighbor_idx ].data.iata_code,d.weight);
            }

  Graph_Delete(&grafo);
  assert(grafo == NULL);
}
