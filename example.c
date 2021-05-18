#include <rbase/rbase.h>
#include <stdarg.h>
#include "mem.h"


static void test_mem(Mem m, bool dofree) {
  const size_t size = 1024*1024;
  auto pch = (char*)memalloc(m, size);
  dlog("allocated [%p-%p)", pch, &pch[size]);
  dlog("  \\%02x\\%02x\\%02x\\%02x...", pch[0], pch[1], pch[2], pch[3]);
  memcpy(pch, "hello world", 12);
  dlog("  '%s'", pch);
  if (dofree)
    memfree(m, pch);
  fprintf(stderr, "\n");
}

int main(int argc, const char** argv) {
  assert(argc == 1);

  auto m = MemJEMalloc();
  test_mem(m, /*dofree=*/true);
  test_mem(m, /*dofree=*/true);

  MemJEMallocArena arena1;
  MemJEMallocArena arena2;
  auto ma1 = MemJEMallocArenaInit(&arena1, 0);
  auto ma2 = MemJEMallocArenaInit(&arena2, 0);
  test_mem(ma1, /*dofree=*/false);
  test_mem(ma2, /*dofree=*/false);
  MemJEMallocArenaFree(&arena1);
  MemJEMallocArenaFree(&arena2);

  ma1 = MemJEMallocArenaInit(&arena1, 0);
  ma2 = MemJEMallocArenaInit(&arena2, 0);
  test_mem(ma1, /*dofree=*/false);
  test_mem(ma2, /*dofree=*/false);
  MemJEMallocArenaFree(&arena1);
  MemJEMallocArenaFree(&arena2);

  test_mem(m, /*dofree=*/true);

  return 0;
}
