#include "djc_arena.h"

#include <Windows.h>
#include <assert.h>
#include <excpt.h>
#include <stdio.h>
#include <stdlib.h>

INT arena_page_fault_handler(struct Arena* arena, DWORD dwCode) {
  if (dwCode != EXCEPTION_ACCESS_VIOLATION) {
    printf("Exception code = %lu.\n", dwCode);
    return EXCEPTION_EXECUTE_HANDLER;
  }

  if (arena->pages_committed >= arena->page_limit) {
    printf("Exception: out of pages.\n");
    return EXCEPTION_EXECUTE_HANDLER;
  }

  void* result = VirtualAlloc(arena->current, arena->page_size, MEM_COMMIT,
                              PAGE_READWRITE);

  if (result == NULL) {
    printf("VirtualAlloc failed.\n");
    return EXCEPTION_EXECUTE_HANDLER;
  }

  arena->pages_committed++;
  return EXCEPTION_CONTINUE_EXECUTION;
}

struct Arena* arena_create(char* name, size_t page_limit) {
  SYSTEM_INFO sys_info;
  GetSystemInfo(&sys_info);

  struct Arena* arena = (struct Arena*)malloc(sizeof(struct Arena));
  arena->page_size = sys_info.dwPageSize;
  arena->page_limit = page_limit;
  arena->pages_committed = 0;
  arena->name = name;
  arena->base_address = VirtualAlloc(NULL, arena->page_limit * arena->page_size,
                                     MEM_RESERVE, PAGE_NOACCESS);
  arena->current = arena->base_address;

  if (arena->base_address == NULL) {
    printf("Virtual alloc failed. calling ExitProcess.");
    ExitProcess(1);
  }

  return arena;
}

void* arena_alloc(struct Arena* arena,
                  size_t sizeof_element,
                  size_t num_elements) {
  size_t offset = (size_t)((char*)arena->current - (char*)arena->base_address);

  size_t num_bytes = sizeof_element * num_elements;

  if (offset + num_bytes > arena->page_limit * arena->page_size) {
    printf("%s allocation failed. Limit reached", arena->name);
    ExitProcess(1);
    return NULL;
  }

  void* allocated_memory = (char*)arena->current;

  __try {
    // Here we are writing zeros to the memory in order
    // to get the page handler to be invoked if
    // the page has not been committed yet.
    // We need to do this so that we don't have to
    // wrap all accesses to the arena memory in __try __except
    // blocks.
    volatile char* test = (char*)allocated_memory;
    for (size_t i = 0; i < num_bytes; i++) {
      test[i] = 0;
    }
  } __except (arena_page_fault_handler(arena, GetExceptionCode())) {
    // This code is executed only if the filter function
    // is unsuccessful in committing the next page.
    printf("Exiting process.\n");
    ExitProcess(GetLastError());
    return NULL;
  }

  arena->current = (char*)arena->current + num_bytes;
  return allocated_memory;
}

void arena_free(struct Arena* arena) {
  assert(arena);
  if (arena->base_address) {
    VirtualFree(arena->base_address, 0, MEM_RELEASE);
  }
  free(arena);
}
