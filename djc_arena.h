#ifndef DJC_ARENA_H_
#define DJC_ARENA_H_

#define ARENA_PAGE_LIMIT 80

typedef struct Arena {
  void* base_address;
  void* current_page;
  DWORD page_size;
  DWORD pages_committed;
} Arena;

Arena* arena_create();
void* arena_alloc(Arena* arena, size_t size);
void arena_free(Arena* arena);

#endif  // DJC_ARENA_H_
