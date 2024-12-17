#ifndef DJC_ARENA_H_
#define DJC_ARENA_H_

#include <stddef.h>

struct Arena {
  char* name;
  void* base_address;
  void* current;
  size_t page_size;
  size_t pages_committed;
};

struct Arena* arena_create(char* name);
void* arena_alloc(struct Arena* arena, size_t size);
void arena_free(struct Arena* arena);

#endif  // DJC_ARENA_H_
