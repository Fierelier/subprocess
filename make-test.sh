#!/usr/bin/env bash
set -e
SCRIPT_ROOT="$(cd "${0%/*}" 2>/dev/null || true; echo "$PWD")"
cd "$SCRIPT_ROOT"
if [ "$CC" == "" ]; then
	CC=gcc
fi

"$CC" -std=gnu99 -Wall -Werror -pedantic $CFLAGS pkg/include/subprocess/test.c -Ipkg/include $LDFLAGS -Wl,--wrap=malloc,--wrap=free -o subprocess-test
#"$CC" -std=gnu99 -Wall -Werror -pedantic $CFLAGS -DSUBPROCESS_C pkg/include/subprocess/test.c -I/usr/local/include -L/usr/local/lib -lsubprocess $LDFLAGS -Wl,--wrap=malloc,--wrap=free -o subprocess-test-linked
