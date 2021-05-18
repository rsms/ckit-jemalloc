#!/bin/sh
cd "$(dirname "$0")"
set -e
PROG=$0
JEMALLOC_SRC_DIR=jemalloc

_log() {
  echo "$PROG: $@" >&2
}

if [ ! -f "${JEMALLOC_SRC_DIR}/lib/libjemalloc.a" ]; then
  pushd "${JEMALLOC_SRC_DIR}" >/dev/null
  _log "building jemalloc $(cut -d- -f1 "${JEMALLOC_SRC_DIR}/VERSION") in $PWD"

  ./configure \
    --enable-static \
    --disable-shared \
    --disable-cxx

  make -j$(nproc)

  popd >/dev/null
fi
