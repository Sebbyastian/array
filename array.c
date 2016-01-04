#include "array.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#ifdef TEST
#include <ctype.h>
#include <stdio.h>

int casecmp(void const *x, void const *y, size_t size) {
    unsigned char const *X = x
                      , *Y = y;
    size_t z = 0;
    while (z < size && tolower(X[z]) == tolower(Y[z])) {
        z++;
    }
    return z == size ? 0 : (tolower(X[z]) > tolower(Y[z]))
                         - (tolower(X[z]) < tolower(Y[z]));
}

int main(void) {
    typedef char array[42];
    void *ptr = NULL;
    size_t size = 0;

    puts("insert test...");
    insert(&ptr, (array) { "hello"   }, &size, sizeof (array), casecmp);
    insert(&ptr, (array) { "world"   }, &size, sizeof (array), casecmp);
    insert(&ptr, (array) { "goodbye" }, &size, sizeof (array), casecmp);
    insert(&ptr, (array) { "galaxy"  }, &size, sizeof (array), casecmp);

    puts("find test...");
    array *array_ptr = ptr;
    for (size_t x = 0; x < size; x++) {
        assert(find(ptr, array_ptr + x, size, sizeof (array), casecmp) == array_ptr + x);
    }

    puts("find&replace test...");
    void *replace = find(ptr, (array) { "hello" }, size, sizeof (array), casecmp);
    void *with = find_and_replace(ptr, (array) { "HELLO" }, size, sizeof (array), casecmp);
    assert(replace == with);
    assert(!strcmp(replace, "HELLO"));

    puts("find/insert test #1...");
    for (size_t x = 0; x < size; x++) {
        size_t size_before_test = size;
        array *found = find_or_insert(&ptr, array_ptr + x, &size, sizeof (array), casecmp);
        assert(ptr == array_ptr);
        assert(size == size_before_test);
        assert(found == array_ptr + x);
    }

    puts("find&delete test...");
    find_and_delete(ptr, (array) { "HELLO" }, &size, sizeof (array), casecmp);
    assert(find(ptr, (array) { "HELLO" }, size, sizeof (array), casecmp) == NULL);
    
    puts("find/insert test #2...");
    void *inserted = find_or_insert(&ptr, (array) { "HELLO" }, &size, sizeof (array), casecmp);
    assert(find(ptr, (array) { "HELLO" }, size, sizeof (array), casecmp) == inserted);

    puts("insert/replace test #1...");
    void *replaced = insert_or_replace(&ptr, (array) { "hello" }, &size, sizeof (array), casecmp);
    assert(find(ptr, (array) { "hello" }, size, sizeof (array), casecmp) == replaced);
    
    puts("insert/replace test #2...");
    find_and_delete(ptr, (array) { "hello" }, &size, sizeof (array), casecmp);
    inserted = insert_or_replace(&ptr, (array) { "hello" }, &size, sizeof (array), casecmp);
    assert(find(ptr, (array) { "hello" }, size, sizeof (array), casecmp) == inserted);
    
    array first, last;
    strncpy(first, array_ptr[0], sizeof (array));
    strncpy(last,  array_ptr[3], sizeof (array));

    puts("rotate right and carry...");
    rotate_right_and_carry(array_ptr, array_ptr + 3, sizeof *array_ptr);
    assert(strncmp(last, array_ptr[0], sizeof (array)) == 0);

    puts("rotate left and carry...");
    rotate_left_and_carry(array_ptr, array_ptr + 3, sizeof *array_ptr);
    assert(strncmp(first, array_ptr[0], sizeof (array)) == 0);

    puts("Tests completed... :)");
    for (size_t x = 0; x < size; x++) {
        puts(array_ptr[x]);
    }
}
#endif

void *push_back(void **base, void const *value, size_t *nelem, size_t size) {
    typedef unsigned char array[size];
    array *b = *base;
    if (SIZE_MAX / sizeof *b <= *nelem) {
        return NULL;
    }
    
    if (*nelem & -~*nelem ? 0 : 1) {
        b = realloc(b, (*nelem * 2 + 1) * sizeof *b);
        if (!b) {
            return NULL;
        }
        *base = b;
    }

    b += (*nelem)++;
    return value
         ? memmove(b, value, sizeof *b)
         : b;
}

void memswap(void *x, void *y, size_t size) {
    for (unsigned char *X = x, *Y = y; size > 0; X++, Y++, size--) {
        unsigned char c = *X;
        *X = *Y;
        *Y = c;
    }
}

void rotate_left_and_carry(void *x, void *y, size_t size) {
    typedef unsigned char array[size];
        for (array *t = x; x < y; x = ++t) {
        memswap(x, t+1, size);
    }
}

void rotate_right_and_carry(void *x, void *y, size_t size) {
    typedef unsigned char array[size];
    for (array *t = x; x < y; x = ++t) {
        memswap(x, y, size);
    }
}

int insertion_compare(void const *x, void const *y) {
    struct insertion_cursor *insertion_cursor = (void *) x;
    int comparison = insertion_cursor->compare(insertion_cursor->value, y, insertion_cursor->size);
    if (comparison <= 0) {
        insertion_cursor->position = (void *) y;
    }
    return comparison;
}

void *find_nearest(void const *base, void const *value, size_t nelem, size_t size, comparer *compare) {
    struct insertion_cursor cursor = { .value =    (void *) value
                                     , .position =          NULL
                                     , .size =              size
                                     , .compare =           compare};

    struct resource *r = bsearch(&cursor, base, nelem, size, insertion_compare);
    return r ? r : cursor.position;
}


void *find(void const *base, void const *value, size_t nelem, size_t size, comparer *compare) {
    void *r = find_nearest(base, value, nelem, size, compare);
    return r && !compare(r, value, size) ? r : NULL;
}

int find_and_delete(void *base, void const *value, size_t *nelem, size_t size, comparer *compare) {
    void *r = find_nearest(base, value, *nelem, size, compare);
    if (!r || compare(r, value, size)) {
        return 0;
    }

    delete(base, r, nelem, size);
    return 1;
}

void *find_and_replace(void *base, void const *value, size_t nelem, size_t size, comparer *compare) {
    typedef unsigned char array[size];
    void *r = find_nearest(base, value, nelem, size, compare);
    return r && !compare(r, value, size) ? memmove(r, value, size) : NULL;
}

void *insert_at(void **base, void *destination, void *value, size_t *nelem, size_t size) {
    typedef unsigned char array[size];
    array *b = *base, *d = destination;
    size_t x = destination ? d - b : SIZE_MAX;
    value = push_back(base, value, nelem, size);
    if (!value) {
        return NULL;
    }
    
    b = *base;
    d = x < SIZE_MAX ? b + x : value;
    rotate_right_and_carry(d, value, size);
    return d;
}

void *find_or_insert(void **base, void const *value, size_t *nelem, size_t size, comparer *compare) {
    typedef unsigned char array[size];
    array *b = *base, *r = find_nearest(b, value, *nelem, size, compare);
    if (r && !compare(r, value, size)) {
        return r;
    }
    return insert_at(base, r, (void *) value, nelem, size);
}

void *insert(void **base, void const *value, size_t *nelem, size_t size, comparer *compare) {
    typedef unsigned char array[size];
    array *b = *base, *r = find_nearest(b, value, *nelem, size, compare);
    if (r && !compare(r, value, size)) {
        return NULL;
    }
    return insert_at(base, r, (void *) value, nelem, size);
}

void *insert_or_replace(void **base, void const *value, size_t *nelem, size_t size, comparer *compare) {
    typedef unsigned char array[size];
    array *b = *base, *r = find_nearest(b, value, *nelem, size, compare);
    if (r && !compare(r, value, size)) {
        return memmove(r, value, size);
    }
    return insert_at(base, r, (void *) value, nelem, size);
}

void delete(void *base, void const *value, size_t *nelem, size_t size) {
    typedef unsigned char array[size];
    array *b = base, *v = (void *) value;
    (*nelem)--;
    memmove(v, v + 1, (b + *nelem - v) * sizeof *b);
}
