#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "life.h"
#include "ADTVector.h"


/*  Create functions  */

static int* create_int(int value)
{
    int* p = malloc(sizeof(int));
    *p = value;
    return p;
}


static LifeCell* create_cell(LifeCell pos)
{
    LifeCell* cel = malloc(sizeof(LifeCell));
    cel->x = pos.x;
    cel->y = pos.y;
    return cel;

}


/*  compare function  */

static int compare_LifeCells(Pointer a, Pointer b) // Arranges them from upper left to down right
{
    LifeCell c = *(LifeCell*)a;
    LifeCell d = *(LifeCell*)b;

    if (c.y < d.y)
        return -1;
    else if (c.y == d.y)
        return c.x-d.x;
    
    return 1;
}


static int compare_states(Pointer a, Pointer b)
{
    // Since we only care about if the states are the same or not, 
    // we only need to return 0 when they are the same, 
    // otherwise if we find one cell different we return 1
    LifeState state1 = a;
    LifeState state2 = b;

    if (map_size(state1) == map_size(state2))
    {   
        for(MapNode node1 = map_first(state1), node2 = map_first(state2); 
            node1 != MAP_EOF; 
            node1 = map_next(state1, node1), node2 = map_next(state2, node2)) 
        { 
            LifeCell* cell1 = map_node_key(state1, node1);
            LifeCell* cell2 = map_node_key(state2, node2);

            if (compare_LifeCells(cell1, cell2) != 0)
                return 1;
            
        }

        return 0;
    }

    return 1;
    
}

/*   other functions   */

static Vector create_neighbours(Vector vec, int x, int y)
{
    LifeCell neighbours;
    vec = vector_create(0, free);

    for (int i = -1; i <= 1; i++)
    {
        neighbours.x = x + i;

        for (int j = -1; j <= 1; j++)
        {
            if (!j && !i)   // Skip the cell itself
                continue;

            neighbours.y = y + j;
            vector_insert_last(vec, create_cell(neighbours));
        }
    }

    return vec;
}


static void write_line(LifeState state, MapNode node, int size, FILE* fp, MapNode end_node)
{
    LifeCell* cell = map_node_key(state, node);
    
    if (cell->x == 1)
        fprintf(fp, "b");
    else if (cell->x > 1)
        fprintf(fp, "%db", cell->x);
    

    int i = 0;
    
    MapNode next_node = node;
    while (i < size)
    {
        next_node =  map_next(state, next_node);
        if (next_node == end_node)
        {
            fprintf(fp, "o");
            break;
        }
        else
        {
            
            LifeCell* next_cell = map_node_key(state, next_node);
            
            int count = 1;
            
            while (next_cell->x == cell->x + 1)
            {
                node = map_next(state, node);
                cell = map_node_key(state, node);
                next_node = map_next(state, next_node);
                if (next_node != MAP_EOF) next_cell = map_node_key(state, next_node);
                    count++;
            }
            
            if (count == 1)
                fprintf(fp, "o");
            else
                fprintf(fp, "%do", count);
            
            int num = next_cell->x - cell->x - 1;

            if (next_node != MAP_EOF)
            {
                if (num == 1)
                    fprintf(fp, "b");
                else if (num > 0)
                    fprintf(fp, "%db", next_cell->x - cell->x - 1);
            }

            
            i += count;
            node = map_next(state, node);

            if (node != MAP_EOF) 
                cell = map_node_key(state, node);

        }
    }

}


static void update_neighbours(LifeState state)
{
    
    /*   we need the values to be 0 before doing this, which we ensure they'll be by the way we insert   */

    for(MapNode node = map_first(state); node != MAP_EOF; node = map_next(state, node)) 
    { 
        LifeCell* key = map_node_key(state, node);
        int x = key->x;
        int y = key->y;
        int neib = 0;

        Vector neighbours = create_neighbours(neighbours, x, y);            
        
        for (int k = 0; k < 8; k++)
        {
            MapNode node_temp = map_find_node(state, vector_get_at(neighbours, k));

            if (node_temp != NULL)
                neib++;            
        }
        
        vector_destroy(neighbours);
        map_insert(state, create_cell(*key), create_int(neib));
    }
}


static LifeState shift(LifeState state, int hor, int vert)
{
    /* mayber check if hor = vert = 0 but i think thats never the case */
    LifeState final = map_create(compare_LifeCells, free, free);    // We shift the map by -hor, horizontally, and -vert, vertically

    for(MapNode node = map_first(state); node != MAP_EOF; node = map_next(state, node)) 
    {
        LifeCell* name = map_node_key(state, node); 

        LifeCell es;
        es.x = name->x - hor;
        es.y = name->y - vert;
        map_insert(final, create_cell(es), create_int(0));
    }

    return final;
    
}


static LifeState bs(LifeState new_state)
{
    /* shifting */
    MapNode node = map_first(new_state);
    
    int vert_shift = ((LifeCell*)map_node_key(new_state, node))->y;
    int hor_shift = ((LifeCell*)map_node_key(new_state, node))->x;


    for( ; node != MAP_EOF; node = map_next(new_state, node)) 
    { 
        int current = ((LifeCell*)map_node_key(new_state,node))->x;

        if (hor_shift > current)
            hor_shift = current;
        
    }
        
    LifeState final = shift(new_state, hor_shift, vert_shift);

    update_neighbours(final);

    return final;
}


/*   Header functions   */

LifeState life_create()
{
    Map map = map_create(compare_LifeCells, free, free); // We have defined LifeState as a Map so we create accordinly.
    assert(map);

    return map;  
}


LifeState life_create_from_rle(char* file)
{
    assert(file);

    LifeState state = life_create();

    int line = 0;
    
    FILE* fp = fopen(file, "r");
    assert(fp);

    char c;

	while ((c = fgetc(fp)) != '!')
    {
        LifeCell pos;   // Dummy LifeCell to pass to the function
        pos.y = line;
        int row = -1;  

        while (c != '$')
        {
            if (c == '!')
            {
                update_neighbours(state);
                fclose(fp);
                return state;
            }
            else if (c <= '9') // With the given format this ensures we read a number 
            {
                int times = c - '0'; // We get a decimal of the number
            

                while ((c = fgetc(fp)) <= '9' && c != '$')    // Convert multi digit number to one 
                    times = times * 10 + c - '0';

                        
                if (c == 'o') // if the next char is o it means we need to create (times) alive cells 
                {
                    for (int k = 0; k < times; k++)
                    {
                        pos.x = ++row;
                        map_insert(state, create_cell(pos), create_int(0));
                    }
                }
                else if (c == '$')
                {
                    line += times;
                    break;
                }
                else    // else we just skip (times) rows
                    row += times;
                
            }
            else if (c == 'o') // if its not a number and it is o we have to create exacly one alive cell
            {
                pos.x = ++row;
                map_insert(state, create_cell(pos), create_int(0));
            }
            else    // at this point we are sure it's b so we just skip exaclyt one row
                row++;
            

            c = fgetc(fp);
        }

        line++; 
	}
    
    
    update_neighbours(state);
    fclose(fp);

    return state;

}


void life_save_to_rle(LifeState final, char* file)
{
    assert(final && file);
    LifeState state = bs(final);
    
    FILE* fp = fopen(file, "w");

    if (fp == NULL)
    {
        printf("NULL\n");
    }

    int lines = 0;
    int nodes = -1;


    MapNode start_at = map_first(state);

    for(MapNode node = map_first(state); node != MAP_EOF; node = map_next(state, node)) 
    {
        
        nodes++;

        if (((LifeCell*)map_node_key(state, node))->y != lines)
        {
            int y = ((LifeCell*)map_node_key(state, node))->y;

            if (y != lines + 1)
            {
                write_line(state, start_at, nodes, fp, node);
                fprintf(fp, "%d$", y - lines - 1);
                lines = y - 1;
                
            }
            else
            {
                write_line(state, start_at, nodes, fp, node);
                fprintf(fp, "$");
            }

            
           
            start_at = map_find_node(state, ((LifeCell*)map_node_key(state, node)));
            lines++;
            nodes = 0;
        }        
    }
    
    
    write_line(state, start_at, nodes + 1, fp, MAP_EOF);
    
    fprintf(fp, "!");

    fclose(fp);

    life_destroy(state);
}


bool life_get_cell(LifeState state, LifeCell cell)
{
    MapNode node = map_find_node(state, &cell);

    if (node != NULL)
        return true;

    return false;
}


void life_set_cell(LifeState state, LifeCell cell, bool value)
{
    assert(state);

    MapNode node= map_find_node(state, &cell);  // see if the cell exists
    if (value == true && node == NULL)  // if it doesnt and we need to make it alive we do it
    {
        map_insert(state, create_cell(cell), create_int(0));
        update_neighbours(state);
    }
    else if (value == false && node != NULL)    // if it does and ewe need to kill it we do
    {
        map_remove(state, &cell);
        update_neighbours(state);
    }
}


LifeState life_evolve(LifeState state)
{
    assert(state);
    LifeState new_state = map_create(compare_LifeCells, free, free);
    Set done_cells = set_create(compare_LifeCells, free);   // We use this to save time by not checking already checked cells

    
    for(MapNode node = map_first(state); node != MAP_EOF; node = map_next(state, node)) 
    { 
        LifeCell* key = map_node_key(state, node);
  
        int* value = map_node_value(state, node);
        
        // If cell is eligible copy it to the new state
        if (*value == 2 || *value == 3)
            map_insert(new_state, create_cell(*key), create_int(0));
        
        int x = key->x;
        int y = key->y;

        Vector neighbours = create_neighbours(neighbours, x, y);    // Neighbours of the alive cell

        for (int k = 0; k < 8; k++)
        {
            LifeCell* cel = vector_get_at(neighbours, k);
            SetNode is_done = set_find_node(done_cells, cel);
            if (is_done == NULL && map_find_node(state, cel) == NULL)   // if we havent check it before and it doesnt belong to the original state
            {
                Vector neighbours2 = create_neighbours(neighbours2, cel->x, cel->y);

                int neib = 0;
                int j = 0;

                while(neib <= 3 && j < 8)
                    neib += life_get_cell(state, *(LifeCell*)vector_get_at(neighbours2, j++));
                
                if (neib == 3)
                    map_insert(new_state, create_cell(*cel), create_int(0));

                vector_destroy(neighbours2);    // Leak prevention
            
            }
            
        }
        vector_destroy(neighbours); // Leak prevention
    }

    
    set_destroy(done_cells);    // Leak prevention
    update_neighbours(new_state);

    return new_state;   // if we made it here the map is empty or it doesnt need shifting and we just return new_state
}


void life_destroy(LifeState state)
{
    // No need to use assert here, map module does it
    map_destroy(state);
}


List life_evolve_many(LifeState state, int steps, ListNode* loop)
{
    assert(state);
    List list = list_create(NULL);

    list_insert_next(list, LIST_BOF, state);

    ListNode node = list_first(list);

    *loop = NULL;

    for (int i = 0; i < steps; i++)
    {

        list_insert_next(list, node, life_evolve(list_node_value(list, node)));
        
        for (ListNode temp = list_first(list); temp != list_last(list); temp = list_next(list, temp))
        {
            int comp = compare_states(list_node_value(list, list_last(list)), list_node_value(list, temp));

            if (comp == 0)
            {
                LifeState ls = list_node_value(list, list_last(list));

                list_remove_next(list, node);

                life_destroy(ls);

                *loop = temp; 

                break;
            }

        }
        node = list_next(list, node);
    }

    return list;
    
}

