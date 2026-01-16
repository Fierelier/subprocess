#!/usr/bin/env bash
set -e
SCRIPT_ROOT="$(cd "${0%/*}" 2>/dev/null || true; echo "$PWD")"
cd "$SCRIPT_ROOT"
if [ "$CC" == "" ]; then
	CC=gcc
fi

"$CC" -std=gnu99 -Wall -Werror -pedantic $CFLAGS -Ipkg/include $LDFLAGS pkg/include/subprocess/test-md5.c -o subprocess-test-md5
