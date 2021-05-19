#include <rbase/rbase.h>
#include <rbase/pool/pool.h>
#include <jemalloc/jemalloc.h>
#include "jemalloc-mem.h"

// MemJEMallocArena is an arena allocator based on jemalloc
typedef struct MemJEMallocArena {
  POOL_ENTRY_HEAD
  MemAllocator m;
  u16          arena_id;
  u16          flags;
} MemJEMallocArena;


#define MEM_TO_ARENA(mem) \
  ((MemJEMallocArena*)( ((u8*)(mem)) - offsetof(MemJEMallocArena, m) ))

#define ARENA_TO_MEM(a) (&(a)->m)

#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)


// pool is a freelist of arenas
static Pool pool = {};


static void* arena_alloc(Mem m, size_t size) {
  assert(MEM_TO_ARENA(m)->arena_id < MALLCTL_ARENAS_ALL);
  void* p = je_mallocx(size, MALLOCX_ARENA(MEM_TO_ARENA(m)->arena_id) | MALLOCX_ZERO);
  return p;
}

static void* arena_realloc(Mem m, void* ptr, size_t newsize) {
  assert(MEM_TO_ARENA(m)->arena_id < MALLCTL_ARENAS_ALL);
  return je_rallocx(ptr, newsize, MALLOCX_ARENA(MEM_TO_ARENA(m)->arena_id));
}

static void arena_free(Mem m, void* ptr) {
  assert(MEM_TO_ARENA(m)->arena_id < MALLCTL_ARENAS_ALL);
  je_dallocx(ptr, MALLOCX_ARENA(MEM_TO_ARENA(m)->arena_id));
}

static void arena_free_dummy(Mem ign1, void* ign2) {}



Mem nullable MemJEMallocArenaAlloc(MemJEMallocArenaFlags flags) {
  auto ma = (MemJEMallocArena*)PoolTake(&pool);
  if (ma) {
    //dlog("[jemalloc] arena init recycle => %u", ma->arena_id);
  } else {
    ma = je_malloc(sizeof(MemJEMallocArena));
    // arenas.create (unsigned, extent_hooks_t *) rw
    //   Explicitly create a new arena outside the range of automatically managed arenas,
    //   with optionally specified extent hooks, and return the new arena index.
    // read number of active arenas and check that we are below the limit
    size_t u32size = sizeof(u32);
    u32 narenas = 0;
    je_mallctl("arenas.narenas", &narenas, &u32size, NULL, 0);
    if (narenas >= MALLCTL_ARENAS_ALL - 1) {
      // too many arenas allocated
      errno = ENOMEM;
      return NULL;
    }
    je_mallctl("arenas.create", &ma->arena_id, &u32size, NULL, 0);
    //dlog("[jemalloc] arena init create => %u", ma->arena_id);
    ma->m.alloc   = arena_alloc;
    ma->m.realloc = arena_realloc;
    ma->m.free = (flags & MemJEMallocArenaDummyFree) ? arena_free_dummy : arena_free;
    ma->flags = flags;
  }
  return ARENA_TO_MEM(ma);
}

void MemJEMallocArenaFree(Mem m) {
  MemJEMallocArena* ma = MEM_TO_ARENA(m);
  //dlog("[jemalloc] arena free => %u", ma->arena_id);
  assert(ma->arena_id < MALLCTL_ARENAS_ALL);

  // arena.<i>.reset (void) --
  //   Discard all of the arena's extant allocations. This interface can only be used with
  //   arenas explicitly created via arenas.create. None of the arena's discarded/cached
  //   allocations may accessed afterward. As part of this requirement, all thread caches
  //   which were used to allocate/deallocate in conjunction with the arena must be flushed
  //   beforehand.
  char buf[sizeof("arena." STRINGIFY(MALLCTL_ARENAS_ALL) ".reset\0")];
  snprintf(buf, sizeof(buf), "arena.%u.reset", ma->arena_id);
  je_mallctl(buf, NULL, NULL, NULL, 0);

  PoolAdd(&pool, (PoolEntry*)ma);
}

