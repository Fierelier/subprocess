#!/usr/bin/env sh
set -e
SCRIPT_ROOT="$(cd "${0%/*}" 2>/dev/null || true; echo "$PWD")"
cd "$SCRIPT_ROOT"
if [ "$PREFIX" == "" ]; then
	PREFIX=/usr/local
fi

tar -xzf subprocess.tar.gz -C "$PREFIX"
