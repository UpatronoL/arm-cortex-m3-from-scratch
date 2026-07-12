#include <stdint.h>

#define MAXSIZE 64

typedef struct Ring_Buffer {
    uint32_t size;
    volatile uint32_t head;
    volatile uint32_t tail;
    char array[MAXSIZE];
} ring_buffer;

void rb_push(ring_buffer *rb, char c);
char rb_pop(ring_buffer *rb);
_Bool rb_is_empty(ring_buffer *rb);
_Bool rb_is_full(ring_buffer *rb);
