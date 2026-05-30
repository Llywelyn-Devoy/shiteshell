#include <stdlib.h>

typedef struct {
    size_t count;
    size_t capacity;
    void **elems;
} ShiteVec;

ShiteVec shitevec_create();

size_t shitevec_count(ShiteVec *vec);

void shitevec_push_back(ShiteVec *vec, void *elem);

void *shitevec_pop_back(ShiteVec *vec);

void shitevec_free(ShiteVec *vec);
