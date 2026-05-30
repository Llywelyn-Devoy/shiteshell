#include "shite_vec.h"
#include "shared.h"
#include <assert.h>
#include <stdlib.h>

#define SHITEVEC_INIT_CAPACITY 16

ShiteVec shitevec_create() {
    ShiteVec vec = {.count = 0, .capacity = 0, .elems = malloc(0)};
    return vec;
}

size_t shitevec_count(ShiteVec *vec) { return vec->count; }

static void _resize(ShiteVec *vec, size_t new_capacity) {
    vec->capacity = new_capacity;
    vec->elems = realloc(vec->elems, sizeof(vec->elems[0]) * new_capacity);
    // Ehh? This is a bit unsafe
    vec->count = new_capacity < vec->count ? new_capacity : vec->count;
}

void shitevec_push_back(ShiteVec *vec, void *elem) {
    if (vec->count == vec->capacity) {
        _resize(vec, vec->capacity == 0 ? SHITEVEC_INIT_CAPACITY
                                        : vec->capacity * 2);
    }

    vec->elems[vec->count++] = elem;
}

void *shitevec_pop_back(ShiteVec *vec) {
    if (vec->count == 0)
        // NOTE: this doesn't allow for NULLs to be stored in the shite vec :(
        return NULL;

    return vec->elems[--vec->count];
}

void shitevec_free(ShiteVec *vec) { shite_free((void **)&vec->elems); }
