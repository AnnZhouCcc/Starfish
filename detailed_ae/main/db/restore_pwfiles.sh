#!/usr/bin/env bash
set -euo pipefail

PARTS_DIR="${1:-.}"
OUT="pwfilesdir.tar.xz"

cat "${PARTS_DIR}"/pwfilesdir.tar.xz.part-* > "$OUT"

# verify integrity
if command -v sha256sum >/dev/null 2>&1; then
  sha256sum -c pwfilesdir.tar.xz.sha256
else
  shasum -a 256 -c pwfilesdir.tar.xz.sha256
fi

# extract
# mkdir -p ../
# tar -xJf "$OUT" -C ../
tar -xJf "$OUT"
echo "Done. Extracted into pwfiles"
