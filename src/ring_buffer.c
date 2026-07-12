#include "ring_buffer.h"

void rb_push(ring_buffer *rb, char c){
    rb->array[rb->head] = c;
    rb->head = (rb->head + 1) & (MAXSIZE - 1);
}

char rb_pop(ring_buffer *rb){
    char c = rb->array[rb->tail];
    rb->tail = (rb->tail + 1) & (MAXSIZE - 1);
    return c;
}
_Bool rb_is_empty(ring_buffer *rb){
    return (rb->head == rb->tail);
}
_Bool rb_is_full(ring_buffer *rb){
    return (((rb->head + 1) & (MAXSIZE - 1)) == rb->tail);
}
