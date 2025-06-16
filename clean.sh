#!/usr/bin/env sh
set -e
SCRIPT_ROOT="$(cd "${0%/*}" 2>/dev/null || true; echo "$PWD")"
cd "$SCRIPT_ROOT"
set +e
rm -r subprocess.o pkg/lib
