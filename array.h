#ifndef INCLUDE_ARRAY_H
#define INCLUDE_ARRAY_H
#include <stddef.h>

typedef int comparer(void const *, void const *, size_t);
struct insertion_cursor { void     *value;
                          void     *position;
                          size_t    size;
                          comparer *compare; };
                          
void  memswap(void *, void *, size_t);
void  rotate_left_and_carry(void *, void *, size_t);
void  rotate_right_and_carry(void *, void *, size_t);
int   insertion_compare(void const *, void const *y);
void *push_back(void **, void const *, size_t *, size_t);
void *find_nearest(void const *, void const *, size_t, size_t, comparer *);
void *find(void const *, void const *, size_t, size_t, comparer *);
int   find_and_delete(void *, void const *, size_t *, size_t, comparer *);
void *find_and_replace(void *, void const *, size_t, size_t, comparer *);
void *find_or_insert(void **, void const *, size_t *, size_t, comparer *);
void *insert(void **, void const *, size_t *, size_t, comparer *);
void *insert_or_replace(void **, void const *, size_t *, size_t, comparer *);
void  delete(void *, void const *, size_t *, size_t);
#endif