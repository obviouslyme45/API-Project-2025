#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define CACHE_SIZE 50
#define MIN_HEAP_SIZE 512

typedef struct heapentry
{
    int id;
    int pathcost;
} heapentry;

typedef struct queueentry
{
    int id;
    int distance;
} queueentry;

typedef struct flight
{
    int destid;
    int cost;
    struct flight *next;
} flight;

typedef struct hexagon
{
    int neighbors[6]; /*Array sorting the ids of the neighbors*/
    int cost;
    int nflights;
    flight *firstflight;
} hexagon;

/* Structure to represent a Heap (Min Heap) */
typedef struct heap
{
    heapentry *array;
    int capacity;
    int size;
} heap;

typedef struct cacheEntry
{
    int start;
    int dest;
    int pathcost;
} cacheEntry;

int columnsglob;
int rowsglob;
hexagon *hexagonarray;
int *visited; /*Used to store whether a hexagon has been visited or not this iteration*/
int *dist;    /*This array is needed to not add duplicates to the heap*/
int arraysize;
cacheEntry cacheTable[CACHE_SIZE];
int cacheposition; /*Used to track the position in the cache to add new elements*/
int iteration;

/*init and subfunctions*/
void init(int columns, int rows);
static inline void init_cache();
void add_edges_collapsed();
void add_corner_edges(int x, int y);
void add_column_edges(int x, int y);
void add_row_edges(int x, int y);
void add_edges(int x, int y);
void free_aux_structures();

void toggle_air_route(int xstart, int ystart, int xdest, int ydest);

/*change_cost and subfunction*/
void change_cost(int x, int y, int value, int radius);
void breadth_first_search(int x, int y, int value, int radius);

/*travel_cost and subfunctions*/
void travel_cost(int xp, int yp, int xd, int yd);
int dijkstra(int xs, int ys, int xd, int yd);
void insert_into_cache(int xs, int ys, int xd, int yd, int cost);
heap *create_heap(int capacity);
static inline void free_heap(heap *heap);
static inline void swap(heapentry *a, heapentry *b);
void min_heapify(heap *heappointer, int i);
void insert_heap(heap *heap, int id, int value);
static void extract_min(heap *heap, heapentry *aux);
static inline void set_dist(int id, int value);
static inline int get_dist(int id);

static inline int calculate_id(int x, int y); /*ID is reversed by doing y=id/numcolumns x=id%numcolumns*/

int main(int argc, char *argv[])
{
    char line[50];
    int values[4]; /*Array containing the integer inputs*/

    while (fgets(line, 50, stdin))
    {

        switch (line[0])
        {
        case 'i':
        {
            sscanf(line, "%*s %d%d", &values[0], &values[1]);
            init(values[0], values[1]);
            break;
        }

        case 'c':
        {
            sscanf(line, "%*s %d%d%d%d", &values[0], &values[1], &values[2], &values[3]);
            change_cost(values[0], values[1], values[2], values[3]);
            break;
        }

        case 't':
        {
            if (line[1] == 'o')
            {
                sscanf(line, "%*s %d%d%d%d", &values[0], &values[1], &values[2], &values[3]);
                toggle_air_route(values[0], values[1], values[2], values[3]);
            }
            else
            {
                sscanf(line, "%*s %d%d%d%d", &values[0], &values[1], &values[2], &values[3]);
                travel_cost(values[0], values[1], values[2], values[3]);
            }

            break;
        }

        default:
            break;
        }
    }

    if (hexagonarray)
        free_aux_structures();

    return 0;
}

void init(int columns, int rows)
{
    int x, y;

    if (hexagonarray != NULL)
    {
        free_aux_structures();
        init_cache();
        iteration = 0;
    }

    arraysize = columns * rows;
    columnsglob = columns;
    rowsglob = rows;

    hexagonarray = malloc(sizeof(hexagon) * arraysize);
    visited = malloc(sizeof(int) * arraysize);
    dist = malloc(sizeof(int) * arraysize);

    if (!hexagonarray || !visited || !dist)
    {
        printf("Initialization not possible\n");
        printf("KO\n");
        return;
    }

    if (rows == 1 || columns == 1)
    {
        add_edges_collapsed();
    }
    else
    {
        for (y = 0; y < rows; y++)
        {
            for (x = 0; x < columns; x++)
            {

                if ((x == 0 && y == 0) || (x == columnsglob - 1 && y == 0) ||
                    (x == 0 && y == rowsglob - 1) || (x == columnsglob - 1 && y == rowsglob - 1)) /*Check for corners*/
                {
                    add_corner_edges(x, y);
                }
                else if (y == 0 || y == rowsglob - 1)
                {
                    add_row_edges(x, y);
                }
                else if (x == 0 || x == columnsglob - 1)
                {
                    add_column_edges(x, y);
                }
                else
                {
                    add_edges(x, y);
                }
            }
        }
    }

    init_cache();

    printf("OK\n");
}

static inline int calculate_id(int x, int y) /*ID is reversed by doing y=id/numcolumns x=id%numcolumns*/
{
    return (y * columnsglob + x);
}

static inline void init_cache()
{
    cacheposition = 0;
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        cacheTable[i].start = -1;
        cacheTable[i].dest = -1;
        cacheTable[i].pathcost = -1;
    }
}

void add_edges_collapsed()
{
    int id = 0;
    int x, y;

    if (rowsglob == 1) /*Case when the map is collapsed on the x axis*/
    {
        for (x = 0; x < columnsglob; x++)
        {
            id = calculate_id(x, 0);

            hexagonarray[id].neighbors[0] = -1;
            hexagonarray[id].neighbors[1] = -1;
            hexagonarray[id].neighbors[2] = -1;
            hexagonarray[id].neighbors[3] = -1;
            hexagonarray[id].neighbors[4] = -1;
            hexagonarray[id].neighbors[5] = -1;

            hexagonarray[id].cost = 1;
            hexagonarray[id].nflights = 0;
            hexagonarray[id].firstflight = NULL;

            if (x == 0) /*Bottom left case*/
            {
                hexagonarray[id].neighbors[0] = 1;
            }
            else if (x == columnsglob - 1)
            {
                hexagonarray[id].neighbors[0] = x - 1;
            }
            else
            {
                hexagonarray[id].neighbors[0] = x + 1;
                hexagonarray[id].neighbors[1] = x - 1;
            }
        }
    }
    else /*Case when the map is collapsed on the y axis*/
    {
        for (y = 0; y < columnsglob; y++)
        {
            id = calculate_id(y, 0);

            hexagonarray[id].neighbors[0] = -1;
            hexagonarray[id].neighbors[1] = -1;
            hexagonarray[id].neighbors[2] = -1;
            hexagonarray[id].neighbors[3] = -1;
            hexagonarray[id].neighbors[4] = -1;
            hexagonarray[id].neighbors[5] = -1;

            hexagonarray[id].cost = 1;
            hexagonarray[id].nflights = 0;
            hexagonarray[id].firstflight = NULL;

            if (y == 0) /*Bottom left case*/
            {
                hexagonarray[id].neighbors[0] = 1;
            }
            else if (y == rowsglob - 1)
            {
                hexagonarray[id].neighbors[0] = y - 1;
            }
            else
            {
                hexagonarray[id].neighbors[0] = y + 1;
                hexagonarray[id].neighbors[1] = y - 1;
            }
        }
    }
}

void add_corner_edges(int x, int y)
{
    int id = calculate_id(x, y);

    hexagonarray[id].neighbors[0] = -1;
    hexagonarray[id].neighbors[1] = -1;
    hexagonarray[id].neighbors[2] = -1;
    hexagonarray[id].neighbors[3] = -1;
    hexagonarray[id].neighbors[4] = -1;
    hexagonarray[id].neighbors[5] = -1;

    hexagonarray[id].cost = 1;
    hexagonarray[id].nflights = 0;
    hexagonarray[id].firstflight = NULL;

    if ((x == 0 && y == 0)) /*Bottom left case*/
    {
        hexagonarray[id].neighbors[0] = calculate_id(x + 1, y);
        hexagonarray[id].neighbors[1] = calculate_id(x, y + 1);
    }
    else if (x == 0 && y == rowsglob - 1) /*Add edges for the top left case*/
    {
        if (y % 2 == 0) /*Distinction between even rows and odd rows, even rows have only 2 connections while odd rows have 3*/
        {
            hexagonarray[id].neighbors[0] = calculate_id(x + 1, y);
            hexagonarray[id].neighbors[1] = calculate_id(x, y - 1);
        }
        else /*Odd rows*/
        {
            hexagonarray[id].neighbors[0] = calculate_id(x + 1, y);
            hexagonarray[id].neighbors[1] = calculate_id(x + 1, y - 1);
            hexagonarray[id].neighbors[2] = calculate_id(x, y - 1);
        }
    }
    else if (x == columnsglob - 1 && y == rowsglob - 1) /*Top right case*/
    {
        if (y % 2 == 0) /*Even row case*/
        {
            hexagonarray[id].neighbors[0] = calculate_id(x - 1, y);
            hexagonarray[id].neighbors[1] = calculate_id(x - 1, y - 1);
            hexagonarray[id].neighbors[2] = calculate_id(x, y - 1);
        }
        else /*Odd row case*/
        {
            hexagonarray[id].neighbors[0] = calculate_id(x, y - 1);
            hexagonarray[id].neighbors[1] = calculate_id(x - 1, y);
        }
    }
    else /*Bottom right case*/
    {
        hexagonarray[id].neighbors[0] = calculate_id(x - 1, y);
        hexagonarray[id].neighbors[1] = calculate_id(x - 1, y + 1);
        hexagonarray[id].neighbors[2] = calculate_id(x, y + 1);
    }
}

void add_column_edges(int x, int y)
{
    int id = calculate_id(x, y);

    hexagonarray[id].neighbors[0] = -1;
    hexagonarray[id].neighbors[1] = -1;
    hexagonarray[id].neighbors[2] = -1;
    hexagonarray[id].neighbors[3] = -1;
    hexagonarray[id].neighbors[4] = -1;
    hexagonarray[id].neighbors[5] = -1;

    hexagonarray[id].cost = 1;
    hexagonarray[id].nflights = 0;
    hexagonarray[id].firstflight = NULL;

    if (x == 0) /*Leftmost column case*/
    {
        if (y % 2 == 0) /*Even row case, 3 edges necessary*/
        {
            hexagonarray[id].neighbors[0] = calculate_id(x, y + 1);
            hexagonarray[id].neighbors[1] = calculate_id(x + 1, y);
            hexagonarray[id].neighbors[2] = calculate_id(x, y - 1);
        }
        else /*Odd row case, 5 edges necessary*/
        {
            hexagonarray[id].neighbors[0] = calculate_id(x, y + 1);
            hexagonarray[id].neighbors[1] = calculate_id(x + 1, y + 1);
            hexagonarray[id].neighbors[2] = calculate_id(x + 1, y);
            hexagonarray[id].neighbors[3] = calculate_id(x + 1, y - 1);
            hexagonarray[id].neighbors[4] = calculate_id(x, y - 1);
        }
    }
    else /*Rightmost column case*/
    {
        if (y % 2 == 0) /*Even row, 5 edges necessary*/
        {
            hexagonarray[id].neighbors[0] = calculate_id(x, y - 1);
            hexagonarray[id].neighbors[1] = calculate_id(x - 1, y - 1);
            hexagonarray[id].neighbors[2] = calculate_id(x - 1, y);
            hexagonarray[id].neighbors[3] = calculate_id(x - 1, y + 1);
            hexagonarray[id].neighbors[4] = calculate_id(x, y + 1);
        }
        else /*Odd row, 3 cases necessary*/
        {
            hexagonarray[id].neighbors[0] = calculate_id(x, y - 1);
            hexagonarray[id].neighbors[1] = calculate_id(x - 1, y);
            hexagonarray[id].neighbors[2] = calculate_id(x, y + 1);
        }
    }
}

void add_row_edges(int x, int y)
{
    int id = calculate_id(x, y);

    hexagonarray[id].neighbors[0] = -1;
    hexagonarray[id].neighbors[1] = -1;
    hexagonarray[id].neighbors[2] = -1;
    hexagonarray[id].neighbors[3] = -1;
    hexagonarray[id].neighbors[4] = -1;
    hexagonarray[id].neighbors[5] = -1;

    hexagonarray[id].cost = 1;
    hexagonarray[id].nflights = 0;
    hexagonarray[id].firstflight = NULL;

    if (y == rowsglob - 1) /*Top row case*/
    {
        if (y % 2 == 0) /*Even row*/
        {
            hexagonarray[id].neighbors[0] = calculate_id(x + 1, y);
            hexagonarray[id].neighbors[1] = calculate_id(x, y - 1);
            hexagonarray[id].neighbors[2] = calculate_id(x - 1, y - 1);
            hexagonarray[id].neighbors[3] = calculate_id(x - 1, y);
        }
        else /*Odd row case*/
        {
            hexagonarray[id].neighbors[0] = calculate_id(x + 1, y);
            hexagonarray[id].neighbors[1] = calculate_id(x + 1, y - 1);
            hexagonarray[id].neighbors[2] = calculate_id(x, y - 1);
            hexagonarray[id].neighbors[3] = calculate_id(x - 1, y);
        }
    }
    else /*Bottom row case*/
    {
        hexagonarray[id].neighbors[0] = calculate_id(x - 1, y);
        hexagonarray[id].neighbors[1] = calculate_id(x - 1, y + 1);
        hexagonarray[id].neighbors[2] = calculate_id(x, y + 1);
        hexagonarray[id].neighbors[3] = calculate_id(x + 1, y);
    }
}

void add_edges(int x, int y)
{
    int id = calculate_id(x, y);

    hexagonarray[id].neighbors[0] = -1;
    hexagonarray[id].neighbors[1] = -1;
    hexagonarray[id].neighbors[2] = -1;
    hexagonarray[id].neighbors[3] = -1;
    hexagonarray[id].neighbors[4] = -1;
    hexagonarray[id].neighbors[5] = -1;

    hexagonarray[id].cost = 1;
    hexagonarray[id].nflights = 0;
    hexagonarray[id].firstflight = NULL;

    if (y % 2 == 0) /*Different cases for even rows and odd rows, here taken in consideration even rows*/
    {
        hexagonarray[id].neighbors[0] = calculate_id(x, y + 1);
        hexagonarray[id].neighbors[1] = calculate_id(x + 1, y);
        hexagonarray[id].neighbors[2] = calculate_id(x, y - 1);
        hexagonarray[id].neighbors[3] = calculate_id(x - 1, y - 1);
        hexagonarray[id].neighbors[4] = calculate_id(x - 1, y);
        hexagonarray[id].neighbors[5] = calculate_id(x - 1, y + 1);
    }
    else /*Case for odd rows*/
    {
        hexagonarray[id].neighbors[0] = calculate_id(x + 1, y + 1);
        hexagonarray[id].neighbors[1] = calculate_id(x + 1, y);
        hexagonarray[id].neighbors[2] = calculate_id(x + 1, y - 1);
        hexagonarray[id].neighbors[3] = calculate_id(x, y - 1);
        hexagonarray[id].neighbors[4] = calculate_id(x - 1, y);
        hexagonarray[id].neighbors[5] = calculate_id(x, y + 1);
    }
}

void free_aux_structures()
{
    int i;
    flight *aux, *previous;

    for (i = 0; i < arraysize; i++)
    {
        if (hexagonarray[i].firstflight != NULL)
        {
            aux = hexagonarray[i].firstflight;
            previous = aux;
            while (aux != NULL)
            {
                aux = aux->next;
                free(previous);
                previous = aux;
            }
        }
    }

    free(hexagonarray);
    free(visited);
    free(dist);
}

void toggle_air_route(int xstart, int ystart, int xdest, int ydest)
{
    hexagon *aux = NULL;
    flight *auxflight, *previous;
    int found = 0;
    int flightcostsum = 0;

    int idstart = calculate_id(xstart, ystart);
    int iddest = calculate_id(xdest, ydest);

    aux = &hexagonarray[idstart];

    if (hexagonarray == NULL) /*Check if the array has been initialized*/
    {
        printf("KO\n");
        return;
    }

    if (xstart < 0 || xstart > columnsglob - 1 || ystart < 0 || ystart > rowsglob - 1 || xdest < 0 || xdest > columnsglob - 1 || ydest < 0 || ydest > rowsglob - 1) /*Check if the coordinates are out of bounds*/
    {
        printf("KO\n");
        return;
    }

    if (aux->nflights == 0) /*Case where I have to add the first flight*/
    {
        aux->firstflight = malloc(sizeof(flight));
        auxflight = aux->firstflight; /*Go to the newly added element*/

        auxflight->destid = iddest;
        auxflight->cost = aux->cost;
        auxflight->next = NULL;
        aux->nflights++;
    }
    else /*Case where I have one or more flights*/
    {
        auxflight = aux->firstflight;
        previous = auxflight;
        flightcostsum = aux->cost;

        while (auxflight->next != NULL && found == 0) /*With this "while" I look if the flight is already present and sum the costs*/
        {
            if (auxflight->destid == iddest)
            {
                found = 1;
            }
            else
            {
                previous = auxflight;
                flightcostsum = flightcostsum + auxflight->cost;
                auxflight = auxflight->next;
            }
        }

        if (auxflight->destid == iddest) /*Repeated one more time for the last element of the list*/
        {
            found = 1;
        }
        else
        {
            flightcostsum = flightcostsum + auxflight->cost;
        }

        if (found) /*Case where I have to eliminate the flight*/
        {
            if (previous == auxflight) /*Case where the flight is the first one in the list*/
            {
                aux->firstflight = auxflight->next;
                free(previous);
            }
            else
            {
                previous->next = auxflight->next;
                free(auxflight);
            }

            aux->nflights--;
        }
        else /*Case where I have to add the flight*/
        {
            if (aux->nflights == 5) /*Too many flights*/
            {
                printf("KO\n");
                return;
            }

            auxflight->next = malloc(sizeof(flight));
            auxflight = auxflight->next;
            auxflight->destid = iddest;
            auxflight->cost = (flightcostsum + aux->cost) / (aux->nflights + 1);
            auxflight->next = NULL;
            aux->nflights++; /*Increase the number of flights in the hexagon*/
        }
    }

    init_cache();

    printf("OK\n");
}

void change_cost(int x, int y, int value, int radius)
{
    if (hexagonarray == NULL) /*Check if the list has been initialized*/
    {
        printf("KO\n");
        return;
    }

    if (((x >= 0) && (x <= columnsglob - 1)) && ((y >= 0) && (y <= rowsglob - 1)) && radius > 0 && (value >= -10 && value <= 10))
    {
        breadth_first_search(x, y, value, radius);
        init_cache(); /*Re-Initializes the cache after changing the cost, otherwise it might create errors*/
        printf("OK\n");
    }
    else
    {
        printf("KO\n");
    }
}

void breadth_first_search(int x, int y, int value, int radius)
{

    queueentry *queuestart = NULL;
    queueentry *aux = NULL;
    queueentry *queuetail = NULL;
    flight *flightaux = NULL;

    iteration++; /*A new iteration needs this to be increased*/

    int delta = 0;
    float factor = 0;

    int id = -1;
    int distance = 0;
    int size = (radius * radius * 4) + 3;

    /*Creation of the list array*/
    queuestart = calloc(size, sizeof(queueentry));

    if (!queuestart)
    {
        printf("Something went wrong with the allocation of queue\n");
        return;
    }

    aux = queuestart;

    id = calculate_id(x, y);

    aux->id = id;
    aux->distance = 0;

    visited[id] = iteration;

    queuetail = queuestart + 1; /*Always points to the element right after the last one*/

    while (aux != queuetail) /*Whenever aux and queuetail meet it means that all the edges have been processed  && distance < radius*/
    {
        distance = aux->distance;

        for (int k = 0; k < 6; k++)
        {
            if (hexagonarray[aux->id].neighbors[k] != -1 && visited[hexagonarray[aux->id].neighbors[k]] != iteration && distance < radius)
            {
                queuetail->id = hexagonarray[aux->id].neighbors[k]; /*The edge is added to the queue*/
                queuetail->distance = distance + 1; /*The distance is increased by 1*/
                queuetail++;

                visited[hexagonarray[aux->id].neighbors[k]] = iteration; /*The neighbor of the hexagon we're currently examining has been discovered and added to the visited list so that only the minimal distance is taken into consideration*/
            }
        }

        /*Change the cost of the exit*/

        factor = (float)(radius - distance) / (float)radius;
        if (factor < 0.0f)
            factor = 0.0f;

        delta = floor((float)value * factor);

        hexagonarray[aux->id].cost += delta;

        if (hexagonarray[aux->id].cost < 0)
            hexagonarray[aux->id].cost = 0;

        /*Change cost of the flights*/

        flightaux = hexagonarray[aux->id].firstflight;
        while (flightaux != NULL)
        {
            factor = (float)(radius - distance) / (float)radius;
            if (factor < 0.0f)
                factor = 0.0f;

            delta = floor((float)value * factor);
            flightaux->cost += delta;

            if (flightaux->cost < 0)
                flightaux->cost = 0;

            flightaux = flightaux->next;
        }

        aux++; /*Go to the next element in the queue*/
    }

    free(queuestart);
}

void travel_cost(int xp, int yp, int xd, int yd)
{
    if (hexagonarray == NULL) /*Check if the list has been initialized*/
    {
        printf("%d\n", -1);
        return;
    }

    if (xp == xd && yp == yd)
    {
        printf("%d\n", 0); /*Case when the start and the destination are the same*/
        return;
    }
    else if (xp < 0 || xp > columnsglob - 1 || yp < 0 || yp > rowsglob - 1 || xd < 0 || xd > columnsglob - 1 || yd < 0 || yd > rowsglob - 1)
    {
        printf("%d\n", -1);
        return;
    }
    else
    {
        for (int i = 0; i < CACHE_SIZE; i++)
        {
            if (cacheTable[i].start == calculate_id(xp, yp) && cacheTable[i].dest == calculate_id(xd, yd))
            {
                printf("%d\n", cacheTable[i].pathcost);
                return;
            }
        }
    }

    printf("%d\n", dijkstra(xp, yp, xd, yd));
}

int dijkstra(int xs, int ys, int xd, int yd)
{
    heap *heap = NULL;
    heapentry *auxheapentry = NULL;

    flight *auxflight = NULL;

    iteration++;

    int cost = -1;
    int idstart = calculate_id(xs, ys);
    int iddest = calculate_id(xd, yd);
    int newcost = 0;

    heap = create_heap(MIN_HEAP_SIZE);
    auxheapentry = malloc(sizeof(heapentry));

    if (!heap || !auxheapentry)
    {
        printf("Something went wrong with the allocation of the heap structures.\n");
        return -1;
    }

    insert_heap(heap, idstart, 0); /*Starting cost is 0*/
    set_dist(idstart, 0);

    while (heap->size != 0 && cost == -1)
    {
        extract_min(heap, auxheapentry);

        if (auxheapentry->pathcost != get_dist(auxheapentry->id))
            continue;

        if (auxheapentry->id == iddest) /*Early termination*/
        {
            cost = auxheapentry->pathcost;

            free_heap(heap);
            free(auxheapentry);

            insert_into_cache(xs, ys, xd, yd, cost);

            return cost;
        }

        /*If the exitcost is different from 0 then the edges must be added to the heap*/
        if (hexagonarray[auxheapentry->id].cost != 0)
        {
            for (int k = 0; k < 6; k++)
            {
                int v = hexagonarray[auxheapentry->id].neighbors[k];
                if (v == -1)
                    continue;

                newcost = auxheapentry->pathcost + hexagonarray[auxheapentry->id].cost;
                if (newcost < get_dist(v))
                {
                    set_dist(v, newcost);
                    insert_heap(heap, v, newcost);
                }
            }
        }

        if (hexagonarray[auxheapentry->id].firstflight != NULL) /*Check and add all the flights*/
        {
            auxflight = hexagonarray[auxheapentry->id].firstflight;

            while (auxflight != NULL)
            {
                if (auxflight->cost != 0)
                {
                    newcost = auxheapentry->pathcost + auxflight->cost;
                    if (newcost < get_dist(auxflight->destid))
                    {
                        set_dist(auxflight->destid, newcost);
                        insert_heap(heap, auxflight->destid, newcost);
                    }
                }

                auxflight = auxflight->next;
            }
        }
    }

    free_heap(heap);
    free(auxheapentry);

    insert_into_cache(xs, ys, xd, yd, cost);

    return -1; /*The destination heaxagon is unreachable*/
}

static inline int get_dist(int id)
{
    return (visited[id] == iteration) ? dist[id] : INT_MAX;
}

static inline void set_dist(int id, int value)
{
    visited[id] = iteration;
    dist[id] = value;
}

void insert_into_cache(int xs, int ys, int xd, int yd, int cost)
{
    if (cacheposition == CACHE_SIZE)
        cacheposition = 0;

    cacheTable[cacheposition].start = calculate_id(xs, ys);
    cacheTable[cacheposition].dest = calculate_id(xd, yd);
    cacheTable[cacheposition].pathcost = cost;

    cacheposition++;
}

/* Function to create a heap*/
heap *create_heap(int capacity)
{
    heap *heappointer = (heap *)malloc(sizeof(heap));
    if (!heappointer)
        printf("Something went wrong with the heap allocation");

    heappointer->size = 0;
    heappointer->capacity = capacity;
    heappointer->array = (heapentry *)malloc(capacity * sizeof(heapentry));
    if (!heappointer->array)
        printf("Something went wrong with the heap allocation");

    return heappointer;
}

/*Function to swap two hexagons*/
static inline void swap(heapentry *a, heapentry *b)
{
    a->id = a->id + b->id;
    a->pathcost = a->pathcost + b->pathcost;

    b->id = a -> id - b -> id;
    b->pathcost = a -> pathcost - b -> pathcost;

    a -> id = a -> id - b -> id;
    a -> pathcost = a -> pathcost - b -> pathcost;

}

/* Function to heapify the node at index i*/
void min_heapify(heap *heappointer, int i)
{
    int smallest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    /*&heappointer->array[smallest])->cost select the cost from the element at position "smallest" of the heap*/

    if (left < heappointer->size && (&heappointer->array[left])->pathcost < (&heappointer->array[smallest])->pathcost)
        smallest = left;

    if (right < heappointer->size && (&heappointer->array[right])->pathcost < (&heappointer->array[smallest])->pathcost)
        smallest = right;

    if (smallest != i)
    {
        swap(&heappointer->array[i], &heappointer->array[smallest]);
        min_heapify(heappointer, smallest);
    }
}

/*Function to insert a new value into the heap*/
void insert_heap(heap *heap, int id, int value)
{

    if (heap->size == heap->capacity)
    {
        int newcap = heap->capacity * 2;
        heapentry *tmp = realloc(heap->array, newcap * sizeof(heapentry));
        if (!tmp)
        {
            printf("Reallocation not possible\n");
            return;
        }
        heap->array = tmp;
        heap->capacity = newcap;
    }

    heap->size++;
    int i = heap->size - 1;
    (&heap->array[i])->id = id;
    (&heap->array[i])->pathcost = value;

    /* Fix the heap property if it is violated */
    while (i != 0 && (&heap->array[(i - 1) / 2])->pathcost > (&heap->array[i])->pathcost) /*If the father is bigger than the son we need to swap the nodes*/
    {
        swap(&heap->array[i], &heap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

/* Function to extract the root (smallest hexagon)*/
static void extract_min(heap *heap, heapentry *aux)
{

    if (heap->size <= 0)
        return;

    if (heap->size == 1)
    {
        heap->size--;
        aux->id = (&heap->array[0])->id;
        aux->pathcost = (&heap->array[0])->pathcost;
        return;
    }

    aux->id = (&heap->array[0])->id;
    aux->pathcost = (&heap->array[0])->pathcost;

    (&heap->array[0])->id = (&heap->array[heap->size - 1])->id;
    (&heap->array[0])->pathcost = (&heap->array[heap->size - 1])->pathcost;

    heap->size--;
    min_heapify(heap, 0);
}

static inline void free_heap(heap *heap)
{
    free(heap->array);
    free(heap);
}