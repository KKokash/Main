//
// Created by karam on 12/14/25.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};


dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

void dpl_free(dplist_t **list, bool free_element) {
    if (list == NULL || *list == NULL) return;

    dplist_node_t *current = (*list)->head;
    while (current != NULL) {
        dplist_node_t *next = current->next;
        if (free_element && (*list)->element_free)
            (*list)->element_free(&current->element);
        free(current);
        current = next;
    }

    free(*list);
    *list = NULL;
}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {
    dplist_node_t *ref_at_index, *list_node;
    if (list == NULL) return NULL;

    list_node = malloc(sizeof(dplist_node_t));
    if (insert_copy)
    {
        list_node->element = list->element_copy(element);
    }
    else
    {
        list_node->element = element;
    }
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

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {
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
    if (free_element && list->element_free){list->element_free(&targetNode->element);}
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

void *dpl_get_element_at_index(dplist_t *list, int index) {
    dplist_node_t *ref = dpl_get_reference_at_index(list, index);
    return (ref==NULL)? NULL : ref->element;
}

int dpl_get_index_of_element(dplist_t *list, void *element) {
    int index = 0;
    int found = 0;
    if (list == NULL){return -1;}
    dplist_node_t *targetNode = list->head;
    for (int i=0; i<dpl_size(list); i++)
    {
        if (list->element_compare(element,targetNode->element)==0){found=1;break;}
        index++;
        targetNode = targetNode->next;
    }
    if (found){return index;}
    return -1;

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

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {
    if (list == NULL || reference == NULL||list->head==NULL){return NULL;}
    int found = 0;
    dplist_node_t *targetNode = list->head;
    for (int i=0; i<dpl_size(list); i++)
    {
        if (targetNode == reference){found=1;break;}
        targetNode = targetNode->next;
    }
    if (found){return reference->element;}
    return NULL;
}
