# pick a safe chunk size under 100 MiB
CHUNK_MIB=95

# split into parts like dataset.tar.xz.part-00, part-01, ...
split -b ${CHUNK_MIB}MiB pwfilesdir.tar.xz pwfilesdir.tar.xz.part-

# write checksum for the original archive (recommended)
sha256sum pwfilesdir.tar.xz > pwfilesdir.tar.xz.sha256   # Linux
# macOS alternative:
# shasum -a 256 pwfilesdir.tar.xz > pwfilesdir.tar.xz.sha256
