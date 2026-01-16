#!/usr/bin/env bash
set -e
SCRIPT_ROOT="$(cd "${0%/*}" 2>/dev/null || true; echo "$PWD")"
cd "$SCRIPT_ROOT"
if [ "$CC" == "" ]; then
	CC=gcc
fi

mkdir -p pkg/lib

"$CC" $CFLAGS -std=gnu99 -Wall -Werror -pedantic -c -fPIC pkg/include/subprocess/subprocess.c $LDFLAGS -o subprocess.o
"$CC" $CFLAGS -std=gnu99 -Wall -Werror -pedantic -shared subprocess.o $LDFLAGS -o pkg/lib/libsubprocess.so
rm subprocess.o
