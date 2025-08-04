/*
 * list.h
 *
 * Definition of the represenation of a list.
 *
 */

#ifndef LIST_H

#define LIST_H

typedef struct list {
    void *head; //Enthält einen Zeiger auf ein Element (eines beliebigen Typs)
    struct list *tail;
} List;

List * list_append(void * head, List * tail);

#endif /* end of include guard: LIST_H */
