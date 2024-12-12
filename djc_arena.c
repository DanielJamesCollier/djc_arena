#include "djc_arena.h"

#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>

static INT arena_page_fault_handler(Arena* arena, DWORD dwCode) {
  if (dwCode != EXCEPTION_ACCESS_VIOLATION) {
    return EXCEPTION_EXECUTE_HANDLER;
  }

  if (arena->pages_committed >= ARENA_PAGE_LIMIT) {
    return EXCEPTION_EXECUTE_HANDLER;
  }

  void* result = VirtualAlloc(arena->current_page, arena->page_size, MEM_COMMIT,
                              PAGE_READWRITE);

  if (result == NULL) {
    return EXCEPTION_EXECUTE_HANDLER;
  }

  arena->pages_committed++;
  arena->current_page = (char*)arena->current_page + arena->page_size;

  return EXCEPTION_CONTINUE_EXECUTION;
}

Arena* arena_create() {
  SYSTEM_INFO sys_info;
  GetSystemInfo(&sys_info);

  Arena* arena = (Arena*)malloc(sizeof(Arena));
  if (!arena) {
    return NULL;
  }

  arena->page_size = sys_info.dwPageSize;
  arena->pages_committed = 0;

  arena->base_address = VirtualAlloc(NULL, ARENA_PAGE_LIMIT * arena->page_size,
                                     MEM_RESERVE, PAGE_NOACCESS);

  if (!arena->base_address) {
    free(arena);
    return NULL;
  }

  arena->current_page = arena->base_address;
  return arena;
}

void* arena_alloc(Arena* arena, size_t size) {
  size_t offset = (char*)arena->current_page - (char*)arena->base_address;

  if (offset + size > ARENA_PAGE_LIMIT * arena->page_size) {
    return NULL;
  }

  void* allocated_memory = (char*)arena->current_page;

  __try {
    volatile char* test = (char*)allocated_memory;
    for (size_t i = 0; i < size; i++) {
      test[i] = 0;
    }
  } __except (arena_page_fault_handler(arena, GetExceptionCode())) {
    return NULL;
  }

  arena->current_page = (char*)arena->current_page + size;
  return allocated_memory;
}

void arena_free(Arena* arena) {
  if (arena->base_address) {
    VirtualFree(arena->base_address, 0, MEM_RELEASE);
  }
  free(arena);
}
