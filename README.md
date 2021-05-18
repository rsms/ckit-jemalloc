jemalloc-based memory allocator for [ckit/rbase](https://github.com/rsms/ckit/blob/main/pkg/rbase/mem.h)

- `Mem MemJEMalloc()` provides a shared generic, thread-safe allocator.
- `MemJEMallocArena*` provides arena allocators

Generic allocator example:

```c
auto mem = MemJEMalloc();
void* a = memalloc(mem, size);
void* b = memalloc(mem, size);
// use a and b
memfree(mem, a);
memfree(mem, b);
```

Arena allocator example:

```c
MemJEMallocArena arena;
auto mem = MemJEMallocArenaInit(&arena, 0);
void* a = memalloc(mem, size);
void* b = memalloc(mem, size);
// use a and b
MemJEMallocArenaFree(&arena); // free all memory allocated in the arena
```

Build & run `example.c` program with [ckit](https://github.com/rsms/ckit):

    ckit watch -r jemalloc-example
