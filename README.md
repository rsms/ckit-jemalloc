jemalloc-based memory allocator for [ckit/rbase](https://github.com/rsms/ckit/blob/main/pkg/rbase/mem.h)

- `Mem MemJEMalloc()` provides a shared generic, thread-safe allocator.
- `Mem MemJEMallocArenaAlloc()` provides arena allocators.


## Examples

Build & run `example.c` program with [ckit](https://github.com/rsms/ckit):

    ckit watch -r jemalloc-example


### Generic allocator example

```c
auto mem = MemJEMalloc();
void* a = memalloc(mem, size);
void* b = memalloc(mem, size);
// use a and b
memfree(mem, a);
memfree(mem, b);
```

> Note on runtime overhead: When compiling with a modern version of GCC or Clang, the above
> becomes direct calls to `je_calloc` and `je_free`, eliminating any overhead.


### Arena allocator example

```c
auto mem = MemJEMallocArenaAlloc(0);
void* a = memalloc(mem, size);
void* b = memalloc(mem, size);
// use a and b
MemJEMallocArenaFree(ma1); // free all memory allocated in the arena
```

