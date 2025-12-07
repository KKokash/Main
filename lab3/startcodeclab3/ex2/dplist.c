#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "dplist.h"



/*
 * The real definition of struct list / struct node
 */
struct dplist_node {
    dplist_node_t *prev, *next;
    element_t element;
};

struct dplist {
    dplist_node_t *head;
    // more fields will be added later
};

dplist_t *dpl_create() {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    list->head = NULL;
  return list;
}

void dpl_free(dplist_t **list) {
    if (list == NULL || *list == NULL)
    {
        return;
    }
    if ((*list)->head == NULL)
    {
        free(*list);
        return;
    }
    int size = dpl_size(*list);
    dplist_node_t *nodes[size];
    dplist_node_t *dummy = (*list)->head;;
    for (int i = 0; i < size; i++)
    {
        nodes[i] = dummy;
        dummy = dummy->next;
    }
    for ( int i = 0; i < size; i++)
    {
        free(nodes[i]);
    }

    free(*list);
    *list = NULL;
    //Do extensive testing with valgrind.  k

}

/* Important note: to implement any list manipulation operator (insert, append, delete, sort, ...), always be aware of the following cases:
 * 1. empty list ==> avoid errors
 * 2. do operation at the start of the list ==> typically requires some special pointer manipulation
 * 3. do operation at the end of the list ==> typically requires some special pointer manipulation
 * 4. do operation in the middle of the list ==> default case with default pointer manipulation
 * ALWAYS check that you implementation works correctly in all these cases (check this on paper with list representation drawings!)
 **/


dplist_t *dpl_insert_at_index(dplist_t *list, element_t element, int index) {
    dplist_node_t *ref_at_index, *list_node;
    if (list == NULL) return NULL;

    list_node = malloc(sizeof(dplist_node_t));

    list_node->element = element;
    // pointer drawing breakpoint
    if (list->head == NULL) { // covers case 1
        list_node->prev = NULL;
        list_node->next = NULL;
        list->head = list_node;
        // pointer drawing breakpoint
    } else if (index <= 0) { // covers case 2
        list_node->prev = NULL;
        list_node->next = list->head;
        list->head->prev = list_node;
        list->head = list_node;
        // pointer drawing breakpoint
    } else {
        ref_at_index = dpl_get_reference_at_index(list, index);
        assert(ref_at_index != NULL);
        // pointer drawing breakpoint
        if (index < dpl_size(list)) { // covers case 4
            list_node->prev = ref_at_index->prev;
            list_node->next = ref_at_index;
            ref_at_index->prev->next = list_node;
            ref_at_index->prev = list_node;
            // pointer drawing breakpoint
        } else { // covers case 3
            assert(ref_at_index->next == NULL);
            list_node->next = NULL;
            list_node->prev = ref_at_index;
            ref_at_index->next = list_node;
            // pointer drawing breakpoint
        }
    }
    return list;
}

dplist_t *dpl_remove_at_index(dplist_t *list, int index) {
    int isEnd = 0;
    if (list==NULL){return NULL;}
    if (list->head == NULL){return list;}
    int size = dpl_size(list)-1;
    dplist_node_t *targetNode = list->head;
    if (index>=size)
    {
        index = size;
        isEnd = 1;
    }
    if (index<=0)
    {
        list->head = list->head->next;
        if (list->head != NULL){list->head->prev=NULL;}
    }

    else
    {
        for (int i=0; i<index;i++)
        {
            targetNode = targetNode->next;
        }
        if (isEnd)
        {
            targetNode->prev->next = NULL;
        }
        else
        {
            targetNode->prev->next = targetNode->next;
            targetNode->next->prev = targetNode->prev;
        }

    }
    free(targetNode);
    return list;

}

int dpl_size(dplist_t *list) {
    if (list==NULL){return -1;}
    if (list->head==NULL){return 0;}
    int size =1;
    dplist_node_t *targetNode = list->head;
    while (1)
    {
        if (targetNode->next == NULL){return size;}
        size++;
        targetNode = targetNode->next;
    }
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    if (list == NULL){return NULL;}
    if (list->head==NULL){return NULL;}
    if (index<=0){return list->head;}
    if (index>dpl_size(list)-1){index = dpl_size(list)-1;}
    dplist_node_t *targetNode = list->head;
    for (int i=0; i<index; i++)
    {
        targetNode = targetNode->next;
    }
    return targetNode;
}

element_t dpl_get_element_at_index(dplist_t *list,  int index) {
    dplist_node_t *ref = dpl_get_reference_at_index(list, index);
    return (ref==NULL)? NULL : ref->element;
}

int dpl_get_index_of_element(dplist_t *list, element_t element) {
    int index = 0;
    int found = 0;
    if (list == NULL){return -1;}
    dplist_node_t *targetNode = list->head;
    for (int i=0; i<dpl_size(list); i++)
    {
        if (targetNode->element == element){found=1;break;}
        index++;
        targetNode = targetNode->next;
    }
    if (found){return index;}
    return -1;
}



