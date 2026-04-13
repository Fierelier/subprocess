#!/usr/bin/env bash
set -e
SCRIPT_ROOT="$(cd "${0%/*}" 2>/dev/null || true; echo "$PWD")"
cd "$SCRIPT_ROOT"
if [ "$CC" == "" ]; then
	CC=gcc
fi

mkdir -p pkg/lib

$CC -c -std=gnu99 -Wall -Werror -pedantic -fPIC $CFLAGS "pkg/include/subprocess/subprocess.c" $LDFLAGS -o "subprocess.o"
$CC -std=gnu99 -Wall -Werror -pedantic -shared $CFLAGS "subprocess.o" $LDFLAGS -o "pkg/lib/libsubprocess.so"
rm subprocess.o
