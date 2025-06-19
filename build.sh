#!/bin/bash

echo "[*] Building decoder..."

# Paths (update if needed)
LIBSNDFILE_SRC=../libsndfile/src
LIBSNDFILE_BUILD=../libsndfile/build

gcc decoder.c -o decoder \
    -I"$LIBSNDFILE_SRC" \
    -L"$LIBSNDFILE_BUILD" \
    -lsndfile -lm

if [ $? -ne 0 ]; then
    echo "[!] Build failed."
    exit 1
fi

echo "[*] Build successful."

echo "[*] Running decoder..."
export LD_LIBRARY_PATH="$LIBSNDFILE_BUILD:$LD_LIBRARY_PATH"
./decoder output.wav

