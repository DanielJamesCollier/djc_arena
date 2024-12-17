#ifndef DJC_ARENA_H_
#define DJC_ARENA_H_

#include <stddef.h>

struct Arena {
  size_t page_size;
  size_t page_limit;
  size_t pages_committed;
  char* name;
  void* base_address;
  void* current;
};

struct Arena* arena_create(char* name, size_t page_limit);
void* arena_alloc(struct Arena* arena,
                  size_t sizeof_element,
                  size_t num_elements);
void arena_free(struct Arena* arena);

#endif  // DJC_ARENA_H_
