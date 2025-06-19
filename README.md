# AudioImage 🎧🖼️

This project encodes images into sound (WAV) and decodes them back into grayscale images using Goertzel frequency detection.

## Usage

### Encode:
```bash
gcc encoder.c -o encoder -lm
./encoder reduce.jpg

Decode
gcc decoder.c -o decoder -lsndfile -lm
./decoder output.wav
Output:

Creates output.wav and decodes it to decoded.png.
Requirements

    libsndfile

    stb_image & stb_image_write

    GCC or Clang
