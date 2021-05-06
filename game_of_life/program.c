#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "life.h"

#define EVOLUTIONS 100


struct stat st = {0};


int main()
{
    
    LifeState state = life_create_from_rle("states/pattern(2).rle"); // Starting state

    ListNode node_null;

    List list = life_evolve_many(state, EVOLUTIONS, &node_null);

    int i = 0;

    if (node_null != NULL)
        printf("The starting state results in an infinite loop!\n");
    

    for(ListNode node = list_first(list);   // Write each step to a file and clear memory
        node != LIST_EOF; 
        node = list_next(list, node), i++) 
    { 
        LifeState ls = list_node_value(list, node);

        if (i == EVOLUTIONS)
        {
            char* name = malloc(13 + (EVOLUTIONS + 9) / 10);
            sprintf(name, "steps/mystep%d", i);

            life_save_to_rle(ls, name);

            free(name);
        }
        
        //life_destroy(ls);   // Leak prevention
    }

    list_destroy(list); // kanei free 2 parapano fores

}


