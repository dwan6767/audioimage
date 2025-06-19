#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define SAMPLE_RATE 8000
#define TONE_DURATION_SEC 0.003f
#define MIN_FREQ 800.0f
#define MAX_FREQ 2000.0f

void write_wav_header(FILE *f, int num_samples) {
    int sample_rate = SAMPLE_RATE;
    int byte_rate = sample_rate * 2;
    int subchunk2_size = num_samples * 2;
    int chunk_size = 36 + subchunk2_size;
    int subchunk1_size = 16;
    short audio_format = 1;
    short num_channels = 1;
    short bits_per_sample = 16;
    short block_align = 2;

    fwrite("RIFF", 1, 4, f);
    fwrite(&chunk_size, 4, 1, f);
    fwrite("WAVEfmt ", 1, 8, f);
    fwrite(&subchunk1_size, 4, 1, f);
    fwrite(&audio_format, 2, 1, f);
    fwrite(&num_channels, 2, 1, f);
    fwrite(&sample_rate, 4, 1, f);
    fwrite(&byte_rate, 4, 1, f);
    fwrite(&block_align, 2, 1, f);
    fwrite(&bits_per_sample, 2, 1, f);
    fwrite("data", 1, 4, f);
    fwrite(&subchunk2_size, 4, 1, f);
}


void write_tone(FILE *f, float freq, float duration) {
    int samples = (int)(SAMPLE_RATE * duration);
    for (int i = 0; i < samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        float sample = sinf(2.0f * M_PI * freq * t);
        int16_t s = (int16_t)(sample * 32767.0f);
        fwrite(&s, sizeof(int16_t), 1, f);
    }
}

float brightness(unsigned char r, unsigned char g, unsigned char b) {
    return (r + g + b) / (3.0f * 255.0f);
}

int main() {
    int width, height, channels;
    unsigned char *img = stbi_load("reduce.jpg", &width, &height, &channels, 3);
    if (!img) {
        printf("Failed to load image\n");
        return 1;
    }

    FILE *f = fopen("output.wav", "wb");
    if (!f) {
        printf("Failed to open output.wav\n");
        return 1;
    }

    int total_pixels = width * height;
    int total_samples = (int)(total_pixels * SAMPLE_RATE * TONE_DURATION_SEC);
    write_wav_header(f, total_samples);

    for (int i = 0; i < total_pixels; i++) {
        int idx = i * 3;
        float b = brightness(img[idx], img[idx + 1], img[idx + 2]);
        float freq = MIN_FREQ + b * (MAX_FREQ - MIN_FREQ);
        write_tone(f, freq, TONE_DURATION_SEC);
    }

    fclose(f);
    stbi_image_free(img);

    printf("Done. Saved to output.wav .Total Pixels %d \n");
    return 0;
}

