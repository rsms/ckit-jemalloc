jemalloc-based memory allocator for rbase

- `Mem MemJEMalloc()` provides a shared generic, thread-safe allocator.
- `MemJEMallocArena*` provides arena allocators

Build & run example with [ckit](https://github.com/rsms/ckit):

    ckit watch -r jemalloc-example
