#include <rbase/rbase.h>
#include "mem.h"

#define _ARENA(mem) ((MemJEMallocArena*)(mem))

#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

static void* arena_alloc(Mem m, size_t size) {
  assert(_ARENA(m)->arena_id < MALLCTL_ARENAS_ALL);
  void* p = je_mallocx(size, MALLOCX_ARENA(_ARENA(m)->arena_id) | MALLOCX_ZERO);
  return p;
}

static void* arena_realloc(Mem m, void* ptr, size_t newsize) {
  assert(_ARENA(m)->arena_id < MALLCTL_ARENAS_ALL);
  return je_rallocx(ptr, newsize, MALLOCX_ARENA(_ARENA(m)->arena_id) | MALLOCX_ZERO);
}

static void arena_free(Mem m, void* ptr) {
  assert(_ARENA(m)->arena_id < MALLCTL_ARENAS_ALL);
  je_dallocx(ptr, MALLOCX_ARENA(_ARENA(m)->arena_id));
}

static void arena_free_dummy(Mem ign1, void* ign2) {}

Mem nullable MemJEMallocArenaInit(MemJEMallocArena* ma, MemJEMallocArenaFlags flags) {
  // arenas.create (unsigned, extent_hooks_t *) rw
  //   Explicitly create a new arena outside the range of automatically managed arenas,
  //   with optionally specified extent hooks, and return the new arena index.
  size_t u32size = sizeof(u32);
  // read number of active arenas and check that we are below the limit
  u32 narenas = 0;
  je_mallctl("arenas.narenas", &narenas, &u32size, NULL, 0);
  if (narenas >= MALLCTL_ARENAS_ALL - 1) {
    // too many arenas allocated
    errno = ENOMEM;
    return NULL;
  }
  u32 arena_id;
  je_mallctl("arenas.create", &arena_id, &u32size, NULL, 0);
  ma->m.alloc   = arena_alloc;
  ma->m.realloc = arena_realloc;
  if (flags & MemJEMallocArenaDummyFree) {
    ma->m.free = arena_free_dummy;
  } else {
    ma->m.free = arena_free;
  }
  ma->arena_id  = arena_id;
  ma->flags     = flags;
  return &ma->m;
}

void MemJEMallocArenaFree(MemJEMallocArena* ma) {
  // arena.<i>.destroy (void) --
  //   Destroy the arena. Discard all of the arena's extant allocations using the same
  //   mechanism as for arena.<i>.reset (with all the same constraints and side effects),
  //   merge the arena stats into those accessible at arena index MALLCTL_ARENAS_DESTROYED,
  //   and then completely discard all metadata associated with the arena. Future calls to
  //   arenas.create may recycle the arena index. Destruction will fail if any threads are
  //   currently associated with the arena as a result of calls to thread.arena.
  assert(ma->arena_id < MALLCTL_ARENAS_ALL);
  char buf[sizeof("arena." STRINGIFY(MALLCTL_ARENAS_ALL) ".destroy\0")];
  snprintf(buf, sizeof(buf), "arena.%u.destroy", ma->arena_id);
  je_mallctl(buf, NULL, NULL, NULL, 0);
  #if DEBUG
  _ARENA(ma)->arena_id = MALLCTL_ARENAS_DESTROYED;
  #endif
}

#undef _ARENA
#undef STRINGIFY_HELPER
#undef STRINGIFY
