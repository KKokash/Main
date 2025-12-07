#define _GNU_SOURCE
/**
 * \author Bert Lagaisse
 *
 * main method that executes some test functions (without check framework)
 */

#include <stdio.h>
#include "dplist.h"
#include  <stdbool.h>

void ck_assert_msg(bool result, char * msg){
    if(!result) printf("%s\n", msg);
}
int main(void)
{
    dplist_t *numbers = NULL;
    numbers = dpl_create();

    ck_assert_msg(numbers != NULL, "numbers = NULL, List not created");
    ck_assert_msg(dpl_size(numbers) == 0, "Numbers may not contain elements.");
     char * name = "bruv";
    dpl_insert_at_index(numbers, name, 0);
    ck_assert_msg(dpl_size(numbers) == 1, "Numbers must contain 1 element.");
    name = "bav";
    dpl_insert_at_index(numbers, name, -1);
    ck_assert_msg(dpl_size(numbers) == 2, "Numbers must contain 2 elements.");
    name = "brom";
    dpl_insert_at_index(numbers, name, 100);
    ck_assert_msg(dpl_size(numbers) == 3, "Numbers must contain 3 elements.");
    printf("%s",dpl_get_element_at_index(numbers,0));
    dpl_free(&numbers);

    return 0;
}