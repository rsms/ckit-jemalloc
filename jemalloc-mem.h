#pragma once

// MemJEMalloc returns the shared, generic, thread-safe allocator.
static Mem MemJEMalloc();

// MemJEMallocArenaFlags can used to customize an arena's behavior
typedef enum {
  MemJEMallocArenaDefault   = 0,
  MemJEMallocArenaDummyFree = 1 << 0, // ignore free() calls
} MemJEMallocArenaFlags;

// MemJEMallocArenaAlloc allocates a new arena.
// Returns null if the upper limit of arenas is reached (about 4000 arenas.)
EXTERN_C Mem nullable MemJEMallocArenaAlloc(MemJEMallocArenaFlags);

// MemJEMallocArenaFree frees all memory allocated in the arena.
// The arena and all memory allocated with it is invalid after this call.
EXTERN_C void MemJEMallocArenaFree(Mem);


// -------------------------------------------------------------------------------------
// implementation

EXTERN_C void *je_calloc(size_t num, size_t size);
EXTERN_C void *je_realloc(void *ptr, size_t size);
EXTERN_C void je_free(void *ptr);

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
static const MemAllocator _mem_jemalloc = {
  .alloc   = _mem_jemalloc_alloc,
  .realloc = _mem_jemalloc_realloc,
  .free    = _mem_jemalloc_free,
};

inline static Mem MemJEMalloc() {
  return &_mem_jemalloc;
}
