#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "life.h"
#include "bmp.h"
#include "gif.h"

void find_dim(List states, int* max_x, int* max_y, int* min_x, int* min_y)
{
    // Finds the min, max coordinates of the states
    int mx = -1;
    int my = -1;

    int mnx = 0;
    int mny = 0;

    for(ListNode node = list_first(states); node != LIST_EOF; node = list_next(states, node)) 
    { 
        LifeState ls = list_node_value(states, node);

        if (map_size(ls) == 0)
            break;
            
        int y = ((LifeCell*)map_node_key(ls, map_first(ls)))->y;

        if (y < mny)
            mny = y;

        for (MapNode mnode = map_first(ls);
             mnode != MAP_EOF;
             mnode = map_next(ls, mnode))
        {
            LifeCell* cell = map_node_key(ls, mnode);
            int x = cell->x;
            y = cell->y;
    
            if (x > mx)
                mx = x;
            else if (x < mnx)
                mnx = x;

        }
        if (y > my)
            my = y;
    }

    *max_x = mx;
    *max_y = my;
    *min_x = mnx;
    *min_y = mny;
}


bool cell_exists(LifeState state, LifeCell* cell)
{
    return map_find(state, cell) != NULL;
}


List evolve_many(LifeState state, int steps)
{
    assert(state);
    List list = list_create(NULL);

    list_insert_next(list, LIST_BOF, state);

    ListNode node = list_first(list);

    for (int i = 0; i < steps; i++,  node = list_next(list, node))
        list_insert_next(list, node, life_evolve(list_node_value(list, node)));

    return list;
    
}

int main(int argc, char *argv[])
{
    assert(argv[6]);

	int frames = atoi(argv[2]);
    assert(frames >= 1);

    int cell_size = atoi(argv[3]);

	int speed = atoi(argv[4]);

	int delay = atoi(argv[5]);


    LifeState state = life_create_from_rle(argv[1]); // Starting state

    List list = evolve_many(state, frames);

    int max_x = 0;
    int max_y = 0;
    int min_x = 0;
    int min_y = 0;

    find_dim(list, &max_x, &max_y, &min_x, &min_y);


    // We need to be able to paint all the cells of all the states  
    int w = (max_x - min_x + 1) * cell_size;
    int h = (max_y - min_y + 1) * cell_size; 


	GIF* gif = gif_create(w, h);
	Bitmap* bitmap = bm_create(w, h);

    // First two numbers give the tranasparency and the other six are the hex color code
	#ifdef SHOW_EVOLUTION
        unsigned int palette[] = { 0xFF000000, 0xFFFFFFF, 0xFF35fc03, 0xFFc3103}; 
        gif_set_palette(gif, palette, 4);
    #else
        unsigned int palette[] = { 0xFF000000, 0xFFFFFFF}; 
        gif_set_palette(gif, palette, 2);
    #endif

	// Default καθυστέρηση μεταξύ των frames, σε εκατοστά του δευτερολέπτου
	gif->default_delay = delay;



    for(ListNode node = list_first(list); node != LIST_EOF; ) 
    { 
        LifeState ls = list_node_value(list, node);

        bm_set_color(bitmap, bm_atoi("white"));
		bm_clear(bitmap);
        bm_set_color(bitmap, bm_atoi("black"));

        for (MapNode mnode = map_first(ls); mnode != MAP_EOF; mnode = map_next(ls, mnode))
        {
            LifeCell* cell = map_node_key(ls, mnode);
            
            int x = (cell->x - min_x) * cell_size;
            int y = (cell->y - min_y) * cell_size;
            bm_fillrect(bitmap, x, y, x + cell_size - 1, y + cell_size - 1);

        }

        gif_add_frame(gif, bitmap);
        

        #ifdef SHOW_EVOLUTION

            ListNode next_node = list_next(list, node);
            
            if (speed == 1 && next_node)
            {

                LifeState next = list_node_value(list, next_node);

                bm_set_color(bitmap, 0xFFc3103);

                for (MapNode mnode = map_first(ls); mnode != MAP_EOF; mnode = map_next(ls, mnode))
                {
                    LifeCell* cell = map_node_key(ls, mnode);

                    if (!cell_exists(next, cell))
                    {
                        int x = (cell->x - min_x) * cell_size;
                        int y = (cell->y - min_y) * cell_size;
                        bm_fillrect(bitmap, x, y, x + cell_size - 1, y + cell_size - 1);
                    
                    }
                }
                
                gif_add_frame(gif, bitmap);

                bm_set_color(bitmap, 0xFF35fc03);

                for (MapNode mnode = map_first(next); mnode != MAP_EOF; mnode = map_next(next, mnode))
                {
                    LifeCell* cell = map_node_key(next, mnode);

                    if (!cell_exists(ls, cell))
                    {

                        int x = (cell->x - min_x) * cell_size;
                        int y = (cell->y - min_y) * cell_size;
                        bm_fillrect(bitmap, x, y, x + cell_size - 1, y + cell_size - 1);
                    }

                    
                }
            
                gif_add_frame(gif, bitmap);

            }

        #endif

       
        life_destroy(ls);   // Leak prevention

        for (int i = 0; node != LIST_EOF && i < speed; i++)
            node = list_next(list, node);

    }

    gif_save(gif, argv[6]);

	// Αποδέσμευση μνήμης
	bm_free(bitmap);
	gif_free(gif);

    list_destroy(list); // kanei free 2 parapano fores


}

