#pragma once
#include <jemalloc/jemalloc.h>

// MemJEMalloc returns the shared, generic, thread-safe allocator.
static Mem MemJEMalloc();

// MemJEMallocArena is an arena allocator based on jemalloc
typedef struct MemJEMallocArena {
  MemAllocator m;
  u16          arena_id;
  u16          flags;
} MemJEMallocArena;

// MemJEMallocArenaFlags can used to customize an arena's behavior
typedef enum {
  MemJEMallocArenaDefault   = 0,
  MemJEMallocArenaDummyFree = 1 << 0, // ignore free() calls
} MemJEMallocArenaFlags;

// MemJEMallocArenaInit initializes a new arena.
// Returns null if the upper limit of arenas is reached (about 4000 arenas.)
Mem nullable MemJEMallocArenaInit(MemJEMallocArena*, MemJEMallocArenaFlags);

// MemJEMallocArenaFree frees all memory allocated in the arena.
// The arena and all memory allocated with it is invalid after this call.
void MemJEMallocArenaFree(MemJEMallocArena*);


// -------------------------------------------------------------------------------------
// implementation

// void *je_calloc(size_t num, size_t size);
// void *je_realloc(void *ptr, size_t size);
// void je_free(void *ptr);

static void* _mem_jemalloc_alloc(Mem _, size_t size) {
  return je_calloc(1, size);
}
static void* _mem_jemalloc_realloc(Mem _, void* ptr, size_t newsize) {
  return je_realloc(ptr, newsize);
}
static void _mem_jemalloc_free(Mem _, void* ptr) {
  je_free(ptr);
}

__attribute__((used))
static MemAllocator _mem_jemalloc = {
  .alloc   = _mem_jemalloc_alloc,
  .realloc = _mem_jemalloc_realloc,
  .free    = _mem_jemalloc_free,
};

inline static Mem MemJEMalloc() {
  return &_mem_jemalloc;
}
