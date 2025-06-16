#!/usr/bin/env sh
set -e
SCRIPT_ROOT="$(cd "${0%/*}" 2>/dev/null || true; echo "$PWD")"
cd "$SCRIPT_ROOT"
tar --numeric-owner --owner=0 --group=0 -czf subprocess.tar.gz -C pkg .
